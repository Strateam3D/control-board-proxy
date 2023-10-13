#pragma once
#include "interface/EquipmentInterface.hpp"
#include "TransportSelector.hpp"
#include "BasicAxis.hpp"
#include "EqException.hpp"
#include "BasicControlBoard.hpp"

// std
#include <unordered_map>
#include <memory>

#include "rapidjson/document.h"
namespace rj = rapidjson;

namespace strateam{
    namespace equipment{
        template<typename Tag>
        class BasicEquipmentFacade : public EquipmentInterface{
        public:// == TYPEs ==
            using Transport = typename TransportSelector<Tag>::type;
            using Axis = BasicAxis<Tag>;
            using AxisPtr = std::unique_ptr<Axis>;
            using Axises = std::unordered_map<AxisType, AxisPtr>;
            using ControlBoard = BasicControlBoard<Tag>;
            using ControlBoardPtr = std::unique_ptr<ControlBoard>;

        public:
            BasicEquipmentFacade( IoCtx& ctx, rapidjson::Value const& config )
            : transport_( sureConfig( config, rj::Pointer("/controlBoard") ) ){
                auto const* val = rj::GetValueByPointer( config, "/axises" );

                if( !val || !val->IsArray() ){
                    throw Exception( "Invalid configuration" );
                }
          
                auto const& axises = config["axises"].GetArray();
                std::size_t id = 0;

                for( rj::Value::ConstValueIterator it = axises.Begin(); it != axises.End(); it++, id ++ ){
                    const rj::Value::ConstObject& axisValue = it->GetObject();
                    std::string axisName = axisValue[ "name" ].GetString();
                    std::cout << "Creating axis " << axisName << std::endl;
                    double stepsPerUm = axisValue[ "stepsPerUm" ].GetDouble();
                    AxisPtr axisPtr = std::make_unique<Axis>( ctx, axisName, transport_, stepsPerUm );
                    int hp = axisValue[ "homePos" ].GetInt();
                    axisPtr->setHomePosition( dim::MotorStep( hp ) );
                    axises_.emplace( static_cast<AxisType>( id ), std::move( axisPtr ) );
                }

                auto const* haydonStepsPerUmValue = rj::GetValueByPointer( config, "/axises/3/stepsPerUm" );
                auto const* beamStepsPerUmValue = rj::GetValueByPointer( config, "/axises/0/stepsPerUm" );
                auto const* kLeft = rj::GetValueByPointer( config, "/equipment/load_cell/kLeft" );
                auto const* kRight = rj::GetValueByPointer( config, "/equipment/load_cell/kRight" );
                controlBoard_ = std::make_unique< ControlBoard >( ctx, transport_, haydonStepsPerUmValue->GetDouble(), beamStepsPerUmValue->GetDouble() );
                auto retval = controlBoard_->setLoadCellCoeffs( kLeft->GetDouble(), kRight->GetDouble() );

                if( retval != decltype(retval)::Success ){
                    throw Exception( "Failed to set laod cell coefficients" );
                }
            }

        public:// == EquipmentInterafce ==
            virtual AxisInterface& axis( AxisType axis ) override{
                if( !axises_.count( axis ) ) throw Exception( "Invalid axis requested" );
                return *axises_[ axis ];
            }

            ControlBoardInterface& controlBoard()override{
                return *controlBoard_;
            }
        private:
            static const rj::Value& sureConfig( rj::Value const& config, rj::Pointer const& path ){
              static const rapidjson::Value _EmptyValue_;
              using namespace rapidjson;
              const Value *v = GetValueByPointer( config, path );
              
              if( v )
                 return *v;
              else
                 return _EmptyValue_;
          }

        private:
            Transport       transport_;
            Axises          axises_;
            ControlBoardPtr controlBoard_;
        };
    }
}