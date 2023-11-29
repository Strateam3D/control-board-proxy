#pragma once
#include "Listenable.hpp"
#include "MotionResult.hpp"
#include "ListenerInterface.hpp"
#include "eq-common/Dimensions.hpp"

namespace strateam{
    namespace equipment{
        class AxisInterface : public Listenable<ListenerInterface>{
        public:
            virtual ~AxisInterface(){}
            virtual bool isMoving() = 0;
            virtual MotionResult move( dim::Um const& offset, dim::UmVelocity speed, double accel=-1, double decel=-1 ) = 0;
            virtual MotionResult moveTo( dim::Um const& target, dim::UmVelocity speed , double accel=-1, double decel=-1 ) = 0;
            virtual MotionResult moveTo( dim::MotorStep const& target, double speed=-1, double accel=-1, double decel=-1 ) = 0;
            virtual MotionResult moveZero( dim::UmVelocity speed, double accel=-1, double decel=-1 ) = 0;
            virtual MotionResult moveHome( dim::UmVelocity speed, double accel=-1, double decel=-1 ) = 0;
            virtual void stop() = 0;
            virtual dim::Um position() = 0;
            virtual dim::Um homePosition() = 0;
            virtual void setHomePosition(dim::Um const& pos) = 0;
        };
    }
}
