#pragma once
#include "../interface/AxisInterface.hpp"
#include "TransportSelector.hpp"
#include "BasicMessage.hpp"

#include <boost/asio.hpp>

namespace strateam{
    namespace equipment{
        template<typename Tag>
        class AxisImpl : public AxisInterface{
        public:// == TYPES ==
            using Transport = TransportSelector<Tag>::type;
            using Message = BasicMessage<Tag>;
            using MessageBuilder = MessageBuilderImpl<Tag>::type;
            using PayloadDecoder = PayloadDecodeImpl<Tag>::type;
        public:// == ctor ==
            AxisImpl( Transport& t ) : transport_( t ){}
            AxisImpl( AxisImpl const& ) = delete;
            AxisImpl& operator =( AxisImpl const& ) = delete;

        public:// == AxisInterface ==
            virtual bool isMoving() override{
                auto response = transport_.sendRequestGetResponse( Message( MessageBuilder::isMoving() ) );
                return PayloadDecoder::isMoving( response.getSource() );
            }

            virtual MotionResult move( dim::MotorStep const& offset, double speed, double accel, double decel ) override{
                auto response = transport_.sendRequestGetResponse( Message( MessageBuilder::move( offset, speed, accel, decel ) ) );
                MotionResult motRet = PayloadDecoder::move( response.getSource() );
                handleMotionResult(motret);
                return motret;
            }

            virtual MotionResult moveTo( dim::MotorStep const& target, double speed, double accel, double decel )override{
                auto response = transport_.sendRequestGetResponse( Message(MessageBuilder::moveTo( target, speed, accel, decel )) );
                MotionResult motRet = PayloadDecoder::moveTo( response.getSource() );
                handleMotionResult(motret);
                return motret;
            }

            virtual MotionResult moveZero( double speed, double accel, double decel ) override{
                auto response = transport_.sendRequestGetResponse( Message( MessageBuilder::moveZero( speed, accel, decel ) ) );
                MotionResult motRet = PayloadDecoder::moveZero( response.getSource() );
                handleMotionResult(motret);
                return motret;
            }

            virtual MotionResult moveHome( double speed, double accel, double decel ) override{
                auto response = transport_.sendRequestGetResponse( Message( MessageBuilder::moveHome( speed, accel, decel ) ) );
                MotionResult motRet = PayloadDecoder::moveHome( response.getSource() );
                handleMotionResult(motret);
                return motret;
            }

            virtual void stop() override{
                isMotionDoneTimer_.cancel();
                transport_.sendRequestGetResponse( Message( MessageBuilder::stop() ) );
            }

            virtual dim::MotorStep position() override{
                auto resp = transport_.sendRequestGetResponse( Message( MessageBuilder::position() ) );
                return PayloadDecoder::position( res.getSource() );
            }

            virtual dim::MotorStep homePosition() override{
                return homePosition_;
            }

            virtual void setHomePosition(dim::MotorStep const& pos) override{
                homePosition_ = pos;
            }

        private:
            void startPeriodicalRequestMotionIsDone();
            void handleMotionResult( MotionResult result );
        private:
            boost::asio::io_context&    ctx_;
            Transport&                  transport_;
            boost::asio::deadline_timer isMotionDoneTimer_{ctx_};
            boost::posix_time::millisec isMotionDoneRequestPeriodMs_{300};
            dim:::MotorStep             homePosition_{0.0};
        };
    }
}