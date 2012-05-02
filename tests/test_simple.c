#include <stacktrace.h>

void bar () {
    struct stacktrace *trace = stacktrace_get(0);
    stacktrace_print(trace);
    stacktrace_free(trace);
}

void foo() {
    bar();
}

int main() {
    foo();
    return 0;
}
