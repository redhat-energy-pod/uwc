/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "Logger.hpp"

#include "ModbusWriteHandler.hpp"
#include "utils/YamlUtil.hpp"
#include <thread>
#include <mutex>
#include <functional>

#include "NetworkInfo.hpp"
#include "ConfigManager.hpp"

#include <sys/msg.h>
#include <fstream>
#include <cstdlib>
#include <stdio.h>
#include "eis/utils/json_config.h"

/// stop thread flag
extern std::atomic<bool> g_stopThread;
/// reference if to store request data

/**
 * Constructor
 */
modWriteHandler::modWriteHandler() : m_bIsWriteInitialized(false)
{
	try
	{
		initWriteSem();
		m_bIsWriteInitialized = true;
	}
	catch(const std::exception& e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS("Unable to initiate write instance"));
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}
}

/**
 * Initialize semaphore
 * @return 	true : on success,
 * 			false : on error
 */
bool modWriteHandler::initWriteSem()
{
	//
	int ok = sem_init(&semaphoreWriteReq, 0, 0);
	if (ok == -1) {
	   std::cout << "*******Could not create unnamed write semaphore\n";
	   return false;
	}
	return true;
}

/**
 * Create error response
 * @param ptMsg 	:[out] pointer to message envelope structure to fill up
 * @param errorCode	:[in] error code
 * @param u8FunCode	:[in] function code
 * @param strTopic	:[in] topic for which to create error response
 * @param txID		:[in] transaction id
 */
void modWriteHandler::createErrorResponse(msg_envelope_t** ptMsg,
		eMbusStackErrorCode errorCode, uint8_t  u8FunCode, std::string &strTopic, unsigned short txID)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start"));

	if( u8FunCode == READ_COIL_STATUS ||
			u8FunCode == READ_HOLDING_REG ||
			u8FunCode == READ_INPUT_STATUS ||
			u8FunCode == READ_INPUT_REG)
	{
		strTopic = PublishJsonHandler::instance().getSReadResponseTopic();
	}
	else
	{
		strTopic = PublishJsonHandler::instance().getSWriteResponseTopic();
	}

	stOnDemandRequest onDemandReqData;
	bool bRetVal = common_Handler::getOnDemandReqData(txID, onDemandReqData);
	if(false == bRetVal)
	{
		throw std::runtime_error("Request not found in map");
	}
	msg_envelope_t *msg = NULL;
	msg = msgbus_msg_envelope_new(CT_JSON);

	msg_envelope_elem_body_t* ptErrorDetails = msgbus_msg_envelope_new_string((
			(to_string(errorCode)) + ", " +  (to_string(2))).c_str());
	msgbus_msg_envelope_put(msg, "error_code", ptErrorDetails);

	msg_envelope_elem_body_t* ptErrorStatus = msgbus_msg_envelope_new_string("Bad");
	msgbus_msg_envelope_put(msg, "status", ptErrorStatus);

	/// topic
	msg_envelope_elem_body_t* ptResTopic = msgbus_msg_envelope_new_string(onDemandReqData.m_strTopic.append("Response").c_str());
	msgbus_msg_envelope_put(msg, "topic", ptResTopic);
	/// wellhead
	msg_envelope_elem_body_t* ptWellhead = msgbus_msg_envelope_new_string(onDemandReqData.m_strWellhead.c_str());
	msgbus_msg_envelope_put(msg, "wellhead", ptWellhead);
	/// application sequence
	msg_envelope_elem_body_t* ptAppSeq = msgbus_msg_envelope_new_string(onDemandReqData.m_strAppSeq.c_str());
	msgbus_msg_envelope_put(msg, "app_seq", ptAppSeq);
	/// metric
	msg_envelope_elem_body_t* ptMetric = msgbus_msg_envelope_new_string(onDemandReqData.m_strMetric.c_str());
	msgbus_msg_envelope_put(msg, "metric", ptMetric);
	/// version
	msg_envelope_elem_body_t* ptVersion = msgbus_msg_envelope_new_string(onDemandReqData.m_strVersion.c_str());
	msgbus_msg_envelope_put(msg, "version", ptVersion);
	/// QOS
	msg_envelope_elem_body_t* ptQos =  msgbus_msg_envelope_new_string(onDemandReqData.m_strQOS.c_str());
	msgbus_msg_envelope_put(msg, "qos", ptQos);
	/// Priority
	msg_envelope_elem_body_t* ptPriority =  msgbus_msg_envelope_new_string(to_string(onDemandReqData.m_lPriority).c_str());
	msgbus_msg_envelope_put(msg, "priority", ptPriority);
	std::string strTimestamp, strUsec;
	common_Handler::getTimeParams(strTimestamp, strUsec);
	/// usec
	msg_envelope_elem_body_t* ptUsec = msgbus_msg_envelope_new_string(strUsec.c_str());
	msgbus_msg_envelope_put(msg, "usec", ptUsec);
	/// timestamp
	msg_envelope_elem_body_t* ptTimestamp = msgbus_msg_envelope_new_string(strTimestamp.c_str());
	msgbus_msg_envelope_put(msg, "timestamp", ptTimestamp);

	CLogger::getInstance().log(DEBUG, LOGDETAILS("End"));

	*ptMsg = msg;
}

/**
 * Thread function for on-demand info handler
 * @return appropriate error code
 */
eMbusStackErrorCode modWriteHandler::onDemandInfoHandler()
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start"));

	stRequest writeReq;
	eMbusStackErrorCode eFunRetType = MBUS_STACK_NO_ERROR;
	unsigned char  m_u8FunCode;
	MbusAPI_t stMbusApiPram = {};
	cJSON *root = NULL;

#ifdef REALTIME_THREAD_PRIORITY
	PublishJsonHandler::instance().set_thread_priority();
#endif

	while(false == g_stopThread.load())
	{
		do
		{
			//sem_wait(&semaphoreWriteReq);
			if((sem_wait(&semaphoreWriteReq))== -1 && errno == EINTR)
			{
				// Continue if interrupted by handler
				continue;
			}
			try
			{
				if(false == getDataToProcess(writeReq))
				{
					return MBUS_STACK_ERROR_QUEUE_SEND;
				}

				/// Enter TX Id
				stMbusApiPram.m_u16TxId = PublishJsonHandler::instance().getTxId();
#ifdef INSTRUMENTATION_LOG
				CLogger::getInstance().log(DEBUG, LOGDETAILS("On-demand request::" + writeReq.m_strMsg));
#endif

				root = cJSON_Parse(writeReq.m_strMsg.c_str());
				stOnDemandRequest reqData;

				eFunRetType = jsonParserForOnDemandRequest(root, stMbusApiPram, m_u8FunCode, stMbusApiPram.m_u16TxId, reqData);

				common_Handler::insertOnDemandReqData(stMbusApiPram.m_u16TxId, reqData);

				if(MBUS_STACK_NO_ERROR == eFunRetType)
				{
					if(MBUS_MIN_FUN_CODE != m_u8FunCode)
					{
						CLogger::getInstance().log(DEBUG, LOGDETAILS("On-Demand Process initiated..."));
						eFunRetType = (eMbusStackErrorCode) Modbus_Stack_API_Call(
								m_u8FunCode,
								&stMbusApiPram,
								(void*)ModbusMaster_AppCallback);
					}
				}

				else
				{
					if(MBUS_APP_ERROR_UNKNOWN_SERVICE_REQUEST != eFunRetType)
					{
						msg_envelope_t *msg = NULL;
						std::string strTopic = "";
						createErrorResponse(&msg, eFunRetType, m_u8FunCode, strTopic, stMbusApiPram.m_u16TxId);

						zmq_handler::stZmqContext msgbus_ctx = zmq_handler::getCTX(strTopic);
						zmq_handler::stZmqPubContext pubCtx = zmq_handler::getPubCTX(strTopic);

						PublishJsonHandler::instance().publishJson(msg, msgbus_ctx.m_pContext, pubCtx.m_pContext, strTopic);
#ifdef INSTRUMENTATION_LOG
						msg_envelope_serialized_part_t* parts = NULL;
						int num_parts = msgbus_msg_envelope_serialize(msg, &parts);
						if(num_parts > 0)
						{
							if(NULL != parts[0].bytes)
							{
								std::string tempStr(parts[0].bytes);

								string temp = "on-demand response received with following parameters :: ";
								temp.append(tempStr);

								CLogger::getInstance().log(DEBUG, LOGDETAILS(temp));

							}

							msgbus_msg_envelope_serialize_destroy(parts, num_parts);
						}
#endif
					}
				}
			}
			catch(const std::exception &e)
			{
				eFunRetType = MBUS_JSON_APP_ERROR_EXCEPTION_RISE;
				CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
			}

			if(NULL != root)
				cJSON_Delete(root);

			if(NULL != stMbusApiPram.m_pu8Data)
			{
				delete[] stMbusApiPram.m_pu8Data;
				stMbusApiPram.m_pu8Data  = NULL;
			}
		}while(0);
	}

	CLogger::getInstance().log(DEBUG, LOGDETAILS("End"));
	return eFunRetType;
}

/**
 * Convert char to int
 * @param input :[in] char to convert to int
 * @return converted integer
 */
int char2int(char input)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start"));
	if(input >= '0' && input <= '9')
		return input - '0';
	if(input >= 'A' && input <= 'F')
		return input - 'A' + 10;
	if(input >= 'a' && input <= 'f')
		return input - 'a' + 10;

	CLogger::getInstance().log(DEBUG, LOGDETAILS("End"));
	throw std::invalid_argument("Invalid input string");
}

/**
 * convert hex to binary
 * @param src	:[in] hex string to convert to binary
 * @param iOpLen:[in]iOpLen
 * @param target:[out] uint8_t
 * @return
 */
int hex2bin(const std::string &src, int iOpLen, uint8_t* target)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start"));
	int iOpCharPos = 0;
	int i = 0;
	try
	{
		int iLen = src.length();
		if(0 != (iLen%2))
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("input string is not proper"));
			return -1;
		}
		// Check if hex string starts with 0x. If yes ignore first 2 letters
		if( ('0' == src[0]) && (('X' == src[1]) || ('x' == src[1])) )
		{
			i = 2;
		}
		iLen = iLen - i;
		if((iLen / 2) != iOpLen)
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("width mismatch " + to_string(iLen) + "!=" + to_string(iOpLen)));
			return -1;
		}
		iLen = iLen + i;
		while (i+1 < iLen)
		{
			unsigned char byte1 = char2int(src[i])*16 + char2int(src[i+1]);
			i = i + 2;
			if ((i+1) < iLen)
			{
				unsigned char byte2 = char2int(src[i])*16 + char2int(src[i+1]);
				target[iOpCharPos] = byte2;
				iOpCharPos++;
			}
			target[iOpCharPos] = byte1;
			iOpCharPos++;
			i = i + 2;
		}
	}
	catch(std::exception &e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		return -1;
	}
	CLogger::getInstance().log(DEBUG, LOGDETAILS("End"));
	return iOpCharPos;
}

/**
 * Validate input json
 * @param stSourcetopic	:[in] source topic
 * @param stWellhead	:[in] well head
 * @param stCommand		:[in] command
 * @return 	true : on success,
 * 			false : on error
 */
bool modWriteHandler::validateInputJson(std::string stSourcetopic, std::string stWellhead, std::string stCommand)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("End"));
	bool retValue = false;
	try
	{
		std::string strSearchStrin = "/";
		std::size_t found = stSourcetopic.find(strSearchStrin);
		std::size_t found1 = stSourcetopic.find(strSearchStrin.c_str(), found+1);
		std::size_t found2 = stSourcetopic.find(strSearchStrin.c_str(), found1+1);
		std::string tempWellhead = stSourcetopic.substr(found1+1, found2-found1-1);

		std::size_t found3 = stSourcetopic.find(strSearchStrin.c_str(), found2+1);
		std::string tempCommand = stSourcetopic.substr(found2+1, found3-found2-1);

		if(tempWellhead == stWellhead && tempCommand == stCommand)
		{
			retValue = true;
		}
	}
	catch(const std::exception &e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		CLogger::getInstance().log(DEBUG, LOGDETAILS("i/p json wellhead or command mismatch with sourcetopic"));
	}

	CLogger::getInstance().log(DEBUG, LOGDETAILS("End"));
	return retValue;
}

/**
 * json parser for on demand requests
 * @param root			:[in] json string
 * @param stMbusApiPram	:[in] modbus API param
 * @param funcCode		:[in] function code
 * @param txID			:[in] transaction id
 * @param reqData		:[in] on demand request
 * @return appropriate error code
 */
eMbusStackErrorCode modWriteHandler::jsonParserForOnDemandRequest(cJSON *root,
											MbusAPI_t &stMbusApiPram,
											unsigned char& funcCode,
											unsigned short txID,
											stOnDemandRequest& reqData)
{
	if(NULL == root)
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS(" Invalid input. cJSON root is null"));
		return MBUS_JSON_APP_ERROR_INVALID_INPUT_PARAMETER;
	}
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start"));

	eMbusStackErrorCode eFunRetType = MBUS_STACK_NO_ERROR;
	string strCommand, strValue, strWellhead, strVersion, strSourceTopic, strAppSeq;
	//stOnDemandRequest reqData;
	network_info::CDataPoint obj;
	bool isWrite = false, isValidJson = false;
	try
	{
		if(MBUS_STACK_NO_ERROR == eFunRetType)
		{
			stMbusApiPram.m_pu8Data = NULL;

			cJSON *appseq = cJSON_GetObjectItem(root,"app_seq");
			cJSON *cmd=cJSON_GetObjectItem(root,"command");
			cJSON *value=cJSON_GetObjectItem(root,"value");
			cJSON *wellhead=cJSON_GetObjectItem(root,"wellhead");
			cJSON *version=cJSON_GetObjectItem(root,"version");
			cJSON *sourcetopic=cJSON_GetObjectItem(root,"sourcetopic");
			cJSON *timestamp=cJSON_GetObjectItem(root,"timestamp");
			cJSON *usec=cJSON_GetObjectItem(root,"usec");

			if(cmd && appseq && wellhead && version && timestamp && usec && sourcetopic)
			{
				isValidJson = true;
				strAppSeq = appseq->valuestring;
				strCommand = cmd->valuestring;
				strWellhead = wellhead->valuestring;
				strVersion = version->valuestring;
				strSourceTopic = sourcetopic->valuestring;

				reqData.m_strAppSeq = strAppSeq;
				reqData.m_strMetric = strCommand;
				reqData.m_strVersion = strVersion;
				reqData.m_strWellhead = strWellhead;
				reqData.m_strTopic = strSourceTopic;
				timespec_get(&reqData.m_obtReqRcvdTS, TIME_UTC);

				//if write then "true" else "false"
				isWrite = strSourceTopic.find("write") <= strSourceTopic.length() ? true : false;
				if(true == isWrite)
				{
					if(value)
						strValue = value->valuestring;
					else
					{
						CLogger::getInstance().log(ERROR, LOGDETAILS(" Invalid input json parameter for write request"));
						eFunRetType = MBUS_JSON_APP_ERROR_INVALID_INPUT_PARAMETER;
					}
				}

				isValidJson = validateInputJson(strSourceTopic, strWellhead, strCommand);
			}
			if(!isValidJson)
			{
				CLogger::getInstance().log(ERROR, LOGDETAILS(" Invalid input json parameter or topic."));
				eFunRetType = MBUS_JSON_APP_ERROR_INVALID_INPUT_PARAMETER;
			}

			string strSearchString = "/";
			std::size_t found = strSourceTopic.find_last_of(strSearchString);
			string stTopic = strSourceTopic.substr(0, found);

			const std::map<std::string, network_info::CUniqueDataPoint>& mpp = network_info::getUniquePointList();
			struct network_info::stModbusAddrInfo addrInfo;
			try
			{
				addrInfo = mpp.at(stTopic).getWellSiteDev().getAddressInfo();
			}
			catch(const std::out_of_range& oor)
			{
				CLogger::getInstance().log(INFO, LOGDETAILS(" Request is not for this application." + std::string(oor.what())));

				return MBUS_APP_ERROR_UNKNOWN_SERVICE_REQUEST;
			}

			obj = mpp.at(stTopic).getDataPoint();
#ifdef MODBUS_STACK_TCPIP_ENABLED
			string stIpAddress = addrInfo.m_stTCP.m_sIPAddress;
			stMbusApiPram.m_u16Port = addrInfo.m_stTCP.m_ui16PortNumber;
			stMbusApiPram.m_u8DevId = addrInfo.m_stTCP.m_uiUnitID;
			CommonUtils::ConvertIPStringToCharArray(stIpAddress,&(stMbusApiPram.m_u8IpAddr[0]));
#else
			stMbusApiPram.m_u8DevId = addrInfo.m_stRTU.m_uiSlaveId;
#endif
			reqData.m_isByteSwap = obj.getAddress().m_bIsByteSwap;
			reqData.m_isWordSwap = obj.getAddress().m_bIsWordSwap;
			reqData.m_strQOS = obj.getPollingConfig().m_usQOS;

			stMbusApiPram.m_u16StartAddr = (uint16_t)obj.getAddress().m_iAddress;
			stMbusApiPram.m_u16Quantity = (uint16_t)obj.getAddress().m_iWidth;
			if(true == isWrite)
			{
				stMbusApiPram.m_lPriority = ON_DEMAND_WRITE_PRIORITY;
			}
			else
			{
				stMbusApiPram.m_lPriority = ON_DEMAND_READ_PRIORITY;
			}
			network_info::eEndPointType eType = obj.getAddress().m_eType;

			switch(eType)
			{
			case network_info::eEndPointType::eCoil:
				funcCode = isWrite ? WRITE_SINGLE_COIL: READ_COIL_STATUS;
				break;
			case network_info::eEndPointType::eHolding_Register:
				funcCode = isWrite ? stMbusApiPram.m_u16Quantity == 1 ? WRITE_SINGLE_REG: WRITE_MULTIPLE_REG : READ_HOLDING_REG;
				break;
			case network_info::eEndPointType::eInput_Register:
				funcCode = READ_INPUT_REG;
				break;
			case network_info::eEndPointType::eDiscrete_Input:
				funcCode = READ_INPUT_STATUS;
				break;
			default:
				CLogger::getInstance().log(ERROR, LOGDETAILS(" Invalid type in datapoint:: " + strCommand));
				break;
			}

			if(isWrite && (funcCode == READ_INPUT_REG || funcCode == READ_INPUT_STATUS))
			{
				funcCode = MBUS_MAX_FUN_CODE;
				return MBUS_APP_ERROR_IMPROPER_METHOD;
			}

			if(WRITE_MULTIPLE_REG == funcCode)
			{
				stMbusApiPram.m_u16ByteCount = stMbusApiPram.m_u16Quantity*2;
			}
			else if(WRITE_MULTIPLE_COILS == funcCode)
			{
				uint8_t u8ByteCount = (0 != (stMbusApiPram.m_u16Quantity%8))
											?((stMbusApiPram.m_u16Quantity/8)+1)
											:(stMbusApiPram.m_u16Quantity/8);

				stMbusApiPram.m_u16ByteCount = (uint8_t)u8ByteCount;
			}
			else if(WRITE_SINGLE_COIL == funcCode ||
					WRITE_SINGLE_REG == funcCode)
			{
				stMbusApiPram.m_u16ByteCount = MODBUS_SINGLE_REGISTER_LENGTH;
			}
			stMbusApiPram.m_pu8Data = new uint8_t[stMbusApiPram.m_u16ByteCount]();
			if(NULL != stMbusApiPram.m_pu8Data)
			{
				if(WRITE_SINGLE_COIL == funcCode)
				{
					// If value is 0x01, then write 0xFF00
					if( (0 == strValue.compare("0x00")) ||
							(0 == strValue.compare("0X00"))  ||
							(0 == strValue.compare("00")))
					{
						strValue = "0x0000";
					}
					else if( (0 == strValue.compare("0x01")) ||
							(0 == strValue.compare("0X01")) ||
							(0 == strValue.compare("01")))
					{
						strValue = "0xFF00";
					}
					else
					{
						eFunRetType = MBUS_JSON_APP_ERROR_INVALID_INPUT_PARAMETER;
					}
				}
				if(true == isWrite && funcCode != WRITE_MULTIPLE_COILS)
				{
					if((true == obj.getAddress().m_bIsByteSwap || true == obj.getAddress().m_bIsWordSwap))
					{
						std::vector<uint8_t> tempVt;
						int i = 0;
						if( ('0' == strValue[0]) && (('X' == strValue[1]) || ('x' == strValue[1])) )
						{
							i = 2;
						}
						int iLen = strValue.length();
						while(i < iLen)
						{
							unsigned char byte1 = char2int(strValue[i])*16 + char2int(strValue[i+1]);
							tempVt.push_back(byte1);
							i = i+2;
						}
						strValue = common_Handler::swapConversion(tempVt,
								!obj.getAddress().m_bIsByteSwap,
								obj.getAddress().m_bIsWordSwap);
					}
					int retVal = hex2bin(strValue, stMbusApiPram.m_u16ByteCount, stMbusApiPram.m_pu8Data);
					if(-1 == retVal)
					{
						CLogger::getInstance().log(FATAL, LOGDETAILS("Invalid value in request json."));
						eFunRetType = MBUS_JSON_APP_ERROR_INVALID_INPUT_PARAMETER;
					}
				}
				/*else
				{
					char* token = strtok(strValue.c_str(), ",");
					for (unsigned int i= 0 ; i< stMbusApiPram.m_u16ByteCount && NULL != token; ++i)
					{
						uint32_t val = stoi(token);
						stMbusApiPram.m_pu8Data[i] = (uint8_t)val;
						token = strtok(NULL, ",");
					}
				}*/
			}
			else
			{
				eFunRetType = MBUS_JSON_APP_ERROR_MALLOC_FAILED;
				CLogger::getInstance().log(ERROR, LOGDETAILS(" Unable to allocate memory. Request not sent"));
			}
		}
	}
	catch(const std::exception &e)
	{
		eFunRetType = MBUS_JSON_APP_ERROR_EXCEPTION_RISE;
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}

	CLogger::getInstance().log(DEBUG, LOGDETAILS("End"));
	return eFunRetType;
}

/**
 * Create write listener
 */
void modWriteHandler::createWriteListener()
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start"));

	std::vector<std::string> stTopics = PublishJsonHandler::instance().getSubTopicList();
	//for(auto sTopic : stTopics)
	for(std::vector<std::string>::iterator it = stTopics.begin(); it != stTopics.end(); ++it)
	{
		if(it->empty()) {
			CLogger::getInstance().log(ERROR, LOGDETAILS("SubTopics are not configured"));
			continue;
		}
		std::thread(&modWriteHandler::subscribeDeviceListener, this, *it).detach();
	}
	CLogger::getInstance().log(DEBUG, LOGDETAILS("End"));
}

/**
 * Return single instance of this class
 * @return
 */
modWriteHandler& modWriteHandler::Instance()
{
	static modWriteHandler _self;
	return _self;
}

/**
 * Process ZMQ message
 * @param msg	:	[in] actual message
 * @param stTopic:	[in] received topic
 */
bool modWriteHandler::processMsg(msg_envelope_t *msg, std::string stTopic)
{
	msg_envelope_serialized_part_t* parts = NULL;
	int num_parts = 0;
	bool bRet = false;

	if(NULL == msg)
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS(
				"NULL pointer received while processing msg."));
		return false;
	}
	num_parts = msgbus_msg_envelope_serialize(msg, &parts);
	if(num_parts <= 0)
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS(
				" Failed to serialize message"));
	}

	if(NULL != parts)
	{
		if(NULL != parts[0].bytes)
		{
			struct stRequest stWriteRequestNode;
			std::string strMsg(parts[0].bytes);

			stWriteRequestNode.m_strTopic = stTopic;
			stWriteRequestNode.m_strMsg = strMsg;

			string strSearchString = "_";
			std::size_t found = stTopic.find_last_of(strSearchString);
			string tempTopic = stTopic.substr(found+1, stTopic.length());
			string strToCompare = "WriteRequest";
			bool bCompareVal =  std::equal(tempTopic.begin(), tempTopic.end(),
					strToCompare.begin(),
		            [](char a, char b) {
		                return tolower(a) == tolower(b);
		            });

			if(bCompareVal)
			{
				stWriteRequestNode.m_lPriority = ON_DEMAND_WRITE_PRIORITY;
			}
			else
			{
				stWriteRequestNode.m_lPriority = ON_DEMAND_READ_PRIORITY;
			}
			string initiate = " write initiated for msg:: ";
			initiate.append(strMsg);

			CLogger::getInstance().log(INFO, LOGDETAILS(initiate));
			/// pushing write request to q to process.
			pushToWriteTCPQueue(stWriteRequestNode);

			bRet = true;
		}
		else
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS(
					"NULL pointer received while processing msg."));
			bRet = false;
		}
	}
	else
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS(
				"NULL pointer received while processing msg."));
		bRet = false;
	}

	if(parts != NULL)
	{
		msgbus_msg_envelope_serialize_destroy(parts, num_parts);
	}
	if(msg != NULL)
	{
		msgbus_msg_envelope_destroy(msg);
	}
	msg = NULL;
	parts = NULL;

	return bRet;
} 

/**
 * Subscribe to device listener
 * @param stTopic	:[in] topic to subscribe
 */
void modWriteHandler::subscribeDeviceListener(const std::string stTopic)
{
	msg_envelope_t *msg = NULL;
	msgbus_ret_t ret;

#ifdef REALTIME_THREAD_PRIORITY
	PublishJsonHandler::instance().set_thread_priority();
#endif

	try
	{
		zmq_handler::stZmqContext msgbus_ctx = zmq_handler::getCTX(stTopic);
		zmq_handler::stZmqSubContext stsub_ctx = zmq_handler::getSubCTX(stTopic);

		while((msgbus_ctx.m_pContext != NULL) && (NULL != stsub_ctx.sub_ctx)
				&& (false == g_stopThread.load()))
		{
			ret = msgbus_recv_wait(msgbus_ctx.m_pContext, stsub_ctx.sub_ctx, &msg);
			if(ret != MSG_SUCCESS)
			{
				CLogger::getInstance().log(ERROR, LOGDETAILS("Failed to receive message errno ::" + std::to_string(ret)));
				continue;
			}

			/// process messages
			processMsg(msg, stTopic);
		}
	}
	catch(const std::exception& e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}
}

/**
 * Push write request to TCP queue
 * @param stWriteRequestNode
 * @return 	true : on success,
 * 			false : on error
 */
bool modWriteHandler::pushToWriteTCPQueue(struct stRequest &stWriteRequestNode)
{
	try
	{
		std::lock_guard<std::mutex> lock(__writeReqMutex);
		stackTCPWriteReqQ.push(stWriteRequestNode);
		// Signal thread to process
		sem_post(&semaphoreWriteReq);

		return true;
	}
	catch(const std::exception& e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}

	return false;
}

/**
 * Initialize write handler threads
 */
void modWriteHandler::initWriteHandlerThreads()
{
	static bool bWriteSpawned = false;
	try
	{
		if(false == bWriteSpawned)
		{
			// Spawn 5 thread to process write request
			//for (int i = 0; i < 5; i++)
			{
				std::thread{std::bind(&modWriteHandler::onDemandInfoHandler, std::ref(*this))}.detach();
			}
			bWriteSpawned = true;
		}
	}
	catch(const std::exception& e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}
}

/**
 * Get data to process from queue
 * @param stWriteProcessNode :[out] request to process
 * @return 	true : on success,
 * 			false : on error
 */
bool modWriteHandler::getDataToProcess(struct stRequest &stWriteProcessNode)
{
	CLogger::getInstance().log(ERROR, LOGDETAILS("Start"));
	bool retvalue = false;
	try
	{
		std::lock_guard<std::mutex> lock(__writeReqMutex);
		if(stackTCPWriteReqQ.empty())
		{
			CLogger::getInstance().log(DEBUG, LOGDETAILS("Response queue is empty"));
			return false;
		}
		stWriteProcessNode = stackTCPWriteReqQ.top();
		stackTCPWriteReqQ.pop();

		retvalue = true;
	}
	catch(const std::exception& e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		retvalue = false;
	}

	CLogger::getInstance().log(ERROR, LOGDETAILS("End"));
	return retvalue;
}
