#include "daScript/daScript.h"

using namespace das;

#define TUTORIAL_NAME   "/scripts/daSCo/co.das"

void tutorial () {
    TextPrinter tout;                               // output stream for all compiler messages (stdout. for stringstream use TextWriter)
    ModuleGroup dummyLibGroup;                      // module group for compiled program
    CodeOfPolicies policies;                        // compiler setup
    auto fAccess = make_smart<FsFileAccess>();      // default file access
#ifdef AOT
    policies.aot = true;
#endif
    // compile program
    auto program = compileDaScript(getDasRoot() + TUTORIAL_NAME, fAccess, tout, dummyLibGroup, false, policies);
    if ( program->failed() ) {
        // if compilation failed, report errors
        tout << "failed to compile\n";
        for ( auto & err : program->errors ) {
            tout << reportError(err.at, err.what, err.extra, err.fixme, err.cerr );
        }
        return;
    }
    // create daScript context
    Context ctx(program->getContextStackSize());
    if ( !program->simulate(ctx, tout) ) {
        // if interpretation failed, report errors
        tout << "failed to simulate\n";
        for ( auto & err : program->errors ) {
            tout << reportError(err.at, err.what, err.extra, err.fixme, err.cerr );
        }
        return;
    }
    // find function 'test' in the context
    auto fnTest = ctx.findFunction("test");
    if ( !fnTest ) {
        tout << "function 'test' not found\n";
        return;
    }
    // evaluate 'test' function in the context
    Sequence it;
    vec4f res = ctx.evalWithCatch(fnTest, nullptr, &it);
    if ( auto ex = ctx.getException() ) {       // if function cased panic, report it
        tout << "exception: " << ex << "\n";
        return;
    }
    // call coroutine until its done
    bool dummy = false;
    while ( builtin_iterator_iterate(it, &dummy, &ctx) ) {
        printf("...\n");
    }
}

int main( int argc, char **argv ) {
    // request all da-script built in modules
    NEED_ALL_DEFAULT_MODULES;
    das::setCommandLineArguments(argc, argv);
    // Initialize modules
    Module::Initialize();
    // run the tutorial
    tutorial();
    // shut-down daScript, free all memory
    Module::Shutdown();
    return 0;
}
