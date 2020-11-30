/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef QMGR_HPP_
#define QMGR_HPP_

#include <atomic>
#include <map>
#include <semaphore.h>
#include "mqtt/async_client.h"
#include "Common.hpp"
#include <queue>
#include <string>
#include "QueueHandler.hpp"

/**
 * namespace for Queue manager
 */
namespace QMgr
{

struct stMqttMsg
{
	std::string m_strSite;
	std::string m_strDevice;
	std::string m_strDataPoint;
	std::string m_mqttTopic;
	mqtt::const_message_ptr m_mqttMsg;
	std::map<std::string, std::string> m_datapoint_key_val;
	/**
	 * Set data point name on which to send message for SCADA
	 * @param a_sTopic :[in] data point name to set
	 * @return none
	 */
	void setDataPointName(std::string a_sDataPoint)
	{
		m_strDataPoint = a_sDataPoint;
	}

	/**
	 * Get data point name on which to send message for SCADA
	 * @param none
	 * @return data point name
	 */
	std::string setDataPointName()
	{
		return m_strDataPoint;
	}

	/**
	 * Set mqtt topic name on which mqtt has received message
	 * @param a_sMqttTopic :[in] mqtt topic name on which mqtt has received message
	 * @return none
	 */
	void setMqttTopic(std::string a_sMqttTopic)
	{
		m_mqttTopic = a_sMqttTopic;
	}

	/**
	 * Get mqtt topic name on which mqtt has received message
	 * @param none
	 * @return mqtt topic name on which mqtt has received message
	 */
	std::string getMqttTopic()
	{
		return m_mqttTopic;
	}

	/**
	 * Set mqtt-export message
	 * @param a_mqttMsg :[in] mqtt-export message
	 * @return none
	 */
	void setMqttExportMsg(mqtt::const_message_ptr a_mqttMsg)
	{
		m_mqttMsg = a_mqttMsg;
	}

	/**
	 * Get mqtt-export message
	 * @param none
	 * @return mqtt-export message
	 */
	mqtt::const_message_ptr& getMqttExportMsg()
	{
		return m_mqttMsg;
	}

	/**
	 * Insert keys and values received in MQTT payload in map
	 * @param key :[in] param name in payload
	 * @param value :[in] value of param in payload
	 * return none
	 */
	void insertParam(std::string key, std::string value)
	{
		m_datapoint_key_val.insert(std::pair<std::string, std::string> (key, value));
	}

	/**
	 * Get key-value pair received in MQTT payload in map depending on the index
	 * @param a_index :[in] key-value pair at index from the map
	 * @return pair of key and value
	 */
	std::pair<std::string, std::string> getParam(int a_index)
				{
		int i = 0;
		if( m_datapoint_key_val.size() > (size_t)a_index)
		{
			for( auto it = m_datapoint_key_val.begin(); it != m_datapoint_key_val.end(); ++it, ++i )
			{
				if(i == a_index)
				{
					return make_pair(it->first, it->second);
				}
			}
		}
		return std::make_pair("", "");
				}

	/**
	 * Get length of map formed with MQTT msg payload values
	 * @param none
	 * @return length of map
	 */
	int getLength()
	{
		return m_datapoint_key_val.size();
	}
};

//functions to get on-demand operation instances
CQueueHandler& getDatapointsQ();
CQueueHandler& getScadaSubQ();
}
#endif
