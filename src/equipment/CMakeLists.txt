add_definitions( -DRAPIDJSON_HAS_STDSTRING=1 )

include_directories( 
    ${CMAKE_SOURCE_DIR}/3rdParty/rj/include 
)

target_sources( ${CONTROL_BOARD_TARGET} PUBLIC    
    AxisSelector.hpp
    BasicAxis.hpp
    BasicEquipmentFacade.hpp
    BasicMessageWrapper.hpp
    EqException.hpp
    Global.hpp
    IsDlp.hpp
    IsJson.hpp
    Tags.hpp
    TransportSelector.hpp
    
    dlp/DlpAxis.hpp
    dlp/Equipment.hpp
    dlp/DlpTransport.hpp

    interface/AxisInterface.hpp
    interface/EquipmentInterface.hpp
    interface/Listenable.hpp
    interface/ListenerInterface.hpp
    interface/MotionResult.hpp

    serial_port/AsyncSerialPort.cpp
    serial_port/AsyncSerialPort.hpp
)
