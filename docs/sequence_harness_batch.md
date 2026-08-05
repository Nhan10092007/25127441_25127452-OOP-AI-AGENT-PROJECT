sequenceDiagram
    autonumber
    participant Main as main()
    participant HR as HarnessRunner
    participant SL as SkillLoader
    participant TR as ToolRegistry
    participant AL as AgentLoop
    participant TJ as Trajectory
    participant EV as Evaluator

    Main ->> Main: curl_global_init()
    Main ->>+ HR: new HarnessRunner(configPath, skillsPath, tasksPath)
    HR ->> HR: readHarnessConfig() -> LLMClient, Environment
    HR ->> SL: new SkillLoader(skillsPath)
    HR ->> TR: registerTool (calculator, exec, read_file, write_file, memory_save, memory_search, web_search)
    HR ->> HR: readTasks() -> vector~Task~
    HR -->>- Main: ready

    Main ->>+ HR: runBatch()

    loop moi Task trong tasksList
        HR ->> SL: selectSkills(task.instruction)
        HR ->> HR: systemPrompt = toolsDescription + skills

        HR ->> TJ: new Trajectory(task.id, model)
        HR ->> AL: new AgentLoop(client, env, max_steps, threshold, hook -> trajectory.addStep)

        HR ->>+ AL: run({systemMsg, userMsg})
        AL -->>- HR: AgentResult

        HR ->>+ EV: evaluate(finalAnswer, eval_script)
        EV -->>- HR: bool isPass

        HR ->> HR: exportTaskReport()
        HR ->> TJ: exportToJson()
        HR ->> HR: archiveWorkspace()
    end

    HR ->> HR: exportBatchSummary()
    HR -->>- Main: done

    Main ->> Main: curl_global_cleanup()

