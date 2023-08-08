#pragma once
// boost
#include <boost/mpl/vector.hpp>
#include <boost/mpl/inherit.hpp>
#include <boost/mpl/inherit_linearly.hpp>

#include <type_traits>
namespace strateam{
    namespace equipment{
        struct Usb {
            using IsUsb = boost::mpl::true_;
        };

        struct Json {
            using IsJson = boost::mpl::true_;
        };

        struct Text {
            using IsText = boost::mpl::true_;
        };

        struct Dlp{
            using IsDlp = boost::mpl::true_;
        };

        using UsbJsonDlpTags = boost::mpl::vector<Usb, Json, Dlp> ;
        using UsbTextTags = boost::mpl::vector<Usb, Text>;

#ifndef DEFINE_TRANSPORT_TAG
#define DEFINE_TRANSPORT_TAG(name)																				\
	struct name																							\
		: boost::mpl::inherit_linearly< name##Tags,														\
										boost::mpl::inherit< boost::mpl::placeholders::_1,				\
															 boost::mpl::placeholders::_2 > >::type{};
#endif

        DEFINE_TRANSPORT_TAG(UsbJsonDlp)
        // DEFINE_TRANSPORT_TAG(UsbText)

    }
}