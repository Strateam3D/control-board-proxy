#pragma once
#include "../Tags.hpp"
#include "../BasicEquipmentFacade.hpp"
namespace strateam{
    namespace equipment{
        namespace dlp{
            struct DlpEquipment : public BasicEquipmentFacade<UsbJsonDlp>{
                DlpEquipment( IoCtx& ctx, rapidjson::Value const& config ) : BasicEquipmentFacade<UsbJsonDlp>( ctx, config ){}
            };
        }
    }
}