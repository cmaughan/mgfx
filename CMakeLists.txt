cmake_minimum_required(VERSION 3.1)

set (CMAKE_CXX_STANDARD 11)

PROJECT (mgfx)

MESSAGE(STATUS " CMakeLists: mgfx")

# Options
option (PROJECT_UNITTESTS "Build Unit Tests" ON)
option (PROJECT_APP "Build App" ON)
option (PROJECT_MGEO "Build Geo Convertor" ON)
option (PROJECT_MASSETBUILDER "Build AssetBuilder" ON)
option (PROJECT_SHADERCOMPILERS "Build SpirV, glslang" OFF)

# Docs not added for functions yet, but you can generate them
option (PROJECT_DOCS "Generate documentation" OFF)

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

if (CMAKE_BUILD_TYPE MATCHES Coverage)
    set (CMAKE_BUILD_TYPE "Debug")
    set (PROJECT_COVERAGE ON)
    set (PROJECT_UNITTESTS ON)
endif()

# global needed variables
SET (APPLICATION_NAME ${PROJECT_NAME})
SET (APPLICATION_VERSION_MAJOR "0")
SET (APPLICATION_VERSION_MINOR "1")
SET (APPLICATION_VERSION_PATCH "0")
SET (APPLICATION_VERSION "${APPLICATION_VERSION_MAJOR}.${APPLICATION_VERSION_MINOR}.${APPLICATION_VERSION_PATCH}")

# Gather our lists of source code
SET(COMMON_SOURCES "")

# Core stuff
SET(MGFX_SOURCES "")
SET(MGFX_INCLUDE "")

# Third party sources
SET(M3RDPARTY_SOURCE "")
SET(M3RDPARTY_INCLUDE "")

# Assets
SET(ASSETS_SHADERS "")
SET(ASSETS_TEXTURES "")
SET(ASSETS_MODELS "")

FILE(GLOB_RECURSE FOUND_TEXTURES
    assets/*.png
    assets/*.jpg
    assets/*.dds
    assets/*.bmp)

LIST(APPEND ASSETS_TEXTURES ${FOUND_TEXTURES} )

FILE(GLOB_RECURSE FOUND_SHADERS 
    assets/*.fragmentshader
    assets/*.vertexshader 
    assets/*.mhlsl
    )
LIST(APPEND ASSETS_SHADERS ${FOUND_SHADERS})

FILE(GLOB_RECURSE FOUND_MODELS assets/*.mmesh assets/*.obj)
LIST(APPEND ASSETS_MODELS ${FOUND_MODELS})

# Tests
SET(TEST_SOURCES "")

# Link libraries
SET(PLATFORM_LINKLIBS "")

# Set all compiler flags 
INCLUDE(cmake/all.cmake)

# Functions for file copying
INCLUDE(m3rdparty/cmake/copy_files.cmake)

# config_app.h checks
# This makes a config_shared.h file which can be included for system settings
#  Must be after setting up the platforms
CONFIGURE_FILE(${CMAKE_CURRENT_LIST_DIR}/cmake/config_app.h.cmake ${CMAKE_BINARY_DIR}/config_app.h)

INCLUDE(m3rdparty/list.cmake)
INCLUDE(mcommon/list.cmake)
INCLUDE(mgfx_core/list.cmake)

INCLUDE_DIRECTORIES(
    m3rdparty
    mcommon
    ${M3RDPARTY_INCLUDE}
    ${MGFX_INCLUDE}
    ${CMAKE_BINARY_DIR}
    .
    ) 

# Create the library - No depenencies on anything else in this file
SET (MGFX_SOURCES ${MGFX_SOURCES} ${M3RDPARTY_SOURCE} ${COMMON_SOURCES})
ADD_LIBRARY(mgfx_core ${MGFX_SOURCES})

# Create the AssetBuilder - Depends on SDL2, mgfx_core, spirv glslang
IF (PROJECT_MASSETBUILDER)
SET(MASSETBUILDER_APP_SOURCE "")
INCLUDE(massetbuilder/list.cmake)
ADD_EXECUTABLE (massetbuilder ${MASSETBUILDER_APP_SOURCE})
ADD_DEPENDENCIES(massetbuilder sdl2)
TARGET_LINK_LIBRARIES (massetbuilder mgfx_core ${PLATFORM_LINKLIBS})
ENDIF()
# End Create AssetBuilder

# Assets - No dependencies
add_custom_target(Assets ALL
    SOURCES ${ASSETS_MODELS} ${ASSETS_SHADERS} ${ASSETS_TEXTURES}
    COMMAND $<TARGET_FILE:massetbuilder> --input ${CMAKE_SOURCE_DIR}/assets --output $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets VERBATIM
)
set_source_files_properties(${ASSETS_SHADERS} PROPERTIES HEADER_FILE_ONLY TRUE)
#end Assets

# Create the app ***
IF (PROJECT_APP)
SET(MGFX_APP_SOURCES "")
INCLUDE(mgfx/list.cmake)
ADD_EXECUTABLE (${PROJECT_NAME} WIN32 ${MGFX_APP_SOURCES}) # Win32 ignored on non-windows
ADD_DEPENDENCIES(${PROJECT_NAME} sdl2)
TARGET_LINK_LIBRARIES (${PROJECT_NAME} mgfx_core ${PLATFORM_LINKLIBS})
copy_existing_files(${PROJECT_NAME} logger.conf ${CMAKE_CURRENT_BINARY_DIR}/$(Configuration) )
ENDIF()
# End Create App

# Create geometry processor
IF (PROJECT_MGEO)
SET(MGEO_APP_SOURCE "")
SET(MGEO_INCLUDE "")
include(mgeo/list.cmake)
INCLUDE_DIRECTORIES(${MGEO_INCLUDE})
ADD_EXECUTABLE (mgeo ${MGEO_APP_SOURCE})
TARGET_LINK_LIBRARIES (mgeo mgfx_core ${PLATFORM_LINKLIBS})
ADD_DEPENDENCIES(mgeo assimp sdl2)
IF (TARGET_PC)
copy_generated_file(mgeo ${CMAKE_BINARY_DIR}/m3rdparty/src/assimp-build/code/$(Configuration)/assimp.dll ${CMAKE_CURRENT_BINARY_DIR}/$(Configuration))
ENDIF()
ENDIF()
# End Create geometry processor

# Create unit tests
IF (PROJECT_UNITTESTS)
INCLUDE(tests/list.cmake)
enable_testing()
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGTEST_HAS_TR1_TUPLE=0")
SET (TEST_SOURCES
    ${M3RDPARTY_DIR}/googletest/googletest/src/gtest-all.cc
    ${TEST_SOURCES}
)
ADD_EXECUTABLE (unittests ${TEST_SOURCES} )
IF (PROJECT_MGEO)
ADD_DEPENDENCIES(unittests mgeo sdl2)
ENDIF()
IF (PROJECT_MASSETBUILDER)
ADD_DEPENDENCIES(unittests massetbuilder)
ENDIF()

TARGET_LINK_LIBRARIES (unittests mgfx_core ${PLATFORM_LINKLIBS} ${CMAKE_THREAD_LIBS_INIT})
add_test(unittests unittests)
INCLUDE_DIRECTORIES(
    ${M3RDPARTY_DIR}/googletest/googletest/include
    ${M3RDPARTY_DIR}/googletest/googletest
    ${M3RDPARTY_DIR}/googletest/googlemock/include
    ${M3RDPARTY_DIR}/googletest/googlemock
)
ENDIF()
#End Create Unit Tests

# Begin Docs
if(PROJECT_DOCS)
  message(STATUS "Generate documentation")

  find_package(Doxygen)
  if(DOXYGEN_FOUND)
    configure_file(
      ${PROJECT_SOURCE_DIR}/cmake/Doxyfile.in
      ${CMAKE_BINARY_DIR}/Doxyfile
    )
    add_custom_target(gen-doc
      ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMENT "Generating documentation"
    )
  else()
    message(WARNING "Doxygen not found, skipping documentation generation")
  endif()
endif()
# End Docs


SOURCE_GROUP (2D REGULAR_EXPRESSION "2d")
SOURCE_GROUP (3D REGULAR_EXPRESSION "3d")
SOURCE_GROUP (3D\\Device\\GL REGULAR_EXPRESSION "(3d)+.*device.*GL*")
SOURCE_GROUP (3D\\Device\\Null REGULAR_EXPRESSION "(3d)+.*device.*Null*")
SOURCE_GROUP (3D\\Device\\DX12\\MiniEngine REGULAR_EXPRESSION "(3d)+.*device.*DX12.*miniengine.*")
SOURCE_GROUP (3D\\Device\\DX12 REGULAR_EXPRESSION "(3d)+.*device.*DX12*")
SOURCE_GROUP (3D\\Device\\DX12\\Util REGULAR_EXPRESSION "(3d)+.*device.*DX12*Util*")
SOURCE_GROUP (3D\\Geometry REGULAR_EXPRESSION "(3d)+.*geometry*")
SOURCE_GROUP (3D\\Camera REGULAR_EXPRESSION "(3d)+.*camera*")
SOURCE_GROUP (3D\\Device REGULAR_EXPRESSION "(3d)+.*device*")
SOURCE_GROUP (3D\\UI REGULAR_EXPRESSION "(3d)+.*ui*")
SOURCE_GROUP (3D\\Scene REGULAR_EXPRESSION "(3d)+.*scene*")
SOURCE_GROUP (Assets\\Shaders\\GL REGULAR_EXPRESSION "(fragmentshader|vertexshader)+$")
SOURCE_GROUP (Assets\\Shaders\\DX REGULAR_EXPRESSION "(mhlsl)+$")
SOURCE_GROUP (Assets\\Shaders\\GLSLANG REGULAR_EXPRESSION "(vert|tesc|tese|geom|frag|comp|conf)+$")
SOURCE_GROUP (Assets\\Models FILES ${ASSETS_MODELS})
SOURCE_GROUP (Assets\\Textures FILES ${ASSETS_TEXTURES})
SOURCE_GROUP (App REGULAR_EXPRESSION "app" )

SOURCE_GROUP (Tests REGULAR_EXPRESSION ".*.tests?.(cpp|h)+")
SOURCE_GROUP ("mcommon" FILES ${COMMON_SOURCES})

