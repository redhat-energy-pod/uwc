/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#ifndef INCLUDE_INC_COMMON_HPP_
#define INCLUDE_INC_COMMON_HPP_

#include "ZmqHandler.hpp"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#ifdef __cplusplus
extern "C" {
#include "API.h"
}
#endif
#include <map>

#ifdef __linux

typedef int             BOOL;
typedef bool            BOOLEAN;
typedef unsigned long   DWORD;
typedef unsigned long   ULONG;
typedef void            VOID;

#define TRUE true
#define FALSE false
#endif

#define MODBUS_SINGLE_REGISTER_LENGTH (2)

using namespace std;

// This structure defines request data parameters for modbus
typedef struct ReqData
{
	unsigned char  m_u8DataLen;
	unsigned char  *m_pu8Data;
}ReqData_t;

// This structure defines request data parameters for modbus
typedef struct RespData
{
	unsigned char m_u8DataLen;
	unsigned char  *m_pu8Data;
	stException_t  m_stExcStatus;
}RespData_t;

// This structure defines sub tag parameters for modbus request
typedef struct SubTagPart
{
	unsigned char m_u8NodeId;
	unsigned char  m_u8FunCode;
	unsigned char  m_u8Operation;
	unsigned short m_u16StartAddr;
	unsigned short m_u16EndAddr;
	unsigned short m_u16Quantity;
	ReqData_t 	   m_stReqData;
	RespData_t	   m_stRespData;
	struct SubTagPart  *m_pstNextSubTag;
}SubTagPart_t;

#ifdef MODBUS_STACK_TCPIP_ENABLED

// This structure defines generic request parameters for modbus
typedef struct RestMbusReqGeneric
{
	unsigned short m_u16ReffId;
	unsigned char m_u8TagCount;
	unsigned char m_u8IpAddr[4];
	uint16_t		m_u16Port;
	unsigned char m_u8DevId;
	unsigned char m_u8NodeId;
	unsigned char  m_u8FunCode;
	unsigned char  m_u8Operation;
	unsigned short m_u16StartAddr;
	unsigned short m_u16EndAddr;
	unsigned short m_u16Quantity;
	ReqData_t 	   m_stReqData;
	RespData_t	   m_stRespData;
}RestMbusReqGeneric_t;

#else
// This structure defines generic request parameters for modbus RTU
typedef struct RestMbusReqGeneric
{
	unsigned short m_u16ReffId;
	unsigned char m_u8TagCount;
	unsigned char address;
	unsigned char m_u8DevId;
	unsigned char m_u8NodeId;
	unsigned char  m_u8FunCode;
	unsigned char  m_u8Operation;
	unsigned short m_u16StartAddr;
	unsigned short m_u16EndAddr;
	unsigned short m_u16Quantity;
	ReqData_t 	   m_stReqData;
	RespData_t	   m_stRespData;
}RestMbusReqGeneric_t;

#endif

struct stOnDemandRequest
{
	std::string m_strAppSeq;
	std::string m_strWellhead;
	std::string m_strMetric;
	std::string m_strVersion;
	std::string m_strTopic;
	bool m_isByteSwap;
	bool m_isWordSwap;
	bool m_isRT;
	struct timespec m_obtReqRcvdTS;
	//long m_lPriority;
	std::string m_strMqttTime;
	std::string m_strEisTime;
};

// This structure defines generic parameters for modbus common API
typedef struct MbusAPI
{
	unsigned char	m_u8DevId;
	unsigned char	m_u8IpAddr[4];
	uint16_t		m_u16Port;
	unsigned short  m_u16TxId;
	unsigned short  m_u16Quantity;
	unsigned short  m_u16StartAddr;
	unsigned short  m_u16ByteCount;
	unsigned char  *m_pu8Data;
	/** Holds the Msg Priority  */
	long m_lPriority;
	/** Holds the Mse Timeout  */
	uint32_t m_u32mseTimeout;
	int m_nRetry;
	stOnDemandRequest m_stOnDemandReqData;
}MbusAPI_t;

// This structure defines request parameters for read periodic
typedef struct RestRdPeriodicTagPart
{
    bool  IsSubscription;
    bool  bIsRespAwaited;
    unsigned char m_u8FrameReqType;
    unsigned char 	m_u8IpAddr[4];
    unsigned char 	m_u8UnitId;
    unsigned char  	m_u8FunCode;
    unsigned char  *m_pu8RespData;
    unsigned short 	m_u16StartAddr;
    unsigned short 	m_u16EndAddr;
    unsigned short 	m_u16Quantity;
    unsigned short 	m_u16ReqDataLen;
    unsigned short 	m_u16RespDataLen;
    unsigned short  m_u16TxId;
    unsigned int   	m_u32Interval;
    unsigned int	m_u32LocInterval;
	unsigned char 	m_stMQTT_Topic[200];
    stException_t m_stException;
}RestRdPeriodicTagPart_t;

// Function to get function code
unsigned char GetFunctionCode(uint8_t u8ReadWrite,
								  uint16_t u16Quantity,
								  uint8_t u8MbudTable);

// This enumerator defines MODBUS error codes
typedef enum MbusStackErrorCode
{
	MBUS_STACK_NO_ERROR,
	MBUS_STACK_TXNID_OR_UNITID_MISSMATCH,
	MBUS_STACK_ERROR_SOCKET_FAILED,
	MBUS_STACK_ERROR_CONNECT_FAILED,
	MBUS_STACK_ERROR_SEND_FAILED,
	MBUS_STACK_ERROR_RECV_FAILED,
	MBUS_STACK_ERROR_RECV_TIMEOUT,
	MBUS_STACK_ERROR_MALLOC_FAILED,
	MBUS_STACK_ERROR_QUEUE_SEND,
	MBUS_STACK_ERROR_QUEUE_RECIVE,
	MBUS_STACK_ERROR_THREAD_CREATE,
	MBUS_STACK_ERROR_INVALID_INPUT_PARAMETER,
	MBUS_STACK_ERROR_PACKET_LENGTH_EXCEEDED,
	MBUS_JSON_APP_ERROR_MALLOC_FAILED,
	MBUS_JSON_APP_ERROR_INVALID_INPUT_PARAMETER,
	MBUS_JSON_APP_ERROR_PACKET_LENGTH_EXCEEDED,
	MBUS_JSON_APP_ERROR_NULL_POINTER,
	MBUS_JSON_APP_ERROR_INVALID_FUN_CODE,
	MBUS_JSON_APP_ERROR_EXCEPTION_RISE,
	MBUS_APP_TAG_OR_SUBTAG_NOT_FOUND,
	MBUS_APP_SUBSCRIPTION_NOT_FOUND,
	MBUS_PERIODIC_FAILED_TO_FIND_TOPIC,
	MBUS_PERIODIC_INTERVAL_OUT_OF_RANG,
	MBUS_INVALID_ENV_VARIABLE,
	MBUS_APP_ERROR_UNKNOWN_SERVICE_REQUEST,
	MBUS_APP_ERROR_IMPROPER_METHOD,
	MBUS_APP_ERROR_CONTENT_TYPE_NOT_JSON,
	MBUS_APP_ERROR_AUTHENTICATION_FAILED,
}eMbusStackErrorCode;

enum eMbusRequestType
{
	MBUS_REQUEST_NONE,
	MBUS_REQUEST_READ,
	MBUS_REQUEST_READRT,
	MBUS_REQUEST_WRITE,
	MBUS_REQUEST_WRITERT,
};

enum eMbusCallbackType
{
	MBUS_CALLBACK_POLLING,
	MBUS_CALLBACK_POLLING_RT,
	MBUS_CALLBACK_ONDEMAND_READ,
	MBUS_CALLBACK_ONDEMAND_READ_RT,
	MBUS_CALLBACK_ONDEMAND_WRITE,
	MBUS_CALLBACK_ONDEMAND_WRITE_RT,
};

/// function to call Modbus stack APIs
uint8_t Modbus_Stack_API_Call(unsigned char u8FunCode,
								MbusAPI_t *pstMbusApiPram,void* vpCallBackFun);

void OnDemandRead_AppCallback(stMbusAppCallbackParams_t *pstMbusAppCallbackParams, uint16_t uTxID);
void OnDemandReadRT_AppCallback(stMbusAppCallbackParams_t *pstMbusAppCallbackParams, uint16_t uTxID);
void OnDemandWrite_AppCallback(stMbusAppCallbackParams_t *pstMbusAppCallbackParams, uint16_t uTxID);
void OnDemandWriteRT_AppCallback(stMbusAppCallbackParams_t *pstMbusAppCallbackParams, uint16_t uTxID);

using std::string;
#include <string>
#include <vector>

namespace common_Handler
{
/// function for byteswap and wordswap
std::string swapConversion(std::vector<unsigned char> vt, bool a_bIsByteSwap = false, bool a_bIsWordSwap = false);

/// function to read current time and usec
void getTimeParams(std::string &a_sTimeStamp, std::string &a_sUsec);

/// function to get request json
bool getReqData(unsigned short seqno, MbusAPI_t& reqData);

/// function to insert new entry in map
bool insertReqData(unsigned short, MbusAPI_t);

/// function to update map value
bool updateReqData(unsigned short, MbusAPI_t);

/// function to remove entry from the map
void removeReqData(unsigned short);
}

#endif /* INCLUDE_INC_COMMON_HPP_ */
