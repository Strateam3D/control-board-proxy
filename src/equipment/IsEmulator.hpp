#pragma once
#include <type_traits>
#include "Tags.hpp"

namespace strateam{
    namespace equipment{
        template<typename Tag, typename Enabled = void>
        struct IsEmulator : std::false_type{};

        template<typename Tag> 
        struct IsEmulator<Tag, std::enable_if_t< Tag::IsEmu::value >> : std::true_type{};
    }
}