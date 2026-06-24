#include"sandbox_environment.h"

SandboxEnvironment::SandboxEnvironment(std::unique_ptr<Environment> inner, const EnvironmentConfig& config):
    wrapped(std::move(inner)),
    workspaceRoot(fs::weakly_canonical(config.workspace))
{}