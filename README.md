The directory comprises of 2 repos:
* modbus-master:
  This repo hosts the modbus container sources and docker file for building the container. 
* docker-compose.yml :
  This file will deploy the modbus container with EIS.

# Install pre-requisites
1. Follow #Docker Pre-requisites section mentioned in "README.md" file present in IEdgeInsights-v2.0.
2. Copy "docker-compose.yml" file in "</EISv2.0PV/Release-Package/EdgeInsightsSoftware-v2.0/IEdgeInsights/IEdgeInsights-v2.0>/docker_setup" directory in EIS.
3. Copy "modbus-master" directory in </EISv2.0PV/Release-Package/EdgeInsightsSoftware-v2.0/IEdgeInsights/IEdgeInsights-v2.0> directory.
4. Create folder named "UWC_Config" under"</EISv2.0PV/Release-Package/EdgeInsightsSoftware-v2.0/IEdgeInsights/IEdgeInsights-v2.0>/docker_setup/config/>" 
5. Copy YAML files from modbus-master/Modbus-App/config folder (i.e. https://github.impcloud.net/uwc/UWC-Core/tree/UWC-Sprint1/modbus-master/Modbus-App/Config) under created "UWC_Config" folder
# Build and Run Modbus TCP container
1. Follow steps metioned from #EIS Pre-requisites till end to provision and deploy.
