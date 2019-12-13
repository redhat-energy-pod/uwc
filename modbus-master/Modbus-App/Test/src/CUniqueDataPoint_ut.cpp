/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#include "../include/CUniqueDataPoint_ut.h"


void CUniqueDataPoint_ut::SetUp()
{
	// Setup code
}

void CUniqueDataPoint_ut::TearDown()
{
	// TearDown code
}

/******************************CUniqueDataPoint::getID*****************************************/

/* TC001
Test: Return of CUniqueDataPoint::getID.
CWellSiteDevInfo::getID should return the string passed during constructor call.
  string passed here is "Test_String".
 */
TEST_F(CUniqueDataPoint_ut, getID_return) {

	/* Calling constructor of class CUniqueDataPoint with string "Test_String" */
	network_info::CUniqueDataPoint 		CUniqueDataPoint_obj
	{
		"Test_String",
		CWellSiteInfo_obj,
		CWellSiteDevInfo_obj,
		CDataPoint_obj
	};

	EXPECT_EQ("Test_String", CUniqueDataPoint_obj.getID());

}

/******************************CUniqueDataPoint::getID*****************************************/

/* TC002
Test: Return of CUniqueDataPoint::getWellSite.
CWellSiteDevInfo::getWellSite should return the object of class CWellSiteInfo,
  initialized with the value passed while calling constructor
 */
TEST_F(CUniqueDataPoint_ut, getWellSite_return) {
	std::string path("/Device_Config/PL0.yml");
	const char *cEtcdValue  = CfgManager::Instance().getETCDValuebyKey(path.c_str());
	std::string sYamlStr(cEtcdValue);
	YAML::Node baseNode = CommonUtils::loadFromETCD(sYamlStr);
	try
	{

		/* Populating object1 of class CWellSiteInfo */
		CWellSiteInfo_obj.build(baseNode, CWellSiteInfo_obj);

		/* Calling constructor of class CUniqueDataPoint with object1 of class CWellSiteInfo */
		network_info::CUniqueDataPoint 		CUniqueDataPoint_obj
		{
			"Test_String",
			CWellSiteInfo_obj,
			CWellSiteDevInfo_obj,
			CDataPoint_obj
		};

		/* Copying object1 of class CWellSiteInfo to object2  */
		CWellSiteInfo_obj_CUniqueDataPoint = CUniqueDataPoint_obj.getWellSite();
		/* Checking weather the values of both the objects are same */
		EXPECT_EQ( CWellSiteInfo_obj.getID(), CWellSiteInfo_obj_CUniqueDataPoint.getID() );
	}

	catch( YAML::Exception &e)
	{

		//EXPECT_EQ(1, 2);	//Test fails
		EXPECT_EQ("name key not found", (string)e.what());
	}
}


/******************************CUniqueDataPoint::getWellSiteDev*****************************************/

/* TC003
Test: Return of CUniqueDataPoint::getWellSiteDev.
CWellSiteDevInfo::getWellSite should return the object of class CWellSiteDevInfo,
  initialized with the value passed while calling constructor
 */
TEST_F(CUniqueDataPoint_ut, getWellSiteDev_return) {
	std::string path("/Device_Config/PL0.yml");
	const char *cEtcdValue  = CfgManager::Instance().getETCDValuebyKey(path.c_str());
	std::string sYamlStr(cEtcdValue);
	YAML::Node baseNode = CommonUtils::loadFromETCD(sYamlStr);
	try
	{

		/* Populating object1 of class CWellSiteDevInfo */
		for( auto test : baseNode)
		{
			if(test.second.IsSequence() && test.first.as<std::string>() == "devicelist")
			{

				const YAML::Node& list = test.second;
				for( auto nodes : list )
				{
					CWellSiteDevInfo_obj.build(nodes, CWellSiteDevInfo_obj);
				}
			}
		}
		/*		**										*/

		/* Calling constructor of class CUniqueDataPoint with object1 of class CWellSiteDevInfo */
		network_info::CUniqueDataPoint 		CUniqueDataPoint_obj
		{
			"Test_String",
			CWellSiteInfo_obj,
			CWellSiteDevInfo_obj,
			CDataPoint_obj
		};

		/* Copying object1 of class CWellSiteDevInfo to object2  */
		CWellSiteDevInfo_obj_CUniqueDataPoint = CUniqueDataPoint_obj.getWellSiteDev();
		/* Checking weather the values of both the objects are same */
		EXPECT_EQ( CWellSiteDevInfo_obj.getID(), CWellSiteDevInfo_obj_CUniqueDataPoint.getID() );
	}

	catch( YAML::Exception &e)
	{
		PrintError(e.what());
		//EXPECT_EQ(1, 2);	//Test fails

		EXPECT_EQ("name key not found",(string)e.what());


	}

}


/******************************CUniqueDataPoint::getDataPoint*****************************************/

/* TC004
Test: Return of CUniqueDataPoint::getDataPoint.
CWellSiteDevInfo::getDataPoint should return the object of class CDataPoint,
  initialized with the value passed while calling constructor
 */
TEST_F(CUniqueDataPoint_ut, getDataPoint_return) {

	try
	{
		/* Populating object1 of class CDataPoint */
		baseNode = CommonUtils::loadYamlFile("iou_datapoints.yml");
		for( auto test : baseNode)
		{
			if(test.second.IsSequence() && test.first.as<std::string>() == "datapoints")
			{
				const YAML::Node& points =  test.second;
				for( auto nodes : points )
				{
					CDataPoint_obj.build(nodes, CDataPoint_obj);

					/* Calling constructor of class CUniqueDataPoint with object1 of class CDataPoint */
					network_info::CUniqueDataPoint 		CUniqueDataPoint_obj
					{
						"Test_String",
						CWellSiteInfo_obj,
						CWellSiteDevInfo_obj,
						CDataPoint_obj
					};

					/* Copying object1 of class CDataPoint to object2  */
					CDataPoint_obj_CUniqueDataPoint = CUniqueDataPoint_obj.getDataPoint();
					/* Checking weather the values of both the objects are same */
					EXPECT_EQ( CDataPoint_obj.getID(), CDataPoint_obj_CUniqueDataPoint.getID() );
				}
			}
		}
	}

	catch( YAML::Exception &e)
	{
		PrintError(e.what());
		EXPECT_EQ(1, 2);	//Test fails
	}

}
