#pragma once

#include "rapidjson/document.h"
#include "rapidjson/pointer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include "eq-common/Dimensions.hpp"
#include "../interface/MotionResult.hpp"
#include "../EqException.hpp"
#include "rjapi/Helper.hpp"

namespace rj = rapidjson;

namespace strateam{
    namespace equipment{
        namespace tm4c{
            class TM4CControlBoard{
                static constexpr int ParseErrorResponse = 400;
            public:
                struct Moving{
                    bool val{false};

                    operator bool(){ return val; }
                };
            public:
                rj::Document setLoadCellCoeffs( double lk, double rk ){
                    rapidjson::Document doc( rj::kObjectType );
                    rj::SetValueByPointer( doc, "/load_cell/lk", lk );
                    rj::SetValueByPointer( doc, "/load_cell/rk", rk );
                    rj::SetValueByPointer( doc, "/load_cell/iir_reset", true );
                    return doc;
                }

                rj::Document setZero(){
                    rapidjson::Document doc( rj::kObjectType );
                    rj::SetValueByPointer( doc, "/load_cell/setZero", true );
                    return doc;
                }

                rj::Document sendLogsToUart( bool value ){
                    rj::Document req(rj::kObjectType);
                    rj::SetValueByPointer( req, "/load_cell/sendToUart", value );
                    return req;
                }

                rj::Document squeeze( int hnum, int beamOffset, int beamVelocity, int haydonOffset, int haydonVelocity ){
                    rj::Document req(rj::kObjectType);
                    rj::SetValueByPointer( req, "/squeeze/hnum", hnum );
                    rj::SetValueByPointer( req, "/squeeze/bOffset", beamOffset );
                    rj::SetValueByPointer( req, "/squeeze/bv", beamVelocity );
                    rj::SetValueByPointer( req, "/squeeze/hOffset", haydonOffset );
                    rj::SetValueByPointer( req, "/squeeze/hv", haydonVelocity );
                    rj::SetValueByPointer( req, "/squeeze/go", true );
                    return req;
                }

                rj::Document stop(){
                    rj::Document req(rj::kObjectType);
                    rj::SetValueByPointer( req, "/squeeze/go", false );
                    return req;
                }

                MotionResult checkErrors( rj::Document const& doc ){
                    rj::StringBuffer buffer = StringifyRjValue( doc );
                    std::string respStr = buffer.GetString();
                    return respStr.find( "EWroVa" ) == std::string::npos && respStr.find( "EWroTy" ) == std::string::npos && respStr.find( "ENoFou" ) == std::string::npos 
                        ? MotionResult::Success 
                        : MotionResult::Failed;
                }

                MotionResult handleRespondCommandGo( rj::Document const& doc ){
                    auto const* val = rj::GetValueByPointer( doc, "/squeeze/go" );

                    if( !val ){
                        auto const* errval = rj::GetValueByPointer( doc, "/Error" );

                        if( errval && errval->IsInt() && errval->GetInt() == ParseErrorResponse )
                            return MotionResult::ParseError;

                        throw Exception( "Invalid go response" );
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

                rapidjson::Document isMoving(){
                    rapidjson::Document req;
                    rj::SetValueByPointer( req, "/squeeze/go", rj::kNullType );
                    return req;
                }

                template<typename T>
                    T into( rj::Document const& doc );
            };

            template<>
            TM4CControlBoard::Moving TM4CControlBoard::into( rj::Document const& doc ){
                auto const* val = rj::GetValueByPointer( doc, "/squeeze/go" );

                if( !val || !val->IsBool() ){
                    throw Exception("Wrong isMoving response");
                }

                return Moving{ val->GetBool() };
            }
        }
    }
}
