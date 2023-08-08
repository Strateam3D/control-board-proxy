#pragma once
#include "Tags.hpp"
#include <type_traits>

namespace strateam{
    namespace equipment{
        template<typename Tag, typename Enabled = void>
        struct IsDlpCheck : std::false_type{};

        template<typename Tag> 
        struct IsDlpCheck<Tag, std::enable_if_t< Tag::IsDpl::value >> : std::true_type{};
    }
}