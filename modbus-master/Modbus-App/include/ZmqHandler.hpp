/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef INCLUDE_INC_ZMQHANDLDER_HPP_
#define INCLUDE_INC_ZMQHANDLDER_HPP_

#include <string>
#include <map>
#include "cjson/cJSON.h"

#include <eis/utils/thread_safe_queue.h>
#include <eis/utils/config.h>
#include <eis/utils/json_config.h>
#include <eis/msgbus/msgbus.h>
#include <eis/config_manager/env_config.h>
#include <eis/config_manager/config_manager.h>

namespace zmq_handler 
{
	struct stZmqContext 
	{
		void *m_pContext;
	};
	struct stZmqPubContext
	{
		void *m_pContext;
	};
	struct stZmqSubContext
	{
		recv_ctx_t* sub_ctx;
	};

	bool prepareCommonContext(std::string topicType);
	//bool getContext(const std::string &a_sTopic, struct stZmqContext &a_oContext);

	/// function to get message bus context based on topic
	stZmqContext& getCTX(std::string str_Topic);

	/// function to insert new entry in map
	void insertCTX(std::string, stZmqContext );

	/// function to remove entry from the map once reply is sent
	void removeCTX(std::string);

	/// function to get message bus context based on topic
	stZmqSubContext& getSubCTX(std::string str_Topic);

	/// function to insert new entry in map
	void insertSubCTX(std::string, stZmqSubContext );

	/// function to remove entry from the map once reply is sent
	void removeSubCTX(std::string);

	/// function to get message bus publish context based on topic
	stZmqPubContext& getPubCTX(std::string str_Topic);

	/// function to insert new entry in map
	bool insertPubCTX(std::string, stZmqPubContext );

	/// function to remove entry from the map
	void removePubCTX(std::string);
}

#endif /* INCLUDE_INC_ZMQHANDLDER_HPP_ */
