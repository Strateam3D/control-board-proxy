#pragma once

#include <string>
#include <unordered_map>
#include <functional>

#include "rjapi/NamedGetterSetter.hpp"

#include "equipment/interface/ListenerInterface.hpp"

namespace strateam{
    namespace equipment{
        class EquipmentInterface;
    }

    namespace control_board{
        class ControlBoardTU;

        class Dialog : public equipment::ListenerInterface{
        public:// == TYPES ==
            
            using DocumentPtr = std::shared_ptr<rapidjson::Document>;
            
        private:
            ControlBoardTU&                 tu_;
            equipment::EquipmentInterface&  equipment_;
            std::string                     dialogId_;
            NamedGetterSetterMap_T          gettersSetters_;
            rapidjson::Document&            masterDocument_;
            equipment::MotionResult         motret_{ equipment::MotionResult::Success };
            std::string                     axis_;
            int                             offset_{};
            int                             targSpd_{};
        public:
            Dialog( ControlBoardTU& tu, equipment::EquipmentInterface& eq, std::string dialogId, rapidjson::Document& masterDocument );
            Dialog( Dialog const& ) = delete;
            Dialog& operator =( Dialog const& ) = delete;
            ~Dialog();
        public:
            equipment::MotionResult dispatch( DocumentPtr docPtr );

        private:// == ListenerInterface ==
            virtual void motionDone( equipment::MotionResult motret ) override;
            virtual void motionToDone( equipment::MotionResult motret ) override;
            virtual void moveHomeDone( equipment::MotionResult motret) override;
            virtual void moveToZeroDone( equipment::MotionResult motret ) override;
            virtual void squeezingDone( equipment::MotionResult motret ) override;
        }; 
    }
}