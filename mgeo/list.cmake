LIST(APPEND MGEO_APP_SOURCE
    mgeo/app/main.cpp
    mgeo/app/mgeo_settings.cpp
    mgeo/app/mgeo_settings.h
    mgeo/app/load_assimp.cpp
    mgeo/app/load_assimp.h
    mgeo/app/mgeo_app.h
    mgeo/list.cmake
)

LIST(APPEND TEST_SOURCES
    mgeo/app/mgeo_settings.cpp
    mgeo/app/mgeo_settings.h
    mgeo/app/load_assimp.cpp
)

# On PC, compile assimp as a seperate project
INCLUDE(ExternalProject)
ExternalProject_Add(
  assimp
  PREFIX "m3rdparty"
  CMAKE_ARGS -DASSIMP_BUILD_ASSIMP_TOOLS=OFF -DASSIMP_BUILD_SAMPLES=OFF -DLIBRARY_SUFFIX='' -DASSIMP_BUILD_TESTS=OFF
  SOURCE_DIR "${CMAKE_SOURCE_DIR}/mgeo/m3rdparty/assimp"
  TEST_COMMAND ""
  INSTALL_COMMAND "" 
  INSTALL_DIR ""
)

# Include dirs 
LIST(APPEND MGEO_INCLUDE
    mgeo/m3rdparty/assimp
    mgeo/m3rdparty/assimp/include
    ${CMAKE_BINARY_DIR}/m3rdparty/src/assimp-build/include
)

IF (TARGET_PC)
LIST(APPEND PLATFORM_LINKLIBS 
    ${CMAKE_BINARY_DIR}/m3rdparty/src/assimp-build/code/$(Configuration)/assimp.lib
)
ELSE()
LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/m3rdparty/src/assimp-build/code)
IF (TARGET_MAC)
LIST(APPEND PLATFORM_LINKLIBS assimp.dylib)
ELSE()
LIST(APPEND PLATFORM_LINKLIBS assimp.so)
ENDIF()
ENDIF()
