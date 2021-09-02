#!/bin/bash
# Copyright (c) 2021 Intel Corporation.

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

working_dir=$(pwd)
RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
MAGENTA=$(tput setaf 5)
NC=$(tput sgr0)
BOLD=$(tput bold)
INFO=$(tput setaf 3)   # YELLOW (used for informative messages)

# ----------------------------
# Create eiiuser
# ----------------------------
getent passwd $1 > /dev/null 2&>1
if [ $? -eq 0 ]; then
    echo "${GREEN} eiiuser already exists"
else
    echo "${Red} eiiuser does not exist creating now, you will be prompted for your password"
	sudo useradd eiiuser
fi

# ----------------------------
# Creating docker volume dir to store yaml files
# ----------------------------
create_docker_volume_dir()
{
    if [ ! -d /home/eiiuser/eii/uwc_data ]; then
    	echo "${GREEN}uwc_data directory is not present in /home/eiiuser/eii/ directory.${NC}"
    	echo "${GREEN}Creating /home/eiiuser/eii/uwc_data directory.${NC}"
    	mkdir -p /home/eiiuser/eii/uwc_data
		if [ "$?" -eq "0" ]; then
			echo "${GREEN}/home/eiiuser/eii/uwc_data is sucessfully created. ${NC}"
		else
        	echo "${RED}Failed to create docker volume directory${NC}"
			exit 1;
		fi
	
		rm -rf /home/eiiuser/eii/uwc_data/sparkplug-bridge
    	mkdir -p /home/eiiuser/eii/uwc_data/sparkplug-bridge
		if [ "$?" -eq "0" ]; then
			echo "${GREEN}/home/eiiuser/eii/uwc_data/sparkplug-bridge is sucessfully created. ${NC}"
		else
        	echo "${RED}Failed to create docker volume directory${NC}"
			exit 1;
		fi
    fi
	echo "${GREEN}Deleting old /home/eiiuser/eii/container_logs directory.${NC}"
	rm -rf  /home/eiiuser/eii/container_logs
	echo "${GREEN}Done..${NC}"
	echo "${GREEN}Creating /home/eiiuser/eii/container_logs directory.${NC}"
	mkdir -p /home/eiiuser/eii/container_logs/modbus-tcp-master
	mkdir -p /home/eiiuser/eii/container_logs/modbus-rtu-master
	mkdir -p /home/eiiuser/eii/container_logs/mqtt-bridge
	mkdir -p /home/eiiuser/eii/container_logs/sparkplug-bridge
    mkdir -p /home/eiiuser/eii/container_logs/kpi-tactic
	if [ "$?" -eq "0" ]; then
		echo "${GREEN}/home/eiiuser/eii/container_logs is sucessfully created. ${NC}"
	else
		echo "${RED}Failed to create docker volume directory${NC}"
		exit 1;
	fi
    if [ ! -d /home/eiiuser/eii/uwc_data/common_config ]; then
    	echo "${GREEN}common_config directory is not present in /home/eiiuser/eii/ directory.${NC}"
    	echo "${GREEN}Creating /home/eiiuser/eii/uwc_data/common_config directory.${NC}"
    	mkdir -p /home/eiiuser/eii/uwc_data/common_config
		if [ "$?" -eq "0" ]; then
			echo "${GREEN}/home/eiiuser/eii/uwc_data/common_config is sucessfully created. ${NC}"
		else
        	echo "${RED}Failed to create docker volume directory${NC}"
			exit 1;
		fi
    fi
}

# -------------------------------------------
# Coping UWC configurations to /home/eiiuser/eii/ 
# -------------------------------------------

add_UWC_containers_In_EII()
{
    echo "${INFO}Copying UWC Containers in EII...${NC}"   
    cp -r ../Others/Config/UWC/Device_Config/* /home/eiiuser/eii/uwc_data
    cp ../Others/Config/UWC/Global_Config.yml /home/eiiuser/eii/uwc_data/common_config/Global_Config.yml
    copy_verification=$(echo $?)
    if [ "$copy_verification" -eq "0" ]; then
        echo "${GREEN}UWC containers are successfully copied ${NC}"
    else
        echo "${RED}failed to copy UWC containers.${NC}"
	    return 1
    fi
    return 0
}

# ----------------------------------------
# Updating config files according to uwc 
# ----------------------------------------

modify_config()
{
	echo "Modifying the config file"
	search_dir="${working_dir}/../eii_configs/"
	for file_path in "$search_dir"/*
    do
     dir=$(basename "$file_path" .json)
	 if [ -d "${working_dir}/../../${dir}" ]; then
	     cp ${file_path} ${working_dir}/../../${dir}/config.json
		 echo "${GREEN}Done copying from ${file_path} to ${working_dir}/../../${dir}${NC}"
	 else
	     echo "${RED}${working_dir}/../../${dir} doesn't exists, Please check ${working_dir}/../eii_configs for valid config files${NC}"	 
	 fi	
    done
}

# ----------------------------
# clonning modbus-stack modconn
# ----------------------------

clone_modconn()
{

	if [ -d "${working_dir}/../modbus-master/modconn/" ]; then
		echo "modconn directory already exists. Removing the existing modconn directory."
		rm -rf ${working_dir}/../modbus-master/modconn/
	fi
	echo "${GREEN}Git clone modconn into modbus-master .."
	git clone https://github.com/modconn/modconn ${working_dir}/../modbus-master/modconn
	echo "${GREEN}Done cloning"
	cd ${working_dir}/../modbus-master/modconn/
	git checkout b574b6460b00b42dba191f64dfbd7334314faef1
	echo "${GREEN}Git checkout modconn to commit id b574b6460b00b42dba191f64dfbd7334314faef1"
}

# ----------------------------
# Checking for ambiguous networks
# ----------------------------

network_setup()
{
  echo "${INFO}Checking for ambiguous networks${NC}"	
  cmd_output=`docker network ls | grep edgeinsightssoftware_default`
  length=${#cmd_output[@]}
  while [ ${length} -ne 1 ];
  do
      echo "${length}"
      cmd_output=`docker network rm edgeinsightssoftware_default`
      length=$((length-1))
  done
}
echo "${GREEN}============================= Script START ============================================${NC}"

create_docker_volume_dir
add_UWC_containers_In_EII
modify_config
clone_modconn
network_setup
