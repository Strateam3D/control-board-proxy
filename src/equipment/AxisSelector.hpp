#pragma once
#include "IsDlp.hpp"
#include "IsTM4C.hpp"
#include "IsEmulator.hpp"

#include "dlp/DlpAxis.hpp"
#include "tm4c/TM4CAxis.hpp"
#include "emulator/EmulatorAxis.hpp"

namespace strateam{
    namespace equipment{
        template <typename TagT>
		struct UnsupportedTag;

        template<typename TagT, typename Enabled = void>
        struct AxisSelector {
            using type = UnsupportedTag< TagT >;
        };

        template<typename TagT>
        struct AxisSelector<TagT, std::enable_if_t< IsDlpCheck< TagT >::value > > {
            using type = dlp::DlpAxis;
        };

        template<typename TagT>
        struct AxisSelector<TagT, std::enable_if_t< IsTM4CCheck< TagT >::value > > {
            using type = tm4c::TM4CAxis;
        };

        template<typename TagT>
        struct AxisSelector<TagT, std::enable_if_t< IsEmulator< TagT >::value > > {
            using type = emulator::EmulatorAxis;
        };
    }
}