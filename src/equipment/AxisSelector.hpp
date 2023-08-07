#pragma once
#include "IsDlp.hpp"
#include "dlp/DlpAxis.hpp"
namespace strateam{
    namespace equipment{
        template <typename TagT>
		struct UnsupportedTag;

        template<typename TagT, typename Enabled = void>
        struct AxisSelector {
            using type = UnsupportedTag< TagT >;
        };

        template<typename TagT>
        struct AxisSelector<TagT, std::enable_if_t< IsDlp< TagT >::value > > {
            using type = dlp::DlpAxis;
        };
    }
}