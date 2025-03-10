idf_component_register(
    SRCS "owb.c" "owb_gpio.c" "owb_rmt.c"
    INCLUDE_DIRS "."  # diz ao IDF que owb.h, owb_gpio.h, owb_rmt.h estão neste diretório
    REQUIRES driver
)
