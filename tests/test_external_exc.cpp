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
            printf("No trace Reached\n");
            stacktrace_print(trace);
        }
        printf("No trace\n");
    }
    return 0;
}
