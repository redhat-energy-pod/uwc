/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include <string>
#include <atomic>
#include<algorithm>
#include "Common.hpp"
#include "KPIAppConfigMgr.hpp"
#include "CommonDataShare.hpp"
#include "ConfigManager.hpp"
#include "cjson/cJSON.h"
#include <mutex>
#include "QueueMgr.hpp"

/// flag to stop all running threads
std::atomic<bool> g_stopThread(false);

vector<std::thread> g_vThreads;

/**
 * Thread function to read requests from queue filled up by MQTT and send data to EIS
 * @param qMgr 	:[in] pointer to respective queue manager
 * @return None
 */
void postMsgsToWriteOnMQTT(CQueueHandler& qMgr)
{
	DO_LOG_DEBUG("Starting thread to send messages on EIS");

	try
	{
		while (false == g_stopThread.load())
		{
			CMessageObject recvdMsg;
			if(true == qMgr.isMsgArrived(recvdMsg))
			{
				std::string sTopic{recvdMsg.getTopic()};
				if(CKPIAppConfig::getInstance().getControlLoopMapper().isControlLoopPollPoint(sTopic))
				{
					CKPIAppConfig::getInstance().getControlLoopMapper().triggerControlLoops(sTopic, recvdMsg);
				}
			}
		}
		std::cout << "Exiting thread::postMsgsToWriteOnMQTT \n";
	}
	catch (const std::exception &e)
	{
		DO_LOG_ERROR(e.what());
	}
}

/**
 * Thread function to read requests from queue filled up by MQTT and send data to EIS
 * @param qMgr 	:[in] pointer to respective queue manager
 * @return None
 */
void analyzeControlLoopData(CQueueHandler& qMgr)
{
	try
	{
		while (false == g_stopThread.load())
		{
			CMessageObject recvdMsg;
			if(true == qMgr.isMsgArrived(recvdMsg))
			{
				//string strMsg = recvdMsg.getStrMsg();

				if(false == CKPIAppConfig::getInstance().getControlLoopMapper().isControlLoopWrRspPoint(recvdMsg.getTopic()))
				{
					DO_LOG_DEBUG(recvdMsg.getTopic() + ": Not a part of control loop. Ignored");
					continue;
				}

				std::string sAppSeqVal{commonUtilKPI::getValueofKeyFromJSONMsg(recvdMsg.getStrMsg(), "app_seq")};
				if(true == sAppSeqVal.empty())
				{
					DO_LOG_ERROR(recvdMsg.getStrMsg() + ": app_seq key not found. Ignoring the message");
					continue;
				}

				struct stPollWrData oTempData{};
				if(true == CPollNWriteReqMapper::getInstace().getForProcessing(sAppSeqVal, oTempData))
				{
					commonUtilKPI::logAnalysisMsg(oTempData, recvdMsg);
				}
				else
				{
					DO_LOG_ERROR(sAppSeqVal + ": Waited writeRequest is not found in poll mapping");
				}
			}
		}
		std::cout << "Exiting thread::analyzeControlLoopData \n";
	}
	catch (const std::exception &e)
	{
		cout << "Exception occured while publishing data: "<<e.what() << endl;
	}

}

/**
 *
 * DESCRIPTION
 * Function to initialise the structure values of stUWCComnDataVal_t of uwc-lib
 *
 * @param strDevMode	[in] describes is devMode enabled
 * @param strAppName	[in] application name
 *
 * @return
 */
void initializeCommonData(string strDevMode, string strAppName)
{
	stUWCComnDataVal_t stUwcData;
	stUwcData.m_devMode = false;
	stUwcData.m_sAppName = strAppName;
	stUwcData.m_isCommonDataInitialised = true;
	transform(strDevMode.begin(), strDevMode.end(), strDevMode.begin(), ::toupper);
	if("TRUE" == strDevMode)
	{
		stUwcData.m_devMode = true;
	}
	CcommonEnvManager::Instance().ShareToLibUwcCmnData(stUwcData);
}

/**
 * Function to read all environment variables
 *
 * @return
 */
void setEnvData()
{
	std::vector<std::string> vecEnv{"AppName", "MQTT_URL", "DEV_MODE", "WriteRequest_RT", "WriteRequest", "KPIAPPConfigFile"};
	EnvironmentInfo::getInstance().readCommonEnvVariables(vecEnv);
	std::string strDevMode = EnvironmentInfo::getInstance().getDataFromEnvMap("DEV_MODE");
	std::string strAppName = EnvironmentInfo::getInstance().getDataFromEnvMap("AppName");

	initializeCommonData(strDevMode, strAppName);
}

/**
 * This function is entry point for application
 * @param argc [in] argument count
 * @param argv [in] argument value
 * @return int [out] return 1 on success
 */
int main(int argc, char* argv[])
{
	try
	{
		CLogger::initLogger(std::getenv("Log4cppPropsFile"));
		
		DO_LOG_DEBUG("Starting KPI_App ...");

		setEnvData();

		std::vector<std::string> vFullTopics = CcommonEnvManager::Instance().getTopicList();

		if(false == CKPIAppConfig::getInstance().parseYMLFile(EnvironmentInfo::getInstance().getDataFromEnvMap("KPIAPPConfigFile")))
		{
			DO_LOG_ERROR("Error while loading the configuration file");
			return EXIT_FAILURE;

		}
		CKPIAppConfig::getInstance().getControlLoopMapper().configControlLoopOps(CKPIAppConfig::getInstance().isRTModeForWriteOp());

		PlBusMgr::initPlatformBusHandler(CKPIAppConfig::getInstance().isMQTTModeOn());

		//threads to send on-demand requests on EIS
		g_vThreads.push_back(std::thread(postMsgsToWriteOnMQTT, std::ref(QMgr::PollMsgQ())));
		g_vThreads.push_back(std::thread(analyzeControlLoopData, std::ref(QMgr::WriteRespMsgQ())));

		DO_LOG_INFO("Configuration done. Starting operations.");
		auto uiTime = CKPIAppConfig::getInstance().getExecutionTime();
		if(0 != uiTime)
		{
			std::cout << "Starting the Timer for : " << uiTime << " minutes.\n";
			this_thread::sleep_for(std::chrono::minutes(uiTime));
			g_stopThread.store(true);

			if(!CKPIAppConfig::getInstance().isMQTTModeOn())
			{
				CKPIAppConfig::getInstance().getControlLoopMapper().destroySubCtx();
			}
			CKPIAppConfig::getInstance().getControlLoopMapper().stopControlLoopOps();
			QMgr::PollMsgQ().breakWaitOnQ();
			QMgr::WriteRespMsgQ().breakWaitOnQ();
			// give 1 second time for all threads to be signalled
			this_thread::sleep_for(std::chrono::seconds(2));
		}
		else
		{
			std::cout << "Execution limit time is not set. \n";
		}
		for (auto &th : g_vThreads)
		{
			if (th.joinable())
			{
				th.join();
			}
		}

		std::cout << "Timeout done :)\n\n";
	}
	catch (const std::exception &e)
	{
		std::cout << "Exception in main:: fatal::Error in getting arguments: "<<e.what()<< endl;
		DO_LOG_FATAL("fatal::Error in getting arguments: " +
					(string)e.what());

		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

