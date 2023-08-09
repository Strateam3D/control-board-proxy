#pragma once
#include "../Tags.hpp"
#include "../BasicEquipmentFacade.hpp"
namespace strateam{
    namespace equipment{
        namespace dlp{
            struct DlpEquipment : public BasicEquipmentFacade<UsbJsonDlp>{
                DlpEquipment( boost::asio::io_context& ctx, rapidjson::Value const& config ) : BasicEquipmentFacade<UsbJsonDlp>( ctx, config ){}
            };
        }
    }
}