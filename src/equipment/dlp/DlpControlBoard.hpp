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
        namespace dlp{
            class DlpControlBoard{
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
                    return doc;
                }

                rj::Document setZero(){
                    rapidjson::Document doc( rj::kObjectType );
                    rj::SetValueByPointer( doc, "/load_cell/setZero", true );
                    return doc;
                }

                rj::Document sendLogsToUart( bool value ){
                    rj::Document req(rj::kObjectType);
                    rj::SetValueByPointer( req, "/load_cell/setZero", value );
                    return req;
                }

                rj::Document squeeze( int minSignalValueToDetect, int signalDeltaF, int stableTimeDetection, int haydonVelocity, int beamVelocity ){
                    rj::Document req(rj::kObjectType);
                    rj::SetValueByPointer( req, "/load_cell/monitor/squeeze/msv", minSignalValueToDetect );
                    rj::SetValueByPointer( req, "/load_cell/monitor/squeeze/sdf", signalDeltaF );
                    rj::SetValueByPointer( req, "/load_cell/monitor/squeeze/sdt", stableTimeDetection );
                    rj::SetValueByPointer( req, "/load_cell/monitor/squeeze/hv", haydonVelocity );
                    rj::SetValueByPointer( req, "/load_cell/monitor/squeeze/bv", beamVelocity );
                    rj::SetValueByPointer( req, "/load_cell/monitor/squeeze/go", true );
                    return req;
                }

                rj::Document stop(){
                    rj::Document req(rj::kObjectType);
                    rj::SetValueByPointer( req, "/load_cell/monitor/squeeze/go", false );
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
                    auto const* val = rj::GetValueByPointer( doc, "/load_cell/monitor/squeeze/go" );

                    if( !val ){
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
                    rj::SetValueByPointer( req, "/load_cell/monitor/squeeze/go", rj::kNullType );
                    return req;
                }

                template<typename T>
                    T into( rj::Document const& doc );
            };

            template<>
            DlpControlBoard::Moving DlpControlBoard::into( rj::Document const& doc ){
                auto const* val = rj::GetValueByPointer( doc, "/load_cell/monitor/squeeze/go" );

                if( !val || !val->IsBool() ){
                    throw Exception("Wrong isMoving response");
                }

                return Moving{ val->GetBool() };
            }
        }
    }
}