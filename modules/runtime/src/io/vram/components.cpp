#include "../../pch.hpp"
#include "components.hpp"
#include "vram_request.hpp"

namespace skr {
namespace io {

VRAMIOStatusComponent::VRAMIOStatusComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IOStatusComponent(request) 
{
    
}

VRAMIOStagingComponent::VRAMIOStagingComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IORequestComponent(request) 
{
    
}

skr_guid_t VRAMIOStagingComponent::get_tid() const SKR_NOEXCEPT 
{ 
    return IORequestComponentTID<VRAMIOStagingComponent>::Get(); 
}

VRAMBlocksComponent::VRAMBlocksComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IORequestComponent(request) 
{
    
}

skr_guid_t VRAMBlocksComponent::get_tid() const SKR_NOEXCEPT 
{ 
    return IORequestComponentTID<VRAMBlocksComponent>::Get(); 
}

TextureSlicesComponent::TextureSlicesComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IORequestComponent(request) 
{
    
}

skr_guid_t TextureSlicesComponent::get_tid() const SKR_NOEXCEPT 
{ 
    return IORequestComponentTID<TextureSlicesComponent>::Get(); 
}


} // namespace io
} // namespace skr