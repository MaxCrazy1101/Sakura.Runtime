#include "asset/importer.hpp"
#include "bitsery/adapter/buffer.h"
#include "ghc/filesystem.hpp"
#include "google/protobuf/empty.pb.h"
#include "platform/guid.h"
#include "resource/config_resource.h"
#include "resource/resource_header.h"
#include "skrcompiler.grpc.pb.h"
#include "skrcompiler.pb.h"
#include <grpcpp/completion_queue.h>
#include <grpcpp/support/status.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/security/credentials.h>
#include <grpcpp/create_channel.h>
#include "asset/config_asset.hpp"
#include "bitsery/serializer.h"
#include "type/type_registry.h"
#include "utils/defer.hpp"
#include "resource/resource_header.h"
#include "SkrTool/serialize.generated.h"
#include "SkrRT/serialize.generated.h"
class CompileResourceImpl final : public skrcompiler::CompileResource::Service
{
    ::grpc::Status Compile(::grpc::ServerContext* context, const ::skrcompiler::CompileInfo* request, ::skrcompiler::CompileResult* response) override
    {
        using namespace grpc;
        response->set_errorcode(0);
        response->set_errormessage("fuck");
        (void)stub;

        return Status::OK;
    }
    skrcompiler::HostResource::Stub* stub;
};

class HostResourceImpl final : public skrcompiler::HostResource::Service
{
    ::grpc::Status GetPath(::grpc::ServerContext* context, const ::skrcompiler::ResourceId* request, ::skrcompiler::ResourcePath* response) override
    {
        using namespace grpc;
        response->set_path(request->guid() + "asdsad");
        return Status::OK;
    }

    ::grpc::Status Register(::grpc::ServerContext* context, const ::skrcompiler::Port* request, ::google::protobuf::Empty* response) override
    {
        using namespace grpc;
        std::cout << "Worker registering on localhost:" << request->number() << std::endl;
        stub.push_back(std::make_unique<skrcompiler::CompileResource::Stub>(CreateChannel("localhost:" + request->number(), InsecureChannelCredentials())));

        std::cout << "Worker connected on localhost:" << request->number() << std::endl;
        return Status::OK;
    }
    std::vector<std::unique_ptr<skrcompiler::CompileResource::Stub>> stub;
};

void compile_config(skd::asset::SAssetRecord* record)
{
    using namespace skd;
    asset::SAssetRegistry& registry = *asset::GetAssetRegistry();
    asset::SImporterFactory* factory = new asset::SJsonConfigImporterFactory;
    //-----load importer & import resource object
    simdjson::ondemand::parser parser;
    auto doc = parser.iterate(record->meta);
    auto importer = factory->LoadImporter(record, doc.find_field("importer").value_unsafe());
    auto resource = (skr_config_resource_t*)importer->Import(registry.GetAssetRecord(importer->assetGuid));
    //-----cook resource & fetch runtime dependencies
    // todo: get resource references in config
    //-----write resource header
    skr_resource_header_t header;
    header.guid = record->guid;
    header.type = get_type_id_skr_config_resource_t();
    header.version = 0;
    header.dependencies = {};
    eastl::vector<uint8_t> buffer;
    skr::resource::SBinarySerializer archive(buffer);
    bitsery::serialize(archive, header);
    //------write resource object
    skr::resource::SConfigFactory::Serialize(*resource, archive);
    //------save resource to disk
    ghc::filesystem::path output = "F:\\Sakura.Runtime\\samples\\game\\resource";
    ghc::filesystem::create_directories(output);
    output.append(record->path.filename().replace_extension("bin"));
    auto file = fopen(output.u8string().c_str(), "wb");
    SKR_DEFER({ fclose(file); });
    fwrite(buffer.data(), 1, buffer.size(), file);
}

int main(int argc, char** argv)
{
    auto& registry = *skd::asset::GetAssetRegistry();
    registry.ImportAsset("F:\\Sakura.Runtime\\samples\\game\\assets\\myConfig.json");
    compile_config(registry.ImportAsset("F:\\Sakura.Runtime\\samples\\game\\assets\\myConfig.config.meta"));
    return 0;
    using namespace grpc;
    if (argc != 1)
    {
        std::string server_address1("localhost:0");
        CompileResourceImpl service;
        ServerBuilder builder;
        int selectedPort = 0;
        std::stringstream ss;
        builder.AddListeningPort(server_address1, grpc::InsecureServerCredentials(), &selectedPort);
        builder.RegisterService(&service);
        std::unique_ptr<Server> server(builder.BuildAndStart());
        ss << selectedPort;
        std::cout << "Server listening on localhost:" << ss.str() << std::endl;
        skrcompiler::HostResource::Stub stub(CreateChannel("localhost:50052", InsecureChannelCredentials()));
        std::cout << "connected to localhost:50052" << std::endl;
        ClientContext context;
        grpc::CompletionQueue cq;
        skrcompiler::Port id;
        id.set_number(ss.str());
        google::protobuf::Empty empty;
        auto reg = stub.Register(&context, id, &empty);
        std::cout << "registed to localhost:50052" << std::endl;
        server->Wait();
    }
    else
    {
        std::string server_address("localhost:50052");
        HostResourceImpl service;
        ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        std::unique_ptr<Server> server(builder.BuildAndStart());
        std::cout << "Server listening on " << server_address << std::endl;
        server->Wait();
    }
}