#pragma once
#include "Listenable.hpp"
#include "MotionResult.hpp"
#include "ListenerInterface.hpp"
#include "eq-common/Dimensions.hpp"

namespace strateam{
    namespace equipment{
        class AxisInterface : public Listenable<ListenerIntarface>{
        public:
            virtual ~AxisInterface(){}
            virtual bool isMoving() = 0;
            virtual MotionResult move( dim::MotorStep const& offset, double speed=-1, double accel=-1, double decel=-1 ) = 0;
            virtual MotionResult moveTo( dim::MotorStep const& target, double speed=-1, double accel=-1, double decel=-1 ) = 0;
            virtual MotionResult moveZero( double speed=-1, double accel=-1, double decel=-1 ) = 0;
            virtual MotionResult moveHome( double speed=-1, double accel=-1, double decel=-1 ) = 0;
            virtual void stop() = 0;
            virtual dim::MotorStep position() = 0;
            virtual dim::MotorStep homePosition() = 0;
            virtual void setHomePosition(dim::MotorStep const& pos) = 0;
        };
    }
}