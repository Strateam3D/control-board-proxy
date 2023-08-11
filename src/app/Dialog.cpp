#include "Dialog.hpp"
#include "ControlBoardTU.hpp"
#include "equipment/interface/EquipmentInterface.hpp"
#include "equipment/interface/AxisInterface.hpp"
#include "equipment/EqException.hpp"

#include <rapidjson/document.h>
#include "rapidjson/PointedValueHandler.h"
#include "rapidjson/error/en.h"
#include "rapidjson/pointer.h"
#include "rj/include/rapidjson/stringbuffer.h"
#include "rj/include/rapidjson/writer.h"
#include "rjapi/Helper.hpp"
#include <iostream>
#include "rjapi/RegistryHandler.hpp"

// mqtt
#include "mqttStack/MQTTStack.hpp"
#include "mqtt/message.h"


namespace rj = rapidjson;

namespace strateam::control_board{

Dialog::Dialog( ControlBoardTU& tu, equipment::EquipmentInterface& eq, std::string dialogId, rapidjson::Document& masterDocument )
: tu_( tu )
, equipment_( eq )
, dialogId_( std::move( dialogId ) )
, gettersSetters_{
    // x axis
    NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/x/moveHome" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    int  spd = v["spd"].GetInt();
                    auto& axis = equipment_.axis( equipment::AxisType::X );
                    motret_ = axis.moveHome( static_cast<double>( spd ) ); 
                    
                    if ( motret_ == equipment::MotionResult::Accepted ){
                        axis_ = "x";
                        return ResponseCode::Accepted;
                    } else {
                        throw equipment::Exception( motret_.str() );
                    }
                    
                }catch( std::exception const& ex ){
                    std::cout << "move hom err: " << ex.what() << std::endl;
                    return ResponseCode::Failed;
                }
            }
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/x/moveToZero" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    axis_ = "x";
                    int  spd = v["spd"].GetInt();
                    auto& axis = equipment_.axis( equipment::AxisType::X );
                    motret_ = axis.moveZero( static_cast<double>( spd ) );
                    
                    if ( motret_ == equipment::MotionResult::Accepted ){
                        axis_ = "x";
                        return ResponseCode::Accepted;
                    } else {
                        throw equipment::Exception( motret_.str() );
                    }
                }catch( std::exception const& ex ){
                    std::cout << "move zero err: " << ex.what() << std::endl;
                    return ResponseCode::Failed;
                }
            }
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/x/move" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    axis_ = "x";
                    int  spd = v["spd"].GetInt();
                    int  offset = v["offset"].GetInt();
                    auto& axis = equipment_.axis( equipment::AxisType::X );
                    motret_ = axis.move( dim::MotorStep( offset ), static_cast<double>( spd ) );
                    
                    if ( motret_ == equipment::MotionResult::Accepted ){
                        axis_ = "x";
                        return ResponseCode::Accepted;
                    } else {
                        throw equipment::Exception( motret_.str() );
                    }
                }catch( std::exception const& ex ){
                    std::cout << "move  err: " << ex.what() << std::endl;
                    return ResponseCode::Failed;
                }
            }
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/x/stop" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& ) -> ResponseCode{
                try{
                    auto& axis = equipment_.axis( equipment::AxisType::X );
                    axis.stop();
                    return ResponseCode::Success;
                }catch( std::exception const& ex ){
                    std::cout << "stop err: " << ex.what() << std::endl;
                    return ResponseCode::Failed;
                }
            }
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/x/position" ),
        GetterSetter(
            /*get*/[this]() -> rj::Value{
                return rj::Value( equipment_.axis( equipment::AxisType::X ).position().castTo<int>() );
            },
            /*set*/nullptr
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/x/homePosition" ),
        GetterSetter(
            /*get*/[this]() -> rj::Value{
                return rj::Value( equipment_.axis( equipment::AxisType::X ).homePosition().castTo<int>() );
            },
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    equipment_.axis( equipment::AxisType::X ).setHomePosition( dim::MotorStep( v.GetInt() ) ) ;
                    return ResponseCode::Success;
                }catch( std::exception const& ){
                    return ResponseCode::Failed;
                }
            }
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/x/isMoving" ),
        GetterSetter(
            /*get*/[this]() -> rj::Value{
                return rj::Value( equipment_.axis( equipment::AxisType::X ).isMoving() );
            },
            /*set*/nullptr
        )
    }
    // Y axis
    , NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/y/moveHome" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    axis_ = "y";
                    int  spd = v["spd"].GetInt();
                    auto& axis = equipment_.axis( equipment::AxisType::Y );
                    motret_ = axis.moveHome( static_cast<double>( spd ) );
                    
                    if ( motret_ == equipment::MotionResult::Accepted ){
                        axis_ = "y";
                        return ResponseCode::Accepted;
                    } else {
                        throw equipment::Exception( motret_.str() );
                    }
                }catch( std::exception const& ex ){
                    return ResponseCode::Failed;
                }
            }
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/y/moveToZero" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    int  spd = v["spd"].GetInt();
                    auto& axis = equipment_.axis( equipment::AxisType::Y );
                    motret_ = axis.moveZero( static_cast<double>( spd ) );
                    
                    if ( motret_ == equipment::MotionResult::Accepted ){
                        axis_ = "y";
                        return ResponseCode::Accepted;
                    } else {
                        throw equipment::Exception( motret_.str() );
                    }
                }catch( std::exception const& ex ){
                    std::cout << "move zero err: " << ex.what() << std::endl;
                    return ResponseCode::Failed;
                }
            }
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/y/move" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    int  spd = v["spd"].GetInt();
                    int  offset = v["offset"].GetInt();
                    auto& axis = equipment_.axis( equipment::AxisType::Y );
                    motret_ = axis.move( dim::MotorStep( offset ), static_cast<double>( spd ) );
                    
                    if ( motret_ == equipment::MotionResult::Accepted ){
                        axis_ = "y";
                        return ResponseCode::Accepted;
                    } else {
                        throw equipment::Exception( motret_.str() );
                    }
                }catch( std::exception const& ex ){
                    std::cout << "move err: " << ex.what() << std::endl;
                    return ResponseCode::Failed;
                }
            }
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/y/stop" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& ) -> ResponseCode{
                try{
                    auto& axis = equipment_.axis( equipment::AxisType::Y );
                    axis.stop();
                    return ResponseCode::Success;
                }catch( std::exception const& ex ){
                    std::cout << "stop err: " << ex.what() << std::endl;
                    return ResponseCode::Failed;
                }
            }
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/y/position" ),
        GetterSetter(
            /*get*/[this]() -> rj::Value{
                return rj::Value( equipment_.axis( equipment::AxisType::Y ).position().castTo<int>() );
            },
            /*set*/nullptr
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/y/homePosition" ),
        GetterSetter(
            /*get*/[this]() -> rj::Value{
                return rj::Value( equipment_.axis( equipment::AxisType::Y ).homePosition().castTo<int>() );
            },
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    equipment_.axis( equipment::AxisType::Y ).setHomePosition( dim::MotorStep( v.GetInt() ) ) ;
                    return ResponseCode::Success;
                }catch( std::exception const& ex ){
                    return ResponseCode::Failed;
                }
            }
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/y/isMoving" ),
        GetterSetter(
            /*get*/[this]() -> rj::Value{
                return rj::Value( equipment_.axis( equipment::AxisType::Y ).isMoving() );
            },
            /*set*/nullptr
        )
    }

    // Z axis
    , NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/z/moveHome" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    int  spd = v["spd"].GetInt();
                    auto& axis = equipment_.axis( equipment::AxisType::Z );
                    motret_ = axis.moveHome( static_cast<double>( spd ) );
                    
                    if ( motret_ == equipment::MotionResult::Accepted ){
                        axis_ = "z";
                        return ResponseCode::Accepted;
                    } else {
                        throw equipment::Exception( motret_.str() );
                    }
                }catch( std::exception const& ex ){
                    std::cout << "move home err: " << ex.what() << std::endl;
                    return ResponseCode::Failed;
                }
            }
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/z/moveToZero" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    int  spd = v["spd"].GetInt();
                    auto& axis = equipment_.axis( equipment::AxisType::Z );
                    motret_ = axis.moveZero( static_cast<double>( spd ) );
                    
                    if ( motret_ == equipment::MotionResult::Accepted ){
                        axis_ = "z";
                        return ResponseCode::Accepted;
                    } else {
                        throw equipment::Exception( motret_.str() );
                    }
                }catch( std::exception const& ex ){
                    std::cout << "move zero ex: " << ex.what() << std::endl;
                    return ResponseCode::Failed;
                }
            }
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/z/move" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v, rj::Value::AllocatorType& alloc ) -> ResponseCode{
                try{
                    int  spd = v["spd"].GetInt();
                    int  offset = v["offset"].GetInt();
                    auto& axis = equipment_.axis( equipment::AxisType::Z );
                    motret_ = axis.move( dim::MotorStep( offset ), static_cast<double>( spd ) );
                    
                    if ( motret_ == equipment::MotionResult::Accepted ){
                        axis_ = "z";
                        std::cout << motret_.str() << std::endl;
                        rj::Value rsp( int(ResponseCode::Accepted) );
                        v = rsp;
                        return ResponseCode::Accepted;
                    } else {
                        throw equipment::Exception( motret_.str() );
                    }
                }catch( std::exception const& ex ){
                    std::cout << "move ex: " << ex.what() << std::endl;
                    return ResponseCode::Failed;
                }
            }
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/z/stop" ),
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
        rj::Pointer( "/equipment/axis/z/position" ),
        GetterSetter(
            /*get*/[this]() -> rj::Value{
                return rj::Value( equipment_.axis( equipment::AxisType::Z ).position().castTo<int>() );
            },
            /*set*/nullptr
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/z/homePosition" ),
        GetterSetter(
            /*get*/[this]() -> rj::Value{
                return rj::Value( equipment_.axis( equipment::AxisType::Z ).homePosition().castTo<int>() );
            },
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    equipment_.axis( equipment::AxisType::Z ).setHomePosition( dim::MotorStep( v.GetInt() ) ) ;
                    return ResponseCode::Success;
                }catch( std::exception const& ex ){
                    return ResponseCode::Failed;
                }
            }
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/z/isMoving" ),
        GetterSetter(
            /*get*/[this]() -> rj::Value{
                return rj::Value( equipment_.axis( equipment::AxisType::Z ).isMoving() );
            },
            /*set*/nullptr
        )
    }
}
, masterDocument_( masterDocument ){
    equipment_.axis( equipment::AxisType::X ).subscribe( this );
    equipment_.axis( equipment::AxisType::Y ).subscribe( this );
    equipment_.axis( equipment::AxisType::Z ).subscribe( this );
    std::cout << "Dialog::Dialog " << this << std::endl;
}

Dialog::~Dialog(){
    equipment_.axis( equipment::AxisType::X ).unsubscribe( this );
    equipment_.axis( equipment::AxisType::Y ).unsubscribe( this );
    equipment_.axis( equipment::AxisType::Z ).unsubscribe( this );
    std::cout << "Dialog::~Dialog " << this << std::endl;
}

equipment::MotionResult Dialog::dispatch( DocumentPtr docPtr ){
    std::shared_ptr<rapidjson::Document> response( std::make_shared<rj::Document>() );
    RegistryHandler registryHandler( masterDocument_, *response, *docPtr, gettersSetters_ );
    rj::KeyValueSaxHandler<RegistryHandler> saxHandler( registryHandler );
    docPtr->Accept( saxHandler );
    std::string rspTopic = dialogId_ + "/rsp";
    rj::StringBuffer buffer = StringifyRjValue( *response );
    std::cout << "Outgoing response:\n" << "rspTopic:" << rspTopic << "\nbody: " << buffer.GetString();
    mqtt::message_ptr rsp = mqtt::make_message( rspTopic, buffer.GetString() );
    tu_.publish( rsp );
    return motret_;
}

void Dialog::motionDone( equipment::MotionResult motret ){
    std::string responseTopic = dialogId_ + "/notify";

    rj::Document doc;
    rj::SetValueByPointer(doc, rj::Pointer( "/equipment/axis/" + axis_ + "/move" ), motret.success() );
    rj::StringBuffer buffer = StringifyRjValue( doc );
    std::cout << "Dialog::motionDone: " << buffer.GetString() <<  std::endl;
    mqtt::message_ptr rsp = mqtt::make_message( responseTopic, buffer.GetString() );
    tu_.publish( rsp );
    tu_.endDialog( dialogId_ );
}

void Dialog::motionToDone( equipment::MotionResult motret ){
    std::string responseTopic = dialogId_ + "/notify";

    rj::Document doc;
    rj::SetValueByPointer(doc, rj::Pointer( "/equipment/axis/" + axis_ + "/moveTo" ), motret.success() );
    rj::StringBuffer buffer = StringifyRjValue( doc );
    std::cout << __func__ << buffer.GetString() <<  std::endl;
    mqtt::message_ptr rsp = mqtt::make_message( responseTopic, buffer.GetString() );
    tu_.publish( rsp );
    tu_.endDialog( dialogId_ );
}

void Dialog::moveHomeDone( equipment::MotionResult motret){
    std::string responseTopic = dialogId_ + "/notify";

    rj::Document doc;
    rj::SetValueByPointer(doc, rj::Pointer( "/equipment/axis/" + axis_ + "/moveHome" ), motret.success() );
    rj::StringBuffer buffer = StringifyRjValue( doc );
    std::cout << __func__ << buffer.GetString() <<  std::endl;
    mqtt::message_ptr rsp = mqtt::make_message( responseTopic, buffer.GetString() );
    tu_.publish( rsp );
    tu_.endDialog( dialogId_ );
}

void Dialog::moveToZeroDone( equipment::MotionResult motret ){
    std::string responseTopic = dialogId_ + "/notify";

    rj::Document doc;
    rj::SetValueByPointer(doc, rj::Pointer( "/equipment/axis/" + axis_ + "/moveToZero" ), motret.success() );
    rj::StringBuffer buffer = StringifyRjValue( doc );
    std::cout << __func__ << buffer.GetString() <<  std::endl;
    mqtt::message_ptr rsp = mqtt::make_message( responseTopic, buffer.GetString() );
    tu_.publish( rsp );
    tu_.endDialog( dialogId_ );
}

}