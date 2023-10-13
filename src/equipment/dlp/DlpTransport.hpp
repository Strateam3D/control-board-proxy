#pragma once
#include "../BasicMessageWrapper.hpp"
#include "../EqException.hpp"
#include "Symbols.hpp"
#include "spdlog/spdlog.h"

#include "rapidjson/document.h"

#include <iostream>
#include <mutex>
#include <condition_variable>

#include "../serial_port/AsyncSerialPort.hpp"

namespace strateam{
    namespace equipment{
        namespace dlp{
            template<typename Tag>
            class DlpTransport{
            public:// == TYPES ==
                using MessageWrapper = typename BasicMessageWrapper<Tag>::type;
                using SourceType = typename MessageWrapper::SourceType;
                using ByteArray = std::vector<char>;

            private:// == MEMBERS ==
                ByteArray ba_;
                std::mutex mtx_;
                std::condition_variable cv_;
                AsyncSerialPort serialPort_;
                
                std::string deviceName_;
                int baudRate_{ 115200 };

            public:// == CTOR ==
                DlpTransport( rapidjson::Value const& config )
                : deviceName_( "/dev/ttyACM0" ){
                    setConfig(config);
                    openConnection();
                    readAll();
                    sendRawRequestGetResponse( MessageWrapper::fromData( "{\"info\" : null}" ).encode() );
                }

                DlpTransport( DlpTransport const& ) = delete;
                
                DlpTransport& operator =( DlpTransport const& ) = delete;
                
                ~DlpTransport(){
                    closeConnection();
                }

            public:// == DlpTransport ==
                MessageWrapper sendRequestGetResponse( SourceType&& request ){
                    return sendRawRequestGetResponse( MessageWrapper::fromSource( request ).encode() );
                }

            private:
                MessageWrapper sendRawRequestGetResponse( MessageWrapper const& msg ){
                    static std::string const prefix = "ncd-json '";
                    static std::string const suffix = "'\n";

                    std::vector<char> ba(prefix.begin(), prefix.end());
                    ba.insert( ba.end(),  msg.data(), msg.data() + msg.size() );
                    ba.insert( ba.end(), suffix.begin(), suffix.end() );
                    spdlog::get( Symbols::Console() )->debug( "request {}", ba.data());
                    
                    return sendRawRequestGetResponse( ba );;
                }

                MessageWrapper sendRawRequestGetResponse( ByteArray const& ba ){
                    std::unique_lock<std::mutex> _( mtx_ );
                    serialPort_.write( ba );
                    
                    if( cv_.wait_for( _, std::chrono::milliseconds( 5000 ) ) == std::cv_status::no_timeout ){
                        auto resp = std::move( ba_ );
                        assert( ba_.empty() );
                        spdlog::get( Symbols::Console() )->debug( "response {}", resp.empty() ? "EMPTY" : resp.data());
                        return MessageWrapper::fromData( resp.data() ).decode();
                    }else{
                        spdlog::get( Symbols::Console() )->debug( "timeout for req {}", ba.data());
                    }

                    return MessageWrapper();
                }

                static bool findRespondTerminator( ByteArray const& respond ){	
                    int btc;  // bytes to chop (cut from the end)
                    return findResponseTerminator( respond, btc ); 
                }

                static bool findResponseTerminator( ByteArray const& respond, int& bytesToChop ){
                    bool promtInvitationFound = false;
                    int count = 1;
                    
                    for( auto i = respond.rbegin(); i != respond.rend();  ++count, ++i ){
                        if( *i == '>' ){
                            promtInvitationFound = true;
                            break;
                        }
                    }
                    
                    if( promtInvitationFound )
                        bytesToChop = count;
                    
                    return promtInvitationFound;
                }

                static bool findResponseTerminatorAndChop( ByteArray& respond ){
                    int btc;  // bytes to chop (cut from the end)
                    bool promtInvitationFound = findResponseTerminator( respond, btc ); 
                    
                    if( promtInvitationFound )
                        respond.erase( respond.begin() + btc, respond.end() );
                    
                    return promtInvitationFound;
                }

                void asyncHandleRead( const char* data, std::size_t len ){
                    std::unique_lock<std::mutex>_( mtx_ );
                    ba_.insert( ba_.end(), data, data + len );               
                    bool promtInvitationFound = findResponseTerminatorAndChop( ba_ );
                    
                    if( promtInvitationFound ){
                        cv_.notify_one();
                    }
                }

                void setConfig( rapidjson::Value const& config ){
                    auto maybeSetFromConfig = [&]( auto& toVal, auto && path ){
                        using namespace rapidjson;
                        const Value *v = GetValueByPointer( config, path );
                        if( v and v->Is<std::decay_t<decltype(toVal)>>() )
                            toVal = v->Get<std::decay_t<decltype(toVal)>>();
                    };
                    
                    maybeSetFromConfig( deviceName_, "/device" );
                    maybeSetFromConfig( baudRate_, "/baudRate" );
                }

                void openConnection(){
                    spdlog::get( Symbols::Console() )->debug( "device: {}, baud rate: {}", deviceName_, baudRate_ );
                    serialPort_.setCallback( [this]( char const* data, size_t len ){
                        asyncHandleRead( data, len );
                    } );
                    
                    serialPort_.open( deviceName_, baudRate_ );

                    if( !serialPort_.isOpen() ){
                        throw Exception( "Failed to open " + deviceName_ );
                    }
                }

                void readAll(){
                    std::unique_lock<std::mutex> _( mtx_ );
                    
                    if( cv_.wait_for( _, std::chrono::milliseconds( 500 ) ) == std::cv_status::no_timeout ){
                        auto resp = std::move( ba_ );
                        assert( ba_.empty() );
                        spdlog::get( Symbols::Console() )->debug( "readAll: {}", resp.data() );
                    }else{
                        spdlog::get( Symbols::Console() )->error( "Failed to read invitation" );
                    }
                }

                void closeConnection(){
                    try{
                        serialPort_.close();
                    }catch( std::exception const& ex ){
                        spdlog::get( Symbols::Console() )->error( "close err: {}", ex.what() );
                    }
                }
            };
        }
    }
}