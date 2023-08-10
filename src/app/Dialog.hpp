#pragma once

#include <string>
#include <unordered_map>
#include <functional>

#include "rjapi/NamedGetterSetter.hpp"

namespace strateam{
    namespace equipment{
        class EquipmentInterface;
    }

    namespace control_board{
        class ControlBoardTU;

        class Dialog /* : public ListenerInterface*/{
        public:// == TYPES ==
            enum class CallResult{
                Keep,
                Discard
            };
            
            using DocumentPtr = std::shared_ptr<rapidjson::Document>;
            
        private:
            ControlBoardTU&                 tu_;
            equipment::EquipmentInterface&  equipment_;
            std::string                     dialogId_;
            NamedGetterSetterMap_T          gettersSetters_;
            rapidjson::Document&            masterDocument_;
            CallResult                      rval_{ CallResult::Discard };
        public:
            Dialog( ControlBoardTU& tu, equipment::EquipmentInterface& eq, std::string dialogId, rapidjson::Document& masterDocument );
        public:
            CallResult dispatch( DocumentPtr docPtr );
        }; 
    }
}