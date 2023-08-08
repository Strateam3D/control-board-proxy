#pragma once
#include "../BasicMessageWrapper.hpp"
#include "../EqException.hpp"

#include "rapidjson/document.h"

#include <iostream>
#include <mutex>
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
                    std::unique_lock<std::mutex> _( mtx_ );
                    readAll();
                    sendRawRequestGetResponse( MessageWrapper::fromData( "ncd-json  /info \n" ).encode() );
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
                    static constexpr char const* prefix = "ncd-json  ";
                    static constexpr char const* suffix = "\n";
                    std::vector<char> ba(prefix, prefix + sizeof( prefix ));
                    ba.insert( ba.end(),  msg.data(), msg.data() + msg.size() );
                    ba.insert( ba.end(), suffix, suffix + sizeof( suffix ) );
                    std::unique_lock<std::mutex> _( mtx_ );
                    serialPort_.write( ba );
                    
                    if( cv_.wait_for( _, std::chrono::milliseconds( 5000 ) ) == std::cv_status::no_timeout ){
                        return MessageWrapper::fromData( ba_.data() ).decode();
                    }else{
                        std::cout << "Timeout for request " << msg.data() << std::endl;
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
                    std::cout << "response: " << (ba_.empty() ? "EMPTY" : ba_.data() ) << ", promtInvitationFound: " << promtInvitationFound << std::endl;
                    
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
                    std::cout << __func__ << " device: " << deviceName_ << ", baud rate: " << baudRate_ << std::endl;
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
                    
                    if( cv_.wait_for( _, std::chrono::milliseconds( 5000 ) ) == std::cv_status::no_timeout ){
                        std::cout << __func__ << std::string( ba_.data(), ba_.size() ) << std::endl;
                    }else{
                        std::cout << "Failed to read invitation" << std::endl;
                    }
                }

                void closeConnection(){
                    serialPort_.close();
                }
            };
        }
    }
}