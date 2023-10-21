#pragma once

#include <cmath>

namespace strateam{
    namespace equipment{
        /**
          * We have two variables:
          * Z_v - wanted coordinate with floating point.
          * Z_r - actual coordinate with integral precision.
          * Z = F(t)
          * Z_r( n + 1 ) = Z_r( n ) + [ Z_v( n + 1 ) - Z_r( n ) ]
          *
          * Z_r(n) - discrete position on the previous step
          * [ ... ] - integral part of the difference
          * Z_v[ n + 1 ] - next virtual position
          * Z_r[ n + 1 ] - real discrete position
         */
        template< typename ValueType>
        class BzOffset{
            ValueType   zVirtual_{};
            ValueType   zDiscrete_{};
            ValueType   doffset_{};
        public: // == CTOR ==
            BzOffset() = default;
        public: // == FUNCTIONs ==
            void reset(){
                zVirtual_ = {};
                zDiscrete_ = {};
                doffset_ = {};
            }

            void addOffset( const ValueType& offset ){
                zVirtual_ = zVirtual_ + offset;          // z_V(n+1) = Z_v(n) + offset
                ValueType diff( zVirtual_ - zDiscrete_ );
                std::modf( diff, &doffset_ );            // [ Z_v( n + 1 ) - Z_r( n ) ]
                zDiscrete_ = zDiscrete_ + doffset_;      // Z_r( n + 1 ) = Z_r( n ) + [ Z_v( n + 1 ) - Z_r( n ) ]
            }

            ValueType offset()const{
                return doffset_;
            }

            ValueType zvirtual()const{
                return zVirtual_;
            }

            ValueType zdiscrete()const{
                return zDiscrete_;
            }
        };
    }// namespace dlp
}// namespace equipment