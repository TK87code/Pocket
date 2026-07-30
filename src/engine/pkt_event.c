#include "pocket.h"
#include "pkt_event_internal.h"

#define PKT_MAX_EVENTS 32

static struct pkt_event event_queue[PKT_MAX_EVENTS];
static int event_count = 0;
static int head = 0;
static int tail = 0;

// Push a new event into event queue. If event count is exceeded max event counts,
// return -1, otherwise 0 on success.
int __pkt_event_push(struct pkt_event *event)
{
	if (event_count >= PKT_MAX_EVENTS)
		return -1;
	event_queue[tail] = *event;
	tail = (tail + 1) % PKT_MAX_EVENTS;
	event_count++;

	return 0;
}

// Poll event from queue. First in first out.
// Return 0 on success, -1 if no event is in queue.
int __pkt_event_poll(struct pkt_event *event)
{
	if (event_count == 0)
		return -1;

	*event = event_queue[head];
	head = (head + 1) % PKT_MAX_EVENTS;
	event_count--;

	return 0;
}
