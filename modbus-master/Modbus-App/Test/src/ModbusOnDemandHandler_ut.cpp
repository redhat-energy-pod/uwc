/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/


#include "../include/ModbusOnDemandHandler_ut.hpp"
#include "ConfigManager.hpp"
#include "cjson/cJSON.h"

#include "ZmqHandler.hpp"


extern int hex2bin(const std::string &src, int iOpLen, uint8_t* target);
extern int char2int(char input);


void ModbusOnDemandHandler_ut::SetUp()
{
	// Setup code
}

void ModbusOnDemandHandler_ut::TearDown()
{
	// TearDown code
}

/**
 * Test case to check jsonParserForOnDemandRequest() with invalid service request
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ModbusOnDemandHandler_ut, jsonParserForOnDemandRequest_InvServiceReq)
{

	stMbusApiPram.m_stOnDemandReqData.m_isByteSwap				= true;
	stMbusApiPram.m_stOnDemandReqData.m_isRT					= true;
	stMbusApiPram.m_stOnDemandReqData.m_isWordSwap				= true;
	stMbusApiPram.m_stOnDemandReqData.m_obtReqRcvdTS.tv_nsec	= 21132323;
	stMbusApiPram.m_stOnDemandReqData.m_obtReqRcvdTS.tv_sec		= 1;
	stMbusApiPram.m_stOnDemandReqData.m_strAppSeq				= "455";
	stMbusApiPram.m_stOnDemandReqData.m_strEisTime				= "2020-03-31 12:34:56";
	stMbusApiPram.m_stOnDemandReqData.m_strMetric				= "Flow";
	stMbusApiPram.m_stOnDemandReqData.m_strMqttTime				= "2020-03-13 12:34:56";
	stMbusApiPram.m_stOnDemandReqData.m_strTopic				= "/flowmeter/PL0/Flow/read";
	stMbusApiPram.m_stOnDemandReqData.m_strVersion				= "2.1";
	stMbusApiPram.m_stOnDemandReqData.m_strWellhead				= "PL0";
	stMbusApiPram.m_stOnDemandReqData.m_sUsec					= "0";
	stMbusApiPram.m_stOnDemandReqData.m_sTimestamp				= "0:0:0";

	bool isWrite = false;

	try
	{
		eFunRetType = onDemandHandler::Instance().jsonParserForOnDemandRequest(stMbusApiPram,
				m_u8FunCode,
				stMbusApiPram.m_u16TxId,
				isWrite);
		EXPECT_EQ(APP_ERROR_UNKNOWN_SERVICE_REQUEST, eFunRetType);
	}
	catch( std::exception &e)
	{
		EXPECT_EQ("", e.what());
	}

}

/**
 * Test case to check jsonParserForOnDemandRequest() with invalid topic message
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ModbusOnDemandHandler_ut, jsonParserForOnDemandRequest_InvalidTopicMsg)
{

	stMbusApiPram.m_u16TxId = PublishJsonHandler::instance().getTxId();
	stMbusApiPram.m_stOnDemandReqData.m_strAppSeq = "1234";
	stMbusApiPram.m_stOnDemandReqData.m_strMetric =  "Flow";
	stMbusApiPram.m_stOnDemandReqData.m_sValue = "0X00";
	stMbusApiPram.m_stOnDemandReqData.m_strWellhead = "PL0";
	stMbusApiPram.m_stOnDemandReqData.m_strVersion = "version";
	stMbusApiPram.m_stOnDemandReqData.m_strTopic = "Invalid";
	stMbusApiPram.m_stOnDemandReqData.m_sTimestamp = "2020-02-12 06:14:15";
	stMbusApiPram.m_stOnDemandReqData.m_sUsec = "1581488055204186";
	stMbusApiPram.m_stOnDemandReqData.m_strMqttTime = "2020-03-13 12:34:56";
	stMbusApiPram.m_stOnDemandReqData.m_strEisTime = "2020-03-31 12:34:56";
	stMbusApiPram.m_stOnDemandReqData.m_isRT = true;
	stMbusApiPram.m_nRetry = 1;
	stMbusApiPram.m_lPriority = 1;

	try
	{
		eFunRetType = onDemandHandler::Instance().jsonParserForOnDemandRequest(stMbusApiPram,
				m_u8FunCode,
				16,
				true);

		EXPECT_EQ(APP_ERROR_UNKNOWN_SERVICE_REQUEST, eFunRetType);
	}

	catch(std::exception &e)
	{
		EXPECT_EQ("", e.what());

	}
}

/**
 * Test case to check jsonParserForOnDemandRequest() with Valid Input Json
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ModbusOnDemandHandler_ut, jsonParserForOnDemandRequest_ValidIpJason)
{

	stMbusApiPram.m_u16TxId = PublishJsonHandler::instance().getTxId();
	stMbusApiPram.m_stOnDemandReqData.m_strAppSeq = "1234";
	stMbusApiPram.m_stOnDemandReqData.m_strMetric =  "D1";
	stMbusApiPram.m_stOnDemandReqData.m_sValue = "0X00";
	stMbusApiPram.m_stOnDemandReqData.m_strWellhead = "PL0";
	stMbusApiPram.m_stOnDemandReqData.m_strVersion = "version";
#ifdef MODBUS_STACK_TCPIP_ENABLED
	stMbusApiPram.m_stOnDemandReqData.m_strTopic = "/flowmeter/PL0/D1/read";
#else
	stMbusApiPram.m_stOnDemandReqData.m_strTopic = "/iou/PL0/D1/read";
#endif
	stMbusApiPram.m_stOnDemandReqData.m_sTimestamp = "2020-02-12 06:14:15";
	stMbusApiPram.m_stOnDemandReqData.m_sUsec = "1581488055204186";
	stMbusApiPram.m_stOnDemandReqData.m_strMqttTime = "2020-03-13 12:34:56";
	stMbusApiPram.m_stOnDemandReqData.m_strEisTime = "2020-03-31 12:34:56";
	stMbusApiPram.m_stOnDemandReqData.m_strAppSeq = "1";
	stMbusApiPram.m_stOnDemandReqData.m_isRT = true;
	stMbusApiPram.m_nRetry = 1;
	stMbusApiPram.m_lPriority = 1;


	try
	{
		eFunRetType = onDemandHandler::Instance().jsonParserForOnDemandRequest(stMbusApiPram,
				m_u8FunCode,
				16,
				true);

		EXPECT_EQ(APP_ERROR_INVALID_INPUT_JSON, eFunRetType);
	}

	catch( std::exception &e)
	{
		EXPECT_EQ("", e.what());

	}
}

/**
 * modWriteHandler_getInstance() Check the instance type returned by function
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ModbusOnDemandHandler_ut, modWriteHandler_getInstance)
{
	EXPECT_EQ(typeid(onDemandHandler), typeid(onDemandHandler::Instance()));
}

/**
 * checks the behaviour of the createWriteListener() function for valid topic and type of topic
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ModbusOnDemandHandler_ut, createWriteListner_test)
{

	/*setenv("WRITE_RESPONSE_TOPIC", "PL_0", 1);

	std::string topic = std::getenv("WRITE_RESPONSE_TOPIC");*/

	std::string sTopic = "MQTT-Export/PL0_flowmeter1_write";
	//std::string sTopic = "MQTT-Export/Modbus-TCP-Master_ReadRequest";
	std::string topictype = "sub";
	try
	{

		onDemandHandler::Instance().createOnDemandListener();

	}
	catch(std::exception &e)
	{
		EXPECT_EQ("basic_string::_M_construct null not valid", (string)e.what());
	}
}

/**
 * Check the behaviour of subscribeDeviceListener() function
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ModbusOnDemandHandler_ut, subscribeDeviceListener_CallbaclNULL)
{

	globalConfig::COperation a_refOps;
	void *vpCallback = NULL;
	string SbTopic = "MQTT_Export/MQTT_Export_RdReq";


	try
	{

		onDemandHandler::Instance().subscribeDeviceListener(topic,
				a_refOps,
				false,
				vpCallback,
				0,
				1,
				false);

	}
	catch(std::exception &e)
	{

		std::cout<<e.what()<<std::endl;
		EXPECT_EQ("",(string)e.what());
	}

}

/**
 * checks the behaviour of the hex2bin with invalid input structure
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ModbusOnDemandHandler_ut, hex2bin_InvalidInputStructure)
{

	stMbusApiPram.m_lPriority = 1;
	stMbusApiPram.m_u16ByteCount = 2;
	stMbusApiPram.m_u16Quantity = 4;
	stMbusApiPram.m_u16StartAddr = 60000;
	stMbusApiPram.m_u16TxId = 23;
	stMbusApiPram.m_u8DevId = 5;

	try
	{
		hex2bin(tValue, stMbusApiPram.m_u16ByteCount, stMbusApiPram.m_pu8Data);
	}
	catch(std::exception &e)
	{
		EXPECT_NE("", (string)e.what());
	}

}

/**
 * checks the behaviour of the hex2bin when there is odd num of characters in input string
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ModbusOnDemandHandler_ut, hex2bin_OddCharInString)
{

	stMbusApiPram.m_lPriority = 1;
	stMbusApiPram.m_u16ByteCount = 2;
	//	stMbusApiPram.m_u16Port = 1234;
	stMbusApiPram.m_u16Quantity = 4;
	stMbusApiPram.m_u16StartAddr = 564;
	stMbusApiPram.m_u16TxId = 23;
	stMbusApiPram.m_u8DevId = 5;

	try
	{
		int result = hex2bin("input", stMbusApiPram.m_u16ByteCount, stMbusApiPram.m_pu8Data);
		EXPECT_EQ(-1, result);

	}
	catch(std::exception &e)
	{
		EXPECT_NE("", (string)e.what());
	}

}

/**
 * checks the behaviour of the hex2bin with invalid input string
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ModbusOnDemandHandler_ut, hex2bin_InvalidInputString)
{
	hex2bin(tValue, stMbusApiPram.m_u16ByteCount, stMbusApiPram.m_pu8Data);
	try
	{
		byte1 = char2int(tValue[i])*16 + char2int(tValue[i+1]);

	}
	catch(std::exception &e)
	{
		EXPECT_EQ("Invalid input string",(string)e.what());
	}
}

/**
 * checks the behaviour of the validateInputJson() with a valid jason string
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ModbusOnDemandHandler_ut, validateInputJson_ValidJason)
{

	try
	{
		bool Value = onDemandHandler::Instance().validateInputJson("/flowmeter/PL0/Flow/read",
				"PL0",
				"Flow");

		EXPECT_EQ(true, Value);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ(" ", (string)e.what());

	}

}

/**
 * checks the behaviour of the validateInputJson() with a invalid jason string
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ModbusOnDemandHandler_ut, validateInputJson_InValidJason)
{

	try
	{
		bool Value = onDemandHandler::Instance().validateInputJson("/flowmeter/PL0/Flow/read",
				"Invalid",
				"Flow");

		EXPECT_EQ(false, Value);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ(" ", (string)e.what());

	}

}

/**
 * checks the behaviour of the createErrorResponse() with read response
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ModbusOnDemandHandler_ut, createErrorResponseRead)
{

	stMbusApiPram.m_lPriority = 1;
	stMbusApiPram.m_u16ByteCount = 2;
	stMbusApiPram.m_u16Quantity = 4;
	stMbusApiPram.m_u16StartAddr = 500;
	stMbusApiPram.m_u16TxId = 23;
	stMbusApiPram.m_u8DevId = 5;

	bool isWrite = true;
	stOnDemandRequest reqData;
	reqData.m_isByteSwap = true;
	reqData.m_isRT = true;
	reqData.m_isWordSwap = false;
	reqData.m_obtReqRcvdTS.tv_nsec = 21132323;
	reqData.m_obtReqRcvdTS.tv_sec = 1;
	reqData.m_strAppSeq = "455";
	reqData.m_strMetric = "Test";
	reqData.m_strTopic = "kzdjfhdszh";
	reqData.m_strVersion = "2.1";
	reqData.m_strWellhead = "test";
	MbusAPI_t stMbusApiPram = {};

	try
	{

		common_Handler::insertReqData(stMbusApiPram.m_u16TxId, stMbusApiPram);
		onDemandHandler::Instance().createErrorResponse(eFunRetType, u8FunCode, stMbusApiPram.m_u16TxId, true, isWrite);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ("Request not found in map", (string)e.what());
	}

}

/**
 * checks the behaviour of the createErrorResponse() with write response
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ModbusOnDemandHandler_ut, createErrorResponseWrite)
{

	bool isWrite = false;
	stOnDemandRequest reqData;
	reqData.m_isByteSwap = true;
	reqData.m_isWordSwap = false;
	reqData.m_obtReqRcvdTS.tv_nsec = 21132323;
	reqData.m_obtReqRcvdTS.tv_sec = 1;
	reqData.m_strAppSeq = "455";
	reqData.m_strMetric = "Test";
	reqData.m_strTopic = "kzdjfhdszh";
	reqData.m_strVersion = "2.1";
	reqData.m_strWellhead = "test";

	try
	{
		onDemandHandler::Instance().createErrorResponse(eFunRetType, m_u8FunCode, stMbusApiPram.m_u16TxId, true , isWrite);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ("Request not found in map", (string)e.what());
	}

}

/**
 * checks the behaviour of the createErrorResponse() with non RT response
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ModbusOnDemandHandler_ut, createErrorResponseNonRT)
{
	bool isWrite = true;
	stOnDemandRequest reqData;
	reqData.m_isByteSwap = true;
	reqData.m_isWordSwap = false;
	reqData.m_obtReqRcvdTS.tv_nsec = 21132323;
	reqData.m_obtReqRcvdTS.tv_sec = 1;
	reqData.m_strAppSeq = "455";
	reqData.m_strMetric = "Test";
	reqData.m_strTopic = "kzdjfhdszh";
	reqData.m_strVersion = "2.1";
	reqData.m_strWellhead = "test";

	try
	{
		onDemandHandler::Instance().createErrorResponse(eFunRetType, m_u8FunCode, stMbusApiPram.m_u16TxId, false , isWrite);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ("Request not found in map", (string)e.what());
	}

}

/**
 * checks the behaviour of the createErrorResponse() with RT response
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ModbusOnDemandHandler_ut, createErrorResponseRt)
{
	bool isWrite = false;
	bool isRT = false;
	stOnDemandRequest reqData;
	reqData.m_isByteSwap = true;
	reqData.m_isWordSwap = false;
	reqData.m_obtReqRcvdTS.tv_nsec = 21132323;
	reqData.m_obtReqRcvdTS.tv_sec = 1;
	reqData.m_strAppSeq = "455";
	reqData.m_strMetric = "Test";
	reqData.m_strTopic = "kzdjfhdszh";
	reqData.m_strVersion = "2.1";
	reqData.m_strWellhead = "test";

	try
	{
		onDemandHandler::Instance().createErrorResponse(eFunRetType, m_u8FunCode, stMbusApiPram.m_u16TxId, isRT , isWrite);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ("Request not found in map", (string)e.what());
	}

}

/**
 * checks the behaviour of the onDemandInfoHandler() when read data received from ZMQ is NULL
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ModbusOnDemandHandler_ut, OnDemandInfoHandler_MbusAPI_tNULL)
{
	eMbusAppErrorCode eMbusAppErrorCode_variable;


	eMbusAppErrorCode_variable = onDemandHandler::Instance().onDemandInfoHandler(NULL,
			"Topic",
			(void *)0x01,
			false);

	EXPECT_EQ(APP_INTERNAL_ERORR, eMbusAppErrorCode_variable);

}

/**
 * checks the behaviour of the onDemandInfoHandler() when stack callback is NULL
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ModbusOnDemandHandler_ut, OnDemandInfoHandler_CallbackNULL)
{
	eMbusAppErrorCode eMbusAppErrorCode_variable;
	MbusAPI_t MbusAPI_t_obj;


	eMbusAppErrorCode_variable = onDemandHandler::Instance().onDemandInfoHandler(&MbusAPI_t_obj,
			"Topic",
			NULL,
			false);

	EXPECT_EQ(APP_INTERNAL_ERORR, eMbusAppErrorCode_variable);

}

/**
 * checks the behaviour of the getMsgElement() when message received from ZMQ is NULL
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ModbusOnDemandHandler_ut, getMsgElement_NULL)
{
	string RetStr = onDemandHandler::Instance().getMsgElement(NULL, "app_seq");

	EXPECT_EQ("", RetStr);

}

/**
 * checks the behaviour of the processMsg() when message received from ZMQ is NULL
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ModbusOnDemandHandler_ut, processMsg_MsgEnvNULL)
{
	std::string topic = "";
	bool RetVal = onDemandHandler::Instance().processMsg(NULL,
			topic,
			false,
			NULL,
			1,
			1,
			false);

	EXPECT_EQ(RetVal, false);
}
