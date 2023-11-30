#pragma once

#ifdef _WIN32
#include <intrin.h> // IWYU pragma: export
#include <new.h> // IWYU pragma: export
#endif

#include <chrono> // IWYU pragma: export
#include "SkrBase/misc/debug.h"  // IWYU pragma: export
#include "SkrRT/platform/guid.hpp" // IWYU pragma: export
#include <SkrRT/platform/filesystem.hpp> // IWYU pragma: export

#include "SkrRT/misc/log.h" // IWYU pragma: export
#include "SkrRT/misc/log.hpp" // IWYU pragma: export

#include <SkrRT/containers_new/concurrent_queue.h> // IWYU pragma: export
#include <SkrRT/containers_new/sptr.hpp> // IWYU pragma: export
#include <SkrRT/containers_new/string.hpp> // IWYU pragma: export
#include <SkrRT/containers_new/vector.hpp> // IWYU pragma: export
#include <SkrRT/containers_new/hashmap.hpp> // IWYU pragma: export

#include "SkrProfile/profile.h" // IWYU pragma: export