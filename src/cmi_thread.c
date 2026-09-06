/*
 * cmi_thread.c - small cross-platform thread and mutex abstraction
 *
 * Copyright (c) FBarrca 2026.
 * Licensed under the Apache License, Version 2.0.
 */

#include <stdint.h>
#include <stdlib.h>

#include "cmi_thread.h"

#if CMI_OS == CMI_WINDOWS

#include <process.h>

struct cmi_thread_start {
    cmi_thread_func *func;
    void *arg;
};

static unsigned __stdcall cmi_thread_start_wrapper(void *raw)
{
    struct cmi_thread_start *start = raw;
    cmi_thread_func *func = start->func;
    void *arg = start->arg;
    free(start);
    (void)func(arg);
    return 0u;
}

void cmi_mutex_lock(cmi_mutex_t *mutex)
{
    AcquireSRWLockExclusive(mutex);
}

void cmi_mutex_unlock(cmi_mutex_t *mutex)
{
    ReleaseSRWLockExclusive(mutex);
}

int cmi_thread_create(cmi_thread_t *thread,
                      cmi_thread_func *func,
                      void *arg)
{
    struct cmi_thread_start *start = malloc(sizeof(*start));
    if (start == NULL) {
        return -1;
    }
    start->func = func;
    start->arg = arg;

    uintptr_t handle = _beginthreadex(
        NULL, 0u, cmi_thread_start_wrapper, start, 0u, NULL
    );
    if (handle == 0u) {
        free(start);
        return -1;
    }
    *thread = (HANDLE)handle;
    return 0;
}

int cmi_thread_join(cmi_thread_t thread)
{
    const DWORD wait_result = WaitForSingleObject(thread, INFINITE);
    const BOOL close_result = CloseHandle(thread);
    return (wait_result == WAIT_OBJECT_0 && close_result) ? 0 : -1;
}

CMI_NORETURN void cmi_thread_exit(void)
{
    _endthreadex(0u);
    abort();
}

static BOOL CALLBACK cmi_once_callback(PINIT_ONCE once, PVOID arg, PVOID *context)
{
    (void)once;
    (void)context;
    void (**func)(void) = arg;
    (*func)();
    return TRUE;
}

void cmi_call_once(cmi_once_t *once, void (*func)(void))
{
    if (!InitOnceExecuteOnce(once, cmi_once_callback, &func, NULL)) abort();
}

cmi_thread_id_t cmi_thread_self(void) { return GetCurrentThreadId(); }
bool cmi_thread_equal(cmi_thread_id_t a, cmi_thread_id_t b) { return a == b; }

#else

void cmi_call_once(cmi_once_t *once, void (*func)(void))
{
    if (pthread_once(once, func) != 0) abort();
}

cmi_thread_id_t cmi_thread_self(void) { return pthread_self(); }
bool cmi_thread_equal(cmi_thread_id_t a, cmi_thread_id_t b) { return pthread_equal(a, b); }

void cmi_mutex_lock(cmi_mutex_t *mutex)
{
    (void)pthread_mutex_lock(mutex);
}

void cmi_mutex_unlock(cmi_mutex_t *mutex)
{
    (void)pthread_mutex_unlock(mutex);
}

int cmi_thread_create(cmi_thread_t *thread,
                      cmi_thread_func *func,
                      void *arg)
{
    return pthread_create(thread, NULL, func, arg);
}

int cmi_thread_join(cmi_thread_t thread)
{
    return pthread_join(thread, NULL);
}

CMI_NORETURN void cmi_thread_exit(void)
{
    pthread_exit(NULL);
    abort();
}

#endif
