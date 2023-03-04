#pragma once
#include "../private/gdi_private.h"
#include "nanovg.h"

namespace skr {
namespace gdi {

struct SGDIElementNVG : public SGDIElementPrivate
{
    inline SGDIElementNVG() : nvg(nullptr) {}

    void begin_frame(float devicePixelRatio) final;
    void begin_path() final;
    void rect(float x, float y, float w, float h) final;
    void rounded_rect_varying(float x, float y, float w, float h, float radTopLeft, float radTopRight, float radBottomRight, float radBottomLeft) final;
    
    void move_to(float x, float y) final;
    void line_to(float x, float y) final;

    void stroke_color(uint32_t r, uint32_t g, uint32_t b, uint32_t a) final;
    void stroke_color(float r, float g, float b, float a) final;
    void stroke_width(float size) final;
    void stroke() final;

    void fill_color(uint32_t r, uint32_t g, uint32_t b, uint32_t a) final;
    void fill_color(float r, float g, float b, float a) final;
    void fill_paint(SGDIPaint* paint) final;
    void fill() final;

    NVGcontext* nvg = nullptr;
};

struct SGDIPaintNVG : public SGDIPaintPrivate
{
    void set_pattern(float cx, float cy, float w, float h, float angle, SGDITextureId texture, skr_float4_t ocol) SKR_NOEXCEPT final;
    void set_pattern(float cx, float cy, float w, float h, float angle, SGDIMaterialId texture, skr_float4_t ocol) SKR_NOEXCEPT final;

    NVGpaint nvg_paint;
};

struct SGDICanvasGroupNVG : public SGDICanvasGroupPrivate
{
    // void add_canvas(SGDICanvas* canvas) final;
    // void remove_canvas(SGDICanvas* canvas) final;
};

struct SGDICanvasNVG : public SGDICanvasPrivate
{
    void add_element(SGDIElement* element) SKR_NOEXCEPT final;
};

struct SGDIDeviceNVG : public SGDIDevicePrivate
{
    ~SGDIDeviceNVG();
    int initialize() SKR_NOEXCEPT;
    int finalize() SKR_NOEXCEPT;

    SGDICanvas* create_canvas() final;
    void free_canvas(SGDICanvas* canvas) final;

    SGDICanvasGroup* create_canvas_group() final;
    void free_canvas_group(SGDICanvasGroup* group) final;

    SGDIElement* create_element() final;
    void free_element(SGDIElement* element) final;

    SGDIPaint* create_paint() final;
    void free_paint(SGDIPaint* paint) final;
};


} }