#pragma once

namespace strateam{
    class Symbols{
    public:
        static char const* AxisHomePos(){ return "/equipment/axises/{}/homePos"; }
        static constexpr char const* Console(){ return "console"; }
        static char const* KLeft(){ return "/equipment/load_cell/lk"; }
        static char const* KRight(){ return "/equipment/load_cell/lk"; }
        static char const* ZAxisHomePos(){ return "/equipment/axises/2/homePos"; }
    };
}