/*
 * Copyright (c) 2008 GeNUA mbH <info@genua.de>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _AQUEUE_H
#define _AQUEUE_H

#include <config.h>

#include <sys/time.h>
#include <sys/queue.h>
#include <event.h>

#include "amsg.h"

/* Forward declaration of private entry type. */
struct queue_entry;

/**
 * A fifo queue with opaque entries. The queue does not know anything
 * about the type of its elements.
 */
struct queue_hd {
	/**
	 * The tailq of struct queue_entries.
	 */
	TAILQ_HEAD(, queue_entry)	 list;

	/**
	 * If this value is not NULL it must be a libevent event that will
	 * take care of flushing of the queue. Each time an entry is added
	 * to the queue, the event is added to libevents list of interesting
	 * events.
	 */
	struct event			*ev;
};
typedef struct queue_hd			 Queue;

/**
 * Initialize a pre-allocated queue.
 *
 * @param queue The queue.
 * @param ev The libevent that will flush the queue. May be NULL.
 * @return None.
 */
static inline
void queue_init(Queue *queue, struct event *ev)
{
	TAILQ_INIT(&queue->list);
	queue->ev = ev;
}


/* Documentaion is in aqueue.c */
extern void			 enqueue(Queue *, void *);
extern void			*dequeue(Queue *);
extern void			*queue_peek(Queue *);
extern void			 queue_delete(Queue *, void *);
extern int			 dispatch_write_queue(Queue *q, int fd);

#endif /* !_AQUEUE_H */
