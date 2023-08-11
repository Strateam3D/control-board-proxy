#pragma once

#include <boost/asio.hpp>

#ifndef BUILD_ARM
        using IoCtx = boost::asio::io_context;
        using IoWork = boost::asio::io_context::work;
#else
        using IoCtx = boost::asio::io_service;
        using IoWork = boost::asio::io_service::work;
#endif
