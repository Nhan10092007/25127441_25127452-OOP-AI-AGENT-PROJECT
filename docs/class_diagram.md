classDiagram
    direction TB

    class LLMClient {
        <<abstract>>
        +sendRequest(vector~Message~)* LLMResponse
    }

    class OllamaClient {
        -string _baseURL
        -string _modelName
        -float _temperature
        -int _numPredict
        +sendRequest(vector~Message~) LLMResponse
    }

    class Environment {
        <<abstract>>
        +step(ToolInput)* ToolResultxxxxx
    }

    class NativeEnvironment {
        -shared_ptr~ToolRegistry~ toolRegistry
        +step(ToolInput) ToolResult
    }

    class SandboxEnvironment {
        -unique_ptr~Environment~ wrapped
        -fs::path workspaceRoot
        +step(ToolInput) ToolResult
    }

    class Tool {
        <<abstract>>
        #string name
        #string description
        +getName() string
        +getDescription() string
        +execute(string)* string
    }

    class CalculatorTool {
        +execute(string) string
    }

    class ExecTool {
        +execute(string) string
    }

    class ReadFileTool {
        +execute(string) string
    }

    class WriteFileTool {
        +execute(string) string
    }

    class MemorySave {
        +execute(string) string
    }

    class MemorySearch {
        +execute(string) string
    }

    class WebTool {
        +execute(string) string
    }

    class ToolRegistry {
        -unordered_map factories
        +registerTool~T~(string) void
        +createTool(string) unique_ptr~Tool~
        +getToolsDescription() string
    }

    class Evaluator {
        <<abstract>>
        +evaluate(string, string)* bool
    }

    class KeywordEvaluator {
        +evaluate(string, string) bool
    }

    class FunctionalEvaluator {
        +evaluate(string, string) bool
    }

    class LoopDetector {
        -vector~CallRecord~ toolCallHistory
        -LoopThreshold loopThreshold
        +record(string, string) LoopStatus
        +reset() void
    }

    class SkillLoader {
        -unordered_map~string, string~ skillStorage
        -unordered_map~string, vector_string~ skillKeywords
        +getSkills(vector~string~) string
        +selectSkills(string) vector~string~
    }

    class AgentLoop {
        #LLMClient* llmClient
        #Environment* environment
        #LoopDetector loopDetector
        #vector~Message~ messages
        #int max_steps
        +run(vector~Message~) AgentResult
        #observe() void
        #think() LLMResponse
        #parseAction(string) optional~Action~
        #act(ToolInput) ToolResult
    }

    class Trajectory {
        -string task_id
        -string model
        -bool success
        -vector~StepRecord~ steps
        +addStep(StepRecord) void
        +setSuccess(bool) void
        +exportToJson(path) void
    }

    class HarnessRunner {
        -HarnessConfig config
        -shared_ptr~ToolRegistry~ toolRegistry
        -SkillLoader skillLoader
        -unique_ptr~LLMClient~ client
        -unique_ptr~Environment~ env
        -vector~Task~ tasksList
        +HarnessRunner(path, path, path)
        +runBatch() void
    }

    LLMClient <|-- OllamaClient
    Environment <|-- NativeEnvironment
    Environment <|-- SandboxEnvironment
    Tool <|-- CalculatorTool
    Tool <|-- ExecTool
    Tool <|-- ReadFileTool
    Tool <|-- WriteFileTool
    Tool <|-- MemorySave
    Tool <|-- MemorySearch
    Tool <|-- WebTool
    Evaluator <|-- KeywordEvaluator
    Evaluator <|-- FunctionalEvaluator

    AgentLoop *-- LoopDetector : composition
    HarnessRunner *-- SkillLoader : composition
    SandboxEnvironment o-- "1" Environment : wraps (Decorator)
    NativeEnvironment o-- ToolRegistry
    HarnessRunner o-- "1" LLMClient
    HarnessRunner o-- "1" Environment
    HarnessRunner o-- "1" ToolRegistry

    AgentLoop ..> LLMClient : depends
    AgentLoop ..> Environment : depends
    ToolRegistry ..> Tool : creates
    HarnessRunner ..> AgentLoop : creates
    HarnessRunner ..> Trajectory : creates
    HarnessRunner ..> Evaluator : uses

