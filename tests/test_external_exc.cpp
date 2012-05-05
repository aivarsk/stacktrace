#include <stacktrace.h>
#include <string>
#include <cstdio>

void bar () {
    std::string x = "aaa";
    x.substr(100, 3);
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
        } else {
            printf("No exception backtrace\n");
        }
    }
    return 0;
}
