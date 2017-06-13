#pragma once

// Don't let windows include the silly min/max macro
#define NOMINMAX

// Standard library, used all the time.
#include <cstdint>  // uint64_t, etc.
#include <string>
#include <chrono>     // Timing
#include <algorithm>  // Various useful things
#include <memory>     // shared_ptr

// Containers, used all the time.
#include <vector>
#include <map>

// IO
#include <iostream>
#include <fstream>

// Include GLM Math library
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_LANG_STL11_FORCED
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm/glm.hpp"
#include "glm/glm/gtc/matrix_transform.hpp"
#include "glm/glm/gtx/rotate_vector.hpp"
#include "glm/glm/gtc/quaternion.hpp"
#include "glm/glm/gtc/random.hpp"
#include "glm/glm/gtx/string_cast.hpp"

// 2D Gui
#include <imgui/imgui.h>

// Platform toolkit ; audio, windows, input, etc.
#include <sdl/include/SDL.h>

// For logging events to file
#include <easylogging/src/easylogging++.h>

#include "config_shared.h"
