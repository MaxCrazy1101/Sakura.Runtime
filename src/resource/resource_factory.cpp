#include "resource/resource_factory.h"
#include "bitsery/brief_syntax.h"
#include "bitsery/deserializer.h"
#include "bitsery/details/adapter_common.h"
#include "resource/resource_header.h"
#include "platform/debug.h"

namespace skr
{
namespace resource
{
bool SResourceFactory::Deserialize(skr_resource_record_t* record, SBinaryArchive& archive)
{
    bitsery::serialize(archive, record->header);
    return archive.adapter().error() != bitsery::ReaderError::NoError;
}

bool SResourceFactory::Unload(skr_resource_record_t* record)
{
    record->header.dependencies.clear();
    DestroyResource(record);
    return true;
}

void SResourceFactory::DestroyResource(skr_resource_record_t* record)
{
    if (record->destructor)
        record->destructor(record->resource);
}

ESkrInstallStatus SResourceFactory::UpdateInstall(skr_resource_record_t* record)
{
    SKR_UNREACHABLE_CODE();
    return ESkrInstallStatus::SKR_INSTALL_STATUS_SUCCEED;
}
} // namespace resource
} // namespace skr