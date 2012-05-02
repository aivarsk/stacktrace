#ifndef STACK_TRACE_H_INCLUDED_
#define STACK_TRACE_H_INCLUDED_

#include <stdio.h>

struct stack_trace;

struct stack_trace *stack_trace_get();
void stack_trace_resolve(struct stack_trace *st);
void stack_trace_free(struct stack_trace *st);

void stack_trace_print(struct stack_trace *st);
void stack_trace_fprint(struct stack_trace *st, FILE *);

#ifdef __cplusplus
//struct stack_trace *stack_trace_get();
#endif

#endif
