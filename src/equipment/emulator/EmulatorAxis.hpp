#pragma once
#include "rapidjson/document.h"
#include "rapidjson/pointer.h"

#include "eq-common/Dimensions.hpp"
#include "../interface/MotionResult.hpp"
#include "../EqException.hpp"
#include "spdlog/spdlog.h"
#include "eq-common/rjapi/Helper.hpp"

namespace strateam{
    namespace equipment{
        namespace emulator{
            class EmulatorAxis{
                std::string axisName_;
                std::string pointerName_;

            public:// == CTOR ==
                EmulatorAxis( std::string const& axisName )
                : axisName_( axisName )
                , pointerName_( std::string( "/" ) + axisName_ )
                { }
            public:
                struct Moving{
                    bool val{false};

                    operator bool(){ return val; }
                };
            public:
                template<typename T>
                T into( rj::Document const& doc );

                rapidjson::Document isMoving(){
                    rj::Pointer p( (pointerName_ + "/go") .c_str() );
                    rj::Pointer pos( ( pointerName_ + "/pos").c_str() );
                    rapidjson::Document req;
                    rj::SetValueByPointer( req, p, rj::kNullType );
                    rj::SetValueByPointer( req, pos, rj::kNullType );
                    return req;
                }

                rj::Document move( dim::MotorStep const& offset, double speed = -1, double  = -1, double = -1 ){
                    rapidjson::Document req(rj::kObjectType);
                    auto& alloc = req.GetAllocator();
                    rj::Value params( rj::kObjectType );
                    params.AddMember("targspd", static_cast<int>( speed ), alloc);
                    params.AddMember("offset",  offset.castTo<int>(), alloc);
                    params.AddMember("go", true, alloc);
                    req.AddMember( rj::Value( axisName_, alloc ), params, alloc );
                    return req;
                }

                MotionResult handleRespondCommandGo( rj::Document const& doc ){
                    auto const* val = rj::GetValueByPointer( doc, rj::Pointer( ( pointerName_ + "/go" ).c_str() ) );

                    if( !val ){
                        auto const* errval = rj::GetValueByPointer( doc, "/Error" );

                        if( errval && errval->IsInt() )
                            return MotionResult::ParseError;

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

                rj::Document moveZero(int homeOffset, double speed = -1, double = -1, double = -1  ){
                    rapidjson::Document req(rj::kObjectType);
                    auto& al = req.GetAllocator();
                    rj::Value params( rj::kObjectType );
                    params.AddMember("targspd",static_cast<int>( speed ),al);
                    params.AddMember("homeOffset", homeOffset, al);
                    params.AddMember("goZero",true,al);
                    req.AddMember( rj::Value( axisName_, al ), params, al );
                    return req;
                }

                MotionResult handleRespondCommandGoZero( rj::Document const& doc ){
                    auto const* val = rj::GetValueByPointer( doc, rj::Pointer( ( pointerName_ + "/goZero" ).c_str() ) );

                    if( !val ){
                        auto const* errval = rj::GetValueByPointer( doc, "/Error" );

                        if( errval && errval->IsInt() )
                            return MotionResult::ParseError;
                            
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
                    return val && val->IsInt() ? dim::MotorStep( val->GetInt() ) : dim::MotorStep{ 111111111 };
                }
            };

            template<>
            EmulatorAxis::Moving EmulatorAxis::into( rj::Document const& doc ){
                auto const* val = rj::GetValueByPointer( doc, rj::Pointer( ( pointerName_ + "/go" ).c_str() ) );

                if( !val || !val->IsBool() ){
                    throw Exception("Wrong isMoving response");
                }

                return Moving{ val->GetBool() };
            }

            template<>
            dim::MotorStep EmulatorAxis::into( rj::Document const& doc ){
                auto const* val = rj::GetValueByPointer( doc, rj::Pointer( ( pointerName_ + "/pos" ).c_str() ) );

                if( !val || !val->IsInt() ){
                    spdlog::get( "console" )->debug( "debug: {}", StringifyRjValue( doc ).GetString() );
                    throw Exception("Wrong position response");
                }

                return dim::MotorStep{  static_cast<double>(val->GetInt()) };
            }
        }
    }
}