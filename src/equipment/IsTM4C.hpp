#pragma once
#include "Tags.hpp"
#include <type_traits>

namespace strateam{
    namespace equipment{
        template<typename Tag, typename Enabled = void>
        struct IsTM4CCheck : std::false_type{};

        template<typename Tag> 
        struct IsTM4CCheck<Tag, std::enable_if_t< Tag::IsTM4CBoard::value >> : std::true_type{};
    }
}