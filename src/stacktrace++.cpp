#include <stacktrace.h>

extern "C" void _stack_trace_set_exc();
extern "C" struct stack_trace *_stack_trace_get_exc();

extern "C" struct stack_trace *stack_trace_get_exc() {
    return _stack_trace_get_exc();
}

extern "C" void __cxa_throw(void *thrown_exception, std::type_info *tinfo, void (*dest)(void *))
        __attribute__(( noreturn ));

extern "C" void __wrap___cxa_throw(void *thrown_exception, std::type_info *tinfo, void (*dest)(void *)) {
    _stack_trace_set_exc();
    __cxa_throw(thrown_exception, tinfo, dest);
}
