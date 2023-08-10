#pragma once

#include "rapidjson/document.h"
#include <memory>
#include <boost/asio/io_context.hpp>

namespace strateam{
    namespace equipment{
        class EquipmentInterface;

        auto makeEquipment(boost::asio::io_context& ctx, rapidjson::Document const& config ) -> std::unique_ptr< EquipmentInterface >;
    }
}
