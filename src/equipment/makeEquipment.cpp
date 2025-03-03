#include "makeEquipment.hpp"
#include "dlp/DlpEquipment.hpp"
#include "tm4c/TM4CEquipment.hpp"
#include "emulator/Equipment.hpp"

#include "rapidjson/pointer.h"
namespace rj = rapidjson;

namespace strateam::equipment{

static std::string getBoardName( rj::Value const& config ){
    std::string board;   
    auto const* rjboard = rj::GetValueByPointer( config, "/controlBoard" );
    
    if( rjboard != nullptr ){
        if( rjboard->IsObject() ){
            auto rjname = rj::GetValueByPointer( *rjboard, "/name" );
    
            if( rjname && rjname->IsString() )
                board = rjname->GetString();
        }else{
            throw std::runtime_error("/controlBoard is not of string type and not is object!"); 
        }        
    }else{
        throw std::runtime_error("Key /controlBoard not found!"); 
    }

    return board;
}

static bool isDlp( std::string const& name ){
    return name == "dlp" || name == "DLP";
}

static bool isTM4C( std::string const& name ){
    return name == "tm4c" || name == "TM4C";
}

static bool isEmulator( std::string const& name ){
    return name == "emulator" || name == "emul";
}

auto makeEquipment(IoCtx& ctx, rapidjson::Value const& config ) -> std::unique_ptr< EquipmentInterface >{
    std::string board = getBoardName(config);
    
    if( isDlp(board) ){
        return std::make_unique< dlp::DlpEquipment >( ctx, config );
    }if( isTM4C( board ) ){
        return std::make_unique< tm4c::TM4CEquipment >( ctx, config );
    }else if( isEmulator( board ) ){
        return std::make_unique< emulator::Equipment >( ctx, config );
    } else{
        throw std::runtime_error(std::string("Unsupported control board, name: ")+board);
    }
};

}
