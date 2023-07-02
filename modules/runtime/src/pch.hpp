#pragma once

#ifdef _WIN32
#include <intrin.h>
#include "platform/windows/winheaders.h"
#endif

#include <chrono>
#include "platform/debug.h"
#include <platform/filesystem.hpp>

#include <containers/sptr.hpp>
#include <containers/string.hpp>
#include <containers/vector.hpp>
#include <containers/hashmap.hpp>

#include "tracy/Tracy.hpp"