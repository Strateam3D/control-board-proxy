#pragma once
#include "IsSerial.hpp"
#include "IsUsb.hpp"
#include "dlp/DlpTransport.hpp"
#include "tm4c/USBTransport.hpp"
namespace strateam{
    namespace equipment{
        template <typename TagT>
		struct UnsupportedTag;

        template<typename TagT, typename Enabled = void>
        struct TransportSelector {
            using type = UnsupportedTag< TagT >;
        };

        template<typename TagT>
        struct TransportSelector<TagT, std::enable_if_t< IsSerial< TagT >::value > > {
            using type = dlp::DlpTransport<TagT>;
        };

        template<typename TagT>
        struct TransportSelector<TagT, std::enable_if_t< IsUsb< TagT >::value > > {
            using type = tm4c::USBTransport<TagT>;
        };
    }
}