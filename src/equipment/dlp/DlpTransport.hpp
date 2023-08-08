#pragma once
#include "BasicMessageWrapper.hpp"
#include "EqException.hpp"

#include "rapidjson/document.h"

#include <iostream>
#include <mutex>
#include "serial_port/AsyncSerialPort.hpp"

namespace strateam{
    namespace equipment{
        template<typename Tag>
        class DlpTransport{
        public:// == TYPES ==
            using MessageWrapper = BasicMessageWrapper<Tag>;
            using SourceType = typename MessageWrapper::SourceType;
            using ByteArray = std::vector<char>;

        private:// == MEMBERS ==
            ByteArray ba_;
            std::mutex mtx_;
            std::condition_variable cv_;
            AsyncSerialPort serialPort_;
            int btc_;
        public:// == CTOR ==
            DlpTransport( rapidjson::Value const& config ){
                setConfig(config);
                openConnection();
                readAll();
         
                if( !findRespondTerminator( ba_ ) ){
                    std::cout << "Did not receive invite message." << std:::endl;
                }

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
                static constexpr char* prefix = "ncd-json  ";
                static constexpr char* suffix = "\n";
                std::vector<char> ba(prefix, prefix + sizeof( prefix ));
                ba.insert( ba.end(),  msg.data(), msg.data() + msg.size() );
                ba.insert( ba.end(), suffix, suffix + sizeof( suffix ) );
                std::unique_lock<std::mutex> _( mtx_ );
                serialPort_.write( ba );
                
                if( cv_.wait_for( _, std::chrono::milliseconds( 5000 ) ) != std::cv_status::no_timeout ){
                    return MessageWrapper();
                }

                return MessageWrapper::fromData( ba_.data() ).decode();
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
        };
    }
}