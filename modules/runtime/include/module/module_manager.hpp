#pragma once
#include "module.hpp"
#include "utils/dependency_graph.hpp"

RUNTIME_EXTERN_C RUNTIME_API 
skr::ModuleManager* skr_get_module_manager();

namespace skr
{
struct ModuleProperty : public DependencyGraphNode 
{
    bool bActive = false;
    eastl::string name;
};
using module_registerer = eastl::function<eastl::unique_ptr<IModule>(void)>;
class ModuleManager
{
    friend struct IModule;
public:
    ModuleManager() = default;
    virtual ~ModuleManager() = default;
    virtual IModule* get_module(const eastl::string& name) = 0;
    virtual const struct ModuleGraph* make_module_graph(const eastl::string& entry, bool shared = true) = 0;
    virtual bool patch_module_graph(const eastl::string& name, bool shared = true, int argc = 0, char** argv = nullptr) = 0;
    virtual int init_module_graph(int argc, char** argv) = 0;
    virtual bool destroy_module_graph(void) = 0;
    virtual void mount(const char8_t* path) = 0;
    virtual eastl::string_view get_root(void) = 0;
    virtual ModuleProperty& get_module_property(const eastl::string& name) = 0;

    virtual void registerStaticallyLinkedModule(const char* moduleName, module_registerer _register) = 0;

protected:
    virtual IModule* spawnStaticModule(const eastl::string& moduleName) = 0;
    virtual IModule* spawnDynamicModule(const eastl::string& moduleName) = 0;
};

template <typename ModuleClass>
struct SStaticallyLinkedModuleRegistrant {
    SStaticallyLinkedModuleRegistrant(const char* InModuleName)
    {
        eastl::function<eastl::unique_ptr<IModule>(void)> func =
        []() {
            return eastl::make_unique<ModuleClass>();
        };
        skr_get_module_manager()->registerStaticallyLinkedModule(InModuleName, func);
    }
};
} // namespace skr
