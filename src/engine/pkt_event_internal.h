#ifndef PKT_EVENT_INTERNAL_H
#define PKT_EVENT_INTERNAL_H

struct pkt_event;

int __pkt_event_push(struct pkt_event *event);
int __pkt_event_poll(struct pkt_event *event);

#endif
