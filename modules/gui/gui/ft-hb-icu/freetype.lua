target("freetype")
    set_group("00.thirdparty")
    set_kind("static")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    add_includedirs("freetype/include", {public=true})
    add_files(
        "freetype/src/autofit/autofit.c",
        "freetype/src/base/ftbase.c",
        "freetype/src/base/ftbbox.c",
        "freetype/src/base/ftbdf.c",
        "freetype/src/base/ftbitmap.c",
        "freetype/src/base/ftcid.c",
        "freetype/src/base/ftfstype.c",
        "freetype/src/base/ftgasp.c",
        "freetype/src/base/ftglyph.c",
        "freetype/src/base/ftgxval.c",
        "freetype/src/base/ftinit.c",
        "freetype/src/base/ftmm.c",
        "freetype/src/base/ftotval.c",
        "freetype/src/base/ftpatent.c",
        "freetype/src/base/ftpfr.c",
        "freetype/src/base/ftstroke.c",
        "freetype/src/base/ftsynth.c",
        "freetype/src/base/fttype1.c",
        "freetype/src/base/ftwinfnt.c",
        "freetype/src/bdf/bdf.c",
        "freetype/src/bzip2/ftbzip2.c",
        "freetype/src/cache/ftcache.c",
        "freetype/src/cff/cff.c",
        "freetype/src/cid/type1cid.c",
        "freetype/src/gzip/ftgzip.c",
        "freetype/src/lzw/ftlzw.c",
        "freetype/src/pcf/pcf.c",
        "freetype/src/pfr/pfr.c",
        "freetype/src/psaux/psaux.c",
        "freetype/src/pshinter/pshinter.c",
        "freetype/src/psnames/psnames.c",
        "freetype/src/raster/raster.c",
        "freetype/src/sdf/sdf.c",
        "freetype/src/sfnt/sfnt.c",
        "freetype/src/smooth/smooth.c",
        "freetype/src/svg/svg.c",
        "freetype/src/truetype/truetype.c",
        "freetype/src/type1/type1.c",
        "freetype/src/type42/type42.c",
        "freetype/src/winfonts/winfnt.c"
    )
    if (is_plat("windows")) then
        add_files("freetype/builds/windows/ftsystem.c", "freetype/builds/windows/ftdebug.c")
    else
        add_files("freetype/src/base/ftsystem.c", "freetype/src/base/ftdebug.c")
    end
    add_defines("FT2_BUILD_LIBRARY", {public=false})