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

#include "anoubisd.h"


struct queue_entry {
   struct queue_entry *next;
   void *entry;
};
typedef struct queue_entry  Qentry;
typedef struct queue_entry *Qentryp;

/*
 * This is an extremely simple fifo queue.
 * If head is NULL, then the queue is empty.
 */
struct queue_hd {
   Qentryp head;
   Qentryp tail;
};
typedef struct queue_hd  Queue;
typedef struct queue_hd *Queuep;

#define queue_init(queue) { queue.head = queue.tail = NULL; }

int	 enqueue(Queuep, void *);
void	*dequeue(Queuep);
Qentryp	 queue_head(Queuep);
Qentryp	 queue_walk(Queuep, Qentryp);
void	*queue_peek(Queuep);
void	*queue_find(Queuep, void *, int(*cmp)(const void *, const void *));
int	 queue_delete(Queuep, void *);

#endif /* !_AQUEUE_H */
