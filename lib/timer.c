#define LOG_PREFIX "timer"
#include "tetrapol/log.h"
#include "tetrapol/timer.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
    timer_callback_t func;
    void *ptr;
} callback_t;

struct _timer_t {
    int ncallbacks;
    callback_t *callbacks;
    timeval_t tv;
};

timer_t *timer_create(void)
{
    timer_t *timer = calloc(1, sizeof(timer_t));
    if (!timer) {
        return NULL;
    }

    return timer;
}

void timer_destroy(timer_t *timer)
{
    if (!timer) {
        return;
    }
    free(timer->callbacks);
    free(timer);
}

void timer_tick(timer_t *timer, int usec)
{
    timer->tv.tv_usec += usec;
    timer->tv.tv_sec += timer->tv.tv_usec / 1000000;
    timer->tv.tv_usec %= 1000000;

    for (int i = 0; i < timer->ncallbacks; ++i) {
        timer->callbacks[i].func(&timer->tv, timer->callbacks[i].ptr);
    }
}

bool timer_register(timer_t *timer, timer_callback_t timer_func, void *ptr)
{
    // check for double-registration
    for (int i = 0; i < timer->ncallbacks; ++i) {
        if (timer->callbacks[i].func == timer_func &&
                timer->callbacks[i].ptr == ptr) {
            LOG(WTF, "double registration of callback");
            return false;
        }
    }

    ++timer->ncallbacks;
    callback_t *p = realloc(timer->callbacks, sizeof(callback_t) * timer->ncallbacks);
    if (!p) {
        LOG(ERR, "ERR OOM");
        return false;
    }
    timer->callbacks = p;
    timer->callbacks[timer->ncallbacks - 1].func = timer_func;
    timer->callbacks[timer->ncallbacks - 1].ptr = ptr;

    return true;
}

void timer_cancel(timer_t *timer, timer_callback_t timer_func, void *ptr)
{
    for (int i = 0; i < timer->ncallbacks; ++i) {
        if (timer->callbacks[i].func == timer_func &&
                timer->callbacks[i].ptr == ptr) {
            memmove(&timer->callbacks[i], &timer->callbacks[i + 1],
                    sizeof(callback_t) * (timer->ncallbacks - i - 1));
            --timer->ncallbacks;
            return;
        }
    }
    LOG(WTF, "callback not found");
}

int timeval_abs_delta(const timeval_t *tv1, const timeval_t *tv2)
{
    int d = tv2->tv_usec - tv1->tv_usec;
    d += (tv2->tv_sec - tv1->tv_sec) * 1000000;

    return abs(d);
}
