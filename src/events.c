//
// Created by hecke on 3/5/16.
//

#include <malloc.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <event2/thread.h>

#include "events.h"

static void _destroy_events_event(struct events_event *evs_ev);
static void* _event_thread(void *arg);
static void _event_single_shot_proxy(evutil_socket_t sock, short event_type, void *usr);
static void _event_continuous_shot_proxy(evutil_socket_t sock, short event_type, void *usr);
static int _events_schedule(struct events_ctx *ctx, void (*cb)(void *usr), void *usr_data, struct timeval tv, events_event_id *id, bool persistent);

static void _nop(evutil_socket_t fd, short what, void *arg)
{
    /* NOP */
}

struct events_ctx* events_setup()
{
    struct events_ctx *ctx;
    struct timeval tv = {3600, 0};

    ctx = calloc(1, sizeof(*ctx));
    if (ctx == NULL)
    {
        return NULL;
    }
    EVENTS_CTX_INIT(ctx);

    evthread_use_pthreads();

    ctx->base = event_base_new();
    if (ctx->base == NULL)
    {
        goto err_has_ctx;
    }

    ctx->no_exit_timer_event = event_new(ctx->base, -1, EV_TIMEOUT | EV_PERSIST, _nop, NULL);
    if (ctx->no_exit_timer_event == NULL)
    {
        goto err_has_base;
    }
    event_add(ctx->no_exit_timer_event, &tv);

    if(pthread_create(&ctx->event_thread, NULL, _event_thread, ctx))
    {
        goto err_has_timeout_event;
    }

    return ctx;

err_has_timeout_event:
    event_del(ctx->no_exit_timer_event);

err_has_base:
    event_base_free(ctx->base);

err_has_ctx:
    free(ctx);

    return NULL;
}

void events_teardown(struct events_ctx *ctx)
{
    event_base_loopbreak(ctx->base);
    pthread_cancel(ctx->event_thread);
    pthread_join(ctx->event_thread, NULL);

    while(ctx->events_events.lh_first != NULL)
    {
        _destroy_events_event(ctx->events_events.lh_first);
    }

    event_base_free(ctx->base);

    pthread_mutex_destroy(&ctx->events_events_mtx);
    free(ctx);
}

static void _event_single_shot_proxy(evutil_socket_t sock, short event_type, void *usr)
{
    struct events_event *evs_ev = usr;

    evs_ev->cb(evs_ev->usr_data);
    _destroy_events_event(evs_ev);
}

static void _event_continuous_shot_proxy(evutil_socket_t sock, short event_type, void *usr)
{
    struct events_event *evs_ev = usr;

    evs_ev->cb(evs_ev->usr_data);
}

static void _event_reader_proxy(evutil_socket_t sock, short event_type, void *usr)
{
    struct events_event *evs_ev = usr;

    evs_ev->data_cb(sock, evs_ev->usr_data);
}

static void _destroy_events_event(struct events_event *evs_ev)
{
    // TODO: remove event del if we switch to libevent >=2.1.x & use the finalize callback
    event_del(evs_ev->ev);
    event_free(evs_ev->ev);
    pthread_mutex_lock(evs_ev->pevents_events_mtx);
    LIST_REMOVE(evs_ev, neighbors);
    pthread_mutex_unlock(evs_ev->pevents_events_mtx);

    free(evs_ev);
}

static int _events_schedule(struct events_ctx *ctx, void (*cb)(void *usr), void *usr_data, struct timeval tv, events_event_id *id, bool persistent)
{
    int rc = 0;
    struct events_event *evs_ev;
    int events;
    event_callback_fn proxy;

    if (persistent == true)
    {
        proxy = _event_continuous_shot_proxy;
        events = EV_PERSIST;
    } else {
        proxy = _event_single_shot_proxy;
        events = EV_TIMEOUT;
    }

    if (!(evs_ev = calloc(1, sizeof(*evs_ev)))) {
        rc = ENOMEM;
        goto err;
    }

    evs_ev->cb = cb;
    evs_ev->usr_data = usr_data;
    evs_ev->pevents_events_mtx = &ctx->events_events_mtx;
    evs_ev->_id = (ctx->_id_cnt++);
    if (id)
    {
        *id = evs_ev->_id;
    }

    if (!(evs_ev->ev = event_new(ctx->base, 0, events, proxy, evs_ev)))
    {
        rc = EFAULT;
        goto err_has_evs_ev;
    }

    pthread_mutex_lock(&ctx->events_events_mtx);
    LIST_INSERT_HEAD(&ctx->events_events, evs_ev, neighbors);

    if(event_add(evs_ev->ev, &tv))
    {
        pthread_mutex_unlock(&ctx->events_events_mtx);
        rc = EINVAL;
        goto err_has_ev;
    }
    pthread_mutex_unlock(&ctx->events_events_mtx);

    return 0;

err_has_ev:
err_has_evs_ev:
    _destroy_events_event(evs_ev);
err:
    return rc;
}

int events_schedule_single_shot(struct events_ctx *ctx, void (*cb)(void *usr), void *usr_data, struct timeval tv, events_event_id *id)
{
    return _events_schedule(ctx, cb, usr_data, tv, false, id);
}

int events_schedule_periodic(struct events_ctx *ctx, void (*cb)(void *usr), void *usr_data, struct timeval tv, events_event_id *id)
{
    return _events_schedule(ctx, cb, usr_data, tv, id, true);
}

int events_add_reader(struct events_ctx *ctx, int fd, void (*cb)(int fd, void *usr), void *usr_data, events_event_id *id)
{
    int rc = 0;
    struct events_event *evs_ev;

    if (!(evs_ev = calloc(1, sizeof(*evs_ev)))) {
        rc = ENOMEM;
        goto err;
    }

    evs_ev->data_cb = cb;
    evs_ev->usr_data = usr_data;
    evs_ev->pevents_events_mtx = &ctx->events_events_mtx;
    evs_ev->_id = (ctx->_id_cnt++);
    if (id)
    {
        *id = evs_ev->_id;
    }

    if (!(evs_ev->ev = event_new(ctx->base, fd, EV_READ | EV_PERSIST , _event_reader_proxy, evs_ev)))
    {
        rc = EFAULT;
        goto err_has_evs_ev;
    }

    pthread_mutex_lock(&ctx->events_events_mtx);
    LIST_INSERT_HEAD(&ctx->events_events, evs_ev, neighbors);

    if(event_add(evs_ev->ev, 0))
    {
        pthread_mutex_unlock(&ctx->events_events_mtx);
        rc = EINVAL;
        goto err_has_ev;
    }
    pthread_mutex_unlock(&ctx->events_events_mtx);

    return 0;

    err_has_ev:
    err_has_evs_ev:
    _destroy_events_event(evs_ev);
    err:
    return rc;
}

static void _events_finalize(struct event *ev, void *usr)
{
    struct events_event *evs_ev = usr;

    _destroy_events_event(evs_ev);
}

int events_cancel(struct events_ctx *ctx, events_event_id id)
{
    struct events_event *evs_ev;
    pthread_mutex_lock(&ctx->events_events_mtx);

    for (evs_ev = ctx->events_events.lh_first; evs_ev != NULL; evs_ev = evs_ev->neighbors.le_next)
    {
        if (evs_ev->_id == id)
        {
            pthread_mutex_unlock(&ctx->events_events_mtx);
            /*
             * TODO: use eventz_finalize after switching to libevent >=2.1.x
             * event_finalize(0, evs_ev->ev, _events_finalize);
             */
            _destroy_events_event(evs_ev);

            return EXIT_SUCCESS;
        }
    }

    pthread_mutex_unlock(&ctx->events_events_mtx);
    return EXIT_FAILURE;
}

static void* _event_thread(void *arg)
{
    struct events_ctx *ctx = arg;
    /*
     *  this only works with >= 2.1.x - for now use the 'add a long running timer' approach
     *  event_base_loop(ctx->base, EVLOOP_NO_EXIT_ON_EMPTY);
     */

    event_base_loop(ctx->base, 0);

    return NULL;
}