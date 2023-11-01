SET(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSROOT /home/tez/STM32MPU_workspace/STM32MP1-Ecosystem-v5.0.0/Developer-Package/SDK/sysroots/cortexa7t2hf-neon-vfpv4-ostl-linux-gnueabi)

set(CROSSTOOL_ROOT /home/tez/STM32MPU_workspace/STM32MP1-Ecosystem-v5.0.0/Developer-Package/SDK/sysroots/x86_64-ostl_sdk-linux/usr/bin/arm-ostl-linux-gnueabi)
set(CMAKE_FIND_ROOT_PATH  ${CMAKE_SYSROOT} )
set(CMAKE_PREFIX_PATH ${CMAKE_SYSROOT} )

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

SET(CMAKE_C_COMPILER "${CROSSTOOL_ROOT}/arm-ostl-linux-gnueabi-gcc")
SET(CMAKE_CXX_COMPILER "${CROSSTOOL_ROOT}/bin/arm-linux-gnueabihf-g++")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath-link,/home/kyb/raspbian/sysroot/lib/arm-linux-gnueabihf")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath-link,/home/kyb/raspbian/sysroot/usr/lib/arm-linux-gnueabihf")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath-link,/home/kyb/raspbian/sysroot/opt/vc/lib")
# SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address" )

set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} --sysroot=\"${CMAKE_SYSROOT}\"")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --sysroot=\"${CMAKE_SYSROOT}\"")
SET(CMAKE_PREFIX_PATH /home/kyb/raspbian/sysroot)

set(PAHO_MQTT_C_LIBRARIES /home/kyb/raspbian/sysroot/usr/local/lib/libpaho-mqtt3a.so )

option(BUILD_ARM "Build arm" TRUE)

