#pragma once
#include "SkrTool/module.configure.h"
#include "EASTL/vector.h"
#include "EASTL/shared_ptr.h"
#include "platform/memory.h"
#include "platform/guid.hpp"
#include "platform/thread.h"
#include "utils/hashmap.hpp"
#include "utils/parallel_for.hpp"
#include "resource/resource_header.h"
#include "task/task.hpp"
#include "platform/filesystem.hpp"
#include "simdjson.h"

struct skr_vfs_t;
namespace skr::io
{
class RAMService;
}
namespace skd::asset sreflect
{
struct SImporter;
struct SCookSystem;
struct SCooker;
struct SCookContext;

struct TOOL_API SProject {
    skr::filesystem::path assetPath;
    skr::filesystem::path outputPath;
    skr::filesystem::path dependencyPath;
    skr_vfs_t* vfs = nullptr;
    ~SProject() noexcept;
};

struct SAssetRecord {
    skr_guid_t guid;
    skr_guid_t type;
    skr::filesystem::path path;
    SProject* project;
    simdjson::padded_string meta;
};

struct TOOL_API SCooker {
    static constexpr uint32_t kDevelopmentVersion = UINT32_MAX;
    virtual ~SCooker() {}
    virtual uint32_t Version() = 0;
    virtual bool Cook(SCookContext* ctx) = 0;
    SCookSystem* system;
};
struct TOOL_API SCookContext { // context per job
    SAssetRecord* record = nullptr;
    class skr::io::RAMService* ioService = nullptr;
    skr_guid_t importerType;
    size_t importerVersion = 0;
    SImporter* importer = nullptr;
    size_t cookerVersion = 0;
    skr::task::event_t counter;
    skr::filesystem::path output;
    eastl::vector<skr::filesystem::path> fileDependencies;
    eastl::vector<skr_guid_t> staticDependencies;
    eastl::vector<skr_guid_t> runtimeDependencies;

    skr::filesystem::path AddFileDependency(const skr::filesystem::path& path);
    void AddRuntimeDependency(skr_guid_t resource);
    void* AddStaticDependency(skr_guid_t resource);
    void* _Import();
    void _Destroy(void*);
    template <class T>
    T* Import()
    {
        return (T*)_Import();
    }
    template <class T>
    void Destroy(T* ptr)
    {
        if (ptr) 
        {
            _Destroy(ptr);
        }
    }
    template <class S>
    void WriteHeader(S& s, SCooker* cooker)
    {
        skr_resource_header_t header;
        header.guid = record->guid;
        header.type = record->type;
        header.version = cooker->Version();
        header.dependencies.insert(header.dependencies.end(), runtimeDependencies.begin(), runtimeDependencies.end());
        skr::binary::Archive(&s, header);
    }
};

struct TOOL_API SCookSystem {
    SCookSystem() noexcept;
    ~SCookSystem() noexcept;
    void Initialize() {}
    void Shutdown() {}
    skr::task::event_t AddCookTask(skr_guid_t resource);
    void* CookOrLoad(skr_guid_t resource);
    skr::task::event_t EnsureCooked(skr_guid_t resource);
    void RegisterCooker(skr_guid_t type, SCooker* cooker);
    void UnregisterCooker(skr_guid_t type);
    void WaitForAll();
    skr::flat_hash_map<skr_guid_t, SCooker*, skr::guid::hash> cookers;
    using CookingMap = skr::parallel_flat_hash_map<skr_guid_t, SCookContext*, skr::guid::hash>;
    CookingMap cooking;
    SMutex ioMutex;
    class skr::io::RAMService* getIOService();
    static constexpr uint32_t ioServicesMaxCount = 4;
    class skr::io::RAMService* ioServices[ioServicesMaxCount];
    skr::task::counter_t mainCounter;

    SAssetRecord* GetAssetRecord(const skr_guid_t& guid);
    SAssetRecord* ImportAsset(SProject* project, skr::filesystem::path path);
    skr::flat_hash_map<skr_guid_t, SAssetRecord*, skr::guid::hash> assets;
    SMutex assetMutex;

    template <class F, class Iter>
    void ParallelFor(Iter begin, Iter end, size_t batch, F f)
    {
        skr::parallel_for(std::move(begin), std::move(end), batch, std::move(f));
    }
};
TOOL_API SCookSystem* GetCookSystem();
#define sregister_cooker(literal) sstatic_ctor(skd::asset::RegisterCooker<$T>(skr::guid::make_guid_unsafe(literal)))
template<class T>
void RegisterCooker(skr_guid_t guid)
{
    static T instance;
    GetCookSystem()->cookers.insert(std::make_pair(guid, &instance));
}
} // namespace skd::assetsreflect