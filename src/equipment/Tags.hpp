#pragma once
// boost
#include <boost/mpl/vector.hpp>
#include <boost/mpl/inherit.hpp>
#include <boost/mpl/inherit_linearly.hpp>

#include <type_traits>
namespace strateam{
    namespace equipment{
        struct Usb {
            using IsUsb = std::true_type;
        };

        struct Json {
            using IsJson = std::true_type;
        };

        struct Text {
            using IsText = std::true_type;
        };

        struct DlpTag{
            using IsDlp = std::true_type;
        };

        using UsbJsonDlpTags = boost::mpl::vector<Usb, Json, DlpTag> ;
        using UsbTextTags = boost::mpl::vector<Usb, Text>;

#ifndef DEFINE_TRANSPORT_TAG
#define DEFINE_TRANSPORT_TAG(name)																				\
	struct name																							\
		: boost::mpl::inherit_linearly< name##Tags,														\
										boost::mpl::inherit< boost::mpl::placeholders::_1,				\
															 boost::mpl::placeholders::_2 > >::type{};
#endif

        DEFINE_TRANSPORT_TAG(UsbJsonDlp)
        DEFINE_TRANSPORT_TAG(UsbText)

    }
}