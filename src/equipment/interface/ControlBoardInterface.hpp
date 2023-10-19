#pragma once
#include "Result.hpp"
#include "eq-common/Dimensions.hpp"
#include "ListenerInterface.hpp"

namespace strateam{
    namespace equipment{
        class ControlBoardInterface : public Listenable<ListenerInterface>{
        public:
            virtual ~ControlBoardInterface(){}
            virtual Result setLoadCellCoeffs( double lk, double rk ) = 0;
            virtual Result setZero() = 0;
            virtual Result sendLogsToUart( bool bOn ) = 0;
            virtual MotionResult squeeze( dim::Um const& targetPos, dim::Gram const& minSignalValueToDetect, dim::Gram const& signalDeltaF, dim::MilliSecond const& stableTimeDetection, dim::UmVelocity const& haydonVelocity, dim::UmVelocity const& beamVelocity ) = 0;
            virtual bool isMoving() = 0;
            virtual void stop() = 0;
        };
    }
}