#include "equipment/dlp/DlpEquipment.hpp"

#include <iostream>
#include <thread>
#include <chrono>

#include "rapidjson/pointer.h"
#include "rapidjson/document.h"

#include "eq-common/Dimensions.hpp"

using namespace strateam::equipment;
using namespace std::literals::chrono_literals;
namespace rj = rapidjson;


constexpr const char* DefaultCfg = R"||(
{
    "equipment": {
            "controlBoard" : {
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
        "clientId" : "mqtt-cb-equipment"
    }
}
)||";

rj::Document DocumentFromString( const char* str ){
	rj::Document doc;
	doc.Parse<kParseRelaxedFLAGS>(str);
	assert( !doc.HasParseError()  &&  __func__ );
	return doc;
}


int main(){
    rapidjson::Document doc = DocumentFromString( DefaultCfg );
    boost::asio::io_context ctx;
    std::shared_ptr<boost::asio::io_context::work> w = std::make_shared< boost::asio::io_context::work >( ctx );
    strateam::equipment::dlp::DlpEquipment eq( ctx, doc );
    
    std::thread thrd( [&ctx](){
        try{
            ctx.run();
        }catch( std::exception const& ex ){
            std::cout  << "io err: " << ex.what() << std::endl;
        }
    } );
 
    try{
            auto& motor1 = eq.axis( AxisType::X );
            auto rval = motor1.move( strateam::dim::MotorStep( -5000 ), 2000, 1600, 1600 );
            assert( rval );
            std::cout << "motor1 resp: "  << rval.str() << std::endl;
            std::this_thread::sleep_for( 6s );

            // auto& motor2 = eq.axis( AxisType::Y );
            // rval = motor2.move( strateam::dim::MotorStep( -5000 ), 1000, 1600, 1600 );
            // assert( rval );
            // std::cout << "motor2 resp: "  << rval.str() << std::endl;
            // std::this_thread::sleep_for( 6s );

            // auto& motor3 = eq.axis( AxisType::Z );
            // rval = motor3.move( strateam::dim::MotorStep( -5000 ), 1000, 1600, 1600 );
            // assert( rval );
            // std::cout << "motor3 resp: "  << rval.str() << std::endl;
            // std::this_thread::sleep_for( 6s );

            std::cout << "shutdown" << std::endl;
            w.reset();
    }catch( std::exception const& ex ){
        std::cout << "move err: " << ex.what() << std::endl;
    }

    ctx.stop();
    thrd.join();
    
    return 0;
}