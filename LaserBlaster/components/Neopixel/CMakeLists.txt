set(sources "Neopixel.c")
set(includes "include")

idf_component_register(
    SRCS ${sources}
    INCLUDE_DIRS ${includes}
    REQUIRES Neopixel
    PRIV_REQUIRES driver
)
