#pragma once
#include <chrono>

#include "../BasicMessageWrapper.hpp"
#include "rapidjson/document.h"
#include "rapidjson/pointer.h"
#include "spdlog/spdlog.h"

namespace rj = rapidjson;

namespace strateam{
    namespace equipment{
        namespace emulator{
            template<typename Tag>
            class Transport{
            public: // == TYPES ==
                using MessageWrapper = typename BasicMessageWrapper<Tag>::type;
                using SourceType = typename MessageWrapper::SourceType;

            private:
                IoCtx&                      ctx_;
                boost::asio::deadline_timer motTimer_;
                int                         position_{ 0 };
                int                         targetPos_{ 0 };
                bool                        running_{ false };
                std::string                 axis_;
            public:// == CTOR ==
                Transport( IoCtx& ctx, rj::Value const& )
                : ctx_( ctx )
                , motTimer_( ctx )
                {
                    spdlog::get( "console" )->debug( "[Emulator] Transport::Transport" );
                }

                Transport( Transport const& ) = delete;
                Transport& operator =( Transport const& ) = delete;

            public:// == Transport ==
                MessageWrapper sendRequestGetResponse( SourceType&& req ){
                    auto axisVal = findAxis( req );
                    // auto axisNameValue

                    if( !axisVal ){
                        return {};
                    }

                    rj::Value const& request = *axisVal;
                    rj::Value const* goValue = rj::GetValueByPointer( request, "/go" );

                    if( goValue ){
                        if(goValue->IsBool() ){
                            if( goValue->GetBool() ){
                                int offset = request[ "offset" ].GetInt();
                                int spd = request[ "targspd" ].GetInt();
                                spdlog::get( "console" )->debug( "[Emulator] offset {}, spd {}", offset, spd );
                                targetPos_ = position_ + offset;
                                initTimer( 300 );
                                rj::Document ans( rj::kObjectType );
                                rj::SetValueByPointer( ans, rj::Pointer( axis_ + "/go" ), true );
                                return MessageWrapper::fromSource( ans );
                            }else{
                                motTimer_.cancel();
                                return {};
                            }
                        }else if( goValue->IsNull() ){
                            rj::Document ans( rj::kObjectType );
                            rj::SetValueByPointer( ans, rj::Pointer( axis_ + "/go" ), running_ );
                            return MessageWrapper::fromSource( ans );
                        }                        
                    }

                    rj::Value const* goZeroValue = rj::GetValueByPointer( request, "/goZero" );

                    if( goZeroValue ){
                        if( goZeroValue->GetBool() ){
                            targetPos_ = 0;
                            initTimer( 300 );
                            rj::Document ans( rj::kObjectType );
                            rj::SetValueByPointer( ans, rj::Pointer( axis_ + "/goZero" ), true );
                            return MessageWrapper::fromSource( ans );
                        }else{
                            motTimer_.cancel();
                            return {};
                        }
                    }

                    rj::Value const* goHomeValue = rj::GetValueByPointer( request, "/goHome" );

                    if( goHomeValue ){
                        if( goHomeValue->GetBool() ){
                            targetPos_ = -20000;
                            initTimer( 1000 );
                            rj::Document ans( rj::kObjectType );
                            rj::SetValueByPointer( ans, rj::Pointer( axis_ + "/goHome" ), true );
                            return MessageWrapper::fromSource( ans );
                        }else{
                            motTimer_.cancel();
                            return {};
                        }
                    }

                    rj::Value const* posValue = rj::GetValueByPointer( request, "/pos" );

                    if( posValue && posValue->IsNull() ){
                        rj::Document ans( rj::kObjectType );
                        rj::SetValueByPointer( ans, rj::Pointer( axis_ + "/pos" ), position_ );
                        return MessageWrapper::fromSource( ans );
                    }

                    return {};
                }


            private:
                void initTimer( int motTimeMS ){
                    running_ = true;
                    motTimer_.expires_from_now( boost::posix_time::milliseconds( motTimeMS ) );
                    motTimer_.async_wait( [ this ]( boost::system::error_code err ){
                        if( !err ){
                            spdlog::get( "console" )->debug( "[Emulator] mot done, targpos {}", targetPos_ );
                            position_ = targetPos_;
                            running_ = false;
                        }
                    } );
                }

                rj::Value const* findAxis( rj::Document const& doc ){
                    rj::Value const* zAxis = rj::GetValueByPointer( doc, "/z" );

                    if( zAxis ){
                        axis_ = "/z";
                    }
                    return zAxis;
                }
            };
        }
    }
}