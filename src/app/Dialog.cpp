#include "Dialog.hpp"
#include "ControlBoardTU.hpp"
#include "equipment/interface/EquipmentInterface.hpp"
#include "equipment/interface/AxisInterface.hpp"

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
                    auto motret = axis.moveHome( static_cast<double>( spd ) ); 
                    
                    if ( motret == equipment::MotionResult::Accepted ){
                        axis_ = "x";
                        rval_ = CallResult::Keep;
                        return ResponseCode::Accepted;
                    } else {
                        rval_ =  CallResult::Discard;
                        return ResponseCode::Failed;
                    }
                    
                }catch( std::exception const& ex ){
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
                    auto motret = axis.moveZero( static_cast<double>( spd ) );
                    
                    if ( motret == equipment::MotionResult::Accepted ){
                        axis_ = "x";
                        rval_ = CallResult::Keep;
                        return ResponseCode::Accepted;
                    } else {
                        rval_ =  CallResult::Discard;
                        return ResponseCode::Failed;
                    }
                }catch( std::exception const& ex ){
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
                    auto motret = axis.move( dim::MotorStep( offset ), static_cast<double>( spd ) );

                    if ( motret == equipment::MotionResult::Accepted ){
                        axis_ = "x";
                        rval_ = CallResult::Keep;
                        return ResponseCode::Accepted;
                    } else {
                        rval_ =  CallResult::Discard;
                        return ResponseCode::Failed;
                    }
                }catch( std::exception const& ex ){
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
                }catch( std::exception const& ex ){
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
                    auto motret = axis.moveHome( static_cast<double>( spd ) );
                    
                    if ( motret == equipment::MotionResult::Accepted ){
                        axis_ = "y";
                        rval_ = CallResult::Keep;
                        return ResponseCode::Accepted;
                    } else {
                        rval_ =  CallResult::Discard;
                        return ResponseCode::Failed;
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
                    auto motret = axis.moveZero( static_cast<double>( spd ) );
                    
                    if ( motret == equipment::MotionResult::Accepted ){
                        axis_ = "y";
                        rval_ = CallResult::Keep;
                        return ResponseCode::Accepted;
                    } else {
                        rval_ =  CallResult::Discard;
                        return ResponseCode::Failed;
                    }
                }catch( std::exception const& ex ){
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
                    auto motret = axis.move( dim::MotorStep( offset ), static_cast<double>( spd ) );
                    
                    if ( motret == equipment::MotionResult::Accepted ){
                        axis_ = "y";
                        rval_ = CallResult::Keep;
                        return ResponseCode::Accepted;
                    } else {
                        rval_ =  CallResult::Discard;
                        return ResponseCode::Failed;
                    }
                }catch( std::exception const& ex ){
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
                    auto motret = axis.moveHome( static_cast<double>( spd ) );
                    
                    if ( motret == equipment::MotionResult::Accepted ){
                        axis_ = "z";
                        rval_ = CallResult::Keep;
                        return ResponseCode::Accepted;
                    } else {
                        rval_ =  CallResult::Discard;
                        return ResponseCode::Failed;
                    }
                }catch( std::exception const& ex ){
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
                    auto motret = axis.moveZero( static_cast<double>( spd ) );
                    
                    if ( motret == equipment::MotionResult::Accepted ){
                        axis_ = "z";
                        rval_ = CallResult::Keep;
                        return ResponseCode::Accepted;
                    } else {
                        rval_ =  CallResult::Discard;
                        return ResponseCode::Failed;
                    }
                }catch( std::exception const& ex ){
                    return ResponseCode::Failed;
                }
            }
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/z/move" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    int  spd = v["spd"].GetInt();
                    int  offset = v["offset"].GetInt();
                    auto& axis = equipment_.axis( equipment::AxisType::Z );
                    auto motret = axis.move( dim::MotorStep( offset ), static_cast<double>( spd ) );
                    
                    if ( motret == equipment::MotionResult::Accepted ){
                        axis_ = "z";
                        rval_ = CallResult::Keep;
                        return ResponseCode::Accepted;
                    } else {
                        rval_ =  CallResult::Discard;
                        return ResponseCode::Failed;
                    }
                }catch( std::exception const& ex ){
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
}

Dialog::~Dialog(){
    equipment_.axis( equipment::AxisType::X ).unsubscribe( this );
    equipment_.axis( equipment::AxisType::Y ).unsubscribe( this );
    equipment_.axis( equipment::AxisType::Z ).unsubscribe( this );
}

Dialog::CallResult Dialog::dispatch( DocumentPtr docPtr ){
    return rval_;
}

void Dialog::motionDone( equipment::MotionResult motret ){
    std::string responseTopic = dialogId_ + "/notify";

    rj::Document doc;
    rj::SetValueByPointer(doc, rj::Pointer( "/equipment/axis" + axis_ + "/move" ), motret.success() );
    rj::StringBuffer buffer = StringifyRjValue( doc );
    std::cout << __func__ << buffer.GetString() <<  std::endl;
    mqtt::message_ptr rsp = mqtt::make_message( responseTopic, buffer.GetString() );
    tu_.publish( rsp );
    tu_.endDialog( dialogId_ );
}

void Dialog::motionToDone( equipment::MotionResult motret ){
    std::string responseTopic = dialogId_ + "/notify";

    rj::Document doc;
    rj::SetValueByPointer(doc, rj::Pointer( "/equipment/axis" + axis_ + "/moveTo" ), motret.success() );
    rj::StringBuffer buffer = StringifyRjValue( doc );
    std::cout << __func__ << buffer.GetString() <<  std::endl;
    mqtt::message_ptr rsp = mqtt::make_message( responseTopic, buffer.GetString() );
    tu_.publish( rsp );
    tu_.endDialog( dialogId_ );
}

void Dialog::moveHomeDone( equipment::MotionResult motret){
    std::string responseTopic = dialogId_ + "/notify";

    rj::Document doc;
    rj::SetValueByPointer(doc, rj::Pointer( "/equipment/axis" + axis_ + "/moveHome" ), motret.success() );
    rj::StringBuffer buffer = StringifyRjValue( doc );
    std::cout << __func__ << buffer.GetString() <<  std::endl;
    mqtt::message_ptr rsp = mqtt::make_message( responseTopic, buffer.GetString() );
    tu_.publish( rsp );
    tu_.endDialog( dialogId_ );
}

void Dialog::moveToZeroDone( equipment::MotionResult motret ){
    std::string responseTopic = dialogId_ + "/notify";

    rj::Document doc;
    rj::SetValueByPointer(doc, rj::Pointer( "/equipment/axis" + axis_ + "/moveToZero" ), motret.success() );
    rj::StringBuffer buffer = StringifyRjValue( doc );
    std::cout << __func__ << buffer.GetString() <<  std::endl;
    mqtt::message_ptr rsp = mqtt::make_message( responseTopic, buffer.GetString() );
    tu_.publish( rsp );
    tu_.endDialog( dialogId_ );
}

}