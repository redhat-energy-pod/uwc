/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

#include "MQTTHandler.hpp"

// constructor
CMQTTHandler::CMQTTHandler(std::string strPlBusUrl) : client(strPlBusUrl, CLIENTID),
ConfigState(MQTT_CLIENT_CONNECT_STATE)
{
	try
	{
		mqtt::message willmsg("MQTTConfiguration", LWT_PAYLOAD, QOS, true);
		mqtt::will_options will(willmsg);
		conopts.set_will(will);

		conopts.set_keep_alive_interval(60);
		conopts.set_clean_session(true);
		conopts.set_automatic_reconnect(1, 10);

#ifdef TLSENABLED
		mqtt::ssl_options sslopts;
		char *RootCAPath = getenv("PLBUS_ROOTCAPATH");
		if(NULL == RootCAPath)
		{
			BOOST_LOG_SEV(lg, info) << "fatal:: Error in reading PLBUS_ROOTCAPATH ";
			return;
		}
		sslopts.set_trust_store(RootCAPath);

		char *ClientCertPath = getenv("BACNET_PLBUS_CLIENT_CERT");
		if(NULL == ClientCertPath)
		{
			BOOST_LOG_SEV(lg, info) << "fatal:: Error in reading BACNET_PLBUS_CLIENT_CERT ";
			return;
		}
		sslopts.set_key_store(ClientCertPath);
		char *ClientKeyPath = getenv("BACNET_PLBUS_CLIENT_KEY");
		if(NULL == ClientKeyPath)
		{
			BOOST_LOG_SEV(lg, info) << "fatal:: Error in reading BACNET_PLBUS_CLIENT_KEY ";
			return;
		}
		sslopts.set_private_key(ClientKeyPath);
		sslopts.set_enable_server_cert_auth(true);
		conopts.set_ssl(sslopts);
#endif
		client.set_callback(callback);
	}
	catch (const std::exception &e)
	{
		std::cout << "fatal::Exception in CMQTTHandler constructor: "<< e.what();
	}
}

// function to get single instance of this class
CMQTTHandler& CMQTTHandler::instance()
{
	static bool bPlBusUrl = false;
	const char *platformBusURL = NULL;
	string strPlBusUrl;

	if(!bPlBusUrl)
	{
		/// get the platform bus env variable
		platformBusURL = std::getenv("MQTT_URL_FOR_EXPORT");
		/// check for null
		if(NULL == platformBusURL)
		{
			std::cout << __func__ << ":MQTT_URL_FOR_EXPORT Environment variable is not set!";
			cout << __func__ << ": " << "Error::MQTT_URL_FOR_EXPORT Environment variable is not set!"<<endl;
			exit(EXIT_FAILURE);
		}
		else
		{
			strPlBusUrl = platformBusURL;
			std::cout << __func__ << ":MQTT_URL_FOR_EXPORT Environment variable is set to : "<< strPlBusUrl;
			cout << __func__ << ": " << "MQTT_URL_FOR_EXPORT Environment variable is set to : "<< strPlBusUrl<<endl;
			bPlBusUrl = true;
		}
	}

	static CMQTTHandler handler(platformBusURL);
	return handler;
}

bool CMQTTHandler::connect()
{
	bool bFlag = true;
	try
	{
		std::lock_guard<std::mutex> lock(mqttMutexLock);
		conntok = client.connect(conopts, nullptr, listener);
		/// Wait for 2 seconds to get connected
		/*if (false == conntok->wait_for(2000))
		{
			std::cout << "Error::Failed to connect to the platform bus ";
			bFlag = false;
		}*/
	}
	catch(const std::exception& e)
	{
		std::cout << __func__ <<":Exception in MQTT connect: "<<e.what();
		bFlag = false;
	}
	return bFlag;
}

bool CMQTTHandler::getMsgFromQ(stMsgData &a_msg)
{
	bool bRet = true;
	try
	{
		std::lock_guard<std::mutex> lock(m_mutexMsgQ);
		/// Ensure that only on thread can execute at a time
		if (false == m_qMsgData.empty())
		{
			a_msg = m_qMsgData.front();
			m_qMsgData.pop();
		}
		else
		{
			bRet = false;
		}
	}
	catch(const std::exception& e)
	{
		std::cout << "Error::Exception CMQTTHandler::getMsgFromQ " <<e.what();
		bRet = false;
	}
	return bRet;
}

bool CMQTTHandler::pushMsgInQ(const stMsgData &a_msg)
{
	bool bRet = true;
	try
	{
		/// Ensure that only on thread can execute at a time
		std::lock_guard<std::mutex> lock(m_mutexMsgQ);
		m_qMsgData.push(a_msg);
	}
	catch(const std::exception& e)
	{
		std::cout << "Error::Exception CMQTTHandler::pushMsgInQ " <<e.what();
		bRet = false;
	}
	return bRet;

}

void CMQTTHandler::postPendingMsgsThread()
{
	bool bDoRun = false;
	try
	{
		stMsgData msg;
		do
		{
			{
				if(false == client.is_connected())
				{
					bDoRun = false;
					break;
				}
				if(false == getMsgFromQ(msg))
				{
					bDoRun = false;
					break;
				}

				bDoRun = true;
				publish(msg.m_sMsg, msg.m_sTopic, true);
#ifdef PERFTESTING
				m_uiQReqTried++;
#endif
			}
		} while(true == bDoRun);
	}
	catch(const std::exception& e)
	{
		std::cout << "Error::Exception in postPendingMsgsThread: "<<e.what();
#ifdef PERFTESTING
		CMQTTHandler::m_ui32PublishSkipped++;
#endif
	}
}

void CMQTTHandler::postPendingMsgs()
{
	// Create a new thread to post messages to MQTT
	std::thread{std::bind(&CMQTTHandler::postPendingMsgsThread, std::ref(*this))}.detach();
}

Mqtt_Config_state_t CMQTTHandler::getMQTTConfigState()
{
	return ConfigState;
}

void CMQTTHandler::setMQTTConfigState(Mqtt_Config_state_t tempConfigState )
{
	ConfigState = tempConfigState;
}

#ifdef PERFTESTING
std::atomic<uint32_t> CMQTTHandler::m_ui32PublishReq(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32PublishReqErr(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32Published(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32PublishFailed(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32ConnectionLost(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32Connection(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32PublishSkipped(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32PublishExcep(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32PublishReqTimeOut(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32Disconnected(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32PublishStrReq(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32PublishStrReqErr(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32PublishStrExcep(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32DelComplete(0);
std::atomic<uint32_t> CMQTTHandler::m_uiQReqTried(0);

void CMQTTHandler::printCounters()
{
	std::cout << "\n\nReq rcvd: " << m_ui32PublishReq;
	std::cout << "\nReq err: " << m_ui32PublishReqErr;
	std::cout << "\nReq sendmsg excep: " << m_ui32PublishSkipped;
	std::cout << "\nReq publish excep: " << m_ui32PublishExcep;
	std::cout << "\nReq published: " << m_ui32Published;
	std::cout << "\nReq publish failed: " << m_ui32PublishFailed;
	std::cout << "\nReq publish timeout: " << m_ui32PublishReqTimeOut;
	std::cout << "\nReq during no connection: " << m_ui32Disconnected;
	std::cout << "\nReq conn lost: " << m_ui32ConnectionLost;
	std::cout << "\nReq conn done: " << m_ui32Connection;
	std::cout << "\n*****Str Req: " << m_ui32PublishStrReq;
	std::cout << "\n*****Str Req err: " << m_ui32PublishStrReqErr;
	std::cout << "\n*****Str Req excep: " << m_ui32PublishStrExcep;
	std::cout << "\n----Pending Q Size: " << instance().m_qMsgData.size();
	std::cout << "\n++++Req posted from Q: " << m_uiQReqTried;
	std::cout << "\n$$$$Delivery completed: " << m_ui32DelComplete << endl;
}
#endif

bool CMQTTHandler::publish(std::string a_sMsg, const char *topic)
{
	static bool bIsFirst = true;
	if(true == bIsFirst)
	{
		connect();
		bIsFirst = false;
	}

	std::string sTopic(topic);
	
	try
	{
		publish(a_sMsg, sTopic);
	}
	catch (const mqtt::exception& exc)
	{
#ifdef PERFTESTING
		m_ui32PublishExcep++;
#endif
		std::cout << "Fatal::Platform Bus: Exception in MQTT_PublishMessage"<<exc.what();
	}
	return false;
}

bool CMQTTHandler::publish(std::string &a_sMsg, std::string &a_sTopic, bool a_bFromQ)
{
	try
	{
		std::lock_guard<std::mutex> lock(mqttMutexLock);

		// Check if topic is blank
		if(true == a_sTopic.empty())
		{
			if(true == a_sMsg.empty())
			{
				std::cout << "Info::Blank topic and blank Message";
			}
			else
			{
				std::cout << "Info::Blank topic. Message not posted";
			}
			return false;
		}

#ifdef PERFTESTING
		CMQTTHandler::m_ui32PublishReq++;
#endif
		if(true == client.is_connected())
		{
			mqtt::message_ptr pubmsg = mqtt::make_message(a_sTopic, a_sMsg, 0, false);
			//std::cout << "Info::Platform Bus StrMsg: Publishing message ";

			client.publish(pubmsg, nullptr, listener);

			return true;
		}
		else
		{
			pushMsgInQ(stMsgData(a_sMsg, a_sTopic));
#ifdef PERFTESTING
			CMQTTHandler::m_ui32Disconnected++;
#endif
		}
	}
	catch (const mqtt::exception& exc)
	{

		if(false == a_bFromQ)
		{
#ifdef PERFTESTING
			m_ui32PublishStrExcep++;
#endif
			pushMsgInQ(stMsgData(a_sMsg, a_sTopic));
		}
		else
		{
			//g_uiStrMsgNotPublished++;
		}

		std::cout << "Error::Platform Bus StrMsg: Exception in MQTT_PublishMessage :"<<exc.what();
	}
	return false;
}