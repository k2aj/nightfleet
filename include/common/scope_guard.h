#pragma once

#define _concat(a,b) _concat2(a,b)
#define _concat2(a,b) a##b

template<typename Callable>
class ScopeExit {

    private:
    Callable callable;

    public:
    ScopeExit(Callable callable) : callable(callable) {}
    ~ScopeExit() {
        callable();
    }
};

#define scope_exit(block) ScopeExit _concat(_scopeGuard, __LINE__) ([&]{block;})