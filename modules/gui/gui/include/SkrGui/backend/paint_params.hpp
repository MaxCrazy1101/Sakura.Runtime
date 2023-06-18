#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/math/color.hpp"

// TODO. remove gdi
#include "SkrGui/dev/gdi/gdi_types.hpp"

namespace skr::gui
{
using ITexture = IGDITexture;
using IMaterial = IGDIMaterial;
using CustomPaintfunction_ref = CustomVertexPainter;

enum class EPaintType : uint8_t
{
    Custom,
    Color,
    Texture,
    Material,
};

enum class EStrokeCap : uint8_t
{
    Butt,
    Round,
    Square,
};

enum class EStrokeJoin : uint8_t
{
    Miter,
    Round,
    Bevel,
};

enum class EBlendFactor : uint8_t
{
    Zero,
    One,
    SrcColor,
    OneMinusSrcColor,
    DstColor,
    OneMinusDstColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha,
    SrcAlphaSaturate,
};

enum class ESwizzleChannel
{
    R,
    G,
    B,
    A,
    Zero,
    One,
};

struct Swizzle {
    ESwizzleChannel r = ESwizzleChannel::R;
    ESwizzleChannel g = ESwizzleChannel::G;
    ESwizzleChannel b = ESwizzleChannel::B;
    ESwizzleChannel a = ESwizzleChannel::A;
};

struct BlendMode {
    EBlendFactor src_color = EBlendFactor::One;
    EBlendFactor dst_color = EBlendFactor::OneMinusSrcAlpha;
    EBlendFactor src_alpha = EBlendFactor::One;
    EBlendFactor dst_alpha = EBlendFactor::OneMinusSrcAlpha;
};
} // namespace skr::gui