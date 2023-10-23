set(sources "DFRobotDFPlayerMini/DFRobotDFPlayerMini.cpp")
set(includes "DFRobotDFPlayerMini")

idf_component_register(
    SRCS ${sources}
    INCLUDE_DIRS ${includes}
    REQUIRES DFPlayerMini
)
