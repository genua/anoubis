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

#include "config.h"

#include <errno.h>

#include "amsg.h"
#include "aqueue.h"
#include "anoubisd.h"
#include "anoubis_alloc.h"

/**
 * An entry in a generic queue. The queue does not need to know
 * about the type of its entries and the entries need not know that
 * they are (or will be) part of a queue. Fields:
 * next: The link to the next entry in the queue.
 * data: An opaque pointer to the data stored in the queue.
 */
struct queue_entry {
	TAILQ_ENTRY(queue_entry)	 next;
	void				*data;
};

/**
 * Add en entry to the end of the queue.
 *
 * @param queue The queue.
 * @param data The data to add to the queue.
 * @return None.
 */
void
enqueue(Queue * queue, void *data)
{
	struct queue_entry	 *entry;

	entry = abuf_alloc_type(struct queue_entry);
	if (entry == NULL) {
		log_warn("enqueue: can't allocate memory");
		master_terminate();
	}
	entry->data = data;
	TAILQ_INSERT_TAIL(&queue->list, entry, next);
	if (queue->ev)
		event_add(queue->ev, NULL);
}

/**
 * Remove the first entry in the queue and return a pointer to the data.
 *
 * @param queue The queue.
 * @return A pointer to the data stored in the first queue entry or
 *     NULL if the queue is empty.
 */
void *
dequeue(Queue *queue)
{
	struct queue_entry	*entry;
	void			*ret;

	entry = TAILQ_FIRST(&queue->list);
	if (entry == NULL)
		return NULL;
	TAILQ_REMOVE(&queue->list, entry, next);
	ret = entry->data;
	abuf_free_type(entry, struct queue_entry);
	return ret;
}

/**
 * Return the data stored in the first queue entry without modifying
 * the queue.
 *
 * @param queue The queue.
 * @return The data stored in the first queue entry or NULL if the queue
 *     is empty.
 */
void *
queue_peek(Queue *queue)
{
	struct queue_entry	*entry = TAILQ_FIRST(&queue->list);

	if (entry == NULL)
		return NULL;
	return entry->data;
}

/**
 * Remove a particular data item from the queue. The data item
 * is first searched in the queue and then removed.
 *
 * @param queue The queue.
 * @param data The data item to remove.
 * @return None.
 */
void
queue_delete(Queue *queue, void *data)
{
	struct queue_entry		*entry;

	TAILQ_FOREACH(entry, &queue->list, next) {
		if (entry->data == data)
			break;
	}
	if (entry == NULL)
		return;
	TAILQ_REMOVE(&queue->list, entry, next);
	abuf_free_type(entry, struct queue_entry);
}

/**
 * Assume that the given queue contains anoubid_msg structures that
 * must be written to the given file descriptor in order. This function
 * tries to use send_msg to send messages until the queue write blocks.
 * Before returning this function adds the libevent event associated
 * with the queue if the queue is not empty or the file descriptors
 * buffer still has pending data.
 *
 * @param The queue with anoubisd_msg structures.
 * @param The file descriptor to write the messages to.
 * @return Negative if a permanent write error occured for a message.
 *     The message with the error is removed from the queue and dropped.
 *     Zero if some write is still pending. Positive if all messages
 *     in the queue were sent and all buffers were flushed.
 */
int
dispatch_write_queue(Queue *q, int fd)
{
	struct anoubisd_msg	*msg;
	int			 ret = 0;

	DEBUG(DBG_TRACE, ">dispatch_write_queue: %p", q);

	while (queue_peek(q) || msg_pending(fd)) {
		msg = queue_peek(q);
		/*
		 * Call send_msg even if msg == NULL (will flush pending
		 * message data).
		 */
		ret = send_msg(fd, msg);
		/*
		 * ret > 0:  send_msg will take over and free the message.
		 * ret == 0: Buffers flushed but message not sent.
		 * ret < 0:  Permanent error: Drop message (if any).
		 */
		if (ret == 0 && q->ev) {
			event_add(q->ev, NULL);
			break;
		}
		if (ret < 0) {
			if (msg) {
				/* Permanent error. Dequeue and free. */
				dequeue(q);
				DEBUG(DBG_QUEUE,
				    " Dropping unsent message: %p", msg);
				free(msg);
			}
			event_add(q->ev, NULL);
			break;
		}
		/*
		 * Message (if any) sent and probably already freed.
		 * Remove its ptr from the queue, too.
		 */
		if (msg)
			dequeue(q);
	}
	DEBUG(DBG_TRACE, "<dispatch_write_queue: %p", q);
	return ret;
}
