#pragma once
#include "../Tags.hpp"
#include "../BasicEquipmentFacade.hpp"
namespace strateam{
    namespace equipment{
        namespace dlp{
            class DlpEquipment : public BasicEquipmentFacade<UsbJsonDlp>{};
        }
    }
}