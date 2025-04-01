idf_component_register(SRCS "src/misc.c" "src/scli.c"
                       INCLUDE_DIRS "include"
                       PRIV_REQUIRES bt console esp_driver_uart)
