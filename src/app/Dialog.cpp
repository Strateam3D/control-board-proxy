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
                    rval_ = motret == equipment::MotionResult::Accepted ? CallResult::Keep : CallResult::Discard; 
                    return motret == equipment::MotionResult::Accepted ? ResponseCode::Accepted : ResponseCode::Failed;
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
                    int  spd = v["spd"].GetInt();
                    auto& axis = equipment_.axis( equipment::AxisType::X );
                    auto motret = axis.moveZero( static_cast<double>( spd ) );
                    rval_ = motret == equipment::MotionResult::Accepted ? CallResult::Keep : CallResult::Discard;
                    return motret == equipment::MotionResult::Accepted ? ResponseCode::Accepted : ResponseCode::Failed;
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
                    int  spd = v["spd"].GetInt();
                    int  offset = v["offset"].GetInt();
                    auto& axis = equipment_.axis( equipment::AxisType::X );
                    auto motret = axis.move( dim::MotorStep( offset ), static_cast<double>( spd ) );
                    rval_ = motret == equipment::MotionResult::Accepted ? CallResult::Keep : CallResult::Discard;
                    return motret == equipment::MotionResult::Accepted ? ResponseCode::Accepted : ResponseCode::Failed;
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
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
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
                    int  spd = v["spd"].GetInt();
                    auto& axis = equipment_.axis( equipment::AxisType::Y );
                    auto motret = axis.moveHome( static_cast<double>( spd ) );
                    rval_ = motret == equipment::MotionResult::Accepted ? CallResult::Keep : CallResult::Discard; 
                    return motret == equipment::MotionResult::Accepted ? ResponseCode::Accepted : ResponseCode::Failed;
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
                    rval_ = motret == equipment::MotionResult::Accepted ? CallResult::Keep : CallResult::Discard;
                    return motret == equipment::MotionResult::Accepted ? ResponseCode::Accepted : ResponseCode::Failed;
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
                    rval_ = motret == equipment::MotionResult::Accepted ? CallResult::Keep : CallResult::Discard;
                    return motret == equipment::MotionResult::Accepted ? ResponseCode::Accepted : ResponseCode::Failed;
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
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
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
                    rval_ = motret == equipment::MotionResult::Accepted ? CallResult::Keep : CallResult::Discard; 
                    return motret == equipment::MotionResult::Accepted ? ResponseCode::Accepted : ResponseCode::Failed;
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
                    rval_ = motret == equipment::MotionResult::Accepted ? CallResult::Keep : CallResult::Discard;
                    return motret == equipment::MotionResult::Accepted ? ResponseCode::Accepted : ResponseCode::Failed;
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
                    rval_ = motret == equipment::MotionResult::Accepted ? CallResult::Keep : CallResult::Discard;
                    return motret == equipment::MotionResult::Accepted ? ResponseCode::Accepted : ResponseCode::Failed;
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
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
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
    
}

Dialog::CallResult Dialog::dispatch( DocumentPtr docPtr ){
    return rval_;
}

}