#pragma once
#include "Tags.hpp"
#include <type_traits>

namespace strateam{
    namespace equipment{
        template<typename Tag, typename Enabled = void>
        struct IsJson : std::false_type{};

        template<typename Tag >
        struct IsJson<Tag, std::enable_if_t< Tag::IsJson::value>> : std::true_type{};
    }
}