#include"native_environment.h"


NativeEnvironment::NativeEnvironment(std::shared_ptr<ToolRegistry> registry):
    toolRegistry(registry)
{}