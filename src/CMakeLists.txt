add_compile_options(-Wall -Wextra )
set(cb_src)


include_directories( 
    ${CMAKE_SOURCE_DIR}  
    ${CMAKE_SOURCE_DIR}/src/eq-common 
    ${CMAKE_SOURCE_DIR}/3rdParty/paho_mqtt_cpp/src 
    ${CMAKE_SOURCE_DIR}/src 
    ${CMAKE_SOURCE_DIR}/3rdParty/rj/include 
    ${CMAKE_SOURCE_DIR}/3rdParty
)

list( APPEND cb_src
    main.cpp
    app/ControlBoardTU.cpp
    app/ControlBoardTU.hpp
    app/Dialog.cpp
    app/Dialog.hpp
    app/ControlBoardApp.cpp
    app/ControlBoardApp.hpp

    utils/StringUtils.hpp
)

set(CONTROL_BOARD_TARGET control_board)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_definitions( -DRAPIDJSON_HAS_STDSTRING=1 )

if( BUILD_ARM )

    # message(status "!!!!!!!!!!!!!!!!!!BUILD_ARM")
    find_package(Boost 1.58 REQUIRED)
    # include_directories(${Boost_INCLUDE_DIR})
    # link_directories(${Boost_LIBRARY_DIR})
    add_definitions(-DBUILD_ARM) 
else()
    find_package(Boost 1.71 REQUIRED)
endif()

if( BUILD_ARM )

    link_directories(/usr/lib/arm-linux-gnueabihf/)
    add_executable(${CONTROL_BOARD_TARGET} ${cb_src})
    target_link_libraries(control_board 
    PUBLIC 
        boost_system
        asan
        mqttStack
        rjapi
        boost_program_options
    )
else()
    add_executable(${CONTROL_BOARD_TARGET} ${cb_src})
    target_link_libraries(control_board 
    PUBLIC 
        paho-mqttpp3
        boost_system
        mqttStack
        rjapi
        boost_program_options
    )    
endif()

add_dependencies(${CONTROL_BOARD_TARGET}
    paho-mqttpp3
    mqttStack
)

add_subdirectory(eq-common)
add_subdirectory(equipment)
