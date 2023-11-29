#pragma once

#include "rapidjson/document.h"
#include "rapidjson/pointer.h"

#include "eq-common/Dimensions.hpp"
#include "../interface/MotionResult.hpp"
#include "../EqException.hpp"

#include <string>
// #include <iostream>
#include <stdexcept>

namespace strateam{
    namespace equipment{
        namespace dlp{
            namespace rj = rapidjson;
            class DlpAxis{
            public:// == CONSTANTS ==
                static constexpr double DefaultAccel = 200; //should be 12000  steps per square second [step/(s^2)], if microStep_ == 1
                static constexpr double DefaultDecel = 200; //should be 12000, if microStep_ == 1
                static constexpr double DefaultSpeedLimit = 512.5;// if microStep_ == 1
                static constexpr unsigned short DefaultMicroStep = 8;
                static constexpr dim::MotorStep InvalidPositionMin = dim::MotorStep{ std::numeric_limits<double>::min()};
                static constexpr dim::MotorStep InvalidPositionMax = dim::MotorStep{ std::numeric_limits<double>::max()};
                static constexpr dim::MotorStep InvalidPosition = InvalidPositionMin;

            public:
                struct Moving{
                    bool val{false};

                    operator bool(){ return val; }
                };
            
            public:// == DlpAxis ==
                DlpAxis( std::string const& axisName ) 
                : axisName_( axisName )
                , pointerName_( std::string( "/" ) + axisName_ )
                {}

                template<typename T>
                T into( rj::Document const& doc );


                rapidjson::Document isMoving(){
                    // std::cout << "axis: " << axisId_ << ", pointerName_: " << pointerName_
                    rj::Pointer p( (pointerName_ + "/go") .c_str() );
                    // std::cout << "pointerName_1: " << pointerName_ << std::endl;
                    rj::Pointer pos( ( pointerName_ + "/pos").c_str() );
                    // std::cout << "pointerName_2: " << pointerName_ << std::endl;
                    rapidjson::Document req;
                    rj::SetValueByPointer( req, p, rj::kNullType );
                    rj::SetValueByPointer( req, pos, rj::kNullType );
                    return req;
                }

                bool isMovingResult( rj::Document const& doc ){
                    rj::Pointer p( ( pointerName_ + "/go" ).c_str());
                    auto const* val = rj::GetValueByPointer( doc, p );

                    if( !val || !val->IsBool() )
                        throw std::runtime_error( "Invalid go response" );
                        
                    return val->GetBool();
                }

                rj::Document move( dim::MotorStep const& offset, double speed = -1, double accel = -1, double decel = -1 ){
                    fixSpeedAccelDecel( speed,accel,decel );
                    rapidjson::Document req(rj::kObjectType);
                    auto& alloc = req.GetAllocator();
                    rj::Value params( rj::kObjectType );
                    params.AddMember("pos", rj::kNullType, alloc);  //for debug
                    params.AddMember("offset",  offset.castTo<int>(), alloc);
                    params.AddMember("targpos", rj::kNullType, alloc);  //for debug
                    params.AddMember("targspd", speed, alloc);
                    params.AddMember("accel", accel, alloc);  //Q_UNUSED(accel);//
                    params.AddMember("decel", decel, alloc);  //Q_UNUSED(decel);//
                    params.AddMember("go", true, alloc);
                    req.AddMember( rj::Value( axisName_, alloc ), params, alloc );
                    return req;
                }

                MotionResult handleRespondCommandGo( rj::Document const& doc ){
                    auto const* val = rj::GetValueByPointer( doc, rj::Pointer( ( pointerName_ + "/go" ).c_str() ) );

                    if( !val ){
                        return MotionResult::FAILED;
                    }

                    if( val->IsString() ){
                        std::string rsp = val->GetString();

                        if( rsp.find( "Already in the targetPosition" ) != std::string::npos || rsp.find( "AlreadyInPosition" ) != std::string::npos ){
                            // std::cout << "aaaaaaaaaa AlreadyInPosition\n";
                            return MotionResult::AlreadyInPosition;
                        }else{
                            // std::cout << __func__ << "ERR: " << rsp << std::endl;
                            return MotionResult::FAILED;
                        }
                    }

                    return val->GetBool() ? MotionResult::Accepted : MotionResult::FAILED;
                }

                rj::Document moveTo( dim::MotorStep const& target, double speed = -1, double accel = -1, double decel = -1  ){
                    fixSpeedAccelDecel( speed,accel,decel );
                    rapidjson::Document req(rj::kObjectType);
                    auto& alloc = req.GetAllocator();
                    rj::Value params( rj::kObjectType );
                    params.AddMember("pos", rj::kNullType, alloc);  //request current position, for debug
                    params.AddMember("targpos", target.castTo<int>(), alloc);
                    params.AddMember("targspd", speed, alloc);
                    params.AddMember("accel", accel, alloc);  //Q_UNUSED(accel);//
                    params.AddMember("decel", decel, alloc);  //Q_UNUSED(decel);//
                    params.AddMember("go", true, alloc);
                    req.AddMember( rj::Value( axisName_, alloc ), params, alloc );
                    return req;
                }

                rj::Document moveZero(int homeOffset, double speed = -1, double accel = -1, double decel = -1  ){
                    fixSpeedAccelDecel( speed,accel,decel );
                    rapidjson::Document req(rj::kObjectType);
                    auto& al = req.GetAllocator();
                    rj::Value params( rj::kObjectType );
                    params.AddMember("targspd",speed,al);
                    params.AddMember("accel", accel, al);  //Q_UNUSED(accel);//
                    params.AddMember("decel", decel, al);  //Q_UNUSED(decel);//
		            
                    params.AddMember("homeOffset", homeOffset, al);

                    params.AddMember("goZero",true,al);
                    req.AddMember( rj::Value( axisName_, al ), params, al );
                    return req;
                }

                MotionResult handleRespondCommandGoZero( rj::Document const& doc ){
                    auto const* val = rj::GetValueByPointer( doc, rj::Pointer( ( pointerName_ + "/goZero" ).c_str() ) );

                    if( !val ){
                        return MotionResult::FAILED;
                    }

                    if( val->IsString() ){
                        std::string rsp = val->GetString();

                        if( rsp.find( "Already in the targetPosition" ) != std::string::npos || rsp.find( "AlreadyInPosition" ) != std::string::npos ){
                            // std::cout << "aaaaaaaaaa AlreadyInPosition\n";
                            return MotionResult::AlreadyInPosition;
                        }else{
                            // std::cout << __func__ << "ERR: " << rsp << std::endl;
                            return MotionResult::FAILED;
                        }
                    }

                    return val->GetBool() ? MotionResult::Accepted : MotionResult::FAILED;
                }

                rj::Document stop(){
                    rj::Document req;
                    rj::SetValueByPointer( req, rj::Pointer( ( pointerName_ + "/go" ).c_str() ), false );
                    rj::SetValueByPointer( req, rj::Pointer( ( pointerName_ + "/pos" ).c_str() ), rj::kNullType );
                    return req;
                }

                rj::Document position(){
                    rj::Document req;
                    rj::SetValueByPointer( req, rj::Pointer( ( pointerName_ +  "/pos" ).c_str() ), rj::kNullType );
                    return req;
                }

                dim::MotorStep positionResult( rj::Document const& doc ){
                    auto const* val = rj::GetValueByPointer( doc, rj::Pointer( ( pointerName_ + "/pos" ).c_str() ) );
                    return val && val->IsInt() ? dim::MotorStep( val->GetInt() ) : InvalidPosition;
                }
            private:
                void fixSpeedAccelDecel( double& speed, double&accel, double&decel ){
                    if( speed < 0 ) 
                        speed = DefaultSpeedLimit * microstep_; 
                    if( accel < 0 ) 
                        accel = DefaultAccel * microstep_; 
                    if( decel < 0 ) 
                        decel = DefaultDecel * microstep_; 
                }

            protected:// == MEMBERS ==
                unsigned short microstep_{DefaultMicroStep};
                std::string axisName_;
                std::string pointerName_;
            };

            template<>
            DlpAxis::Moving DlpAxis::into( rj::Document const& doc ){
                auto const* val = rj::GetValueByPointer( doc, rj::Pointer( ( pointerName_ + "/go" ).c_str() ) );

                if( !val || !val->IsBool() ){
                    throw Exception("Wrong isMoving response");
                }

                return Moving{ val->GetBool() };
            }

            template<>
            dim::MotorStep DlpAxis::into( rj::Document const& doc ){
                auto const* val = rj::GetValueByPointer( doc, rj::Pointer( ( pointerName_ + "/pos" ).c_str() ) );

                if( !val || !val->IsInt() ){
                    throw Exception("Wrong position response");
                }

                return dim::MotorStep{  static_cast<double>(val->GetInt()) };
            }
        }// namespace dlp
    }// namespace equipment
}// namespace strateam
