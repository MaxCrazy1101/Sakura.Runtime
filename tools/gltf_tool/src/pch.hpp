#pragma once
#include "cgpu/api.h" // IWYU pragma: export
#include "cgltf/cgltf.h" // IWYU pragma: export
#include "MeshOpt/meshoptimizer.h" // IWYU pragma: export

#include "SkrRT/platform/vfs.h" // IWYU pragma: export
#include "SkrRT/platform/filesystem.hpp" // IWYU pragma: export
#include "SkrRT/platform/guid.hpp" // IWYU pragma: export
#include "SkrBase/misc/defer.hpp" // IWYU pragma: export
#include "SkrRT/misc/log.hpp" // IWYU pragma: export
#include "SkrRT/async/parallel_for.hpp" // IWYU pragma: export
#include "SkrRT/async/fib_task.hpp" // IWYU pragma: export
#include "SkrRT/io/ram_io.hpp" // IWYU pragma: export

#include "SkrRT/module/module.hpp" // IWYU pragma: export
#include "SkrRT/resource/config_resource.h" // IWYU pragma: export

#include "SkrContainers/string.hpp" // IWYU pragma: export
#include "SkrContainers/vector.hpp" // IWYU pragma: export
#include "SkrContainers/hashmap.hpp" // IWYU pragma: export

#include "SkrRT/serde/json/reader.h" // IWYU pragma: export
#include "SkrRT/serde/json/writer.h" // IWYU pragma: export
#include "SkrRT/serde/binary/writer.h" // IWYU pragma: export

#include "SkrProfile/profile.h" // IWYU pragma: export