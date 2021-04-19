set(COMPONENT_SRCS "sk6812/sk6812.c"
				   "m5go.cpp"
				   "i2c_bus/i2c_device.c"
				   "mpu6886/mpu6886.c"
				   "ENV/env.c"
				)
set(COMPONENT_ADD_INCLUDEDIRS "." "sk6812" "i2c_bus" "mpu6886" "ENV")

set(COMPONENT_REQUIRES "mbedtls" )

register_component()

component_compile_definitions(USE_PROV_MODULE_FULL)
