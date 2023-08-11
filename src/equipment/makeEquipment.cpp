#include "makeEquipment.hpp"
#include "dlp/DlpEquipment.hpp"

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

auto makeEquipment(boost::asio::io_context& ctx, rapidjson::Value const& config ) -> std::unique_ptr< EquipmentInterface >{
    std::string board = getBoardName(config);
    
    if( isDlp(board) ){
        return std::make_unique< dlp::DlpEquipment >( ctx, config );
    }else{
        throw std::runtime_error(std::string("Unsupported control board, name: ")+board);
    }
};

}
