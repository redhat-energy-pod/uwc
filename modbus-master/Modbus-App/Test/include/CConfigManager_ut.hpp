/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#ifndef TEST_INCLUDE_CCONFIGMANAGER_UT_HPP_
#define TEST_INCLUDE_CCONFIGMANAGER_UT_HPP_

#include <gtest/gtest.h>
#include <string.h>
#include "ConfigManager.hpp"

void etcdOnChangeKeyCb(char* key, char * val);
void etcdOnChangeDirCb(char* key, char * val);
//void CfgManager::registerCallbackOnChangeKey(char *key)


class CConfigManager_ut : public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();

public:

	std::string			Test_Str = "";
	std::string			Expected_output = "";

};

#endif /* TEST_INCLUDE_CCONFIGMANAGER_UT_HPP_ */