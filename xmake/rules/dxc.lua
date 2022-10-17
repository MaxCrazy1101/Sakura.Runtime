rule("utils.dxc")
    set_extensions(".hlsl")
    before_buildcmd_file(function (target, batchcmds, sourcefile_hlsl, opt)
        import("find_sdk")
        dxc = find_sdk.find_program("dxc")

        -- get target profile
        target_profile = sourcefile_hlsl:match("^.+%.(.+)%.")
        hlsl_basename = path.filename(sourcefile_hlsl):match("(.+)%..+%..+")

        -- hlsl to spv
        local targetenv = target:extraconf("rules", "utils.dxc", "targetenv") or "vulkan1.1"
        local spv_outputdir =  path.join(path.absolute(target:autogendir()), "rules", "utils", "dxc-spv")
        local spvfilepath = path.join(spv_outputdir, hlsl_basename .. ".spv")
        local spvTextpath = path.join(spv_outputdir, hlsl_basename .. ".h")

        local spv_last = os.time()
        if not opt.quiet then
            batchcmds:show_progress(opt.progress, "${color.build.object}compiling.spirv %s -> %s", sourcefile_hlsl, hlsl_basename .. ".spv")
        end

        batchcmds:mkdir(spv_outputdir)
        batchcmds:vrunv(dxc.vexec, 
            {"-Wno-ignored-attributes",
            "-spirv",
            -- "-fspv-reflect",
            vformat("-fspv-target-env=vulkan1.1"), 
            -- vformat("-fspv-extension=SPV_GOOGLE_user_type"), 
            -- vformat("-fspv-extension=SPV_GOOGLE_hlsl_functionality1"), 
            "-Fo", spvfilepath, 
            "-T", target_profile,
            path.join(os.projectdir(), sourcefile_hlsl)})

        local spv_now = os.time()
        if not opt.quiet then
            batchcmds:show_progress(opt.progress, "${color.success}compiled.spriv %s cost %s seconds", sourcefile_hlsl, spv_now - spv_last)
        end
    
        -- hlsl to dxil
        local dxil_outputdir = path.join(path.absolute(target:autogendir()), "rules", "utils", "dxc-dxil")
        local dxilfilepath = path.join(dxil_outputdir, hlsl_basename .. ".dxil")

        local dxil_last = os.time()
        if not opt.quiet then
            batchcmds:show_progress(opt.progress, "${color.build.object}compiling.dxil %s -> %s", sourcefile_hlsl, hlsl_basename .. ".dxil")
        end

        batchcmds:mkdir(dxil_outputdir)
        batchcmds:vrunv(dxc.vexec, 
            {"-Wno-ignored-attributes", 
            "-Fo ", dxilfilepath, 
            "-T ", target_profile,
            path.join(os.projectdir(), sourcefile_hlsl)})


        local dxil_now = os.time()
        if not opt.quiet then
            batchcmds:show_progress(opt.progress, "${color.success}compiled.dxil %s cost %s seconds", sourcefile_hlsl, dxil_now - dxil_last)
        end

        -- add deps
        batchcmds:add_depfiles(sourcefile_hlsl)
        batchcmds:set_depmtime(os.mtime(spvfilepath))
        batchcmds:set_depcache(target:dependfile(spvfilepath))
        batchcmds:set_depmtime(os.mtime(dxilfilepath))
        batchcmds:set_depcache(target:dependfile(dxilfilepath))
    end)
    
    after_build(function(target)
        local spv_path = path.join(target:autogendir(), "rules/utils/dxc-spv")
        local spv_outdir = target:extraconf("rules", "utils.dxc", "spv_outdir")
        if(spv_outdir ~= nil) then
            os.cp(spv_path.."/*.spv", path.join(target:targetdir(), spv_outdir).."/")
        end
        local dxil_path = path.join(target:autogendir(), "rules/utils/dxc-dxil")
        local dxil_outdir = target:extraconf("rules", "utils.dxc", "dxil_outdir")
        if(dxil_outdir ~= nil) then
            os.cp(dxil_path.."/*.dxil", path.join(target:targetdir(), dxil_outdir).."/")
        end
    end)