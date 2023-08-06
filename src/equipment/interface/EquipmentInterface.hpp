#pragma once
#include "../Global.hpp"

namespace strateam{
    namespace equipment{
        class AxisInterface;

        class EquipmentInterface{
        public:
            virtual ~EquipmentInterface(){}
            virtual AxisInterface& axis( AxisType axis ) = 0;
        };
    }
}