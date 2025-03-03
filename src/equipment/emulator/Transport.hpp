#pragma once
#include <chrono>

#include "../BasicMessageWrapper.hpp"
#include "rapidjson/document.h"
#include "rapidjson/pointer.h"

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
            public:// == CTOR ==
                Transport( IoCtx& ctx, rj::Value const& )
                : ctx_( ctx )
                , motTimer_( ctx )
                {}

                Transport( Transport const& ) = delete;
                Transport& operator =( Transport const& ) = delete;

            public:// == Transport ==
            MessageWrapper sendRequestGetResponse( SourceType&& request ){
                    rj::Value* goValue = rj::GetValueByPointer( request, "/go" );

                    if( goValue ){
                        if(goValue->IsBool() ){
                            if( goValue->GetBool() ){
                                int offset = request[ "offset" ].GetInt();
                                targetPos_ = position_ + offset;
                                initTimer( 300 );
                                rj::Document ans( rj::kObjectType );
                                rj::SetValueByPointer( ans, "/go", true );
                                return MessageWrapper::fromSource( ans );
                            }else{
                                motTimer_.cancel();
                                return {};
                            }
                        }else if( goValue->IsNull() ){
                            rj::Document ans( rj::kObjectType );
                            rj::SetValueByPointer( ans, "/go", running_ );
                            return MessageWrapper::fromSource( ans );
                        }                        
                    }

                    rj::Value* goZeroValue = rj::GetValueByPointer( request, "/goZero" );

                    if( goZeroValue ){
                        if( goZeroValue->GetBool() ){
                            targetPos_ = 0;
                            initTimer( 300 );
                            rj::Document ans( rj::kObjectType );
                            rj::SetValueByPointer( ans, "/goZero", true );
                            return MessageWrapper::fromSource( ans );
                        }else{
                            motTimer_.cancel();
                            return {};
                        }
                    }

                    rj::Value* posValue = rj::GetValueByPointer( request, "/pos" );

                    if( posValue && posValue->IsNull() ){
                        rj::Document ans( rj::kObjectType );
                        rj::SetValueByPointer( ans, "/pos", position_ );
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
                            position_ = targetPos_;
                            running_ = false;
                        }
                    } );
                }
            };
        }
    }
}