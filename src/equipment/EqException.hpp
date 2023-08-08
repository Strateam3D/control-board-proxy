#pragma once
#include <stdexcept>

namespace strateam{
    namespace equipment{
        struct Exception : public std::runtime_error{
            template< typename...Args >
            explicit Exception( Args&&...args )
            : std::runtime_error( std::forward<Args...>(args)... )
            {}
        };
    }
}