#pragma once

#include "rapidjson/document.h"
#include "rapidjson/pointer.h"

#include "eq-common/Dimensions.hpp"
#include "../interface/MotionResult.hpp"
#include "../EqException.hpp"

#include <string>
#include <iostream>
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

                struct MotorStep{
                    dim::MotorStep val{0.0};

                    operator dim::MotorStep(){ return val; }
                };
            
            public:// == DlpAxis ==
                DlpAxis( std::size_t axisId ) : axisId_( axisId ), axisName_( "/motor" + std::to_string( axisId ) ){}

                template<typename T>
                T into( rj::Document const& doc );


                rapidjson::Document isMoving(){
                    static rj::Pointer p( axisName_.append( "/go" ).c_str() );
                    static rj::Pointer pos( axisName_.append("/pos").c_str() );
                    rapidjson::Document req(rj::kObjectType);
                    rj::SetValueByPointer( req, p, rj::kNullType );
                    rj::SetValueByPointer( req, pos, rj::kNullType );
                    return req;
                }

                bool isMovingResult( rj::Document const& doc ){
                    static rj::Pointer p( axisName_.append( "/go" ).c_str());
                    auto const* val = rj::GetValueByPointer( doc, p );

                    if( !val || !val->IsBool() )
                        throw std::runtime_error( "Invalid go response" );
                        
                    return val->GetBool();
                }

                rj::Document move( dim::MotorStep const& offset, double speed, double accel, double decel ){
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
                    auto const* val = rj::GetValueByPointer( doc, rj::Pointer( axisName_.append( "/go" ).c_str() ) );

                    if( !val ){
                        throw Exception( "Invalid go response" );
                    }

                    if( val->IsString() ){
                        std::string rsp = val->GetString();

                        if( rsp.find( "Already in the targetPosition" ) != std::string::npos ){
                            return MotionResult::AlreadyInPosition;
                        }else{
                            std::cout << __func__ << "ERR: " << rsp << std::endl;
                            return MotionResult::FAILED;
                        }
                    }

                    return val->GetBool() ? MotionResult::Accepted : MotionResult::FAILED;
                }

                rj::Document moveTo( dim::MotorStep const& target, double speed, double accel, double decel ){
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

                rj::Document moveZero( double speed, double accel, double decel ){
                    fixSpeedAccelDecel( speed,accel,decel );
                    rapidjson::Document req(rj::kObjectType);
                    auto& al = req.GetAllocator();
                    rj::Value params( rj::kObjectType );
                    params.AddMember("targspd",speed,al);
                    params.AddMember("accel", accel, al);  //Q_UNUSED(accel);//
                    params.AddMember("decel", decel, al);  //Q_UNUSED(decel);//
                    params.AddMember("goZero",true,al);
                    params.AddMember("go",rj::kNullType,al);
                    req.AddMember( rj::Value( axisName_, al ), params, al );
                    return req;
                }

                rj::Document stop(){
                    rj::Document req;
                    rj::SetValueByPointer( req, rj::Pointer( axisName_.append( "/go" ).c_str() ), false );
                    rj::SetValueByPointer( req, rj::Pointer( axisName_.append( "/pos" ).c_str() ), rj::kNullType );
                    return req;
                }

                rj::Document position(){
                    rj::Document req;
                    rj::SetValueByPointer( req, rj::Pointer( axisName_.append( "/pos" ).c_str() ), rj::kNullType );
                    return req;
                }

                dim::MotorStep positionResult( rj::Document const& doc ){
                    auto const* val = rj::GetValueByPointer( doc, rj::Pointer( axisName_.append( "/pos" ).c_str() ) );
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

            private:// == MEMBERS ==
                std::size_t axisId_;
                unsigned short microstep_{DefaultMicroStep};
                std::string axisName_;
            };

            template<>
            DlpAxis::Moving DlpAxis::into( rj::Document const& doc ){
                auto const* val = rj::GetValueByPointer( doc, rj::Pointer( axisName_.append( "/go" ).c_str() ) );

                if( !val || !val->IsBool() ){
                    throw Exception("Wrong isMoving response");
                }

                return Moving{ val->GetBool() };
            }

            template<>
            DlpAxis::MotorStep DlpAxis::into( rj::Document const& doc ){
                auto const* val = rj::GetValueByPointer( doc, rj::Pointer( axisName_.append( "/pos" ).c_str() ) );

                if( !val || !val->IsInt() ){
                    throw Exception("Wrong position response");
                }

                return MotorStep{ dim::MotorStep{  static_cast<double>(val->GetInt()) } };
            }
        }// namespace dlp
    }// namespace equipment
}// namespace strateam