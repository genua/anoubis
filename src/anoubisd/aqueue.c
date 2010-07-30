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

#include "anoubis_alloc.h"

#include <stdlib.h>
#ifdef UTEST
#include <assert.h>
#include <stdio.h>
#define log_warn(x)
#endif /* UTEST */

#include <errno.h>

#include "amsg.h"
#include "aqueue.h"

/*
 * Unit Test:
 *   cc -DUTEST  -o aqueue aqueue.c
 *   ./aqueue
 */

int
enqueue(Queue * queuep, void *msgp)
{
	Qentry *qep;

	if (queuep == NULL) {
		log_warnx("uninitialized queue");
		free(msgp);
		return 0;
	}
	if ((qep = abuf_alloc_type(Qentry)) == NULL) {
		log_warn("enqueue: can't allocate memory");
		free(msgp);
		master_terminate(ENOMEM);
		/*
		 * In case of OOM situation, we can live with potential
		 * unfreed storage. We therefore ignore the splint warning
		 * at this point
		 */
		/*@i@*/return 0;
	}
	qep->next = NULL;
	qep->entry = msgp;

	if (queuep->tail)
		queuep->tail->next = qep;
	/*@i@*/queuep->tail = qep;
	if (queuep->head == NULL)
		queuep->head = queuep->tail;
	if (queuep->event)
		event_add(queuep->event, NULL);
	return 1;
}

void *
dequeue(Queue *queuep)
{
	Qentry *qep;
	void *msgp;

	if (queuep == NULL) {
		log_warnx("uninitialized queue");
		return 0;
	}
	if (queuep->head == NULL)
		return NULL;
	qep = queuep->head;
	msgp = queuep->head->entry;
	queuep->head = queuep->head->next;
	abuf_free_type(qep, Qentry);
	if (queuep->head == NULL)
		queuep->tail = NULL;
	return msgp;
}

Qentry *
queue_head(Queue *queuep)
{
	if (queuep == NULL) {
		log_warnx("uninitialized queue");
		return 0;
	}
	return queuep->head;
}

Qentry *
queue_walk(Queue *queuep, Qentry *qep)
{
	if (queuep == NULL) {
		log_warnx("uninitialized queue");
		return 0;
	}
	if (qep == NULL)
		return NULL;
	return qep->next;
}

void *
queue_peek(Queue *queuep)
{
	if (queuep == NULL) {
		log_warnx("uninitialized queue");
		return 0;
	}
	if (queuep->head == NULL)
		return NULL;
	return queuep->head->entry;
}

void *
queue_find(Queue *queuep, void *msgp, int(*cmp)(void *, void *))
{
	Qentry *qep;

	if (queuep == NULL) {
		log_warnx("uninitialized queue");
		return 0;
	}
	if (queuep->head == NULL)
		return NULL;
	qep = queuep->head;
	while (qep) {
		if ((*cmp)(msgp, qep->entry))
			return qep->entry;
		qep = qep->next;
	}
	return NULL;
}

int
queue_delete(Queue *queuep, void *msgp)
{
	Qentry *qep;
	Qentry *lqep = NULL;

	if (queuep == NULL) {
		log_warnx("uninitialized queue");
		return 0;
	}
	if (queuep->head == NULL)
		return 0;
	qep = queuep->head;
	while (qep) {
		if (msgp == qep->entry) {
			if (qep == queuep->head) {
				/* common queue operation, ignore splint here */
				/*@i@*/queuep->head = qep->next;
				if (queuep->head == NULL)
					queuep->tail = NULL;
				abuf_free_type(qep, Qentry);
				return 1;
			}
			/*
			 * no null pointer dereference possible here,
			 * since the first loop run is always caught by
			 * the if-branch directly above this comment. On
			 * the 2nd loop-run lqep is always defined to
			 * the previous queue element (read: nonnull).
			 */
			/*@i@*/lqep->next = qep->next;
			if (queuep->tail == qep)
				queuep->tail = lqep;
			abuf_free_type(qep, Qentry);
			/*
			 * splint warns here that the parameter queuep
			 * might indirectly point to deallocated storage
			 * via queuep->head. However, this is no problem
			 * here. Queue operations like this ARE
			 * difficult to understand by splint.
			 */
			/*@i@*/return 1;
		}
		lqep = qep;
		qep = qep->next;
	}
	return 0;
}

int
dispatch_write_queue(Queue *q, int fd)
{
	anoubisd_msg_t		*msg;
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
		if (ret == 0 && q->event) {
			event_add(q->event, NULL);
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
			event_add(q->event, NULL);
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

#ifdef UTEST
void
queue_dump(Queue *q)
{
	Qentry *qep;

	printf("q->head %x\n", q->head);
	printf("q->tail %x\n", q->tail);
	qep = q->head;
	while (qep) {
		printf("qep->entry %x\n", qep->entry);
		printf("qep->next %x\n", qep->next);
		qep = qep->next;
	}
	printf("\n");
}

int
main(int ac, char *av[])
{
	Queue q;
	void *mp;

	printf("init\n");
	queue_init(&q, NULL);
	queue_dump(&q);

	mp = malloc(sizeof(int));
	printf("mp: %x\n", mp);
	*(int *)mp = 1;
	printf("enqueue: %d\n", enqueue(&q, mp));
	queue_dump(&q);

	mp = malloc(2 * sizeof(int));
	printf("mp: %x\n", mp);
	*(int *)mp = 2;
	((int *)mp)[1] = 3;
	printf("enqueue: %d\n", enqueue(&q, mp));
	queue_dump(&q);

	mp = dequeue(&q);
	printf("dequeue: %d\n", *(int *)mp);
	assert(*(int *)mp == 1);
	free(mp);
	queue_dump(&q);

	mp = dequeue(&q);
	printf("dequeue: %d %d\n",
	    *(int *)mp,
	    ((int *)mp)[1]);
	assert(*(int *)mp == 2);
	assert(((int *)mp)[1] == 3);
	free(mp);
	queue_dump(&q);

	mp = dequeue(&q);
	assert(mp == NULL);
	printf("dequeue: empty\n");
	queue_dump(&q);

	return 0;
}
#endif /* UTEST */
