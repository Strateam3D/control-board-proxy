#pragma once
#include "interface/EquipmentInterface.hpp"
#include "TransportSelector.hpp"
#include "BasicAxis.hpp"
// std
#include <unordered_map>
#include <memory>

namespace strateam{
    namespace equipment{
        template<typename Tag>
        class BasicEquipmentFacade : public EquipmentInterface{
        public:// == TYPEs ==
            using Transport = TransportSelector<Tag>::type;
            using Axis = BasicAxis<Tag>;
            using AxisPtr = std::unique_ptr<Axis>;
            using Axises = std::unordered_map<AxisType, AxisPtr>;

        public:// == EquipmentInterafce ==
            virtual AxisInterface& axis( AxisType axis ) override;
        private:
            Transport   transport_;
            Axises      axises_;
        };
    }
}