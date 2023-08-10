#pragma once

#include <memory>
#include <string>
#include <boost/asio.hpp>

namespace strateam{
    class MQTTStack;

    namespace equipment{
        class EquipmentInterface;
    }

    namespace control_board{
        class ControlBoardTU;

        class ControlBoardApp{
            std::shared_ptr<boost::asio::io_context>        ioCtx_;
            std::shared_ptr<boost::asio::io_context::work>  work_;
            std::unique_ptr<MQTTStack>                      stackPtr_;
            std::unique_ptr<equipment::EquipmentInterface>  eqPtr_;
            std::unique_ptr<ControlBoardTU>                 tuPtr_;
        private:
            ControlBoardApp();
        public:
            static ControlBoardApp& getInstance();
        public:
            void init( std::string const& cfgPath );
            int run();
            void shutdown();
        };
    }
}