add_requires("lemon 1.3.1")
add_requires("parallel-hashmap >=1.3.11-skr")
add_requires("simdjson >=3.0.0-skr")

shared_module("SkrCore", "SKR_CORE", engine_version)
    set_group("01.modules")
    -- add source files
    add_packages("lemon", {public = false, inherit = false})
    add_packages("simdjson", {public = true, inherit = true})
    add_packages("parallel-hashmap", {public = true, inherit = true})
    add_deps("SkrProfile", "SkrBase", {public = true, inherit = true})
    add_deps("mimalloc", {public = false})
    add_includedirs("include", {public = true})
    add_defines("SKR_MEMORY_IMPL", {public = false})
    add_files("src/build.*.c", "src/build.*.cpp")
    -- add OpenString defines
    add_defines("OPEN_STRING_API=SKR_MEMORY_API", {public = true})