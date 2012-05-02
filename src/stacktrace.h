#ifndef STACKTRACE_H_INCLUDED_
#define STACKTRACE_H_INCLUDED_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct stack_trace;

struct stack_trace *stack_trace_get(unsigned skip);
void stack_trace_resolve(struct stack_trace *st);
void stack_trace_free(struct stack_trace *st);

void stack_trace_print(struct stack_trace *st);
void stack_trace_fprint(struct stack_trace *st, FILE *);

#ifdef __cplusplus
struct stack_trace *stack_trace_get_exc();
#endif

#ifdef __cplusplus
}
#endif

#endif
