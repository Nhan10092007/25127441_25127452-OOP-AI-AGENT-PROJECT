#pragma once
#include "agent_loop.h"

// Biến thể của AgentLoop dành cho GUI Agent:
// thay vì nhét chuỗi base64 vào phần text, nó đưa ảnh vào kênh 'images' của Message
// để VLM (gemma3/qwen-vl qua Ollama) thực sự "nhìn" được màn hình.
class VisionAgentLoop : public AgentLoop {
public:
    using AgentLoop::AgentLoop;  // inherit constructor
protected:
    void observe() override;
private:
    void dropPreviousImages();
};
