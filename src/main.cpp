#include "equipment/dlp/DlpEquipment.hpp"

#include <iostream>

#include "rapidjson/pointer.h"

using namespace strateam::equipment;
namespace rj = rapidjson;

int main(){
    rapidjson::Document doc;
    rj::SetValueByPointer( doc, "/equipment/controlBoard/device", "/dev/ttyACM0" );
    rj::SetValueByPointer( doc, "/equipment/controlBoard/baudRate", 115200 );
    strateam::equipment::dlp::DlpEquipment eq( doc );
    
    return 0;
}