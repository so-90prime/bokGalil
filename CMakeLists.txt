project(indi-bokgalil C CXX)
cmake_minimum_required(VERSION 2.8)

include(GNUInstallDirs)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/)

find_package(INDI REQUIRED)
find_package(Nova REQUIRED)
find_package(ZLIB REQUIRED)
find_package(GSL REQUIRED)

set(CDRIVER_VERSION_MAJOR 1)
set(CDRIVER_VERSION_MINOR 0)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/indi_GalilFilterWheel.xml.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/indi_GalilFilterWheel.xml
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/)
include_directories(${INDI_INCLUDE_DIR})
include_directories(${NOVA_INCLUDE_DIR})
include_directories(${EV_INCLUDE_DIR})

include(CMakeCommon)

#///////////////////////////////////////////////////////////////////////////////
# bokGalil dependency
#///////////////////////////////////////////////////////////////////////////////
add_library(bokGalil INTERFACE)
target_include_directories(bokGalil INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/)
target_link_libraries(bokGalil INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/lib/libbokGalil.so)

#///////////////////////////////////////////////////////////////////////////////
# indi driver output
#///////////////////////////////////////////////////////////////////////////////
add_executable(
    indi_galilfw
    ${CMAKE_CURRENT_SOURCE_DIR}/src/indi_GalilFilterWheel.cpp
)

target_include_directories(indi_galilfw
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(indi_galilfw
    ${INDI_LIBRARIES}
    ${NOVA_LIBRARIES}
    ${GSL_LIBRARIES}
    bokGalil
)

#///////////////////////////////////////////////////////////////////////////////
# tell cmake where to install our executable
#///////////////////////////////////////////////////////////////////////////////
install(TARGETS indi_galilfw RUNTIME DESTINATION bin)

# and where to put the driver's xml file. Every driver has an xml file
install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/indi_galilfw.xml
    DESTINATION ${INDI_DATA_DIR}
)
