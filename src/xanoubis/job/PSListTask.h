/*
 * Copyright (c) 2010 GeNUA mbH <info@genua.de>
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

#ifndef _PSLISTTASK_H_
#define _PSLISTTASK_H_

#include <anoubis_msg.h>
#include "ListTask.h"

/**
 * This task fetches a list of all processes of the current user from the
 * anoubis daemon. It inherits from ListIteratorTask, i.e. you can
 * use the iterator from the base class to iterate over all records in
 * the reply.
 */
class PSListTask : public ListIteratorTask<Anoubis_ProcRecord>
{
	public:
		/**
		 * Constructor.
		 * @param None.
		 */
		PSListTask(void);

		/**
		 * Implementation of Task::getEventType().
		 *
		 * @param None.
		 * @return The type of the event that should be sent
		 *     once the task completes.
		 */
		wxEventType getEventType(void) const;

		/**
		 * Return a pointer to the current process record
		 * according to the iterator implemented in the base class.
		 *
		 * @param None.
		 * @return A pointer to the current record. The pointer is
		 *     invalidated if the task is destroyed or reset. The
		 *     caller must not modify the record.
		 */
		const Anoubis_ProcRecord *getProc(void) const;
};

#endif	/* _PSLISTTASK_H_ */