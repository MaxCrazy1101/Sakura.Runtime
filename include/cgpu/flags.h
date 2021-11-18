#pragma once
#include "cgpu_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// enums
typedef enum ECGpuNvAPI_Status
{
    CGPU_NVAPI_OK = 0, //!< Success. Request is completed.
    CGPU_NVAPI_NONE = 1,
    CGPU_NVAPI_ERROR = -1, //!< Generic error
} ECGpuNvAPI_Status;

typedef enum ECGpuAGSReturnCode
{
    CGPU_AGS_SUCCESS,                 ///< Successful function call
    CGPU_AGS_FAILURE,                 ///< Failed to complete call for some unspecified reason
    CGPU_AGS_INVALID_ARGS,            ///< Invalid arguments into the function
    CGPU_AGS_OUT_OF_MEMORY,           ///< Out of memory when allocating space internally
    CGPU_AGS_MISSING_D3D_DLL,         ///< Returned when a D3D dll fails to load
    CGPU_AGS_LEGACY_DRIVER,           ///< Returned if a feature is not present in the installed driver
    CGPU_AGS_NO_AMD_DRIVER_INSTALLED, ///< Returned if the AMD GPU driver does not appear to be installed
    CGPU_AGS_EXTENSION_NOT_SUPPORTED, ///< Returned if the driver does not support the requested driver extension
    CGPU_AGS_ADL_FAILURE,             ///< Failure in ADL (the AMD Display Library)
    CGPU_AGS_DX_FAILURE,              ///< Failure from DirectX runtime
    CGPU_AGS_NONE
} ECGpuAGSReturnCode;

typedef enum ECGpuVertexFormat
{
    VF_UNDEFINED,
    VF_UCHAR2,
    VF_UCHAR4,
    VF_CHAR2,
    VF_CHAR4,
    VF_UCHAR2_NORM,
    VF_UCHAR4_NORM,
    VF_CHAR2_NORM,
    VF_CHAR4_NORM,
    VF_USHORT2,
    VF_USHORT4,
    VF_SHORT2,
    VF_SHORT4,
    VF_USHORT2_NORM,
    VF_USHORT4_NORM,
    VF_SHORT2_NORM,
    VF_SHORT4_NORM,
    VF_HALF2,
    VF_HALF4,
    VF_FLOAT,
    VF_FLOAT2,
    VF_FLOAT3,
    VF_FLOAT4,
    VF_UINT,
    VF_UINT2,
    VF_UINT3,
    VF_UINT4,
    VF_INT,
    VF_INT2,
    VF_INT3,
    VF_INT4,
    VF_COUNT,

    VF_R8G8_UNORM = VF_UCHAR2_NORM,
    VF_R8G8_NORM = VF_CHAR2_NORM,
    VF_R8G8B8A8_UNORM = VF_UCHAR4_NORM,
    VF_R8G8B8A8_NORM = VF_CHAR4_NORM,
    VF_R16G16_UNORM = VF_USHORT2_NORM,
    VF_R16G16_NORM = VF_SHORT2_NORM,
    VF_R16G16B16A16_UNORM = VF_USHORT4_NORM,
    VF_R16G16B16A16_NORM = VF_SHORT4_NORM,
    VF_R32G32B32A32_UINT = VF_UINT4,
    VF_R32G32B32A32_INT = VF_INT4,
    VF_R32G32B32A32_SINT = VF_INT4,
    VF_R32G32_UINT = VF_UINT2,
    VF_R32G32_INT = VF_INT2,
    VF_R32G32_SINT = VF_INT2,
} ECGpuVertexFormat;

typedef enum ECGpuPixelFormat
{
    PF_UNDEFINED = 0,
    PF_R32G32B32A32_UINT = 1,
    PF_R32G32B32A32_SINT = 2,
    PF_R32G32B32A32_FLOAT = 3,
    PF_R32G32B32_UINT = 4,
    PF_R32G32B32_SINT = 5,
    PF_R32G32B32_FLOAT = 6,
    PF_R16G16B16A16_UNORM = 7,
    PF_R16G16B16A16_SNORM = 8,
    PF_R16G16B16A16_UINT = 9,
    PF_R16G16B16A16_SINT = 10,
    PF_R16G16B16A16_FLOAT = 11,
    PF_R32G32_UINT = 12,
    PF_R32G32_SINT = 13,
    PF_R32G32_FLOAT = 14,
    PF_R10G10B10A2_UNORM = 15,
    PF_R10G10B10A2_UINT = 16,
    PF_R9G9B9E5_UFLOAT = 17,
    PF_R8G8B8A8_UNORM = 18,
    PF_R8G8B8A8_SNORM = 19,
    PF_R8G8B8A8_UINT = 20,
    PF_R8G8B8A8_SINT = 21,
    PF_R8G8B8A8_UNORM_SRGB = 22,
    PF_B8G8R8A8_UNORM = 23,
    PF_B8G8R8A8_UNORM_SRGB = 24,
    PF_R11G11B10_FLOAT = 25,
    PF_R16G16_UNORM = 26,
    PF_R16G16_SNORM = 27,
    PF_R16G16_UINT = 28,
    PF_R16G16_SINT = 29,
    PF_R16G16_FLOAT = 30,
    PF_R32_UINT = 31,
    PF_R32_SINT = 32,
    PF_R32_FLOAT = 33,
    PF_B5G5R5A1_UNORM = 34,
    PF_B5G6R5_UNORM = 35,
    PF_R8G8_UNORM = 36,
    PF_R8G8_SNORM = 37,
    PF_R8G8_UINT = 38,
    PF_R8G8_SINT = 39,
    PF_R16_UNORM = 40,
    PF_R16_SNORM = 41,
    PF_R16_UINT = 42,
    PF_R16_SINT = 43,
    PF_R16_FLOAT = 44,
    PF_R8_UNORM = 45,
    PF_R8_SNORM = 46,
    PF_R8_UINT = 47,
    PF_R8_SINT = 48,
    PF_D24_UNORM_S8_UINT = 49,
    PF_D32_FLOAT = 50,
    PF_D16_UNORM = 51,
    PF_BC1_UNORM = 52,
    PF_BC1_UNORM_SRGB = 53,
    PF_BC2_UNORM = 54,
    PF_BC2_UNORM_SRGB = 55,
    PF_BC3_UNORM = 56,
    PF_BC3_UNORM_SRGB = 57,
    PF_BC4_UNORM = 58,
    PF_BC4_SNORM = 59,
    PF_BC5_UNORM = 60,
    PF_BC5_SNORM = 61,
    PF_BC6H_UF16 = 62,
    PF_BC6H_SF16 = 63,
    PF_BC7_UNORM = 64,
    PF_BC7_UNORM_SRGB = 65,
    PF_Count = PF_BC7_UNORM_SRGB + 1
} ECGpuPixelFormat;

typedef enum ECGpuShaderStage
{
    SS_NONE = 0,

    SS_VERT = 0X00000001,
    SS_TESC = 0X00000002,
    SS_TESE = 0X00000004,
    SS_GEOM = 0X00000008,
    SS_FRAG = 0X00000010,
    SS_COMPUTE = 0X00000020,

    SS_ALL_GRAPHICS = (uint32_t)SS_VERT | (uint32_t)SS_TESC | (uint32_t)SS_TESE | (uint32_t)SS_GEOM | (uint32_t)SS_FRAG,
    SS_COUNT = 6
} ECGpuShaderStage;
typedef uint32_t ECGpuShaderStages;

typedef enum CGpuMemoryUsage
{
    /// No intended memory usage specified.
    MU_UNKNOWN = 0,
    /// Memory will be used on device only, no need to be mapped on host.
    MU_GPU_ONLY = 1,
    /// Memory will be mapped on host. Could be used for transfer to device.
    MU_CPU_ONLY = 2,
    /// Memory will be used for frequent (dynamic) updates from host and reads on device.
    MU_CPU_TO_GPU = 3,
    /// Memory will be used for writing on device and readback on host.
    MU_GPU_TO_CPU = 4,
    MU_COUNT,
    MU_MAX_ENUM = 0x7FFFFFFF
} CGpuMemoryUsage;

typedef enum CGpuDescriptorType
{
    DT_UNDEFINED = 0,
    DT_SAMPLER = 0x01,
    // SRV Read only texture
    DT_TEXTURE = (DT_SAMPLER << 1),
    /// UAV Texture
    DT_RW_TEXTURE = (DT_TEXTURE << 1),
    // SRV Read only buffer
    DT_BUFFER = (DT_RW_TEXTURE << 1),
    DT_BUFFER_RAW = (DT_BUFFER | (DT_BUFFER << 1)),
    /// UAV Buffer
    DT_RW_BUFFER = (DT_BUFFER << 2),
    DT_RW_BUFFER_RAW = (DT_RW_BUFFER | (DT_RW_BUFFER << 1)),
    /// Uniform buffer
    DT_UNIFORM_BUFFER = (DT_RW_BUFFER << 2),
    /// Push constant / Root constant
    DT_ROOT_CONSTANT = (DT_UNIFORM_BUFFER << 1),
    /// IA
    DT_VERTEX_BUFFER = (DT_ROOT_CONSTANT << 1),
    DT_INDEX_BUFFER = (DT_VERTEX_BUFFER << 1),
    DT_INDIRECT_BUFFER = (DT_INDEX_BUFFER << 1),
    /// Cubemap SRV
    DT_TEXTURE_CUBE = (DT_TEXTURE | (DT_INDIRECT_BUFFER << 1)),
    /// RTV / DSV per mip slice
    DT_RENDER_TARGET_MIP_SLICES = (DT_INDIRECT_BUFFER << 2),
    /// RTV / DSV per array slice
    DT_RENDER_TARGET_ARRAY_SLICES = (DT_RENDER_TARGET_MIP_SLICES << 1),
    /// RTV / DSV per depth slice
    DT_RENDER_TARGET_DEPTH_SLICES = (DT_RENDER_TARGET_ARRAY_SLICES << 1),
    DT_RAY_TRACING = (DT_RENDER_TARGET_DEPTH_SLICES << 1),
#if defined(CGPU_USE_VULKAN)
    /// Subpass input (descriptor type only available in Vulkan)
    DT_INPUT_ATTACHMENT = (DT_RAY_TRACING << 1),
    DT_TEXEL_BUFFER = (DT_INPUT_ATTACHMENT << 1),
    DT_RW_TEXEL_BUFFER = (DT_TEXEL_BUFFER << 1),
    DT_COMBINED_IMAGE_SAMPLER = (DT_RW_TEXEL_BUFFER << 1),
#endif
#if defined(CGPU_USE_METAL)
    DT_ARGUMENT_BUFFER = (DT_RAY_TRACING << 1),
    DT_INDIRECT_COMMAND_BUFFER = (DT_ARGUMENT_BUFFER << 1),
    DT_RENDER_PIPELINE_STATE = (DT_INDIRECT_COMMAND_BUFFER << 1),
#endif
} CGpuDescriptorType;
typedef uint32_t CGpuDescriptorTypes;

#ifdef __cplusplus
} // end extern "C"
#endif
