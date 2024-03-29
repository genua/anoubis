/*
 * Copyright (c) 2007 GeNUA mbH <info@genua.de>
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

#ifndef __ModAlfMainPanelImpl__
#define __ModAlfMainPanelImpl__

#include "AnEvents.h"

#include "ModAlfPanelsBase.h"

class ModAlfMainPanelImpl : public ModAlfMainPanelBase
{
	public:
		/**
		 * Constructor of ModAlfMainPanelImpl.
		 * @param[in] 1st The parent window and ID.
		 */
		ModAlfMainPanelImpl(wxWindow*, wxWindowID);

		/**
		 * Destructor of ModAlfMainPanelImpl.
		 * @param None.
		 */
		~ModAlfMainPanelImpl(void);

	private:
		/**
		 * If double click (left) on cell occurs, jump to
		 * corresponding rule in RuleEditor
		 * @param[in] 1st The Event
		 * @return Nothing
		 */
		void OnGridCellLeftDClick(wxGridEvent&);
};

#endif /* __ModAlfMainPanelImpl__ */
