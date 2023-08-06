#pragma once
// boost
#include <boost/mpl/vector.hpp>
#include <boost/mpl/inherit.hpp>
#include <boost/mpl/inherit_linearly.hpp>

#include <type_traits>
namespace strateam{
    namespace equipment{
        struct Usb {
            typedef std::true_type IsUsb;
        };

        struct Json {
            typedef std::true_type IsJson;
        };

        struct Text {
            typedef std::true_type IsText;
        };

        typedef boost::mpl::vector<Usb, Json>   UsbJsonTags;
        typedef boost::mpl::vector<Usb, Text>   UsbTextTags;

#ifndef DEFINE_TRANSPORT_TAG
#define DEFINE_TRANSPORT_TAG(name)																				\
	struct name																							\
		: boost::mpl::inherit_linearly< name##Tags,														\
										boost::mpl::inherit< boost::mpl::placeholders::_1,				\
															 boost::mpl::placeholders::_2 > >::type{};
#endif

        DEFINE_TRANSPORT_TAG(UsbJson)
        DEFINE_TRANSPORT_TAG(UsbText)

    }
}