#include <stacktrace.h>
#include <stdexcept>
#include <cstdio>

void bar () {
    throw std::exception();
}

void foo() {
    bar();
}

int main() {
    try {
        foo();
    } catch (...) {
        struct stack_trace *trace = stack_trace_get_exc();
        if (trace != NULL) {
            stack_trace_print(trace);
        }
    }
    return 0;
}
