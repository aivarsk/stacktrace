#ifndef STACKTRACE_H_INCLUDED_
#define STACKTRACE_H_INCLUDED_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct stacktrace;

struct stacktrace *stacktrace_get(unsigned skip);
void stacktrace_resolve(struct stacktrace *st);
void stacktrace_free(struct stacktrace *st);

void stacktrace_print(struct stacktrace *st);
void stacktrace_fprint(struct stacktrace *st, FILE *);

#ifdef __cplusplus
struct stacktrace *stacktrace_get_exc();
#endif

#ifdef __cplusplus
}
#endif

#endif
