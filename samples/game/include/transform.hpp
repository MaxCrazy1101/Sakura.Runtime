#include "math/matrix.hpp"
#include "math/vector.hpp"
#include "math/rotator.hpp"
#if !defined(__meta__)
    #include "GameRT/transform.dual.generated.hpp"
#endif

struct reflect attr(
    "guid" : "AE2C7477-8A44-4339-BE5D-64D05D7E05B1",
    "component" : true //, "serialize" : "USD"
)
skr_transform_t
{
    skr::math::float4x4 matrix;
};

struct reflect attr(
    "guid" : "78DD218B-87DE-4250-A7E8-A6B4553B47BF",
    "component" : true
)
skr_rotation_t
{
    skr::math::Vector3f euler;
};

struct reflect attr(
    "guid" : "A059A2A1-CC3B-43B0-88B6-ADA7822BA25D",
    "component" : true
)
skr_translation_t
{
    skr::math::Vector3f value;
};

struct reflect attr(
    "guid" : "D045D755-FBD1-44C2-8BF0-C86F2D8485FF",
    "component" : true
)
skr_scale_t
{
    skr::math::Vector3f value;
};