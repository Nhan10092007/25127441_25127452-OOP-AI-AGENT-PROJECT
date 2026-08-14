#pragma once
#include "agent_loop.h"

class VisionAgentLoop : public AgentLoop {
public:
    using AgentLoop::AgentLoop;  // inherit constructor
protected:
    void observe() override;
};