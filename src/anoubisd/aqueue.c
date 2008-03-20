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

#include <stdlib.h>
#ifdef UTEST
#include <assert.h>
#include <stdio.h>
#define log_warn(x)
#endif /* UTEST */

#include "aqueue.h"

/*
 * Unit Test:
 *   cc -DUTEST  -o aqueue aqueue.c
 *   ./aqueue
 */

int
enqueue(Queuep queuep, void *msgp)
{
	Qentry *qep;

	if ((qep = malloc(sizeof(struct queue_entry))) == NULL) {
		log_warn("can't allocate memory");
		return(-1);
	}
	qep->next = NULL;
	qep->entry = msgp;

	if (queuep->tail)
		queuep->tail->next = qep;
	queuep->tail = qep;
	if (queuep->head == NULL)
		queuep->head = queuep->tail;
	return(0);
}

void *
queue_next(Queuep queuep)
{
	if (queuep->head == NULL)
		return NULL;
	return queuep->head->entry;
}

void *
dequeue(Queuep queuep)
{
	Qentry *qep;
	void *msgp;

	if (queuep->head == NULL)
		return NULL;
	qep = queuep->head;
	msgp = queuep->head->entry;
	queuep->head = queuep->head->next;
	free(qep);
	if (queuep->head == NULL)
		queuep->tail = NULL;
	return msgp;
}

void *
queue_find(Queuep queuep, void *msgp)
{
/* XXX void *msg; */
	return msgp; /* XXX msg */
}

int
queue_delete(Queuep queuep, void *msgp)
{
/* XXX */
	return(1);
}

#ifdef UTEST
void
queue_dump(Queuep q)
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
	queue_init(q);
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

	return(0);
}
#endif /* UTEST */