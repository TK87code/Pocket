#ifndef P_EVENT_INTERNAL_H
#define P_EVENT_INTERNAL_H

struct p_event;

int __p_event_push(struct p_event *event);
int __p_event_poll(struct p_event *event);

#endif
