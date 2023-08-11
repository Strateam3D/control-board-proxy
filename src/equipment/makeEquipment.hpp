#pragma once

#include "rapidjson/document.h"
#include <memory>

#include "../Global.hpp"

namespace strateam{
    namespace equipment{
        class EquipmentInterface;

        auto makeEquipment(IoCtx& ctx, rapidjson::Value const& config ) -> std::unique_ptr< EquipmentInterface >;
    }
}
