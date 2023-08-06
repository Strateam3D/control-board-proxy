#pragma once
#include <type_traits>
#include "Tags.hpp"

namespace strateam{
    namespace equipment{
        template<typename Tag, typename Enabled = void>
        struct IsUsb : std::false_type{};

        template<typename Tag> 
        struct IsUsb<Tag, std::enable_if_t< Tag::IsUsb::value >> : std::true_type{};
    }
}