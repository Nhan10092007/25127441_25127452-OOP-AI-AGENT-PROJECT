graph TB
    subgraph ENTRY["Entry Point"]
        run_eval["run_eval.cpp main()"]
    end

    subgraph CLIENT["Client Module"]
        LLMClient_i["LLMClient interface"]
        OllamaClient_c["OllamaClient"]
        LLMClient_i -.- OllamaClient_c
    end

    subgraph AGENT["Agent Module"]
        AgentLoop_c["AgentLoop"]
        LoopDetector_c["LoopDetector"]
        SkillLoader_c["SkillLoader"]
        AgentLoop_c --> LoopDetector_c
    end

    subgraph ENV["Environment Module"]
        Environment_i["Environment interface"]
        NativeEnv_c["NativeEnvironment"]
        SandboxEnv_c["SandboxEnvironment (Decorator)"]
        Environment_i -.- NativeEnv_c
        Environment_i -.- SandboxEnv_c
        SandboxEnv_c -->|wraps| NativeEnv_c
    end

    subgraph TOOLS["Tools Module"]
        Tool_i["Tool interface"]
        ToolRegistry_c["ToolRegistry (Factory)"]
        ConcreteTools["CalculatorTool | ExecTool | ReadFileTool<br/>WriteFileTool | MemorySave | MemorySearch | WebTool"]
        Tool_i -.- ConcreteTools
        ToolRegistry_c --> Tool_i
    end

    subgraph HARNESS["Harness Module"]
        HarnessRunner_c["HarnessRunner"]
        Evaluator_i["Evaluator interface"]
        Trajectory_c["Trajectory"]
        ConcreteEval["KeywordEvaluator | FunctionalEvaluator"]
        Evaluator_i -.- ConcreteEval
    end

    subgraph EXTERNAL["External Libraries"]
        libcurl["libcurl"]
        nlohmann["nlohmann/json"]
        sqlite3["SQLite3"]
    end

    run_eval --> HarnessRunner_c
    HarnessRunner_c --> OllamaClient_c
    HarnessRunner_c --> ToolRegistry_c
    HarnessRunner_c --> AgentLoop_c
    HarnessRunner_c --> Trajectory_c
    HarnessRunner_c --> Evaluator_i
    HarnessRunner_c --> SkillLoader_c
    HarnessRunner_c --> NativeEnv_c
    HarnessRunner_c --> SandboxEnv_c
    AgentLoop_c --> LLMClient_i
    AgentLoop_c --> Environment_i
    NativeEnv_c --> ToolRegistry_c

    OllamaClient_c --> libcurl
    ConcreteTools --> libcurl
    ConcreteTools --> sqlite3
    AGENT --> nlohmann
    HARNESS --> nlohmann
