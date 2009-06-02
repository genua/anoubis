/*
 * Copyright (c) 2009 GeNUA mbH <info@genua.de>
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

#ifndef _MODSB_H_
#define _MODSB_H_

#include "Module.h"
#include "Notification.h"
#include "Observer.h"

/**
 * Base and relative Ids of ModSb.
 */
enum ModSbId {
	MODSB_ID_BASE = 13000,
	MODULE_ID_ENTRY(SB,MAINPANEL),
	MODULE_ID_ENTRY(SB,OVERVIEWPANEL),
	MODULE_ID_ENTRY(SB,TOOLBAR)
};

/**
 * Class implementing Sandbox module (SB)
 */
class ModSb : public Module, private Observer
{
	public:
		/**
		 * Constructor of ModSb.
		 * @param[in] 1st The parent window.
		 */
		ModSb(wxWindow *);

		/**
		 * Destructor of ModSb.
		 * @param None.
		 */
		~ModSb(void);

		/**
		 * Get the BaseId of ModSb.
		 * @param None.
		 * @return The BaseId of ModSb.
		 */
		int     getBaseId(void);

		/**
		 * Get the ToolbarId of ModSb.
		 * @param None.
		 * @return The ToolbarId of ModSb.
		 */
		int     getToolbarId(void);

		/**
		 * This gets called when view ModSb has to be updated.
		 * @param None.
		 * @return Nothing.
		 */
		void    update(void);

		/**
		 * Handle update notification from subject.
		 * @param Subject.
		 * @return Nothing.
		 */
		void update(Subject *);
		void updateDelete(Subject *);

		/**
		 * Returns the state of ModSb.
		 * @param None.
		 * @return True if module is in active state.
		 */
		bool    isActive(void);

	private:
		bool    isActive_;	/**< state of ModSb */
};

#endif	/* _MODSB_H_ */
