cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project(Simulation1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Geant4 REQUIRED ui_all vis_all)

include(${Geant4_USE_FILE})

file(GLOB sources ${PROJECT_SOURCE_DIR}/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/*.hh)

add_executable(sim sim.cc ${sources} ${headers})
target_link_libraries(sim ${Geant4_LIBRARIES})

file(GLOB MACRO_FILES 
    "*.mac"
)

file(GLOB DATA_FILES 
    "*.dat"
)

file(COPY ${MACRO_FILES} DESTINATION ${PROJECT_BINARY_DIR})
file(COPY ${DATA_FILES} DESTINATION ${PROJECT_BINARY_DIR})

add_custom_target(Simulation1 DEPENDS sim)