#include <stacktrace.h>

void bar () {
    struct stack_trace *trace = stack_trace_get(0);
    stack_trace_print(trace);
    stack_trace_free(trace);
}

void foo() {
    bar();
}

int main() {
    foo();
    return 0;
}
