#include "SkrToolCore/asset/cook_system.hpp"
#include "SkrShaderCompiler/assets/material_asset.hpp"
#include "utils/io.h"
#include "SkrToolCore/project/project.hpp"
#include "json/reader.h"

namespace skd
{
namespace asset
{

void* SMaterialTypeImporter::Import(skr_io_ram_service_t* ioService, SCookContext *context)
{
    const auto assetRecord = context->GetAssetRecord();
    skr_async_ram_destination_t destination = {};
    context->AddFileDependencyAndLoad(ioService, jsonPath.c_str(), destination);
    SKR_DEFER({sakura_free(destination.bytes);});

    auto jsonString = simdjson::padded_string((char8_t*)destination.bytes, destination.size);
    simdjson::ondemand::parser parser;
    auto doc = parser.iterate(jsonString);
    if(doc.error())
    {
        SKR_LOG_FMT_ERROR("Import shader options asset {} from {} failed, json parse error {}", assetRecord->guid, jsonPath, simdjson::error_message(doc.error()));
        return nullptr;
    }
    auto json_value = doc.get_value().value_unsafe();

    // create source code wrapper
    auto type_asset = SkrNew<skr_material_type_asset_t>();
    skr::json::Read(std::move(json_value), *type_asset);
    return type_asset;
}

void SMaterialTypeImporter::Destroy(void *resource)
{
    auto type_asset = (skr_material_type_asset_t*)resource;
    SkrDelete(type_asset);
}

bool SMaterialTypeCooker::Cook(SCookContext *ctx)
{
    const auto outputPath = ctx->GetOutputPath();
    const auto assetRecord = ctx->GetAssetRecord();
    //-----load config
    // no cook config for config, skipping
    //-----import resource object
    auto material_type = ctx->Import<skr_material_type_asset_t>();
    if(!material_type) return false;
    SKR_DEFER({ ctx->Destroy(material_type); });
    eastl::vector<uint8_t> buffer;
    skr::binary::VectorWriter writer{&buffer};
    skr_binary_writer_t archive(writer);
    skr::binary::Archive(&archive, *material_type);

    //------save resource to disk
    auto file = fopen(outputPath.u8string().c_str(), "wb");
    if (!file)
    {
        SKR_LOG_FMT_ERROR("[SShaderOptionsCooker::Cook] failed to write cooked file for resource {}! path: {}", 
            assetRecord->guid, assetRecord->path.u8string());
        return false;
    }
    SKR_DEFER({ fclose(file); });
    fwrite(buffer.data(), 1, buffer.size(), file);
    return true;
}


} // namespace asset
} // namespace skd