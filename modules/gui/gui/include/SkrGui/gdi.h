#pragma once
#include "SkrGui/module.configure.h"
#include "utils/types.h"

namespace skr { namespace render_graph { class RenderGraph; } }

namespace skr {
namespace gdi {

using index_t = uint16_t;
struct SGDITexture;
struct SGDIMaterial;
typedef struct SGDITexture* SGDITextureId;
typedef struct SGDIMaterial* SGDIMaterialId;

template<typename T>
struct LiteSpan
{
    inline constexpr uint64_t size() const SKR_NOEXCEPT { return size_; }
    inline SKR_CONSTEXPR T* data() const SKR_NOEXCEPT { return data_; }
    inline SKR_CONSTEXPR T& operator[](uint64_t index) const SKR_NOEXCEPT { return data_[index]; }
    inline SKR_CONSTEXPR T* begin() const SKR_NOEXCEPT { return data_; }
    inline SKR_CONSTEXPR T* end() const SKR_NOEXCEPT { return data_ + size_; }
    inline SKR_CONSTEXPR bool empty() const SKR_NOEXCEPT { return size_ == 0; }
    T* data_ = nullptr;
    uint64_t size_ = 0;
};

enum class EGDIBackend
{
    NANOVG,
    Count
};

struct SGDIPaint
{
    virtual ~SGDIPaint() SKR_NOEXCEPT = default;

    virtual void set_pattern(float cx, float cy, float w, float h, float angle, SGDITextureId texture, skr_float4_t ocol) SKR_NOEXCEPT = 0;
    virtual void set_pattern(float cx, float cy, float w, float h, float angle, SGDIMaterialId material, skr_float4_t ocol) SKR_NOEXCEPT = 0;
};

struct SGDIVertex
{
    skr_float4_t position;
    skr_float2_t texcoord;
    skr_float2_t aa;
    skr_float2_t clipUV; //uv in clipspace
    skr_float2_t clipUV2;
    uint32_t     color; 
};

struct SGDIElementDrawCommand
{
    SGDITextureId texture;
    SGDIMaterialId material;
    uint32_t first_index;
    uint32_t index_count;
};

struct SKR_GUI_API SGDIElement
{
    friend struct SGDIRenderer;
    virtual ~SGDIElement() SKR_NOEXCEPT = default;
    
    virtual void begin_frame(float devicePixelRatio) = 0;
    virtual void begin_path() = 0;
    virtual void rect(float x, float y, float w, float h) = 0;
    virtual void rounded_rect_varying(float x, float y, float w, float h, float radTopLeft, float radTopRight, float radBottomRight, float radBottomLeft) = 0;
    virtual void fill_color(uint32_t r, uint32_t g, uint32_t b, uint32_t a) = 0;
    virtual void fill_color(float r, float g, float b, float a) = 0;
    virtual void fill_paint(SGDIPaint* paint) = 0;
    virtual void fill() = 0;
};

struct SKR_GUI_API SGDICanvas
{
    virtual ~SGDICanvas() SKR_NOEXCEPT = default;

    virtual void add_element(SGDIElement* element, const skr_float4_t& transform) SKR_NOEXCEPT = 0;
    virtual void remove_element(SGDIElement* element) SKR_NOEXCEPT = 0;
    virtual LiteSpan<SGDIElement*> all_elements() SKR_NOEXCEPT = 0;
};

struct SKR_GUI_API SGDICanvasGroup
{
    virtual ~SGDICanvasGroup() SKR_NOEXCEPT = default;

    virtual void add_canvas(SGDICanvas* canvas) SKR_NOEXCEPT = 0;
    virtual void remove_canvas(SGDICanvas* canvas) SKR_NOEXCEPT = 0;
    virtual LiteSpan<SGDICanvas*> all_canvas() SKR_NOEXCEPT = 0;
};

struct SKR_GUI_API SGDIDevice
{
    virtual ~SGDIDevice() SKR_NOEXCEPT = default;

    [[nodiscard]] static SGDIDevice* Create(EGDIBackend backend);
    static void Free(SGDIDevice* device);

    [[nodiscard]] virtual SGDICanvas* create_canvas();
    virtual void free_canvas(SGDICanvas* canvas);

    [[nodiscard]] virtual SGDICanvasGroup* create_canvas_group();
    virtual void free_canvas_group(SGDICanvasGroup* canvas_group);

    [[nodiscard]] virtual SGDIElement* create_element() = 0;
    virtual void free_element(SGDIElement* element) = 0;

    [[nodiscard]] virtual SGDIPaint* create_paint() = 0;
    virtual void free_paint(SGDIPaint* paint) = 0;
};

struct SGDIRendererDescriptor
{
    void* usr_data = nullptr;
};

struct SGDIRenderParams
{
    void* usr_data = nullptr;
};

enum class EGDIResourceState : uint32_t
{
    Requsted     =    0x00000001,
    Loading      =    0x00000002,
    Initializing =    0x00000004,
    Okay         =    0x00000008,
    Finalizing   =    0x00000010,
    Count = 5
};

enum class EGDITextureType : uint32_t
{
    Texture2D,
    Texture2DArray,
    Atlas,
    Count
};

typedef struct SGDITextureDescriptor
{
    const char* u8Uri = nullptr;
    void* usr_data = nullptr;
} SGDITextureDescriptor;

struct SKR_GUI_API SGDITexture
{
    virtual ~SGDITexture() SKR_NOEXCEPT = default;
    
    virtual EGDIResourceState get_state() const SKR_NOEXCEPT = 0;
    virtual EGDITextureType get_type() const SKR_NOEXCEPT = 0;
};

struct SKR_GUI_API SGDIRenderer
{
    virtual ~SGDIRenderer() SKR_NOEXCEPT = default;

    virtual LiteSpan<SGDIVertex> fetch_element_vertices(SGDIElement* element) SKR_NOEXCEPT;
    virtual LiteSpan<index_t> fetch_element_indices(SGDIElement* element) SKR_NOEXCEPT;
    virtual LiteSpan<SGDIElementDrawCommand> fetch_element_draw_commands(SGDIElement* element) SKR_NOEXCEPT;

    // Tier 1
    virtual int initialize(const SGDIRendererDescriptor* desc) SKR_NOEXCEPT = 0;
    virtual int finalize() SKR_NOEXCEPT = 0;
    virtual SGDITextureId create_texture(const SGDITextureDescriptor* descriptor) SKR_NOEXCEPT = 0;
    virtual void free_texture(SGDITextureId texture) SKR_NOEXCEPT = 0;
    virtual void render(SGDICanvasGroup* canvas_group, SGDIRenderParams* params) SKR_NOEXCEPT = 0;

    // Tier 2
};

} }
