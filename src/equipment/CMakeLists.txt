add_definitions( -DRAPIDJSON_HAS_STDSTRING=1 )
add_definitions( -DRAPIDJSON_HAS_CXX11_RVALUE_REFS=1 )

include_directories( 
    ${CMAKE_SOURCE_DIR}/3rdParty/rj/include 
)

target_sources( ${CONTROL_BOARD_TARGET} PUBLIC    
    AxisSelector.hpp
    BasicAxis.hpp
    BasicControlBoard.hpp
    BasicEquipmentFacade.hpp
    BasicMessageWrapper.hpp
    EqException.hpp
    Global.hpp
    IsDlp.hpp
    IsJson.hpp
    Tags.hpp
    TransportSelector.hpp
    
    dlp/DlpAxis.hpp
    dlp/DlpControlBoard.hpp
    dlp/DlpEquipment.hpp
    dlp/DlpTransport.hpp

    interface/AxisInterface.hpp
    interface/EquipmentInterface.hpp
    interface/Listenable.hpp
    interface/ListenerInterface.hpp
    interface/MotionResult.hpp
    serial_port/AsyncSerialPort.hpp
PRIVATE
    serial_port/AsyncSerialPort.cpp
    makeEquipment.cpp
    
)


target_include_directories(${CONTROL_BOARD_TARGET} 
PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)
