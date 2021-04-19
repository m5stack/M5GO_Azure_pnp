# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set (EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_BINARY_DIR}/../esp-azure/port"
                           "${CMAKE_CURRENT_BINARY_DIR}/components/LovyanGFX"
                        )
project(pnp_m5stack)
