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

#ifndef _SYNCHRONIZEDQUEUE_H_
#define _SYNCHRONIZEDQUEUE_H_

#include <queue>

#include <wx/thread.h>

/**
 * A queue.
 *
 * You can insert an element at the end of the queue
 * (SynchronizedQueue::push()) and remove an element from the beginning
 * (SynchronizedQueue::pop()).
 *
 * Access to the queue is synchronized with an mutex. Additionally a semaphore
 * can be enabled. In this case pop() waits until the queue is not empty.
 */
template <class T>
class SynchronizedQueue {
	public:
		/**
		 * C'tor.
		 *
		 * @param sem If set to true, the push() and post() operations
		 *            are additionally protected by a semaphore.
		 */
		SynchronizedQueue(bool sem) {
			semEnabled_ = sem;
		}

		/**
		 * Inserts an element at the end of the queue.
		 *
		 * @param t Element to be inserted
		 */
		void push(T *t)
		{
			mutex_.Lock();
			queue_.push(t);
			mutex_.Unlock();

			if (semEnabled_)
				sem_.Post();
		}

		/**
		 * Removes an element from the beginning of the queue.
		 *
		 * @param timeout If a semaphore is enable for the queue, you
		 *                can specify how long to wait for the
		 *                semaphore. A timeout of 0 means to block
		 *                indefinitely. If no semaphore is enabled, the
		 *                argument is ignored.
		 *
		 * @return Element which was removed. If the queue is empty
		 *         and/or the semaphore wait-timeout exceeded, 0 is
		 *         returned.
		 */
		T *pop(int timeout = 0)
		{
			if (semEnabled_) {
				wxSemaError err = sem_.WaitTimeout(timeout);

				if (err == wxSEMA_TIMEOUT)
					return (0);
			}

			mutex_.Lock();

			if (queue_.empty()) {
				mutex_.Unlock();
				return (0);
			}

			T *t = queue_.front();
			queue_.pop();

			mutex_.Unlock();

			return (t);
		}

	private:
		std::queue<T *> queue_;
		wxMutex mutex_;
		bool semEnabled_;
		wxSemaphore sem_;
};

#endif	/* _SYNCHRONIZEDQUEUE_H_ */
