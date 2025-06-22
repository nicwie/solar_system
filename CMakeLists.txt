cmake_minimum_required(VERSION 3.10)
project(solar_system)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- FIND PACKAGES ---
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(Assimp REQUIRED)

# --- CREATE EXECUTABLE TARGET ---
add_executable(solar_system main.cpp)

# --- INCLUDE DIRECTORIES ---
# Add the include directories from Assimp and other libraries
target_include_directories(
  solar_system PRIVATE ${GLEW_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS}
                       ${ASSIMP_INCLUDE_DIRS})

# --- LINK LIBRARIES TO TARGET ---
# Link all required libraries to your executable
target_link_libraries(solar_system PRIVATE OpenGL::GL glfw GLEW::GLEW
                                           ${ASSIMP_LIBRARIES})

# --- OPTIONAL: CUSTOM TARGET TO RUN THE PROGRAM ---
add_custom_target(
  run_solar_system
  COMMAND $<TARGET_FILE:solar_system>
  DEPENDS solar_system
  WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
  COMMENT "Running solar_system...")
