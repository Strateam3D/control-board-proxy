#pragma once
#include "IsUsb.hpp"

namespace strateam{
    namespace equipment{
        template <typename TagT>
		struct UnsupportedTag;

        template<typename TagT, typename Enabled = void>
        struct TransportSelector {
            using type = UnsupportedTag< TagT >;
        };

        template<typename TagT>
        struct TransportSelector<TagT, std::enable_if_t< IsUsb< TagT >::value > > {
            using type = UsbSerialPortTransport<TagT>;
        };
    }
}