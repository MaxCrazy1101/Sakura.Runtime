set_languages("c11", "cxx17")
add_rules("mode.debug", "mode.release", "mode.releasedbg", "mode.asan")

target("simdjson")
    set_kind("static")    
    set_optimize("fastest")
    add_includedirs("simdjson/include", {public=true})
    add_files("simdjson/source/simdjson.cpp")
    add_headerfiles("simdjson/include/(**.h)")
    add_defines("SIMDJSON_IMPLEMENTATION_HASWELL=0")
    add_defines("SIMDJSON_AVX512_ALLOWED=0")