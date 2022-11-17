#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.16...3.21)
project(barrel-length)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available Vis drivers by default
# You can set WITH_GEANT4_VIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
#
option(WITH_GEANT4_VIS "Build example with Geant4 Vis drivers" ON)
if(WITH_GEANT4_VIS)
  find_package(Geant4 REQUIRED gdml vis_all)
else()
  find_package(Geant4 REQUIRED gdml)
endif()

#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
#
include(${Geant4_USE_FILE})

#----------------------------------------------------------------------------
# Locate sources and headers for this project
#
include_directories(${PROJECT_SOURCE_DIR}/include 
                    ${Geant4_INCLUDE_DIR})
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
add_executable(barrel barrel.cc ${sources} ${headers})
target_link_libraries(barrel ${Geant4_LIBRARIES} )

#----------------------------------------------------------------------------
# Add program to the project targets
# (this avoids the need of typing the program name after make)
#
add_custom_target(barrel-length DEPENDS barrel)

function(copyFiles sourceFiles)
  foreach(sourceFile ${sourceFiles})
    # Replace source path with destination path
    string(REGEX REPLACE "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}" destFile ${sourceFile})
    # Copy file to build folder
    configure_file(
      ${sourceFile} ${destFile}
      COPYONLY
    )
  endforeach()
endfunction()

#----------------------------------------------------------------------------
# Copy all scripts to the build directory. Easier to pass macros to executable.

file(GLOB_RECURSE barel_gdml_files CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/gdml/*.*)
# message(STATUS "Found list of Geant4 Macros: ${glass_MACROS}")
copyFiles("${barel_gdml_files}")

#----------------------------------------------------------------------------
# Copy macro files

file(GLOB_RECURSE barel_macro_files CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/macros/*.*)
copyFiles("${barel_macro_files}")

#----------------------------------------------------------------------------
# Copy slurm scripts

file(GLOB_RECURSE barel_slurm_files CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/slurm/*.*)
copyFiles("${barel_slurm_files}")

#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
install(TARGETS barrel DESTINATION bin)
