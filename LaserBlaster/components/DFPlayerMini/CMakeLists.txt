set(sources "DFRobotDFPlayerMini/DFRobotDFPlayerMini.cpp" "DFPlayerMini_Arduino.cpp" "DFPlayerMini_PlayerMini.cpp" "DFPlayerMini.cpp")
set(includes "DFRobotDFPlayerMini" "include")

idf_component_register(
    SRCS ${sources}
    INCLUDE_DIRS ${includes}
    REQUIRES DFPlayerMini
    PRIV_REQUIRES esp_timer
)
