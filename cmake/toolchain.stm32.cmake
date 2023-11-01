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
SET(CMAKE_CXX_COMPILER "${CROSSTOOL_ROOT}/arm-ostl-linux-gnueabi-g++")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mthumb -mfpu=neon-vfpv4 -mfloat-abi=hard -mcpu=cortex-a7" )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mthumb -mfpu=neon-vfpv4 -mfloat-abi=hard -mcpu=cortex-a7")


