﻿#include <platform/filesystem.hpp>
#include "utils/defer.hpp"
#include "utils/format.hpp"
#include "platform/vfs.h"
#include "resource/local_resource_registry.h"
#include "resource/resource_header.h"

namespace skr::resource
{
SLocalResourceRegistry::SLocalResourceRegistry(skr_vfs_t* vfs)
    : vfs(vfs)
{
}
bool SLocalResourceRegistry::RequestResourceFile(SResourceRequest* request)
{
    namespace bin = skr::binary;
    //简单实现，直接在 resource 路径下按 guid 找到文件读信息，没有单独的数据库
    auto guid = request->resourceRecord->header.guid;
    skr::filesystem::path path = fmt::format("{}.bin", guid);
    // TODO: 检查文件存在？
    auto file = skr_vfs_fopen(vfs, path.u8string().c_str(), SKR_FM_READ, SKR_FILE_CREATION_OPEN_EXISTING);
    if (!file) return false;
    char buffer[sizeof(skr_resource_header_t)];
    skr_vfs_fread(file, buffer, 0, sizeof(skr_resource_header_t));
    SKR_DEFER({ skr_vfs_fclose(file); });
    struct SpanReader
    {
        gsl::span<char> data;
        size_t offset = 0;
        int read(void* dst, size_t size)
        {
            if (offset + size > data.size())
                return -1;
            memcpy(dst, data.data() + offset, size);
            offset += size;
            return 0;
        }
    } reader = {buffer};
    skr_binary_reader_t archive{reader};
    skr_resource_header_t header;
    uint32_t function = 1;
    int ret = bin::Archive(&archive, function);
    if (ret != 0)
        return false;
    //NOTE: this code is derived from resource header serialization
    SKR_ASSERT(function == 1);
    ret = bin::Archive(&archive, header.version);
    if (ret != 0)
        return false;
    ret = bin::Archive(&archive, header.guid);
    if (ret != 0)
        return false;
    SKR_ASSERT(header.guid == guid);
    ret = bin::Archive(&archive, header.type);
    if (ret != 0)
        return false;
    // TODO: 把这些封装起来传过去少暴露一些细节？
    request->resourceRecord->header.type = header.type;
    request->resourceRecord->header.version = header.version;
    request->path = path;
    request->vfs = vfs;
    request->OnRequestFileFinished();
    return true;
}

void SLocalResourceRegistry::CancelRequestFile(SResourceRequest* requst)
{

}
} // namespace skr::resource