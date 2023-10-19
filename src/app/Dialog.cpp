#include "Dialog.hpp"
#include "ControlBoardTU.hpp"
#include "equipment/interface/EquipmentInterface.hpp"
#include "equipment/interface/AxisInterface.hpp"
#include "interface/ControlBoardInterface.hpp"
#include "equipment/EqException.hpp"

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
    // x axis
    NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/m1/moveHome" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    dim::UmVelocity  spd( v["spd"].GetInt() );
                    auto& axis = equipment_.axis( equipment::AxisType::M1 );
                    motret_ = axis.moveHome( spd ); 
                    
                    if ( motret_ == equipment::MotionResult::Accepted ){
                        axis_ = "m1";
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
        rj::Pointer( "/equipment/axis/m1/moveToZero" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    axis_ = "x";
                    dim::UmVelocity  spd( v["spd"].GetInt() );
                    auto& axis = equipment_.axis( equipment::AxisType::M1 );
                    motret_ = axis.moveZero( spd );
                    
                    if ( motret_ == equipment::MotionResult::Accepted ){
                        axis_ = "m1";
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
        rj::Pointer( "/equipment/axis/m1/move" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    axis_ = "x";
                    dim::UmVelocity  spd( v["spd"].GetInt() );
                    int  offset = v["offset"].GetInt();
                    auto& axis = equipment_.axis( equipment::AxisType::M1 );
                    motret_ = axis.move( dim::Um( offset ), spd  );
                    
                    if ( motret_ == equipment::MotionResult::Accepted ){
                        axis_ = "m1";
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
        rj::Pointer( "/equipment/axis/m1/stop" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& ) -> ResponseCode{
                try{
                    auto& axis = equipment_.axis( equipment::AxisType::M1 );
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
        rj::Pointer( "/equipment/axis/m1/position" ),
        GetterSetter(
            /*get*/[this]() -> rj::Value{
                return rj::Value( equipment_.axis( equipment::AxisType::M1 ).position().castTo<int>() );
            },
            /*set*/nullptr
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/m1/homePosition" ),
        GetterSetter(
            /*get*/[this]() -> rj::Value{
                return rj::Value( equipment_.axis( equipment::AxisType::M1 ).homePosition().castTo<int>() );
            },
            /*set*/nullptr
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/m1/isMoving" ),
        GetterSetter(
            /*get*/[this]() -> rj::Value{
                return rj::Value( equipment_.axis( equipment::AxisType::M1 ).isMoving() );
            },
            /*set*/nullptr
        )
    }
    // Y axis
    , NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/m2/moveHome" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    axis_ = "m2";
                    dim::UmVelocity  spd( v["spd"].GetInt() );
                    auto& axis = equipment_.axis( equipment::AxisType::M2 );
                    motret_ = axis.moveHome( spd );
                    
                    if ( motret_ == equipment::MotionResult::Accepted ){
                        axis_ = "m2";
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
        rj::Pointer( "/equipment/axis/m2/moveToZero" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    
                    dim::UmVelocity  spd( v["spd"].GetInt() );
                    auto& axis = equipment_.axis( equipment::AxisType::M2 );
                    motret_ = axis.moveZero( spd );
                    
                    if ( motret_ == equipment::MotionResult::Accepted ){
                        axis_ = "m2";
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
        rj::Pointer( "/equipment/axis/m2/move" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    dim::UmVelocity spd( v["spd"].GetInt() );
                    dim::Um  offset( v["offset"].GetInt() );
                    auto& axis = equipment_.axis( equipment::AxisType::M2 );
                    motret_ = axis.move( offset ,spd );
                    
                    if ( motret_ == equipment::MotionResult::Accepted ){
                        axis_ = "m2";
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
        rj::Pointer( "/equipment/axis/m2/stop" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& ) -> ResponseCode{
                try{
                    auto& axis = equipment_.axis( equipment::AxisType::M2 );
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
        rj::Pointer( "/equipment/axis/m2/position" ),
        GetterSetter(
            /*get*/[this]() -> rj::Value{
                return rj::Value( equipment_.axis( equipment::AxisType::M2 ).position().castTo<int>() );
            },
            /*set*/nullptr
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/m2/homePosition" ),
        GetterSetter(
            /*get*/[this]() -> rj::Value{
                return rj::Value( equipment_.axis( equipment::AxisType::M2 ).homePosition().castTo<int>() );
            },
            /*set*/nullptr
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/m2/isMoving" ),
        GetterSetter(
            /*get*/[this]() -> rj::Value{
                return rj::Value( equipment_.axis( equipment::AxisType::M2 ).isMoving() );
            },
            /*set*/nullptr
        )
    }

// ----------------------------- Z axis -----------------------------
    , NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/z/moveHome" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    dim::UmVelocity  spd( v["spd"].GetInt() );
                    auto& axis = equipment_.axis( equipment::AxisType::Z );
                    motret_ = axis.moveHome( spd );
                    // std::cout << "z move home "  << motret_.str() << std::endl;
                    
                    if ( motret_ == equipment::MotionResult::Accepted ){
                        axis_ = "z";
                        return ResponseCode::Accepted;
                    } else if ( motret_ == equipment::MotionResult::AlreadyInPosition ){
                        return ResponseCode::Success;
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
                    dim::UmVelocity  spd( v["spd"].GetInt() );
                    auto& axis = equipment_.axis( equipment::AxisType::Z );
                    motret_ = axis.moveZero( spd );
                    
                    if ( motret_ == equipment::MotionResult::Accepted ){
                        axis_ = "z";
                        return ResponseCode::Accepted;
                    }else if ( motret_ == equipment::MotionResult::AlreadyInPosition ){
                        return ResponseCode::Success;
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
            /*set*/[ this ]( rj::Value& v, rj::Value::AllocatorType& ) -> ResponseCode{
                try{
                    dim::UmVelocity  spd( v["spd"].GetInt() );
                    dim::Um  offset( v["offset"].GetInt() );
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
            /*set*/nullptr
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
    // beam
    , NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/beam/moveHome" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    dim::UmVelocity  spd( v["spd"].GetInt() );
                    auto& axis = equipment_.axis( equipment::AxisType::beam );
                    motret_ = axis.moveHome( spd );
                    
                    if ( motret_ == equipment::MotionResult::Accepted ){
                        axis_ = "beam";
                        return ResponseCode::Accepted;
                    } else if ( motret_ == equipment::MotionResult::AlreadyInPosition ){
                        axis_ = "beam";
                        return ResponseCode::Success;
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
        rj::Pointer( "/equipment/axis/beam/moveToZero" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    dim::UmVelocity  spd( v["spd"].GetInt() );
                    auto& axis = equipment_.axis( equipment::AxisType::beam );
                    motret_ = axis.moveZero( spd );
                    
                    if ( motret_ == equipment::MotionResult::Accepted ){
                        axis_ = "beam";
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
        rj::Pointer( "/equipment/axis/beam/move" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v, rj::Value::AllocatorType& ) -> ResponseCode{
                try{
                    dim::UmVelocity  spd( v["spd"].GetInt() );
                    dim::Um  offset( v["offset"].GetInt() );
                    auto& axis = equipment_.axis( equipment::AxisType::beam );
                    motret_ = axis.move( offset , spd );
                    
                    if ( motret_ == equipment::MotionResult::Accepted ){
                        axis_ = "beam";
                        std::cout << motret_.str() << std::endl;
                        // rj::Value rsp( int(ResponseCode::Accepted) );
                        // v = rsp;
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
        rj::Pointer( "/equipment/axis/beam/stop" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& ) -> ResponseCode{
                try{
                    auto& axis = equipment_.axis( equipment::AxisType::beam );
                    axis.stop();
                    return ResponseCode::Success;
                }catch( std::exception const& ex ){
                    return ResponseCode::Failed;
                }
            }
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/beam/position" ),
        GetterSetter(
            /*get*/[this]() -> rj::Value{
                return rj::Value( equipment_.axis( equipment::AxisType::beam ).position().castTo<int>() );
            },
            /*set*/nullptr
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/beam/homePosition" ),
        GetterSetter(
            /*get*/[this]() -> rj::Value{
                return rj::Value( equipment_.axis( equipment::AxisType::beam ).homePosition().castTo<int>() );
            },
            /*set*/nullptr
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/beam/isMoving" ),
        GetterSetter(
            /*get*/[this]() -> rj::Value{
                return rj::Value( equipment_.axis( equipment::AxisType::beam ).isMoving() );
            },
            /*set*/nullptr
        )
    }
// ----------------------------- h1 axis -----------------------------
    , NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/h1/moveHome" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    dim::UmVelocity  spd( v["spd"].GetInt() );
                    auto& axis = equipment_.axis( equipment::AxisType::H1 );
                    motret_ = axis.moveHome( spd );
                    
                    if ( motret_ == equipment::MotionResult::Accepted ){
                        axis_ = "h1";
                        return ResponseCode::Accepted;
                    } else if ( motret_ == equipment::MotionResult::AlreadyInPosition ){
                        return ResponseCode::Success;
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
        rj::Pointer( "/equipment/axis/h1/moveToZero" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    dim::UmVelocity  spd( v["spd"].GetInt() );
                    auto& axis = equipment_.axis( equipment::AxisType::H1 );
                    motret_ = axis.moveZero( spd );
                    
                    if ( motret_ == equipment::MotionResult::Accepted ){
                        axis_ = "h1";
                        return ResponseCode::Accepted;
                    } else if ( motret_ == equipment::MotionResult::AlreadyInPosition ){
                        return ResponseCode::Success;
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
        rj::Pointer( "/equipment/axis/h1/move" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v, rj::Value::AllocatorType& ) -> ResponseCode{
                try{
                    dim::UmVelocity  spd( v["spd"].GetInt() );
                    dim::Um  offset( v["offset"].GetInt() );
                    auto& axis = equipment_.axis( equipment::AxisType::H1 );
                    motret_ = axis.move( offset , spd );
                    
                    if ( motret_ == equipment::MotionResult::Accepted ){
                        axis_ = "h1";
                        std::cout << motret_.str() << std::endl;
                        // rj::Value rsp( int(ResponseCode::Accepted) );
                        // v = rsp;
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
        rj::Pointer( "/equipment/axis/h1/stop" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& ) -> ResponseCode{
                try{
                    auto& axis = equipment_.axis( equipment::AxisType::H1 );
                    axis.stop();
                    return ResponseCode::Success;
                }catch( std::exception const& ex ){
                    return ResponseCode::Failed;
                }
            }
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/h1/position" ),
        GetterSetter(
            /*get*/[this]() -> rj::Value{
                return rj::Value( equipment_.axis( equipment::AxisType::H1 ).position().castTo<int>() );
            },
            /*set*/nullptr
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/h1/homePosition" ),
        GetterSetter(
            /*get*/[this]() -> rj::Value{
                return rj::Value( equipment_.axis( equipment::AxisType::H1 ).homePosition().castTo<int>() );
            },
            /*set*/nullptr
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/h1/isMoving" ),
        GetterSetter(
            /*get*/[this]() -> rj::Value{
                return rj::Value( equipment_.axis( equipment::AxisType::H1 ).isMoving() );
            },
            /*set*/nullptr
        )
    }
// ----------------------------- h2 axis -----------------------------
    , NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/h2/moveHome" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    dim::UmVelocity  spd( v["spd"].GetInt() );
                    auto& axis = equipment_.axis( equipment::AxisType::H2 );
                    motret_ = axis.moveHome( spd );
                    
                    if ( motret_ == equipment::MotionResult::Accepted ){
                        axis_ = "h2";
                        return ResponseCode::Accepted;
                    } else if ( motret_ == equipment::MotionResult::AlreadyInPosition ){
                        return ResponseCode::Success;
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
        rj::Pointer( "/equipment/axis/h2/moveToZero" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    dim::UmVelocity  spd( v["spd"].GetInt() );
                    auto& axis = equipment_.axis( equipment::AxisType::H2 );
                    motret_ = axis.moveZero( spd );
                    
                    if ( motret_ == equipment::MotionResult::Accepted ){
                        axis_ = "h2";
                        return ResponseCode::Accepted;
                    } else if ( motret_ == equipment::MotionResult::AlreadyInPosition ){
                        return ResponseCode::Success;
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
        rj::Pointer( "/equipment/axis/h2/move" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v, rj::Value::AllocatorType& ) -> ResponseCode{
                try{
                    dim::UmVelocity  spd( v["spd"].GetInt() );
                    dim::Um  offset( v["offset"].GetInt() );
                    auto& axis = equipment_.axis( equipment::AxisType::H2 );
                    motret_ = axis.move( offset , spd );
                    
                    if ( motret_ == equipment::MotionResult::Accepted ){
                        axis_ = "h2";
                        std::cout << motret_.str() << std::endl;
                        // rj::Value rsp( int(ResponseCode::Accepted) );
                        // v = rsp;
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
        rj::Pointer( "/equipment/axis/h2/stop" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& ) -> ResponseCode{
                try{
                    auto& axis = equipment_.axis( equipment::AxisType::H2 );
                    axis.stop();
                    return ResponseCode::Success;
                }catch( std::exception const& ex ){
                    return ResponseCode::Failed;
                }
            }
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/h2/position" ),
        GetterSetter(
            /*get*/[this]() -> rj::Value{
                return rj::Value( equipment_.axis( equipment::AxisType::H2 ).position().castTo<int>() );
            },
            /*set*/nullptr
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/h2/homePosition" ),
        GetterSetter(
            /*get*/[this]() -> rj::Value{
                return rj::Value( equipment_.axis( equipment::AxisType::H2 ).homePosition().castTo<int>() );
            },
            /*set*/nullptr
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/axis/h2/isMoving" ),
        GetterSetter(
            /*get*/[this]() -> rj::Value{
                return rj::Value( equipment_.axis( equipment::AxisType::H2 ).isMoving() );
            },
            /*set*/nullptr
        )
    }
// ----------------------------------------- control board -----------------------------------------
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/controlBoard/coefs" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& coefs ) -> ResponseCode{
                try{
                    auto const* lval = rj::GetValueByPointer( coefs, "/lk" );
                    auto const* rval = rj::GetValueByPointer( coefs, "/rk" );
                    
                    if( lval && lval->IsDouble() && rval &&rval->IsDouble() ){
                        auto& cb = equipment_.controlBoard();
                        double lk = lval->GetDouble();
                        double rk = rval->GetDouble();
                        auto retval = cb.setLoadCellCoeffs( lk, rk );
                        return retval.success() ? ResponseCode::Success : ResponseCode::Failed;
                    }else{
                        return ResponseCode::Failed;
                    }
                }catch( std::exception const& ex ){
                    spdlog::get( Symbols::Console() )->error( "coef err {}", ex.what() );
                    return ResponseCode::Failed;
                }
            }
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/controlBoard/setZero" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& ) -> ResponseCode{
                try{
                    auto retval = equipment_.controlBoard().setZero();
                    return retval.success() ? ResponseCode::Success : ResponseCode::Failed;
                }catch( std::exception const& ex ){
                    spdlog::get( Symbols::Console() )->error( "set zero err {}", ex.what() );
                    return ResponseCode::Failed;
                }
            }
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/controlBoard/sendToUart" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    auto retval = equipment_.controlBoard().sendLogsToUart( v.GetBool() );
                    return retval.success() ? ResponseCode::Success : ResponseCode::Failed;
                }catch( std::exception const& ex ){
                    spdlog::get( Symbols::Console() )->error( "sendToUart err {}", ex.what() );
                    return ResponseCode::Failed;
                }
            }
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/controlBoard/squeeze" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& v ) -> ResponseCode{
                try{
                    dim::Gram  msv( v["msv"].GetInt() );
                    dim::Gram  sdf( v["sdf"].GetInt() );
                    dim::MilliSecond  sdt( v["sdt"].GetInt() );
                    dim::UmVelocity  hv( v["hv"].GetDouble() * 1000 );
                    dim::UmVelocity  bv( v["bv"].GetDouble() * 1000 );
                    dim::Um targetPos( v["targetPos"].GetDouble() * 1000 );
                    motret_ = equipment_.controlBoard().squeeze(targetPos, msv, sdf, sdt, hv, bv );
                    return motret_ ? ResponseCode::Accepted : ResponseCode::Failed;
                }catch( std::exception const& ex ){
                    spdlog::get( Symbols::Console() )->error( "sendToUart err {}", ex.what() );
                    return ResponseCode::Failed;
                }
            }
        )
    }
    ,NamedGetterSetterPair{
        rj::Pointer( "/equipment/controlBoard/stop" ),
        GetterSetter(
            /*get*/nullptr,
            /*set*/[ this ]( rj::Value& ) -> ResponseCode{
                try{
                    equipment_.controlBoard().stop();
                    return ResponseCode::Success;
                }catch( std::exception const& ex ){
                    spdlog::get( Symbols::Console() )->error( "sendToUart err {}", ex.what() );
                    return ResponseCode::Failed;
                }
            }
        )
    }
}
, masterDocument_( masterDocument ){
    equipment_.axis( equipment::AxisType::M1 ).subscribe( this );
    equipment_.axis( equipment::AxisType::M2 ).subscribe( this );
    equipment_.axis( equipment::AxisType::Z ).subscribe( this );
    equipment_.axis( equipment::AxisType::H1 ).subscribe( this );
    equipment_.axis( equipment::AxisType::H2 ).subscribe( this );
    equipment_.axis( equipment::AxisType::beam ).subscribe( this );
    equipment_.controlBoard().subscribe( this );
    spdlog::get( Symbols::Console() )->debug( "Dialog::Dialog" );
}

Dialog::~Dialog(){
    equipment_.axis( equipment::AxisType::M1 ).unsubscribe( this );
    equipment_.axis( equipment::AxisType::M2 ).unsubscribe( this );
    equipment_.axis( equipment::AxisType::Z ).unsubscribe( this );
    equipment_.axis( equipment::AxisType::H1 ).unsubscribe( this );
    equipment_.axis( equipment::AxisType::H2 ).unsubscribe( this );
    equipment_.axis( equipment::AxisType::beam ).unsubscribe( this );
    equipment_.controlBoard().unsubscribe( this );
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
    rj::SetValueByPointer(doc, rj::Pointer( "/equipment/axis/" + axis_ + "/move" ), motret.success() );
    rj::StringBuffer buffer = StringifyRjValue( doc );
    spdlog::get( Symbols::Console() )->debug( "{} {}", __func__, buffer.GetString() );
    mqtt::message_ptr rsp = mqtt::make_message( responseTopic, buffer.GetString() );
    tu_.publish( rsp );
    tu_.endDialog( dialogId_ );
}

void Dialog::motionToDone( equipment::MotionResult motret ){
    std::string responseTopic = dialogId_ + "/notify";

    rj::Document doc;
    rj::SetValueByPointer(doc, rj::Pointer( "/equipment/axis/" + axis_ + "/moveTo" ), motret.success() );
    rj::StringBuffer buffer = StringifyRjValue( doc );
    spdlog::get( Symbols::Console() )->debug( "{} {}", __func__, buffer.GetString() );
    mqtt::message_ptr rsp = mqtt::make_message( responseTopic, buffer.GetString() );
    tu_.publish( rsp );
    tu_.endDialog( dialogId_ );
}

void Dialog::moveHomeDone( equipment::MotionResult motret){
    std::string responseTopic = dialogId_ + "/notify";

    rj::Document doc;
    rj::SetValueByPointer(doc, rj::Pointer( "/equipment/axis/" + axis_ + "/moveHome" ), motret.success() );
    rj::StringBuffer buffer = StringifyRjValue( doc );
    spdlog::get( Symbols::Console() )->debug( "{} {}", __func__, buffer.GetString() );
    mqtt::message_ptr rsp = mqtt::make_message( responseTopic, buffer.GetString() );
    tu_.publish( rsp );
    tu_.endDialog( dialogId_ );
}

void Dialog::moveToZeroDone( equipment::MotionResult motret ){
    std::string responseTopic = dialogId_ + "/notify";

    rj::Document doc;
    rj::SetValueByPointer(doc, rj::Pointer( "/equipment/axis/" + axis_ + "/moveToZero" ), motret.success() );
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