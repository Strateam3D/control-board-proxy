#pragma once
#include "../Tags.hpp"
#include "../BasicEquipmentFacade.hpp"
namespace strateam{
    namespace equipment{
        namespace dlp{
            struct DlpEquipment : public BasicEquipmentFacade<UsbJsonDlp>{
                DlpEquipment( rapidjson::Value const& config ) : BasicEquipmentFacade<UsbJsonDlp>( config ){}
            };
        }
    }
}