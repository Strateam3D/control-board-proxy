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
        class BasicAxis : public AxisInterface, protected AxisSelector<TagT>::type{
        public:// == TYPES ==
            using Transport = typename TransportSelector<TagT>::type;
            using AxisImpl = typename AxisSelector<TagT>::type;
            using F = void(*)(MotionResult);
        public:// == ctor ==
            BasicAxis( std::size_t axisId, Transport& t ) : AxisImpl( axisId ), transport_( t ){}
            BasicAxis( AxisImpl const& ) = delete;
            BasicAxis& operator =( BasicAxis const& ) = delete;

        public:// == AxisInterface ==
            virtual bool isMoving() override{
                auto response = transport_.sendRequestGetResponse( AxisImpl::isMoving() );
                return AxisImpl::into<AxisImpl::Moving>( response.getSource() );
            }

            virtual MotionResult move( dim::MotorStep const& offset, double speed, double accel, double decel ) override{
                f_ = &ListenerIntarface::motionDone;
                auto response = transport_.sendRequestGetResponse( AxisImpl::move( offset, speed, accel, decel ) );
                MotionResult motret = AxisImpl::handleRespondCommandGo( response.getSource() );
                handleMotionResult(motret);
                return motret;
            }

            virtual MotionResult moveTo( dim::MotorStep const& target, double speed, double accel, double decel )override{
                f_ = &ListenerIntarface::motionToDone;
                auto response = transport_.sendRequestGetResponse( AxisImpl::moveTo( target, speed, accel, decel ) );
                MotionResult motret = AxisImpl::handleRespondCommandGo( response.getSource() );
                handleMotionResult(motret);
                return motret;
            }

            virtual MotionResult moveZero( double speed, double accel, double decel ) override{
                f_ = &ListenerIntarface::moveToZeroDone;
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
                return AxisImpl::into<AxisImpl::MotorStep>( resp.getSource() );
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
                    if( result == MotionResult::Success || result == MotionResult::Accepted)
                        startPeriodicalRequestMotionIsDone();
                }
            }

            void isMotionDonePeriodicalCallback( boost::system::error_code err ){
                using boost::system::error_code;
                
                if(!err){
                    try{
                        if( isMoving() ){  
                            auto waitingForMotionDoneDuration = std::chrono::system_clock::now() - beginWaitingTimePoint_;
                            
                            if( waitingForMotionDoneDuration > waitForMotionDoneMaxDuration_ ){
                                try{
                                    stop();
                                }catch(...){}
                                
                                notify( f_, MotionResult::Timeout );
                            }else{  // continue requesting
                                isMotionDoneTimer_.expires_from_now( isMotionDoneRequestPeriodMs_ );
                                isMotionDoneTimer_.async_wait( [this]( error_code err ){ 
                                    this->isMotionDonePeriodicalCallback( err ); 
                                } );
                            }
                        }else{ // 
                            notify( f_, MotionResult::Success );
                        }
                    }catch(...){}
                }else{  // if canceled, expected this is done from stop()
                    notify( f_, MotionResult::Stopped );
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