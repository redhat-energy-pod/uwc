# Universal Wellpad Controller (UWC) 

```
* Copyright (c) 2021 Intel Corporation.

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:

* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
```

# Contents:
- [Universal Wellpad Controller (UWC)](#universal-wellpad-controller-uwc)
- [Contents:](#contents)
  - [Directory details](#directory-details)
  - [Install generic pre-requisites](#install-generic-pre-requisites)
  - [Install UWC specific pre-requisites](#install-uwc-specific-pre-requisites)
  - [Provision EII and UWC services](#provision-eii-and-uwc-services)
  - [Build and Run all EII and UWC services](#build-and-run-all-eii-and-uwc-services)
  - [Verify container status](#verify-container-status)
  - [Apply configuration changes](#apply-configuration-changes)
  - [Uninstallation script](#uninstallation-script)
  - [Unit Tests](#unit-tests)
  - [Debugging steps](#debugging-steps)
  - [Notes](#notes)
  - [Troubleshooting](#troubleshooting)

## Directory details
The directory comprises of following:
* modbus-master :
  This directory contains the modbus TCP1 and RTU1 containers sources and docker file for building the container. It also has the ingredient docker-compose.yml & config.json for modbus TCP & RTU services.
  For detail, please refer to `README-modbus-master.md` file of modbus-master folder
* mqtt-bridge :
  This directory contains the mqtt-bridge container sources and docker file for building the container. It also has the ingredient docker-compose.yml & config.json for mqtt-bridge service. 
  For detail, please refer to `README-mqtt-bridge.md` file of mqtt-bridge folder
* MQTT:
  This directory contains the mqtt container sources and docker file for building the container. It also has the ingredient docker-compose.yml & config.json for MQTT internal broker service.
  For detail, please refer to `README-MQTT.md` file of MQTT folder
* sparkplug-bridge:
  This directory contains the sparkplug-bridge sources and docker file for building the container. It also has the ingredient docker-compose.yml & config.json for SPARKPLUG-BRIDGE service.
  For detail, please refer to `README_sparkplug_bridge.md` file of sparkplug-bridge folder
* uwc-common:
  This directory contains sources for uwc common library container & Dockerfile to install all the dependencies and libraries needed by all the containers. For detail, please refer to `README-UWC_Common.md` file of uwc_common folder
* kpi-tactic:
  This directory contains the kpi-tactic container sources and docker file for building the container 
  For detail, please refer to `README-kpi-tactic.md` file of kpi-tactic folder
* Others:
  This directory contains configurations for ETCD required during provisioning
* uwc_common:
  This directory contains common dockefiles for UWC

## Install generic pre-requisites

1. Follow the steps in the section `EII-Prerequisites-Installation` of `<working-directory>/IEdgeInsights/README.md`  to install all the pre-requisites.

  ```sh
    $ <working-dir>/IEdgeInsights/build
    $ sudo ./pre_requisites.sh --help
  ```
     Note - If the error "Docker CE installation step is failed" is seen while running pre-requisite.sh script on a fresh system then kindly re-run the pre_requisite.sh script again.
            This is a known bug in docker community for Docker CE.

2. If the required UWC code base is not yet repo synched or (git repositories cloned), then kindly follow the repo or git steps from  `<working-directory>/IEdgeInsights/../.repo/manifests/README.md` to repo sync or git clone the codebase.

## Install UWC specific pre-requisites
All the below UWC specific scripts need to be run from the directory `IEdgeInsights\uwc`:
  ```sh
    $ cd <working-dir>/IEdgeInsights/uwc/build_scripts
    $ sudo ./01_uwc_pre_requisites.sh
  ```

## Provision EII and UWC services
Runs the builder script enabling to choose the UWC recipe needed for the use case. Next it prompts the user to select `develeopment mode` or `production mode` & prepares the set up to run in the selected mode. Finally it does the provisioning of EII & UWC services based on the recipe & mode selected. 
```sh
    $ cd <working-dir>/IEdgeInsights/uwc/build_scripts
    $ sudo ./02_provision_UWC.sh
```
Note: Above example will execute the script in interactive mode. 
The script will run in non-interactive mode when the command line arguments are provided. The help option describes all command line arguments.
```sh
    $ cd <working-dir>/IEdgeInsights/uwc/build_scripts
    $ sudo ./02_provision_UWC.sh --help
```

## Build and Run all EII and UWC services
Builds all the micro-services of the recipe & runs them as containers in background (daemon process).
```sh
  $ cd <working-dir>/IEdgeInsights/uwc/build_scripts
  $ sudo ./03_Build_Run_UWC.sh 
```

## Verify container status
```
Execute below command on terminal to verify container status.
sudo docker ps
```

## Apply configuration changes
If any configuration changes are done to UWC YML files, then run this script which will bring down the containers & bring them up back. Note that the services are not built here.
  ```sh
  $ cd <working-dir>/IEdgeInsights/uwc/build_scripts
  $ 05_applyConfigChanges.sh
  ```

## Uninstallation script
Used to uninstall & remove the UWC installation.
  ```sh
  $ cd <working-dir>/IEdgeInsights/uwc/build_scripts
  $ 04_uninstall_UWC.sh
  ```
## Unit Tests
All the UWC modules have unit tests enabled in production mode. In order to run the unit tests, follow the below steps:
```sh
$ cd <working-dir>/IEdgeInsights/uwc/build_scripts
$ sudo ./06_UnitTestRun.sh "false"
```
Now check for the unit test reports for all services of UWC in "<working-dir>/IEdgeInsights/build/unit_test_reports/".

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
7. The container logs for each of the UWC micro-service is volume mounted in the directory "/opt/intel/eii/container_logs/", with a separate sub-directory for each micro-service.
8. Also the UWC configuration files can be edited in the sub-directory "/opt/intel/eii/uwc_data", if there is a need to change the configurations.

## Redirect docker logs to file including errors
docker logs modbus-tcp-container > docker.log 2>&1
```

1. ETCD UI is available on `http://localhost:7070/etcdkeeper/` URL for dev mode & `https://localhost:7071/etcdkeeper/` for prod mode . (username - root , password- eii123)
2. EII Message Bus related interface configurations (including secrets) for all UWC & EII containers are stored in ia_etcd server, which can be accessed using EtcdUI as mentioned above.

## Notes
*  If docker-compose.yml is modified then bring down the containers & then bring them up as mentioned above.
*  If previous containers are running on deploy machine, then stop those containers using command for bringing down containers as mentioned above.
*  Can use MQTT.FX or any other MQTT client to verify teh flow or all 6 operations for RT/Non-RT, read/write,polling operations

## Troubleshooting
```
1. Follow Method 2 from here https://www.thegeekdiary.com/how-to-configure-docker-to-use-proxy/ to set proxy for docker.
2. In prod mode, the "Certificates" directory in "<working-dir>/IEdgeInsights/build/provision" needs 'sudo su" to be accessed. i.e. to open Certificates folder do the following:
```sh
  $ cd <working-dir>/IEdgeInsights/build/provision
  $ sudo su
  $ cd Certificates
  # After accessing Certificates, enter "exit" command & terminal would return back to normal mode.
  $ exit
```
3. Troubleshooting steps:
1. If KPI-Tactic Application is seen crashing, container restarting or AnalysisKPI.log files not getting generated after building the UWC containers then as a troubleshooting step kindly run the 05_applyConfigChanges.sh, which would bring the containers down & up.
