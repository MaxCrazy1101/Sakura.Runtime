#pragma once
#include "SkrLive2D/skr_live2d.configure.h"

#ifdef __cplusplus
namespace Live2D { namespace Cubism { namespace Framework {
    class CubismModelSettingJson;
    class csmUserModel;
    class csmString;
    class ACubismMotion;

    class csmMotionMap;
    class csmBreathVector;
    class csmIdVector;
}}}
typedef class Live2D::Cubism::Framework::CubismModelSettingJson* CubismModelSettingJsonId;
typedef class Live2D::Cubism::Framework::csmUserModel* csmUserModelId;
typedef class Live2D::Cubism::Framework::csmMotionMap* csmMotionMapId;
typedef class Live2D::Cubism::Framework::csmBreathVector* csmBreathVectorId;
typedef class Live2D::Cubism::Framework::csmIdVector* csmIdVectorId;
#else
typedef struct CubismModelSettingJson* CubismModelSettingJsonId;
typedef struct csmUserModel* csmUserModelId;
typedef struct csmMotionMap* csmMotionMapId;
typedef struct csmBreathVector* csmBreathVectorId;
typedef struct csmIdVector* csmIdVectorId;
#endif

// Made up with: 
// [Setting], [Model], [Expression], [Motions], [Physics]
// [Pose], EyeBlink, [Breath], [UserData], [EyeBlinkIds], [LipSyncIds]
struct skr_live2d_model_resource_t {
    CubismModelSettingJsonId model_setting;
    csmUserModelId model;
    csmMotionMapId expression_map;
    csmMotionMapId motion_map;
    csmBreathVectorId breath_vector;
    csmIdVectorId eye_blink_ids;
    csmIdVectorId lip_sync_ids;
};
typedef struct skr_live2d_model_resource_t skr_live2d_model_resource_t;
typedef struct skr_live2d_model_resource_t* skr_live2d_model_resource_id;

#include "utils/io.h"

typedef void (*skr_async_live2d_io_callback_t)(struct skr_live2d_ram_io_request_t* request, void* data);
typedef struct skr_live2d_ram_io_request_t {
    struct skr_vfs_t* vfs_override;
    skr_async_io_request_t settingsRequest;
    SAtomic32 liv2d_status;
    skr_live2d_model_resource_id model_resource;
#ifdef __cplusplus
    bool is_ready() const SKR_NOEXCEPT
    {
        return get_status() == SKR_ASYNC_IO_STATUS_OK;
    }
    SkrAsyncIOStatus get_status() const SKR_NOEXCEPT
    {
        return (SkrAsyncIOStatus)skr_atomic32_load_acquire(&liv2d_status);
    }
#endif
} skr_live2d_ram_io_request_t;

#ifndef SKR_SERIALIZE_GURAD
SKR_LIVE2D_EXTERN_C SKR_LIVE2D_API void 
skr_live2d_model_create_from_json(skr_io_ram_service_t* ioService, const char* path, skr_live2d_ram_io_request_t* request);
#endif

SKR_LIVE2D_EXTERN_C SKR_LIVE2D_API void 
skr_live2d_model_free(skr_live2d_model_resource_id live2d_resource);