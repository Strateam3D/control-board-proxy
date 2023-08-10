#include "ControlBoardTU.hpp"
#include "equipment/makeEquipment.hpp"
#include "Dialog.hpp"
#include "eq-common/rjapi/Settings.hpp"

//eq-common
#include "eq-common/rjapi/Helper.hpp"

// mqtt
#include "mqttStack/MQTTStack.hpp"
#include "mqtt/message.h"

// rj

#include <rapidjson/document.h>
#include "rapidjson/error/en.h"
#include "rapidjson/pointer.h"
#include "rj/include/rapidjson/stringbuffer.h"
#include "rj/include/rapidjson/writer.h"
#include "rjapi/Helper.hpp"

namespace rj = rapidjson;


constexpr unsigned kParseRelaxedFLAGS =
		rj::kParseNanAndInfFlag 
		| rj::kParseCommentsFlag 
		| rj::kParseTrailingCommasFlag 
		| rj::kParseStopWhenDoneFlag;   // rj::kParseInsituFlag | rj::kParseNumbersAsStringsFlag |

static rj::Document DocumentFromString( const char* str );

constexpr const char* RegistryJsonStr = R"||(
{
	"equipment": {
        "axis" : {
            "x" : {
                "moveHome" : {
                    "spd" : 1000
                },
                "moveToZero": : {
                    "spd" : 1000
                },
                "move": {
                    "offset" : 1000,
                    "spd" : 1000
                },
                "stop": true,
                "position": 12345,
                "homePosition" : 123345,
                "isMoving" : true
            },
            "y" : {
                "moveHome": true,
                "moveToZero": true,
                "move": {
                    "offset" : 1000,
                    "spd" : 1000
                },
                "stop": true,
                "position": 12345,
                "homePosition" : 123345,
                "isMoving" : true
            },
            "z" : {
                "moveHome": true,
                "moveToZero": true,
                "move": {
                    "offset" : 1000,
                    "spd" : 1000
                },
                "stop": true,
                "position": 12345,
                "homePosition" : 123345,
                "isMoving" : true
            }
        }
    }
}
)||";

static rj::Document MasterNcdJsonDom = DocumentFromString(RegistryJsonStr);

rj::Document DocumentFromString( const char* str ){
	rj::Document doc;
	doc.Parse<kParseRelaxedFLAGS>(str);
	assert( ! MasterNcdJsonDom.HasParseError()  &&  __func__ );
	return doc;
}


namespace strateam::control_board{

ControlBoardTU::ControlBoardTU( MQTTStack& stack, equipment::EquipmentInterface&  equipment, std::shared_ptr<AbstractTU::IoCtx> ioPtr )
: AbstractTU( ioPtr )
, stack_( stack )
, equipment_( equipment )
{
}

void ControlBoardTU::dispatchIncoming( std::shared_ptr<strateam::MessageInterface>  msg ){
    msg->accept( *this );
}

void ControlBoardTU::shutdown(){
    AbstractTU::shutdown();
}

void ControlBoardTU::visit( ConnectedMessage & ){
    stack_.subscribe( RequestTopic(), 1 );
}

void ControlBoardTU::visit( ApplicationMessage& msg ){
    std::string const& dialogId = msg.msg_->get_topic();
    std::string const& payload = msg.msg_->to_string();
    
    try{
        auto doc = std::make_shared<rj::Document>();
        doc->Parse( payload.data() );

        if( doc->HasParseError() ){
            rapidjson::ParseErrorCode ec = doc->GetParseError();
            std::cout << "Parse error: " <<  rj::GetParseError_En( ec ) << std::endl;
            std::cout << payload << std::endl;
            static const std::string HelpResponse =
            R"||(
            "Usage syntax 1 named full json-style:
            '{"equipment": { "axis" : { "x" : { "move" : { "offset" : 1000 } } } } }'  # To move offset.
            '{"equipment": { "axis" : { "x" : { "position" : null } } } }'  # To get current position.
            )||";
            std::string rspTopic = dialogId + "/rsp";
            mqtt::message_ptr pubmsg = mqtt::make_message( rspTopic, HelpResponse );
            stack_.publish( pubmsg );
            return;
        }

        std::cout << "Incoming request:\n" << StringifyRjValue( *doc ).GetString() << std::endl;

        if(dialogs_.count( dialogId )){
            // it means we have long running motion event, anothe motion event will be rejected,
            // so no need to check
            dialogs_[ dialogId ]->dispatch( doc );
        }else{
            auto dialogPtr = std::make_shared< Dialog >( *this, equipment_, dialogId, MasterNcdJsonDom );
            
            if( dialogPtr->dispatch( doc ) == equipment::MotionResult::Accepted ){
                dialogs_[ dialogId ] = dialogPtr;
            }
        }
    }catch( std::exception const& ex ){
        std::cout << __func__ << ex.what() << std::endl;
    }
}

void ControlBoardTU::endDialog( std::string dialogId ){
    dialogs_.erase( dialogId );
}

void ControlBoardTU::publish( mqtt::const_message_ptr msg ){
    stack_.publish( msg );
}

}