#include <stacktrace.h>

extern "C" void _stacktrace_set_exc();
extern "C" struct stacktrace *_stacktrace_get_exc();

extern "C" struct stacktrace *stacktrace_get_exc() {
    return _stacktrace_get_exc();
}

extern "C" void __cxa_throw(void *thrown_exception, std::type_info *tinfo, void (*dest)(void *))
        __attribute__(( noreturn ));

extern "C" void __wrap___cxa_throw(void *thrown_exception, std::type_info *tinfo, void (*dest)(void *)) {
    _stacktrace_set_exc();
    __cxa_throw(thrown_exception, tinfo, dest);
}
