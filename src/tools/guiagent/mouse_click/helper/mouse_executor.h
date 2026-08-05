#pragma once
class IMouseExecutor {
public:
    virtual ~IMouseExecutor() = default;
    virtual bool click(int x, int y) = 0;
};

class MacMouseExecutor : public IMouseExecutor {
public:
    bool click(int x, int y) override;
};

class WindowsMouseExecutor : public IMouseExecutor {
public:
    bool click(int x, int y) override;
};

class LinuxMouseExecutor : public IMouseExecutor {
public:
    bool click(int x, int y) override;
};