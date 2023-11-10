#pragma once
// boost
#include <boost/mpl/vector.hpp>
#include <boost/mpl/inherit.hpp>
#include <boost/mpl/inherit_linearly.hpp>

#include <type_traits>
namespace strateam{
    namespace equipment{
        struct Serial {
            using IsSerial = boost::mpl::true_;
        };

        struct Usb {
            using IsUsb = boost::mpl::true_;
        };

        struct Json {
            using IsJson = boost::mpl::true_;
        };

        struct Text {
            using IsText = boost::mpl::true_;
        };

        struct DlpBoard{
            using IsDlpBoard = boost::mpl::true_;
        };

        struct Tm4CBoard{
            using IsTM4CBoard = boost::mpl::true_;
        };

        using SerialJsonDlpTags = boost::mpl::vector<Serial, Json, DlpBoard> ;
        using UsbJsonTM4CTags = boost::mpl::vector<Usb, Json, Tm4CBoard>;

#ifndef DEFINE_TRANSPORT_TAG
#define DEFINE_TRANSPORT_TAG(name)																				\
	struct name																							\
		: boost::mpl::inherit_linearly< name##Tags,														\
										boost::mpl::inherit< boost::mpl::placeholders::_1,				\
															 boost::mpl::placeholders::_2 > >::type{};
#endif

        DEFINE_TRANSPORT_TAG(SerialJsonDlp)
        DEFINE_TRANSPORT_TAG(UsbJsonTM4C)

    }
}