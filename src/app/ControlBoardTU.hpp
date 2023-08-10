#pragma once

// mqtt
#include "mqttStack/AbstractTU.hpp"
#include "mqttStack/TUMessageVisitor.hpp"


// std
#include <unordered_map>
#include <memory>

namespace strateam{
    class MQTTStack;
    class ConnectedMessage;
    class ApplicationMessage;
    
    namespace equipment{
        class EquipmentInterface;
    }

    namespace control_board{
        class Dialog;
        
        class ControlBoardTU : public AbstractTU, public TUMessageVisitor{
        public: // == CONSTANTS ==
            static constexpr char const* RequestTopic(){ return "/strateam/+/control-board/+"; }

        public:// == TYPES ==
            using DialogPtr = std::shared_ptr<Dialog>;
            using DialogSetMap = std::unordered_map<std::string, DialogPtr>;
        public:// == CTOR ==
            ControlBoardTU( MQTTStack& stack, equipment::EquipmentInterface& equipment );
            ControlBoardTU( ControlBoardTU const& ) = delete;
            ControlBoardTU& operator =( ControlBoardTU const& ) = delete;


        public:// == TransactionUserInterafce ==
            virtual void dispatchIncoming( std::shared_ptr<strateam::MessageInterface>  msg ) override;
            virtual void shutdown()override;

            void endDialog( std::string dialogId );
            void publish( std::shared_ptr<const mqtt::message> msg );
        public:// == TUVisitor ==
            virtual void visit( ConnectedMessage & ) override;
            virtual void visit( ApplicationMessage& msg ) override;

        private:
            MQTTStack&                      stack_;
            equipment::EquipmentInterface&  equipment_;
            DialogSetMap                    dialogs_;
        };
    }
}