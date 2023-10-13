#pragma once
#include "../Global.hpp"

namespace strateam{
    namespace equipment{
        class AxisInterface;
        class ControlBoardInterface;        

        class EquipmentInterface{
        public:
            virtual ~EquipmentInterface(){}
            virtual AxisInterface& axis( AxisType axis ) = 0;
            virtual ControlBoardInterface& controlBoard() = 0;
        };
    }
}