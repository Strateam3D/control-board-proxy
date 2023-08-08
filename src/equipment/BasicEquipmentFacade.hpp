#pragma once
#include "interface/EquipmentInterface.hpp"
#include "TransportSelector.hpp"
#include "BasicAxis.hpp"
#include "EqException.hpp"

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

        public:
            BasicEquipmentFacade( rapidjson::Value const& config )
            : transport_( sureConfig( config, rj::Pointer("/controlBoard") ) )
            {
                //TODO: create axises
            }

        public:// == EquipmentInterafce ==
            virtual AxisInterface& axis( AxisType axis ) override{
                if( !axises_.count( axis ) ) throw Exception( "Invalid axis requested" );
                return *axises_[ axis ];
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
            Transport   transport_;
            Axises      axises_;
        };
    }
}