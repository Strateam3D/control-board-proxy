#include "Dialog.hpp"
#include "ControlBoardTU.hpp"
#include "equipment/interface/EquipmentInterface.hpp"
#include "equipment/interface/AxisInterface.hpp"
#include "interface/ControlBoardInterface.hpp"
#include "equipment/EqException.hpp"
#include "eq-common/rjapi/Settings.hpp"
#include "Symbols.hpp"

#include <rapidjson/document.h>
#include "rapidjson/PointedValueHandler.h"
#include "rapidjson/error/en.h"
#include "rapidjson/pointer.h"
#include "rj/include/rapidjson/stringbuffer.h"
#include "rj/include/rapidjson/writer.h"
#include "rjapi/Helper.hpp"
#include "eq-common/Converter.hpp"

#include "rjapi/RegistryHandler.hpp"

// mqtt
#include "mqttStack/MQTTStack.hpp"
#include "mqtt/message.h"

#include "Symbols.hpp"
#include "spdlog/spdlog.h"

namespace rj = rapidjson;

namespace strateam::control_board{

Dialog::Dialog( ControlBoardTU& tu, equipment::EquipmentInterface& eq, std::string dialogId, rapidjson::Document& masterDocument )
: tu_( tu )
, equipment_( eq )
, dialogId_( std::move( dialogId ) )
, gettersSetters_{
    NamedGetterSetterPair{
        rj::Pointer( "/z/offset" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v, rj::Value::AllocatorType& ) -> ResponseCode{
                try{
                    offset_ = v.GetInt();
                    return ResponseCode::Success;
                }catch( std::exception const& ex ){
                    std::cout << "invalid offset: " << ex.what() << std::endl;
                    return ResponseCode::Failed;
                }
            }
        )
    }
    , NamedGetterSetterPair{
        rj::Pointer( "/z/targspd" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v, rj::Value::AllocatorType& ) -> ResponseCode{
                try{
                    targSpd_ = v.GetInt();
                    return ResponseCode::Success;
                }catch( std::exception const& ex ){
                    std::cout << "invalid spd: " << ex.what() << std::endl;
                    return ResponseCode::Failed;
                }
            }
        )
    }
   , NamedGetterSetterPair{
        rj::Pointer( "/z/go" ),
        GetterSetter(
            /*get*/[this]() -> rj::Value{
                return rj::Value( equipment_.axis( equipment::AxisType::Z ).isMoving() );
            },
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    if( v.GetBool() ){
                        dim::UmVelocity  spd( targSpd_ );
                        dim::Um  offset( offset_ );
                        auto& axis = equipment_.axis( equipment::AxisType::Z );
                        motret_ = axis.move( offset , spd );
                        
                        if ( motret_ == equipment::MotionResult::Accepted ){
                            axis_ = "z";
                            std::cout << motret_.str() << std::endl;
                            // rj::Value rsp( int(ResponseCode::Accepted) );
                            // v = rsp;
                            return ResponseCode::Accepted;
                        } else {
                            throw equipment::Exception( motret_.str() );
                        }
                    }else{
                        auto& axis = equipment_.axis( equipment::AxisType::Z );
                        axis.stop();
                        return ResponseCode::Success;
                    }
                }catch( std::exception const& ex ){
                    std::cout << "move ex: " << ex.what() << std::endl;
                    return ResponseCode::Failed;
                }
            }
        )
    }
    , NamedGetterSetterPair{
        rj::Pointer( "/z/goZero" ),
        GetterSetter(
            /*get*/[this]() -> rj::Value{
                return rj::Value( equipment_.axis( equipment::AxisType::Z ).isMoving() );
            },
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    if( v.GetBool() ){
                        dim::UmVelocity  spd( targSpd_ );
                        dim::Um  offset( offset_ );
                        auto& axis = equipment_.axis( equipment::AxisType::Z );
                        motret_ = axis.moveZero( spd );
                        
                        if ( motret_ == equipment::MotionResult::Accepted ){
                            axis_ = "z";
                            std::cout << motret_.str() << std::endl;
                            // rj::Value rsp( int(ResponseCode::Accepted) );
                            // v = rsp;
                            return ResponseCode::Accepted;
                        } else {
                            throw equipment::Exception( motret_.str() );
                        }
                    }else{
                        auto& axis = equipment_.axis( equipment::AxisType::Z );
                        axis.stop();
                        return ResponseCode::Success;    
                    }
                }catch( std::exception const& ex ){
                    std::cout << "move ex: " << ex.what() << std::endl;
                    return ResponseCode::Failed;
                }
            }
        )
    }
    , NamedGetterSetterPair{
        rj::Pointer( "/z/goHome" ),
        GetterSetter(
            /*get*/[this]() -> rj::Value{
                return rj::Value( equipment_.axis( equipment::AxisType::Z ).isMoving() );
            },
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    if( v.GetBool() ){
                        auto& axis = equipment_.axis( equipment::AxisType::Z );
                        dim::UmVelocity  spd( targSpd_ );
                        motret_ = axis.moveHome( spd );
                        
                        if ( motret_ == equipment::MotionResult::Accepted ){
                            axis_ = "z";
                            std::cout << motret_.str() << std::endl;
                            // rj::Value rsp( int(ResponseCode::Accepted) );
                            // v = rsp;
                            return ResponseCode::Accepted;
                        } else {
                            throw equipment::Exception( motret_.str() );
                        }
                    }else{
                        auto& axis = equipment_.axis( equipment::AxisType::Z );
                        axis.stop();
                        return ResponseCode::Success;    
                    }
                }catch( std::exception const& ex ){
                    std::cout << "move ex: " << ex.what() << std::endl;
                    return ResponseCode::Failed;
                }
            }
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/z/stop" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& ) -> ResponseCode{
                try{
                    auto& axis = equipment_.axis( equipment::AxisType::Z );
                    axis.stop();
                    return ResponseCode::Success;
                }catch( std::exception const& ex ){
                    return ResponseCode::Failed;
                }
            }
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/z/pos" ),
        GetterSetter(
            /*get*/[this]() -> rj::Value{
                return rj::Value( equipment_.axis( equipment::AxisType::Z ).position().castTo<int>() );
            },
            /*set*/nullptr
        )
    }
}
, masterDocument_( masterDocument ){
    equipment_.axis( equipment::AxisType::Z ).subscribe( this );
    spdlog::get( Symbols::Console() )->debug( "Dialog::Dialog" );
}

Dialog::~Dialog(){
    equipment_.axis( equipment::AxisType::Z ).unsubscribe( this );
    spdlog::get( Symbols::Console() )->debug( "Dialog::~Dialog" );
}

equipment::MotionResult Dialog::dispatch( DocumentPtr docPtr ){
    std::shared_ptr<rapidjson::Document> response( std::make_shared<rj::Document>() );
    RegistryHandler registryHandler( masterDocument_, *response, *docPtr, gettersSetters_ );
    rj::KeyValueSaxHandler<RegistryHandler> saxHandler( registryHandler );
    docPtr->Accept( saxHandler );
    std::string rspTopic = dialogId_ + "/rsp";
    rj::StringBuffer buffer = StringifyRjValue( *response );
    
    spdlog::get( Symbols::Console() )->debug( "Outgoing response:\n{}\nbody: {}", rspTopic, buffer.GetString() );
    mqtt::message_ptr rsp = mqtt::make_message( rspTopic, buffer.GetString() );
    tu_.publish( rsp );
    return motret_;
}

void Dialog::motionDone( equipment::MotionResult motret ){
    std::string responseTopic = dialogId_ + "/notify";

    rj::Document doc;
    rj::SetValueByPointer(doc, rj::Pointer( "/" + axis_ + "/go" ), motret.success() );
    rj::StringBuffer buffer = StringifyRjValue( doc );
    spdlog::get( Symbols::Console() )->debug( "{} {}", __func__, buffer.GetString() );
    mqtt::message_ptr rsp = mqtt::make_message( responseTopic, buffer.GetString() );
    tu_.publish( rsp );
    tu_.endDialog( dialogId_ );
}

void Dialog::motionToDone( equipment::MotionResult motret ){
    // std::string responseTopic = dialogId_ + "/notify";

    // rj::Document doc;
    // rj::SetValueByPointer(doc, rj::Pointer( "/equipment/axis/" + axis_ + "/moveTo" ), motret.success() );
    // rj::StringBuffer buffer = StringifyRjValue( doc );
    // spdlog::get( Symbols::Console() )->debug( "{} {}", __func__, buffer.GetString() );
    // mqtt::message_ptr rsp = mqtt::make_message( responseTopic, buffer.GetString() );
    // tu_.publish( rsp );
    // tu_.endDialog( dialogId_ );
}

void Dialog::moveHomeDone( equipment::MotionResult motret){
    std::string responseTopic = dialogId_ + "/notify";

    rj::Document doc;
    rj::SetValueByPointer(doc, rj::Pointer( "/" + axis_ + "/goHome" ), motret.success() );
    rj::StringBuffer buffer = StringifyRjValue( doc );
    spdlog::get( Symbols::Console() )->debug( "{} {}", __func__, buffer.GetString() );
    mqtt::message_ptr rsp = mqtt::make_message( responseTopic, buffer.GetString() );
    tu_.publish( rsp );
    tu_.endDialog( dialogId_ );
}

void Dialog::moveToZeroDone( equipment::MotionResult motret ){
    std::string responseTopic = dialogId_ + "/notify";

    rj::Document doc;
    rj::SetValueByPointer(doc, rj::Pointer( "/" + axis_ + "/goZero" ), motret.success() );
    rj::StringBuffer buffer = StringifyRjValue( doc );
    spdlog::get( Symbols::Console() )->debug( "{} {}", __func__, buffer.GetString() );
    mqtt::message_ptr rsp = mqtt::make_message( responseTopic, buffer.GetString() );
    tu_.publish( rsp );
    tu_.endDialog( dialogId_ );
}

void Dialog::squeezingDone( equipment::MotionResult motret ){
    std::string responseTopic = dialogId_ + "/notify";

    rj::Document doc;
    rj::SetValueByPointer(doc, "/equipment/controlBoard/squeeze", motret.success() );
    rj::StringBuffer buffer = StringifyRjValue( doc );
    spdlog::get( Symbols::Console() )->debug( "{} {}", __func__, buffer.GetString() );
    mqtt::message_ptr rsp = mqtt::make_message( responseTopic, buffer.GetString() );
    tu_.publish( rsp );
    tu_.endDialog( dialogId_ );
}

}
