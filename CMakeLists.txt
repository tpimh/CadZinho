cmake_minimum_required(VERSION 3.10)

project(CadZinho)

add_definitions("-DPLATFORM_${CMAKE_SYSTEM_NAME}")

find_package(OpenGL REQUIRED)
message(STATUS "OpenGL found")
find_package(GLEW REQUIRED)
message(STATUS "GLEW found: version ${GLEW_VERSION}")
find_package(SDL2 REQUIRED)
message(STATUS "SDL2 found: version ${SDL2_VERSION}")
find_package(Lua REQUIRED)
message(STATUS "Lua found: version ${LUA_VERSION_STRING}")

include_directories(${OPENGL_INCLUDE_DIR} ${GLEW_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS} ${LUA_INCLUDE_DIR})
link_libraries(${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${SDL2_LIBRARIES} ${LUA_LIBRARIES})

set(CMAKE_C_STANDARD 11)
file(GLOB SOURCES src/*.c)

add_executable(cadzinho ${SOURCES})
