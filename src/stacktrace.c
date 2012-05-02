#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unwind.h>

#include <stacktrace.h>
#include <pthread.h>

struct stacktrace_frame {
    void *addr;
    char *file;
    char *func;
    int line;
};

struct file_map {
    unsigned long long start;
    unsigned long long end;
    unsigned long long  offset;
    char *file;
};

struct stacktrace {
    char *exe;  /* Name of executable file */
    char *maps; /* Process memory map for this snapshot */

    size_t frames_size;
    size_t frames_len;
    struct stacktrace_frame *frames;

    size_t files_len;
    struct file_map *files;

    unsigned skip;
};

static char *read_whole_file(char *fname) {

    /* procfs files don't have size, so read chunks until EOF is reached */
    char *data = NULL;
    FILE *f;

    f = fopen(fname, "r");
    if (f != NULL) {
        int n, len = 0;
        int size = 1024;

        data = malloc(size);
        for (;;) {
            int max = size - len;

            n = fread(data + len, 1, max, f);
            if (n > 0) {
                len += n;
            }

            if (n != max) {
                break;
            }
            size *= 2;
            data = realloc(data, size);
        }
        data[len] = '\0';
        fclose(f);
    }
    return data;
}

static _Unwind_Reason_Code collect(struct _Unwind_Context *ctx, void *p) {
    struct stacktrace *trace = p;
    struct stacktrace_frame frame;

    if (trace->skip > 0) {
        trace->skip--;
        return _URC_NO_REASON;
    }

    frame.addr = (void *)_Unwind_GetIP(ctx);
    frame.file = NULL;
    frame.func = NULL;
    frame.line = 0;

    if (trace->frames_len == trace->frames_size) {
        trace->frames_size = trace->frames_size * 2;
        trace->frames = realloc(trace->frames, sizeof(struct stacktrace_frame) * trace->frames_size);
    }
    trace->frames[trace->frames_len++] = frame;
    return _URC_NO_REASON;
}

struct stacktrace *stacktrace_get(unsigned skip) {
    char procf[512];
    int len, n;

    struct stacktrace *trace = malloc(sizeof(struct stacktrace));
    trace->skip = skip + 1;
    trace->maps = NULL;
    trace->exe = NULL;

    trace->frames_size = 128;
    trace->frames_len = 0;
    trace->frames = malloc(sizeof(struct stacktrace_frame) * trace->frames_size);

    trace->files_len = 0;
    trace->files = NULL;

    snprintf(procf, sizeof(procf), "/proc/%d/exe", (int)getpid());

    for (len = 512; ; len *= 2) {
        trace->exe = realloc(trace->exe, len);

        n = readlink(procf, trace->exe, len);
        if (n == -1) {
            break;
        }
        if (n < len) {
            break;
        }
    }

    snprintf(procf, sizeof(procf), "/proc/%d/maps", (int)getpid());
    trace->maps = read_whole_file(procf);

    _Unwind_Backtrace(collect, trace);

    return trace;
}

void stacktrace_free(struct stacktrace *trace) {
    if (trace != NULL) {
        free(trace->exe);
        free(trace->maps);
        if (trace->frames != NULL) {
            int i;
            for (i = 0; i < trace->frames_len; i++) {
                free(trace->frames[i].func);
                free(trace->frames[i].file);
            }
        }
        free(trace->frames);
        free(trace->files);
        free(trace);
    }
}

void stacktrace_print(struct stacktrace *trace) {
    stacktrace_fprint(trace, stdout);
}

static void read_map(struct stacktrace *trace) {
    char *saveptr;
    char *line;

    if (trace->files_len > 0) {
        return;
    }

    size_t files_size = 1;
    trace->files = malloc(sizeof(struct file_map) * files_size);

    line = strtok_r(trace->maps, "\n", &saveptr);
    while (line != NULL) {
        char *p;
        char *saveptr2;
        unsigned long long start, end, offset;
        char *name;

        if (trace->files_len >= files_size) {
            files_size *= 2;
            trace->files = realloc(trace->files, sizeof(struct file_map) * files_size);
        }

        /* sscanf requires different format strings for 32/64 bits :( */
        p = strtok_r(line, "-", &saveptr2);
        if (p != NULL) {
            start = strtoull(p, NULL, 16);
            p = strtok_r(NULL, " ", &saveptr2);
            if (p != NULL) {
                end = strtoull(p, NULL, 16);
                p = strtok_r(NULL, " ", &saveptr2);
                if (p != NULL) {
                    p = strtok_r(NULL, " ", &saveptr2);
                    if (p != NULL) {
                        offset = strtoull(p, NULL, 16);
                        p = strtok_r(NULL, " ", &saveptr2);
                        if (p != NULL) {
                            p = strtok_r(NULL, " ", &saveptr2);
                            if (p != NULL) {
                                p = strtok_r(NULL, " ", &saveptr2);
                                if (p != NULL) {
                                    name = p;
                                } else {
                                    name = trace->exe;
                                }

                                trace->files[trace->files_len].start = start;
                                trace->files[trace->files_len].end = end;
                                trace->files[trace->files_len].offset = offset;
                                trace->files[trace->files_len].file = name;
                                trace->files_len++;
                            }
                        }
                    }
                }
            }
        }

        line = strtok_r(NULL, "\n", &saveptr);
    }
}

static struct file_map *_find_file(struct stacktrace *trace, unsigned long long addr) {
    int i;

    for (i = 0; i < trace->files_len; i++) {
        if (trace->files[i].start <= addr && addr <= trace->files[i].end) {
            return &trace->files[i];
        }
    }
    return NULL;
}

static void _addr2line(struct stacktrace_frame *frame, struct file_map *file) {
    FILE *f;
    char cmd[1024];
    char line[1024];
    snprintf(cmd, sizeof(cmd), "addr2line -C -f -e %s 0x%llx",
            file->file, (unsigned long long)(file->offset + frame->addr));
    /*printf("CMD: %s\n", cmd);*/
    f = popen(cmd, "r");
    if (f == NULL) {
        return;
    }
    if (fgets(line, sizeof(line), f) != NULL) {
        char *p = strchr(line, '\n');
        if (p != NULL) {
            *p = '\0';
        }
        frame->func = strdup(line);
        if (fgets(line, sizeof(line), f) != NULL) {
            p = strchr(line, ':');

            if (p != NULL) {
                *p++ = '\0';
                frame->line = atoi(p);
            }
            if (strcmp(line, "??") == 0) {
                frame->file = strdup(file->file);
            } else {
                frame->file = strdup(line);
            }
        }
    }
    pclose(f);
}

void stacktrace_resolve(struct stacktrace *trace) {
    int i;

    read_map(trace);

    for (i = 0; i < trace->frames_len; i++) {
        struct file_map *file;
        file = _find_file(trace, (unsigned long long)trace->frames[i].addr);
        if (file == NULL) {
            continue;
        }
        _addr2line(&trace->frames[i], file);
    }
}

void stacktrace_fprint(struct stacktrace *trace, FILE *f) {
    int i;

    stacktrace_resolve(trace);

    for (i = 0; i < trace->frames_len; i++) {
        struct stacktrace_frame *frame = &trace->frames[i];
        fprintf(f, "#%d %p - %s in %s:%d\n", i, frame->addr, 
                frame->func ? frame->func : "??", frame->file ? frame->file : "??", frame->line);
    }
}


static pthread_once_t _stacktrace_once = PTHREAD_ONCE_INIT;
static pthread_key_t _stacktrace_key;

struct stacktrace_tls {
    struct stacktrace *trace;
};

static void _stacktrace_tls_free(void *p) {
    struct stacktrace_tls *tls = p;
    free(tls->trace);
    free(tls);
}

static void _stacktrace_key_create() {
    pthread_key_create(&_stacktrace_key, _stacktrace_tls_free);
}

static struct stacktrace_tls *_stacktrace_get_tls() {
    pthread_once(&_stacktrace_once, _stacktrace_key_create);
    struct stacktrace_tls *tls = pthread_getspecific(_stacktrace_key);
    if (tls == NULL) {
        tls = malloc(sizeof(struct stacktrace_tls));
        tls->trace = NULL;
        pthread_setspecific(_stacktrace_key, tls);
    }
    return tls;
}

void _stacktrace_set_exc() {
    struct stacktrace_tls *tls = _stacktrace_get_tls();
    if (tls->trace != NULL) {
        stacktrace_free(tls->trace);
    }
    tls->trace = stacktrace_get(2);
}

struct stacktrace *_stacktrace_get_exc() {
    struct stacktrace_tls *tls = _stacktrace_get_tls();
    return _stacktrace_get_tls()->trace;
}
