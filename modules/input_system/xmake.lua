shared_module("SkrInputSystem", "SKR_INPUTSYSTEM", engine_version)
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    public_dependency("SkrRT", engine_version)
    public_dependency("SkrInput", engine_version)
    add_includedirs("include", {public=true})
    add_files("src/*.cpp")