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
                    "homePos": 0,
                    "stepsPerUm" : 0.8,
                    "msPerInch"  : 20298,
                    "inverted" : false
                },
                {
                    "name": "motor2",
                    "homePos": 0,
                    "stepsPerUm" : 0.8,
                    "msPerInch"  : 20298,
                    "inverted" : false
                },
                {
                    "name": "motor3",
                    "homePos": 0,
                    "stepsPerUm" : 0.8,
                    "msPerInch"  : 20298,
                    "inverted" : false
                },
                {
                    "name": "h1",
                    "homePos": 0,
                    "stepsPerUm" : 0.8,
                    "msPerInch"  : 20298,
                    "inverted" : false
                },
                {
                    "name": "h2",
                    "homePos": 0,
                    "stepsPerUm" : 0.8,
                    "msPerInch"  : 20298,
                    "inverted" : false
                },
                {
                    "name": "beam",
                    "homePos": 0,
                    "stepsPerUm" : 0.8,
                    "msPerInch"  : 20298,
                    "inverted" : false
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
: ioCtx_( std::make_shared< IoCtx >() )
, work_( std::make_shared< IoWork >( *ioCtx_ ) )
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
    return 0;
}

void ControlBoardApp::shutdown(){
    ioCtx_->stop();
    work_.reset();
    stackPtr_->removeTU( tuPtr_.get() );
    tuPtr_ -> shutdown();
    std::cout << "ControlBoardApp::shutdown" << std::endl;
}


}
