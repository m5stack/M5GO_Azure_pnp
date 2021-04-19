set(COMPONENT_SRCS "main_app.cpp"
                "pnp_m5stack.cpp" 
                "pnp_device_client_ll.c"
                "utilities/pnp_deviceinfo_component.cpp"
                "utilities/pnp_telemetries_component.cpp"
                )
set(COMPONENT_ADD_INCLUDEDIRS "." "utilities")

register_component()

component_compile_definitions(SET_TRUSTED_CERT_IN_SAMPLES USE_PROV_MODULE_FULL)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-const-variable)
