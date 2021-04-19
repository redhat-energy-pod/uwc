#!/bin/bash
################################################################################
# The source code contained or described herein and all documents related to
# the source code ("Material") are owned by Intel Corporation. Title to the
# Material remains with Intel Corporation.
#
# No license under any patent, copyright, trade secret or other intellectual
# property right is granted to or conferred upon you by disclosure or delivery of
# the Materials, either expressly, by implication, inducement, estoppel or otherwise.
################################################################################

export LD_LIBRARY_PATH="/SPARKPLUG-BRIDGE/lib:/usr/local/lib"
env
./SPARKPLUG-BRIDGE-TEST > /reports/sparkplug-bridge/SPARKPLUGBRIDGE_status.log 2>&1

# Run GCovr command
gcovr --html -e "../Test" -e "../include/utils" -e "../include/tahu" -e "../include/yaml-cpp" -e "../src/utils" -e "../include/log4cpp/Appender.hh" -e "../include/ConfigManager.hpp" -e "../include/EnvironmentVarHandler.hpp" -e "../include/Logger.hpp" -e "../include/NetworkInfo.hpp" --exclude-throw-branches -o /reports/sparkplug-bridge/SPARKPLUG-BRIDGE_Report.html -r .. .