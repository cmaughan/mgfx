
list(APPEND COMMON_SOURCES

mcommon/schema/common.fbs
mcommon/schema/material.fbs
mcommon/schema/model.fbs
mcommon/schema/model_generated.h

mcommon/ui/ui_manager.cpp
mcommon/ui/ui_manager.h

mcommon/animation/timer.cpp
mcommon/animation/timer.h

mcommon/file/fileutils.cpp
mcommon/file/fileutils.h
mcommon/file/media_manager.cpp
mcommon/file/media_manager.h

mcommon/math/mathutils.h
mcommon/math/mathutils.cpp
mcommon/math/rectstack.h

mcommon/string/stringutils.cpp
mcommon/string/stringutils.h

mcommon/string/hashstring.h
mcommon/string/hashstring.cpp

mcommon/string/murmur_hash.cpp
mcommon/string/murmur_hash.h

mcommon/graphics/primitives2d.cpp
mcommon/graphics/primitives2d.h

mcommon/mcommon.h
mcommon/mcommon.cpp

)
set(MCOMMON_ROOT ${CMAKE_CURRENT_LIST_DIR} CACHE STRING "" FORCE)
