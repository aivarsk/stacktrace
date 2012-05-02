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
        struct stacktrace *trace = stacktrace_get_exc();
        if (trace != NULL) {
            stacktrace_print(trace);
        }
    }
    return 0;
}
