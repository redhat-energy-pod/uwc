# Universal Wellpad Controller (UWC)

```
********************************************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 **********************************************************************************************************************
```
## Directory details
The directory comprises of following:
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/modbus-master">modbus-master</a> :
  This directory contains the modbus container sources and docker file for building the container. 
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/mqtt-export">mqtt-export</a> :
  This directory contains the mqtt-export container sources and docker file for building the container. 
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/MQTT">MQTT</a> :
  This directory contains the mqtt container sources and docker file for building the container.
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/masterOthers">Others</a> :
  This directory contains configurations for ETCD required during provisioning. 
* <a href="https://github.impcloud.net/uwc/UWC-Core/blob/master/docker-compose.yml">docker-compose.yml</a> :
  This file will deploy the modbus container with EIS in IPC with DEV mode.
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/docker-compose_IPC_DEV.yml">docker-compose_IPC_DEV.yml</a> :
  This file is used to deploy UWC containers in IPC with DEV mode.
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/docker-compose_TCP_DEV.yml">docker-compose_TCP_DEV.yml</a> :
  This file is used to deploy UWC containers in TCP with DEV mode.
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/docker-compose_TCP_PROD.yml">docker-compose_TCP_PROD.yml</a> :
  This file is used to deploy UWC containers in TCP with PROD mode.
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/docker-compose_unit_test.yml">docker-compose_unit_test.yml</a> :
  This file is used to run unit test cases
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/Release">Release</a> :
  This directory contains UWC bundle for deployement.
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/create_uwc_package.sh">create_uwc_package.sh</a> :
  This script will be used to create UWC package

## Install pre-requisites
```
1. EdgeInsightsSoftware-v2.1-PV version of EIS should be available on deploy machine before deployment. 
2. Copy files from "Release" diectory (e.g. all shell scripts, tar.gz file, etc.) into "EdgeInsightsSoftware-v2.1-PV/IEdgeInsights" directory. Please ensure that shell scripts have "execute" permission (sudo chmod +x <script name>).
3. Open a terminal and go to EdgeInsightsSoftware-v2.1-PV/IEdgeInsights directory.
4. Uninstall previously deployed and running UWC containers, using "sudo ./04_uninstall_EIS.sh" command on terminal.
5. Run below command on terminal to install all pre-requisites required to deploy UWC containers.
sudo ./01_pre-requisites.sh
```

## Provision EIS
```
Execute below command on terminal for provisioning EIS.
sudo ./02_provisionEIS.sh
```

## Build and Run all UWC containers
```
Execute below command on terminal for container deployment.
sudo ./03_DeployEIS.sh
```

## Verify container status
```
Execute below command on terminal to verify container status.
sudo docker ps
```

## Steps to run unit test cases
```
1. Installed all the pre-requisites mentioned in ##Install pre-requisites section
2. Run 06_UnitTestRun.sh script using "sudo ./06_UnitTestRun.sh" command to run unit test cases.
3. Check the reports in <EIS>/docker_setup/unit_test_reports directory.
```

## Debugging steps
```
1. Checking container logs 
   Syntax - sudo docker logs <container_name>
   E.g. To check modbus-tcp-container logs execute "sudo docker logs modbus-tcp-container" command.
2. Command to check logs inside the container "sudo docker exec -it <container_name> bash"
3. Use "cat <log_file_name>" to see log file inside the container
4. Copying logs from container to host machine
   Syntax - docker cp <container_name>:<file to copy from container> <file to be copied i.e. host directory>
5. To check the IP address of machine, use "ifconfig" command.
6. For Modbus RTU, to check attached COM port for serial communication, use "dmesg | grep tty" command.

## Redirect docker logs to file including errors
docker logs modbus-tcp-container > docker.log 2>&1
```

## ETCD UI access (*Note : This is not required since we are not storing any configuration data for UWC containers in ETCD)
1. ETCD UI is available on `http://localhost:7070/etcdkeeper/` URL. (username - root , password- eis123)

## Steps to apply new configuration (i.e. YML files or docker-compose.yml)
  Once YML files/docker-compose.yml are changed/Modified in /opt/intel/eis/uwc_data directory then execute following command to apply new configurations,
 ```
  sudo ./05_applyConfigChanges.sh
```

## Steps to create bundle out of sources - Optional 
```
Execute below script to create UWC bundle for deployment.
sudo ./create_uwc_package.sh
```

## How to bring up/down UWC containers
```
cd EIS..../docker_setup
export PWD=$(pwd)
docker-compose down  - bring down all containers
docker-compose up - bring up all containers

## Notes
*  If docker-compose.yml is modified then execute 03_DeployEIS.sh script for build and deployment of UWC containers.
*  If previous containers are running on deploy machine, then stop those containers using 04_uninstall_EIS.sh script.

