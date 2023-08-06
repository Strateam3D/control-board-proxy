#pragma once
#include "MotionResult.hpp"

namespace strateam{
    namespace equipment{
        class ListenerIntarface{
        public:
            virtual ~ListenerIntarface(){}
            virtual void motionDone( MotionResult motret ) = 0;
            virtual void motionToDone( MotionResult motret ) = 0;
            virtual void moveHomeDone(MotionResult motret) = 0;
            virtual void moveToZeroDone( MotionResult motret ) = 0;
        };
    }
}