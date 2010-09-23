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

#ifndef _PSLISTCTRL_H_
#define _PSLISTCTRL_H_

#include "AnGenericRowProvider.h"
#include "GenericCtrl.h"
#include "Singleton.h"
#include "Task.h"
#include "TaskEvent.h"
#include "PSEntry.h"

/**
 * Controller class for the process browser.
 */
class PSListCtrl : public GenericCtrl, public Singleton<PSListCtrl>
{
	public:
		/**
		 * Destructor.
		 * The destructor will deregister its event handlers and
		 * free other external resources that it might potentially
		 * hold.
		 *
		 * @param None.
		 */
		~PSListCtrl(void);

		/**
		 * Update the list of processes in the controller. This
		 * function will clear the process list and start a new
		 * PSListTask. Once the list task completes, the process
		 * list in the provider will be updated.
		 *
		 * @param None.
		 * @return None.
		 */
		void updatePSList(void);

		/**
		 * Return a pointer to the process list provider.
		 *
		 * @param None.
		 * @return The process list provider. The provider
		 *     is allocate once and never changes. However, the
		 *     contents may well change.
		 */
		AnRowProvider *getPSListProvider(void);

		/**
		 * Return a description of the process at the given index
		 * in the process list.
		 *
		 * @param idx The index.
		 * @return A pointer to the PSEntry structure.
		 */
		const PSEntry *getEntry(int idx) const;

		/**
		 * Clear the current list of processes. This is useful
		 * in case of a disconnect from the daemon.
		 *
		 * @param None.
		 * @return None.
		 */
		void clearPSList(void);

	protected:
		/**
		 * Constructor.
		 * You can't call it from anywhere. Use instance().
		 */
		PSListCtrl(void);

	private:
		/**
		 * List of process entries.
		 */
		AnGenericRowProvider	psinfo_;

		/**
		 * Event handler for a completed PSListTask.
		 * @param[in] 1st The event containing the completed
		 *     PSListTask.
		 * @return Nothing.
		 */
		void OnPSListArrived(TaskEvent &);

	friend class Singleton<PSListCtrl>;
};

#endif	/* _PSLISTCTRL_H_ */
