
sequenceDiagram
    autonumber
    participant Caller as Caller
    participant AL as AgentLoop
    participant LD as LoopDetector
    participant LLM as LLMClient
    participant ENV as Environment
    participant TR as ToolRegistry
    participant TP as ToolPolicy
    participant T as Tool

    Caller ->>+ AL: run(initialMessages)
    AL ->> LD: reset()

    loop while current_step < max_steps
        AL ->> AL: observe()

        AL ->>+ LLM: sendRequest(messages)
        LLM -->>- AL: LLMResponse

        AL ->> AL: parseAction(response)

        alt FinishAction
            AL -->> Caller: AgentResult(success)
        else ErrorAction
            AL -->> Caller: AgentResult(fail)
        else ToolCallAction
            AL ->>+ LD: record(toolName, args)
            LD -->>- AL: LoopStatus

            alt CRITICAL
                AL -->> Caller: AgentResult(fail, "Loop detected")
            else OK / WARNING
                AL ->>+ ENV: step(ToolInput)
                ENV ->>+ TR: createTool(toolName)
                TR ->>+ TP: isAllowed(toolName)
                TP -->>- TR: bool
                TR -->>- ENV: unique_ptr~Tool~
                ENV ->>+ T: execute(args)
                T -->>- ENV: result
                ENV -->>- AL: ToolResult
            end
        end
    end

    AL -->>- Caller: AgentResult(fail, "Max steps reached")
```
