#pragma once
#include "interface/ControlBoardInterface.hpp"
#include "TransportSelector.hpp"
#include "IsDlp.hpp"
#include "IsEmulator.hpp"
#include "dlp/DlpControlBoard.hpp"
#include "tm4c/TM4CControlBoard.hpp"
#include "emulator/ControlBoard.hpp"
#include "interface/Result.hpp"
#include "eq-common/Converter.hpp"
#include "BzOffset.hpp"

// std
#include <chrono>

// bosot
#include <boost/asio.hpp>

namespace strateam{
    namespace equipment{
        template <typename TagT>
		struct UnsupportedTag;

        template<typename TagT, typename Enabled = void>
        struct CBSelector {
            using type = UnsupportedTag< TagT >;
        };

        template<typename TagT>
        struct CBSelector<TagT, std::enable_if_t< IsDlpCheck< TagT >::value> > {
            using type = dlp::DlpControlBoard;
        };

        template<typename TagT>
        struct CBSelector<TagT, std::enable_if_t< IsTM4CCheck< TagT >::value> > {
            using type = tm4c::TM4CControlBoard;
        };

        template<typename TagT>
        struct CBSelector<TagT, std::enable_if_t< IsEmulator< TagT >::value> > {
            using type = emulator::ControlBoard;
        };

        template<typename TagT>
        class BasicControlBoard : public ControlBoardInterface, private CBSelector<TagT>::type{
        public:// == TYPES ==
            using Transport = typename TransportSelector<TagT>::type;
            using ControlBoardImpl = typename CBSelector<TagT>::type;
            using F = std::function<void(Result)>;

        private:// == MEMBERs ==
            IoCtx&                      ctx_;
            Transport&                  transport_;
            const double                haydonStepsPerUM_{1.0};
            const double                beamStepsPerUM_{1.0};
            boost::asio::deadline_timer isMotionDoneTimer_{ctx_};
            boost::posix_time::millisec isMotionDoneRequestPeriodMs_{300};
            F                           f_;

            using time_point_milliseconds = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>;
            using milliseconds = std::chrono::milliseconds;
            time_point_milliseconds beginWaitingTimePoint_ = std::chrono::time_point_cast<milliseconds>(std::chrono::system_clock::now());
            milliseconds waitForMotionDoneMaxDuration_ { 300000 };
            bool inverted_{ false };
            BzOffset<dim::MotorStep>&    beamOffset_;
            BzOffset<dim::MotorStep>&    h1Offset_;
            BzOffset<dim::MotorStep>&    h2Offset_;

        public:// == CTOR ==
            BasicControlBoard( IoCtx& ctx, Transport& t, const double haydonStepsPerUM, const double beamStepsPerUM, bool beamInverted, BzOffset<dim::MotorStep>& beamOffset, BzOffset<dim::MotorStep>& h1Offset, BzOffset<dim::MotorStep>& h2Offset )
            : ctx_( ctx )
            , transport_( t )
            , haydonStepsPerUM_( haydonStepsPerUM )
            , beamStepsPerUM_( beamStepsPerUM )
            , inverted_( beamInverted )
            , beamOffset_( beamOffset )
            , h1Offset_( h1Offset )
            , h2Offset_( h2Offset )
            {}

            BasicControlBoard( BasicControlBoard const& ) = delete;
            BasicControlBoard& operator =( BasicControlBoard const& ) = delete;

        public:// == ControlBoardInterface ==
            virtual Result setLoadCellCoeffs( double lk, double rk ) override{
                auto response = transport_.sendRequestGetResponse( ControlBoardImpl::setLoadCellCoeffs( lk, rk ) );
                return ControlBoardImpl::checkErrors( response.getSource() );
            }

            virtual Result setZero() override{
                auto response = transport_.sendRequestGetResponse( ControlBoardImpl::setZero() );
                return ControlBoardImpl::checkErrors( response.getSource() );
            }

            virtual Result sendLogsToUart( bool bOn ) override{
                auto response = transport_.sendRequestGetResponse( ControlBoardImpl::sendLogsToUart( bOn ) );
                return ControlBoardImpl::checkErrors( response.getSource() );
            }

            virtual MotionResult squeeze( int hnum
                , dim::Um const& beamOffset
                , dim::UmVelocity const& beamVelocity 
                , dim::Um const& haydonOffset
                , dim::UmVelocity const& haydonVelocity ) override{
                if( f_ )
                    return MotionResult::AlreadyMoving;

                f_ = [this]( MotionResult ret ){ notify( &ListenerInterface::squeezingDone, ret ); };
                dim::MotorStep beamOffsetMs = dim::DimensionConverter<dim::MotorStep>::apply( inverted_ ? beamOffset.neg() : beamOffset, beamStepsPerUM_ );
                dim::MotorStepVelocity bvMS = dim::DimensionConverter< dim::MotorStepVelocity >::apply( beamVelocity, beamStepsPerUM_ );
                dim::MotorStep haydonOffsetMs = dim::DimensionConverter<dim::MotorStep>::apply( inverted_ ? haydonOffset.neg() : haydonOffset, haydonStepsPerUM_ );
                dim::MotorStepVelocity hvMS = dim::DimensionConverter< dim::MotorStepVelocity >::apply( haydonVelocity, haydonStepsPerUM_ );
                beamOffset_.addOffset( beamOffsetMs );

                if( hnum == 0 ) h1Offset_.addOffset( haydonOffsetMs );
                else h2Offset_.addOffset( haydonOffsetMs );

                dim::MotorStep  zBOffset = beamOffset_.offset();
                dim::MotorStep zHOffset = hnum == 0 ? h1Offset_.offset() : h2Offset_.offset();
                MotionResult motret;
                int attempts = 2;
                
                do{
                    auto response = transport_.sendRequestGetResponse( ControlBoardImpl::squeeze( 
                        hnum,
                        zBOffset.castTo<int>(),
                        bvMS.castTo<int>(),
                        zHOffset.castTo<int>(),
                        hvMS.castTo<int>()) 
                    );
                    motret = ControlBoardImpl::handleRespondCommandGo( response.getSource() );
                    handleMotionResult(motret);

                    if( motret != MotionResult::ParseError )
                        break;
                    else
                        spdlog::get( Symbols::Console() )->warn( "parse error, retries left {}", attempts );
                    
                    std::this_thread::sleep_for( std::chrono::milliseconds{200} );
                }while( attempts -- > 0 );

                return motret;
            }

            virtual void stop() override{
                isMotionDoneTimer_.cancel();
                transport_.sendRequestGetResponse( ControlBoardImpl::stop() );
            }

            virtual bool isMoving() override{
                auto response = transport_.sendRequestGetResponse( ControlBoardImpl::isMoving() );
                return ControlBoardImpl::template into<typename ControlBoardImpl::Moving>( response.getSource() );
            }
        private:
            void handleMotionResult( MotionResult result ){
                if( result ){
                    if( result == MotionResult::Success || result == MotionResult::Accepted)
                        startPeriodicalRequestMotionIsDone();
                    else if( result == MotionResult::AlreadyInPosition )
                        f_ = {};
                }else{
                    f_ = {};
                }
            }

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

            void isMotionDonePeriodicalCallback( boost::system::error_code err ){
                spdlog::get( Symbols::Console() )->debug( "CBisMotionDonePeriodicalCallback");
                using boost::system::error_code;

                if(!err){
                    try{
                        auto retval = isMoving();
                        spdlog::get( Symbols::Console() )->debug( "retval {}", retval);

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
                            
                            if( f_ )
                                f_( MotionResult::Success );
                            else
                                spdlog::get( Symbols::Console() )->warn( "no callback provided {}", retval );
                          
                            f_ = {};    
                        }
                    }catch(std::exception const& ex){
                        spdlog::get( Symbols::Console() )->debug( "poll err {}", ex.what());
                        f_(MotionResult::FAILED);
                        f_ = {};
                        stop();
                    }
                }
            }
        };
    }
}