#pragma once
#include <type_traits>
#include "Tags.hpp"

namespace strateam{
    namespace equipment{
        template<typename Tag, typename Enabled = void>
        struct IsSerial : std::false_type{};

        template<typename Tag> 
        struct IsSerial<Tag, std::enable_if_t< Tag::IsSerial::value >> : std::true_type{};
    }
}