/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

#ifndef INCLUDE_COMMON_HPP_
#define INCLUDE_COMMON_HPP_

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <list>

using namespace std;

class CCommon {
private:
	// Private constructor so that no objects can be created.
	CCommon();
	CCommon(const CCommon & obj){}
	CCommon& operator=(CCommon const&);

	std::string m_strReadRequest;
	std::string m_strRTReadRequest;
	std::string m_strWriteRequest;
	std::string m_strRTWriteRequest;
	std::string m_strAppName;
	std::string m_strMqttExportURL;

	bool m_devMode;

public:
	virtual ~CCommon();

	bool readCommonEnvVariables();
	bool readEnvVariable(const char *pEnvVarName, std::string &storeVal);

	bool addTimestampsToMsg(std::string &a_sMsg, string tsKey, string strTimestamp);
	void getCurrentTimestampsInString(std::string &a_sMsg);

	/**
	 * Get read request topic
	 * @return topic name in string
	 */
	const std::string& getStrReadRequest() const {
		return m_strReadRequest;
	}

	/**
	 * Set read request topic
	 * @param strReadRequest
	 */
	void setStrReadRequest(const std::string &strReadRequest) {
		m_strReadRequest = strReadRequest;
	}

	/**
	 * Get write request topic
	 * @return topic in string format
	 */
	const std::string& getStrWriteRequest() const {
		return m_strWriteRequest;
	}

	/**
	 * Set write request topic
	 * @param strWriteRequest
	 */
	void setStrWriteRequest(const std::string &strWriteRequest) {
		m_strWriteRequest = strWriteRequest;
	}

	/**
	 * Get RT write request topic
	 * @return RT write request topic in string
	 */
	const std::string& getStrRTWriteRequest() const {
		return m_strRTWriteRequest;
	}

	/**
	 * Set RT write request topic
	 * @param strWriteRequest
	 */
	void setStrRTWriteRequest(const std::string &strWriteRequest) {
		m_strRTWriteRequest = strWriteRequest;
	}

	/**
	 * Get RT read request topic
	 * @return RT read request topic in string
	 */
	const std::string& getStrRTReadRequest() const {
		return m_strRTReadRequest;
	}

	/**
	 * Set RT read request topic
	 * @param strReadRequest
	 */
	void setStrRTReadRequest(const std::string &strReadRequest) {
		m_strRTReadRequest = strReadRequest;
	}

	/**
	 * Set application name
	 * @param strAppName
	 */
	void setStrAppName(const std::string &strAppName) {
		m_strAppName = strAppName;
	}

	/**
	 * Get application name
	 * @return application name in string
	 */
	const std::string& getStrAppName() const {
		return m_strAppName;
	}

	/**
	 * Set MQTT Export URL to connect with MQTT broker
	 * @param strMqttExportURL
	 */
	void setStrMqttExportURL(const std::string &strMqttExportURL) {
		m_strMqttExportURL = strMqttExportURL;
	}

	/**
	 * Get MQTT-Export broker connection URL
	 * @return connection URL in string
	 */
	const std::string& getStrMqttExportURL() const {
		return m_strMqttExportURL;
	}

	/**
	 * Get single instance of this class
	 * @return
	 */
	static CCommon& getInstance() {
		static CCommon _self;
			return _self;
	}

	/**
	 * Check if application set for dev mode or not
	 * @return true if set to devMode
	 * 			false if not set to devMode
	 */
	bool isDevMode() const {
		return m_devMode;
	}

	/**
	 * Set dev mode
	 * @param devMode
	 */
	void setDevMode(bool devMode) {
		m_devMode = devMode;
	}
};


#endif
