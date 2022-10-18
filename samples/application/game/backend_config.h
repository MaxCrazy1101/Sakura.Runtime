#pragma once
#include "platform/configure.h"
#include "utils/types.h"
#include "resource/config_resource.h"
#if !defined(__meta__) && defined(__cplusplus)
    #include "GameRT/backend_config.rtti.generated.hpp"
    #include "GameRT/json_serialize.generated.h"
    #include "GameRT/binary_serialize.generated.h"
#endif

enum sreflect sattr(
"guid" : "b4b7f387-d8c2-465c-9b3a-6d83a3d198b1",
"serialize" : ["json", "bin"],
"rtti" : true
)
ECGPUBackEnd SKRENUM(uint32_t){
    Vulkan,
    DX12,
    Metal
};
typedef enum ECGPUBackEnd ECGPUBackEnd;

struct sreflect sattr(
"guid" : "b537f7b1-6d2d-44f6-b313-bcb559d3f490",
"serialize" : ["json", "bin"],
"rtti" : true
)
config_backend_t
{
    enum ECGPUBackEnd backend;
}
sregister_config(0);
typedef struct config_backend_t config_backend_t;
