#pragma once
#include "../Tags.hpp"
#include "../BasicEquipmentFacade.hpp"
namespace strateam{
    namespace equipment{
        namespace tm4c{
            struct TM4CEquipment : public BasicEquipmentFacade<UsbJsonTM4C>{
                TM4CEquipment( IoCtx& ctx, rapidjson::Value const& config ) : BasicEquipmentFacade<UsbJsonTM4C>( ctx, config ){}
            };
        }
    }
}