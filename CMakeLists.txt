cmake_minimum_required(VERSION 3.10)
project(solar_system)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Find GLFW3
find_package(glfw3 REQUIRED)
if(NOT glfw3_FOUND)
  message(
    FATAL_ERROR
      "GLFW3 not found. Please install it or set GLFW_DIR CMake variable.")
endif()

# Find GLEW
find_package(GLEW REQUIRED)
if(NOT GLEW_FOUND)
  message(
    FATAL_ERROR
      "GLEW not found. Please install it or set GLEW_DIR CMake variable.")
endif()

# Add executable
add_executable(solar_system main.cpp) # Replace main.cpp with your source
# files

# Link libraries
target_link_libraries(solar_system OpenGL::GL glfw GLEW::GLEW)

# Optional: Set up include directories if find_package doesn't do it
# automatically
target_include_directories(solar_system PRIVATE ${GLFW_INCLUDE_DIRS}
                                                ${GLEW_INCLUDE_DIRS})

# Optional: Copy necessary assets (shaders, textures, etc.) to the build
# directory configure_file(path/to/your/asset.txt ${CMAKE_BINARY_DIR}/asset.txt
# COPYONLY) target_include_directories(YourOpenGLProgram PRIVATE
# ${CMAKE_BINARY_DIR})

# add target that runs the program after it is built
add_custom_target(
  run_solar_system
  COMMAND $<TARGET_FILE:solar_system>
  DEPENDS solar_system
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR} # Optional: Set working directory
  COMMENT "Running solar_system...")
