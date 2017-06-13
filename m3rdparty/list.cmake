option (PROJECT_M3RDPARTY_SCINTILLA "Compile Scintilla" OFF)
option (PROJECT_M3RDPARTY_ZIP "Compile Zip decompression" OFF)
option (PROJECT_CPP_FILESYSTEM "Use CPP 14 Filesystem - experimental" ON)

option (PROJECT_DEVICE_GL "Support OpenGL" ON)
option (PROJECT_DEVICE_DX12 "Support DX12" ON)
option (PROJECT_DEVICE_NULL "Support Null Device (for profiling, testing)" OFF)

IF (NOT TARGET_PC)
SET(PROJECT_DEVICE_DX12 OFF)
ENDIF()

# Zip
if (PROJECT_M3RDPARTY_ZIP)
file(GLOB ZIP_SOURCE 
    m3rdparty/ziplib/Source/ZipLib/*.h
    m3rdparty/ziplib/Source/ZipLib/*.cpp
    m3rdparty/ziplib/Source/ZipLib/extlibs/bzip2/*.c
    m3rdparty/ziplib/Source/ZipLib/extlibs/zlib/*.c
)

IF (TARGET_LINUX)
file(GLOB ZIP_SOURCE_LINUX 
  m3rdparty/ziplib/Source/ZipLib/extlibs/lzma/unix/*.c)
set (ZIP_SOURCE ${ZIP_SOURCE} ${ZIP_SOURCE_LINUX})
ENDIF()

IF (TARGET_PC)
file(GLOB ZIP_SOURCE_PC 
    m3rdparty/ziplib/Source/ZipLib/extlibs/lzma/*.c)
set (ZIP_SOURCE ${ZIP_SOURCE} ${ZIP_SOURCE_PC})
ENDIF()
ENDIF()

# Scintilla editor
if (PROJECT_M3RDPARTY_SCINTILLA)
file(GLOB SCINTILLA_SOURCE
    m3rdparty/scintilla/src/*.cxx
    m3rdparty/scintilla/lexlib/*.cxx
    m3rdparty/scintilla/lexers/*.cxx
    m3rdparty/scintilla/src/*.h
    m3rdparty/scintilla/include/*.h
    m3rdparty/imgui/imgui_scintilla.cpp
    m3rdparty/imgui/imgui_scintilla.h
)
IF (TARGET_PC)
#SET (SCINTILLA_SOURCE ${SCINTILLA_SOURCE} m3rdparty/scintilla/win32/PlatWin.cxx)
ENDIF()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSCI_LEXER -DSCI_NAMESPACE")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSCI_LEXER -DSCI_NAMESPACE")
INCLUDE_DIRECTORIES(m3rdparty/scintilla/src)
ENDIF()


# GLM
file(GLOB GLM_SOURCE
    m3rdparty/glm/glm/*.hpp)

# Easy Logging
set (EASYLOGGING_SOURCE 
    m3rdparty/easylogging/src/easylogging++.cc)

# MPC Parser
SET (MPC_SOURCE
    m3rdparty/mpc/mpc.c)
 
SET (MPC_INCLUDE 
    m3rdparty/mpc/mpc.h)

# GL3
IF (PROJECT_DEVICE_GL)
SET (GL_SOURCE 
    m3rdparty/GL/gl3w.c)
LIST(APPEND M3RDPARTY_INCLUDE m3rdparty/GL)
ENDIF()

# ImGui
SET (IMGUI_SOURCE 
    m3rdparty/imgui/imgui_orient.cpp
    m3rdparty/imgui/imgui_demo.cpp
    m3rdparty/imgui/imgui_draw.cpp
    m3rdparty/imgui/imgui.cpp)

SET (IMGUI_INCLUDE 
    imgui/imgui.h
    imgui/imguil_dock.h
    imgui/imgui_orient.h)
        
LIST(APPEND M3RDPARTY_SOURCE 
    ${EASYLOGGING_SOURCE}
    ${IMGUI_SOURCE}
    ${GL_SOURCE}
    ${MPC_SOURCE}
    ${ZIP_SOURCE}
    ${SCINTILLA_SOURCE}
    ${GLM_SOURCE} ${GLM_INCLUDE}
    m3rdparty/m3rdparty.h
    )

LIST(APPEND M3RDPARTY_INCLUDE
    m3rdparty
    ${CMAKE_BINARY_DIR}
    m3rdparty/imgui
    m3rdparty/glm
    m3rdparty/flatbuffers/include
    m3rdparty/tclap/include
    m3rdparty/gli
    m3rdparty/sdl/include
    m3rdparty/scintilla/include
    m3rdparty/sdl
    m3rdparty/scintilla/lexlib
    m3rdparty/googletest
    )

SET (M3RDPARTY_DIR ${CMAKE_CURRENT_LIST_DIR})

IF (TARGET_PC)
# Avoid Win compile errors in the zip library
SET_SOURCE_FILES_PROPERTIES(${ZIP_SOURCE} PROPERTIES COMPILE_FLAGS "/wd4267 /wd4244 /wd4996")
ENDIF()

IF (TARGET_LINUX)
SET_SOURCE_FILES_PROPERTIES(${ZIP_SOURCE} PROPERTIES COMPILE_FLAGS "-Wno-tautological-compare")
ENDIF()

IF (TARGET_UWP)
SET_SOURCE_FILES_PROPERTIES(${WINRT} PROPERTIES COMPILE_FLAGS -ZW)
set_target_properties(${PROJECT_NAME} PROPERTIES STATIC_LIBRARY_FLAGS "/IGNORE:4264")
endif()

# Want ARC on IOS
if (TARGET_IOS)
target_compile_options(${PROJECT_NAME} PUBLIC "-fobjc-arc")
endif()

INCLUDE(ExternalProject)
ExternalProject_Add(
  sdl2
  PREFIX "m3rdparty"
  CMAKE_ARGS "" #-DSDL_SHARED=OFF"
  SOURCE_DIR "${M3RDPARTY_DIR}/sdl"
  TEST_COMMAND ""
  INSTALL_COMMAND "" 
  INSTALL_DIR ""
)
LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/m3rdparty/src/sdl2-build)

IF (TARGET_PC)
LIST(APPEND PLATFORM_LINKLIBS 
    SDL2
    SDL2main
)
ENDIF()
IF (TARGET_MAC)
LIST(APPEND PLATFORM_LINKLIBS 
    sdl2-2.0
    sdl2main
)
ENDIF()
IF (TARGET_LINUX)
LIST(APPEND PLATFORM_LINKLIBS 
    SDL2-2.0
    SDL2main
)
ENDIF()
SOURCE_GROUP ("m3rdparty\\googletest" REGULAR_EXPRESSION "(googlete)+.*") 
SOURCE_GROUP ("m3rdparty\\glm" REGULAR_EXPRESSION "(glm)+.*") 
SOURCE_GROUP ("m3rdparty\\easylogging" REGULAR_EXPRESSION "(easylo)+.*")
SOURCE_GROUP ("m3rdparty\\imgui" REGULAR_EXPRESSION "(imgui)+.*")
SOURCE_GROUP ("m3rdparty\\scintilla" REGULAR_EXPRESSION "(scintilla)+.*")
SOURCE_GROUP ("m3rdparty\\GL" REGULAR_EXPRESSION "(GL)+.*")
SOURCE_GROUP ("m3rdparty\\zip" REGULAR_EXPRESSION "(zip)+.*")
SOURCE_GROUP ("m3rdparty\\mpc" REGULAR_EXPRESSION "(mpc)+.*")

CONFIGURE_FILE(${CMAKE_CURRENT_LIST_DIR}/cmake/config_shared.h.cmake ${CMAKE_BINARY_DIR}/config_shared.h)
