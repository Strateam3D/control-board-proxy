#pragma once
#include "../Tags.hpp"
#include "../BasicEquipmentFacade.hpp"
namespace strateam{
    namespace equipment{
        namespace dlp{
            struct DlpEquipment : public BasicEquipmentFacade<SerialJsonDlp>{
                DlpEquipment( IoCtx& ctx, rapidjson::Value const& config ) : BasicEquipmentFacade<SerialJsonDlp>( ctx, config ){}
            };
        }
    }
}