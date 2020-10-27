####################################################################################
# The source code contained or described herein and all documents related to
# the source code ("Material") are owned by Intel Corporation. Title to the
# Material remains with Intel Corporation.
#
# No license under any patent, copyright, trade secret or other intellectual
# property right is granted to or conferred upon you by disclosure or delivery of
# the Materials, either expressly, by implication, inducement, estoppel or otherwise.
####################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Test/Src/CConfigManager_ut.cpp \
../Test/Src/Logger_ut.cpp \
../Test/Src/NetworkInfo_ut.cpp \
../Test/Src/ZmqHandler_ut.cpp 

OBJS += \
./Test/Src/CConfigManager_ut.o \
./Test/Src/Logger_ut.o \
./Test/Src/NetworkInfo_ut.o \
./Test/Src/ZmqHandler_ut.o 

CPP_DEPS += \
./Test/Src/CConfigManager_ut.d \
./Test/Src/Logger_ut.d \
./Test/Src/NetworkInfo_ut.d \
./Test/Src/ZmqHandler_ut.d 


# Each subdirectory must supply rules for building sources it contributes
Test/Src/%.o: ../Test/Src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++1z -DUNIT_TEST -I/usr/local/include -I../$(PROJECT_DIR)/include -O0 -g3 -ftest-coverage -fprofile-arcs -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


