#pragma once
#include "../BasicMessageWrapper.hpp"
#include "../EqException.hpp"
#include "Symbols.hpp"
#include "spdlog/spdlog.h"

#include "rapidjson/document.h"

#include <iostream>
#include <mutex>
#include <condition_variable>

#include <libusb-1.0/libusb.h>

namespace strateam{
    namespace equipment{
        namespace tm4c{
            template<typename Tag>
            class USBTransport{
            public:// == TYPES ==
                using MessageWrapper = typename BasicMessageWrapper<Tag>::type;
                using SourceType = typename MessageWrapper::SourceType;
                using ByteArray = std::vector<char>;

                static constexpr int VID = 0x1CBE;// generic texas instruments bulk device
                static constexpr int PID = 0x0003;// generic texas instruments bulk device

            private:// == MEMBERS ==
                libusb_device_handle *device_handle_{nullptr};
                uint vid_{ VID };
                uint pid_{ PID };
            public:// == CTOR ==
                USBTransport( rapidjson::Value const& config ){
                    libusb_init(nullptr);
                    setConfig(config);
                    openConnection();
                }

                USBTransport( USBTransport const& ) = delete;
                
                USBTransport& operator =( USBTransport const& ) = delete;
                
                ~USBTransport(){
                    closeConnection();
                    libusb_exit(nullptr);
                    spdlog::get( Symbols::Console() )->debug( "~USBTransport" );
                }

            public:// == DlpTransport ==
                MessageWrapper sendRequestGetResponse( SourceType&& request ){
                    return sendRawRequestGetResponse( MessageWrapper::fromSource( request ).encode() );
                }

            private:
                MessageWrapper sendRawRequestGetResponse( MessageWrapper const& msg ){
                    std::vector<char> ba(msg.data(), msg.data() + msg.size());
                    static const std::size_t AlignSize = 32;

                    if( ba.size() % AlignSize != 0 ){
                        auto sz = ba.size();
                        auto integral = sz / AlignSize;
                        auto diff = AlignSize * ( integral + 1 ) - sz;
                        spdlog::get( Symbols::Console() )->debug( "requestData not aligned to {} -> {}, append {} bytes", AlignSize, sz, diff );
                        ba.insert( ba.end(),  diff, ' ' );
                        assert( ba.size() <= 128 );
                    }
                    ByteArray badbg( ba );
                    badbg.push_back('\0');
                
                    spdlog::get( Symbols::Console() )->debug( "request {}, size {}", badbg.data(), badbg.size());
                    return sendRawRequestGetResponse( ba );
                }

                MessageWrapper sendRawRequestGetResponse( ByteArray const& ba ){
                    unsigned char endpointOUT = 1 | LIBUSB_ENDPOINT_OUT;
                    unsigned char endpointIN = 1 | LIBUSB_ENDPOINT_IN;
                    int actual_length = 0;
                    uint timeoutOUT = 1000;
                    uint timeoutIN = 1000;
                    
                    int errcode = libusb_bulk_transfer( device_handle_, endpointOUT, (unsigned char*)ba.data(), ba.size(), &actual_length, timeoutOUT);

                    if (LIBUSB_SUCCESS != errcode){
                        spdlog::get( Symbols::Console() )->error( "Error libusb_bulk_transfer {}, size: {}, req size: {}", libusb_error_name(errcode), actual_length, ba.size());
                    }else{
                        static const int PACKET_SIZE = 128;
                        std::vector<char> responseData( PACKET_SIZE, 0 );
                        actual_length = 0;
                        int errcode = libusb_bulk_transfer( device_handle_, endpointIN, (unsigned char*)responseData.data(), PACKET_SIZE, &actual_length, timeoutIN );

                        if( LIBUSB_SUCCESS != errcode ){
                            spdlog::get( Symbols::Console() )->error( "Error libusb_bulk_transfer get: {}, size: {}, asked for {}", libusb_error_name(errcode), actual_length, PACKET_SIZE );
                        }else{
                            responseData.push_back( '\0' );
                            spdlog::get( Symbols::Console() )->debug( "response {}, size: {}", responseData.empty() ? "EMPTY" : responseData.data(), responseData.size());
                            return MessageWrapper::fromData( responseData.data() ).decode();
                        }
                    }

                    return MessageWrapper();
                }

                void setConfig( rapidjson::Value const& config ){
                    auto maybeSetFromConfig = [&]( auto& toVal, auto && path ){
                        using namespace rapidjson;
                        const Value *v = GetValueByPointer( config, path );
                        if( v and v->Is<std::decay_t<decltype(toVal)>>() )
                            toVal = v->Get<std::decay_t<decltype(toVal)>>();
                    };
                    
                    maybeSetFromConfig( vid_, "/vid" );
                    maybeSetFromConfig( pid_, "/pid" );
                }

                void openConnection(){
                    spdlog::get( Symbols::Console() )->debug( "vid: {}, pid: {}", vid_, pid_ );
                    device_handle_ = libusb_open_device_with_vid_pid(nullptr, vid_, pid_ );

                    if(device_handle_){
                        if( libusb_kernel_driver_active( device_handle_, 0 ) == 1) { //find out if kernel driver is attached
                            spdlog::get( Symbols::Console() )->warn( "Kernel Driver Active" );

                            if( libusb_detach_kernel_driver( device_handle_, 0 ) == 0 )  //detach it
                                spdlog::get( Symbols::Console() )->warn( "Kernel Driver Detached!" );
                        }

                        int r = libusb_claim_interface( device_handle_, 0 ); //claim interface 0 (the first) of device (mine had jsut 1)

                        if(r < 0) {
                            throw std::runtime_error("Cannot Claim Interface");
                        }
                    }else{
                        throw std::runtime_error( "Failed to open tm4c device" );
                    }
                }

                
                void closeConnection(){
                    if (device_handle_) {
                        int r = libusb_release_interface(device_handle_, 0); //release the claimed interface

                        if(r != 0 && r != LIBUSB_ERROR_NO_DEVICE)
                            spdlog::get( Symbols::Console() )->error( "Cannot Release Interface: {}", libusb_error_name( r ) );
                        else
                            libusb_close(device_handle_); // crash!

                        device_handle_ = nullptr;
                    }
                }
            };
        }
    }
}