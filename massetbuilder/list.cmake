LIST(APPEND MASSETBUILDER_APP_SOURCE
    massetbuilder/app/main.cpp
    massetbuilder/app/assetbuilder.cpp
    massetbuilder/app/assetbuilder.h
    massetbuilder/app/massetbuilder_settings.cpp
    massetbuilder/app/massetbuilder_settings.h
    massetbuilder/app/massetbuilder_app.h
    massetbuilder/list.cmake
)

IF (PROJECT_DEVICE_DX12)
LIST (APPEND MASSETBUILDER_APP_SOURCE
    massetbuilder/app/dxcompiler.cpp
    massetbuilder/app/dxcompiler.h
    )
ENDIF()

LIST(APPEND TEST_SOURCES
    massetbuilder/app/massetbuilder_settings.cpp
    massetbuilder/app/massetbuilder_settings.h
)

IF (PROJECT_SHADERTOOLS)
include (ExternalProject)
ExternalProject_Add(
  glslang
  PREFIX "m3rdparty"
  CMAKE_ARGS -DENABLE_AMD_EXTENSIONS=ON -DENABLE_GLSLANG_BINARIES=ON -DENABLE_NV_EXTENSIONS=ON -DENABLE_HLSL=ON
  SOURCE_DIR "${M3RDPARTY_DIR}/glslang"
  TEST_COMMAND ""
  INSTALL_COMMAND "" 
  INSTALL_DIR ""
)

ExternalProject_Add(
  spirv-cross
  PREFIX "m3rdparty"
  CMAKE_ARGS "" 
  SOURCE_DIR "${M3RDPARTY_DIR}/spirv-cross"
  TEST_COMMAND ""
  INSTALL_COMMAND "" 
  INSTALL_DIR ""
)
ENDIF()