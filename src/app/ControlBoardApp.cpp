#include "ControlBoardApp.hpp"
#include "eq-common/mqttStack/MQTTStack.hpp"
#include "eq-common/rjapi/Settings.hpp"
#include "equipment/makeEquipment.hpp"
#include "ControlBoardTU.hpp"
#include "equipment/interface/EquipmentInterface.hpp"

#include <iostream>

constexpr const char* DefaultDocumentStr = R"||(
{
    "equipment": {
            "controlBoard" : {
                "name" : "dlp",
                "device" : "/dev/ttyACM0",
                "baudRate" : 115200
            },
            "axises" : [
                {
                    "name": "motor1",
                    "homePos": -50000
                },
                {
                    "name": "motor2",
                    "homePos": -50000
                },
                {
                    "name": "motor3",
                    "homePos": -50000
                }
            ]
    },
    "mqtt" : {
        "address" : "tcp://localhost:1883",
        "clientId" : "mqtt-control-board-equipment"
    }
}
)||";


namespace strateam::control_board{

ControlBoardApp::ControlBoardApp()
: ioCtx_( std::make_shared< boost::asio::io_context >() )
, work_( std::make_shared< boost::asio::io_context::work >( *ioCtx_ ) )
{
}

ControlBoardApp& ControlBoardApp::getInstance(){
    static ControlBoardApp instance;
    return instance;
}

void ControlBoardApp::init( std::string const& cfgPath ){   
    Settings::getInstance().load( cfgPath, DefaultDocumentStr );
    auto& settings = Settings::getInstance();
    stackPtr_ = std::make_unique< MQTTStack >( settings.getValue<std::string>( "/mqtt/address" ), settings.getValue<std::string>( "/mqtt/clientId" ) );
    rj::Value const& eqCfg = settings.getDoc()["equipment"];
    eqPtr_ = equipment::makeEquipment( *ioCtx_, eqCfg );
    tuPtr_ = std::make_unique< ControlBoardTU >( *stackPtr_, *eqPtr_, ioCtx_ );
    stackPtr_->addTU( tuPtr_.get() );
}


int ControlBoardApp::run(){
    stackPtr_->start();
    ioCtx_->run();
}

void ControlBoardApp::shutdown(){
    ioCtx_->stop();
    work_.reset();
    stackPtr_->removeTU( tuPtr_.get() );
    tuPtr_ -> shutdown();
    std::cout << "ControlBoardApp::shutdown" << std::endl;
}


}
