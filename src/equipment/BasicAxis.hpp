#pragma once
#include "interface/AxisInterface.hpp"
#include "TransportSelector.hpp"
#include "AxisSelector.hpp"

// boost
#include <boost/asio.hpp>

// std
#include <chrono>

namespace strateam{
    namespace equipment{
        template<typename TagT>
        class BasicAxis : public AxisInterface, private AxisSelector<TagT>::type{
        public:// == TYPES ==
            using Transport = typename TransportSelector<TagT>::type;
            using AxisImpl = typename AxisSelector<TagT>::type;
            using F = std::function<void(MotionResult)>;
        public:// == ctor ==
            BasicAxis( boost::asio::io_context& ctx, std::size_t axisId, Transport& t ) : AxisImpl( axisId ), ctx_( ctx ), transport_( t ){}
            BasicAxis( AxisImpl const& ) = delete;
            BasicAxis& operator =( BasicAxis const& ) = delete;

        public:// == AxisInterface ==
            virtual bool isMoving() override{
                auto response = transport_.sendRequestGetResponse( AxisImpl::isMoving() );
                return AxisImpl::template into<typename AxisImpl::Moving>( response.getSource() );
            }

            // FIXME: check is moving otherwise callback will be rewritten
            virtual MotionResult move( dim::MotorStep const& offset, double speed, double accel, double decel ) override{
                if( f_ )
                    return MotionResult::AlreadyMoving;

                f_ = [this]( MotionResult ret ){ notify( &ListenerIntarface::motionDone, ret ); };
                auto response = transport_.sendRequestGetResponse( AxisImpl::move( offset, speed, accel, decel ) );
                MotionResult motret = AxisImpl::handleRespondCommandGo( response.getSource() );
                std::cout << "ret: " << motret.str() << std::endl;
                handleMotionResult(motret);
                return motret;
            }

            virtual MotionResult moveTo( dim::MotorStep const& target, double speed, double accel, double decel )override{
                if( f_ )
                    return MotionResult::AlreadyMoving;

                f_ = [this]( MotionResult ret ){ notify( &ListenerIntarface::motionDone, ret ); };
                auto response = transport_.sendRequestGetResponse( AxisImpl::moveTo( target, speed, accel, decel ) );
                MotionResult motret = AxisImpl::handleRespondCommandGo( response.getSource() );
                handleMotionResult(motret);
                return motret;
            }

            virtual MotionResult moveZero( double speed, double accel, double decel ) override{
                if( f_ )
                    return MotionResult::AlreadyMoving;

                f_ = [this]( MotionResult ret ){ notify( &ListenerIntarface::moveToZeroDone, ret ); };
                auto response = transport_.sendRequestGetResponse( AxisImpl::moveZero( speed, accel, decel ) );
                MotionResult motret = AxisImpl::handleRespondCommandGo( response.getSource() );
                handleMotionResult(motret);
                return motret;
            }

            virtual MotionResult moveHome( double speed, double accel, double decel ) override{
                return moveTo( homePosition_, speed, accel, decel );
            }

            virtual void stop() override{
                isMotionDoneTimer_.cancel();
                transport_.sendRequestGetResponse( AxisImpl::stop() );
            }

            virtual dim::MotorStep position() override{
                auto resp = transport_.sendRequestGetResponse( AxisImpl::position() );
                return AxisImpl::template into<dim::MotorStep>( resp.getSource() );
            }

            virtual dim::MotorStep homePosition() override{
                return homePosition_;
            }

            virtual void setHomePosition(dim::MotorStep const& pos) override{
                homePosition_ = pos;
            }
        private:
            void startPeriodicalRequestMotionIsDone(){
                using namespace std::chrono;
                using namespace boost::posix_time;
                using boost::system::error_code;
                beginWaitingTimePoint_ = time_point_cast<milliseconds>( system_clock::now() );
                isMotionDoneTimer_.cancel();
                isMotionDoneTimer_.expires_from_now( isMotionDoneRequestPeriodMs_ );
                isMotionDoneTimer_.async_wait( [this](error_code err){ 
                    this->isMotionDonePeriodicalCallback( err ); 
                } );
            }

            void handleMotionResult( MotionResult result ){
                if( result ){
                    std::cout << "22222 result: " << result.str() << std::endl;
                    if( result == MotionResult::Success || result == MotionResult::Accepted)
                        startPeriodicalRequestMotionIsDone();
                }else{
                    std::cout << "olololo: " << result.str() << std::endl;
                }
            }

            void isMotionDonePeriodicalCallback( boost::system::error_code err ){
                std::cout << __func__ << std::endl;
                using boost::system::error_code;
                
                if(!err){
                    try{
                        auto retval = isMoving();
                        std::cout << "retval: " << retval << std::endl;

                        if( retval ){  
                            auto waitingForMotionDoneDuration = std::chrono::system_clock::now() - beginWaitingTimePoint_;
                            
                            if( waitingForMotionDoneDuration > waitForMotionDoneMaxDuration_ ){
                                try{
                                    stop();
                                }catch(...){}
                                
                                f_(MotionResult::Timeout);
                                f_ = {};
                            }else{  // continue requesting
                                isMotionDoneTimer_.expires_from_now( isMotionDoneRequestPeriodMs_ );
                                isMotionDoneTimer_.async_wait( [this]( error_code err ){ 
                                    this->isMotionDonePeriodicalCallback( err ); 
                                } );
                            }
                        }else{ //
                            f_( MotionResult::Success );
                            f_ = {};
                        }
                    }catch(std::exception const& ex){
                        std::cout << "poll err: " << ex.what() << std::endl;
                    }
                }else{  // if canceled, expected this is done from stop()
                    f_( MotionResult::Stopped );
                    f_ = {};
                }
            }
        private:
            boost::asio::io_context&    ctx_;
            Transport&                  transport_;
            boost::asio::deadline_timer isMotionDoneTimer_{ctx_};
            boost::posix_time::millisec isMotionDoneRequestPeriodMs_{300};
            dim::MotorStep             homePosition_{0.0};
            F                           f_;

            using time_point_milliseconds = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>;
            using milliseconds = std::chrono::milliseconds;
            time_point_milliseconds beginWaitingTimePoint_ = std::chrono::time_point_cast<milliseconds>(std::chrono::system_clock::now());
            milliseconds waitForMotionDoneMaxDuration_ { 300000 };
        };
    }
}