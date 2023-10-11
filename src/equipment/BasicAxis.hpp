#pragma once
#include "interface/AxisInterface.hpp"
#include "TransportSelector.hpp"
#include "AxisSelector.hpp"
#include "eq-common/Converter.hpp"
// boost
#include "Global.hpp"

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
            BasicAxis( IoCtx& ctx, std::string const& axisName, Transport& t, const double stepsPerUm ) 
            : AxisImpl( axisName )
            , ctx_( ctx )
            , transport_( t )
            , stepsPerUm_( stepsPerUm )
            {}

            BasicAxis( AxisImpl const& ) = delete;
            BasicAxis& operator =( BasicAxis const& ) = delete;

        public:// == AxisInterface ==
            virtual bool isMoving() override{
                auto response = transport_.sendRequestGetResponse( AxisImpl::isMoving() );
                return AxisImpl::template into<typename AxisImpl::Moving>( response.getSource() );
            }

            // FIXME: check is moving otherwise callback will be rewritten
            virtual MotionResult move( dim::Um const& offset, dim::UmVelocity speed, double , double  ) override{
                if( f_ )
                    return MotionResult::AlreadyMoving;

                f_ = [this]( MotionResult ret ){ notify( &ListenerInterface::motionDone, ret ); };
                dim::MotorStep offMs = dim::DimensionConverter<dim::MotorStep>::apply( offset, stepsPerUm_ );
                dim::MotorStep dExposureDistance = offMs + stepResidual_;
                dim::MotorStep dist{0.0};
                stepResidual_ = ModF( dExposureDistance, dist );
                dim::MotorStepVelocity vMS = dim::DimensionConverter<dim::MotorStepVelocity>::apply( speed, stepsPerUm_ );
                auto response = transport_.sendRequestGetResponse( AxisImpl::move( dist, vMS.value() ) );
                MotionResult motret = AxisImpl::handleRespondCommandGo( response.getSource() );
                std::cout << "ret: " << motret.str() << std::endl;
                handleMotionResult(motret);
                return motret;
            }

            virtual MotionResult moveTo( dim::Um const& target,dim::UmVelocity speed, double, double )override{
                if( f_ )
                    return MotionResult::AlreadyMoving;

                dim::MotorStep tarMs = dim::DimensionConverter<dim::MotorStep>::apply( target, stepsPerUm_ );
                dim::MotorStepVelocity vMS = dim::DimensionConverter<dim::MotorStepVelocity>::apply( speed, stepsPerUm_ );
                f_ = [this]( MotionResult ret ){ notify( &ListenerInterface::motionDone, ret ); };
                auto response = transport_.sendRequestGetResponse( AxisImpl::moveTo( tarMs, vMS.value() ) );
                MotionResult motret = AxisImpl::handleRespondCommandGo( response.getSource() );
                handleMotionResult(motret);
                return motret;
            }

            virtual MotionResult moveTo( dim::MotorStep const& target, double speed, double , double )override{
                if( f_ )
                    return MotionResult::AlreadyMoving;

                f_ = [this]( MotionResult ret ){ notify( &ListenerInterface::motionDone, ret ); };
                auto response = transport_.sendRequestGetResponse( AxisImpl::moveTo( target, speed ) );
                MotionResult motret = AxisImpl::handleRespondCommandGo( response.getSource() );
                handleMotionResult(motret);
                return motret;
            }

            virtual MotionResult moveZero( dim::UmVelocity speed, double , double  ) override{
                if( f_ )
                    return MotionResult::AlreadyMoving;

                dim::MotorStepVelocity vMS = dim::DimensionConverter<dim::MotorStepVelocity>::apply( speed, stepsPerUm_ );
                f_ = [this]( MotionResult ret ){ notify( &ListenerInterface::moveToZeroDone, ret ); };
                auto response = transport_.sendRequestGetResponse( AxisImpl::moveZero( vMS.value() ) );
                MotionResult motret = AxisImpl::handleRespondCommandGo( response.getSource() );
                handleMotionResult(motret);
                return motret;
            }

            virtual MotionResult moveHome( dim::UmVelocity speed, double accel, double decel ) override{
                dim::MotorStepVelocity vMS = dim::DimensionConverter<dim::MotorStepVelocity>::apply( speed, stepsPerUm_ );
                
                f_ = [this]( MotionResult ret ){ notify( &ListenerInterface::moveHomeDone, ret ); };
                auto response = transport_.sendRequestGetResponse( AxisImpl::moveTo( homePosition_, vMS.value() ) );
                MotionResult motret = AxisImpl::handleRespondCommandGo( response.getSource() );
                handleMotionResult(motret);
                return motret;
            }

            virtual void stop() override{
                isMotionDoneTimer_.cancel();
                transport_.sendRequestGetResponse( AxisImpl::stop() );
            }

            virtual dim::Um position() override{
                auto resp = transport_.sendRequestGetResponse( AxisImpl::position() );
                dim::MotorStep posMS = AxisImpl::template into<dim::MotorStep>( resp.getSource() );
                dim::Um posUM = dim::DimensionConverter<dim::Um> :: apply( posMS, stepsPerUm_ );
                return posUM;
            }

            virtual dim::Um homePosition() override{
                return dim::DimensionConverter<dim::Um> :: apply( homePosition_, stepsPerUm_ );
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
                    else if( result == MotionResult::AlreadyInPosition )
                        f_ = {};
                }else{
                    std::cout << "olololo: " << result.str() << std::endl;
                    f_ = {};
                }
            }

            void isMotionDonePeriodicalCallback( boost::system::error_code err ){
                std::cout << __func__ << std::endl;
                using boost::system::error_code;
                std::cout << err.message() << std::endl;
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
                            isMotionDoneTimer_.cancel();
                            f_( MotionResult::Success );
                            f_ = {};    
                        }
                    }catch(std::exception const& ex){
                        std::cout << "poll err: " << ex.what() << std::endl;
                        f_(MotionResult::FAILED);
                        f_ = {};
                        stop();
                    }
                }
                // else{  // if canceled, expected this is done from stop()
                //     f_( MotionResult::Stopped );
                //     f_ = {};
                // }
            }
        private:
            IoCtx&                      ctx_;
            Transport&                  transport_;
            const double                stepsPerUm_{1.0};
            boost::asio::deadline_timer isMotionDoneTimer_{ctx_};
            boost::posix_time::millisec isMotionDoneRequestPeriodMs_{300};
            dim::MotorStep             homePosition_{0.0};
            F                           f_;

            using time_point_milliseconds = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>;
            using milliseconds = std::chrono::milliseconds;
            time_point_milliseconds beginWaitingTimePoint_ = std::chrono::time_point_cast<milliseconds>(std::chrono::system_clock::now());
            milliseconds waitForMotionDoneMaxDuration_ { 300000 };

            dim::MotorStep                  stepResidual_{0.0};
        };
    }
}