#pragma once
#include <chrono>

#include "SkrRT/platform/memory.h"
#include "SkrRT/platform/debug.h"
#include "SkrRT/io/ram_io.hpp"
#include "SkrRT/misc/make_zeroed.hpp"
#include "SkrRT/misc/types.h"
#include "SkrRT/misc/log.hpp"

#include <SkrRT/platform/filesystem.hpp>
#include <SkrRT/containers/sptr.hpp>
#include <SkrRT/containers/vector.hpp>
#include <SkrRT/containers/string.hpp>
#include <SkrRT/containers/hashmap.hpp>

#include "SkrRT/serde/json/reader.h"
#include "SkrRT/serde/json/writer.h"

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
    #endif
    #include <wtypes.h>
    #include <unknwn.h>
    #include <winbase.h>
    #include <winioctl.h>
#endif
#include "dxc/dxcapi.h"


#include "tracy/Tracy.hpp"