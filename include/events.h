//
// Created by hecke on 3/5/16.
//

#ifndef HPAVMOND_EVENTS_H
#define HPAVMOND_EVENTS_H

#include <sys/time.h>
#include <event.h>
#include <pthread.h>
#include <sys/queue.h>
#include <inttypes.h>

typedef uint64_t events_event_id;

struct events_event
{
    struct event *ev;
    void (*cb)(void *usr);
    void (*data_cb)(int fd, void *usr);

    void *usr_data;

    events_event_id _id;

    LIST_ENTRY(events_event) neighbors;
    pthread_mutex_t *pevents_events_mtx;
};

struct events_ctx
{
    struct event_base *base;
    pthread_t event_thread;

    LIST_HEAD(_events_event_list, events_event) events_events;
    pthread_mutex_t events_events_mtx;
    events_event_id _id_cnt;

    struct event *no_exit_timer_event;

};
#define EVENTS_CTX_INIT(CTX_PTR) (CTX_PTR)->_id_cnt = 0; LIST_INIT(&((CTX_PTR)->events_events));  (CTX_PTR)->events_events_mtx = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER; (CTX_PTR)->no_exit_timer_event = NULL;


struct events_ctx* events_setup();
void events_teardown(struct events_ctx *ctx);

int events_schedule_single_shot(struct events_ctx *ctx, void (*cb)(void *usr), void *usr_data, struct timeval tv, events_event_id *id);
int events_schedule_periodic(struct events_ctx *ctx, void (*cb)(void *usr), void *usr_data, struct timeval tv, events_event_id *id);
int events_add_reader(struct events_ctx *ctx, int fd, void (*cb)(int fd, void *usr), void *usr_data, events_event_id *id);

int events_cancel(struct events_ctx *ctx, events_event_id id);

#endif //HPAVMOND_EVENTS_H
