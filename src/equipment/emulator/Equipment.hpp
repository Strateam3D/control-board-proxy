#pragma once
#include "../Tags.hpp"
#include "../BasicEquipmentFacade.hpp"

namespace strateam{
    namespace equipment{
        namespace emulator{
            struct Equipment : public BasicEquipmentFacade<EmuJson>{
                Equipment( IoCtx& ctx, rapidjson::Value const& config ) : BasicEquipmentFacade<EmuJson>( ctx, config ){}
            };
        }
    }
}