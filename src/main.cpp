#include "equipment/dlp/DlpEquipment.hpp"

#include <iostream>

using namespace strateam::equipment;
int main(){
    rapidjson::Document doc;
    strateam::equipment::dlp::DlpEquipment eq( doc );
    
    return 0;
}