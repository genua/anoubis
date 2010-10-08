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

#ifndef _MODPLAYGROUND_H_
#define _MODPLAYGROUND_H_

#include "Module.h"
#include "Notification.h"
#include "Observer.h"

/**
 * Enumeration for different parts of ModPlayground.
 */
enum ModPlaygroundId {
	MODPG_ID_BASE = 15000,
	MODULE_ID_ENTRY(PG,MAINPANEL),
	MODULE_ID_ENTRY(PG,OVERVIEWPANEL),
	MODULE_ID_ENTRY(PG,TOOLBAR)
};

/**
 * Playground GUI module.
 * This ties name, icon, main- and overview-panel together.
 */
class ModPlayground : public Module, private Observer
{
	public:
		/**
		 * Constructor ModPlayground.
		 * @param[in] 1st Parent window (for main and overview panel)
		 */
		ModPlayground(wxWindow *);

		/**
		 * Get ModPlayground base id.
		 * @param None.
		 * @return Base ID.
		 */
		int getBaseId(void);

		/**
		 * Get ModPlayground toolbar id.
		 * @param None.
		 * @return Toolbar ID.
		 */
		int getToolbarId(void);

		/**
		 * Update panels.
		 * @param None.
		 * @return Nothing.
		 */
		void update(void);

		/**
		 * Observer update.
		 * @param[in] 1st Changed subject.
		 * @return Nothing.
		 */
		void update(Subject *);

		/**
		 * Observer remove.
		 * @param[in] 1st Deleted subject.
		 * @return Nothing.
		 */
		void updateDelete(Subject *);

		/**
		 * Get active status.
		 * @param None.
		 * @return True if status is 'active' (aka loaded).
		 */
		bool isActive(void);

	private:
		/**
		 * Store status.
		 */
		bool isActive_;
};

#endif	/* _MODPLAYGROUND_H_ */
