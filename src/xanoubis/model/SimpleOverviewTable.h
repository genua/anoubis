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

#ifndef _SIMPLEOVERVIEWTABLE_H_
#define _SIMPLEOVERVIEWTABLE_H_

#include <wx/event.h>
#include <wx/grid.h>

#include <vector>
#include <typeinfo>

#include "PolicyVisitor.h"

class SimpleOverviewRow;
class SimpleOverviewVisitor;

/**
 * Base overview table.
 *
 * This is the base table for the <i>Simple Overviews</i>. This class is
 * responsible for administration of the data for the table. For this you need
 * to assign type-informatin (wxClassInfo) of the application-policies you want
 * to manage by the table.
 *
 * Derivating classes still needs to implement methods of wxGridTableBase
 * because this base-class cannot tell a wxGrid, which columns should be
 * displayed  and what is the corresponding content of the cells.
 */
class SimpleOverviewTable : public wxGridTableBase,
    private wxEvtHandler, private Observer
{
	public:
		/**
		 * D'tor.
		 */
		~SimpleOverviewTable(void);

		/**
		 * Reloads the table.
		 *
		 * Receives the current user- & admin-policyruleset and fills
		 * the tablke with the information from there.
		 */
		void reload(void);

		/**
		 * Returns the number of rows of the table.
		 *
		 * @return Number of rows.
		 */
		int getNumRows(void) const;

		/**
		 * Returns the row stored at the given index.
		 *
		 * @param idx The requested index
		 * @return The row at the given index
		 */
		SimpleOverviewRow *getRowAt(unsigned int) const;

	protected:
		/**
		 * Creates a SimpleOverviewTable-instance.
		 *
		 * @param classInfo Type-information about application-policies
		 *                  to be displayed.
		 */
		SimpleOverviewTable(const std::type_info &);

	private:
		/**
		 * Id of our RuleSet.
		 */
		long	userRuleSetId_;

		/**
		 * Id of our admin RuleSet.
		 */
		long	adminRuleSetId_;

		/**
		 * The assigned visitor.
		 * The source of the data to be displayed.
		 */
		SimpleOverviewVisitor *visitor_;

		/**
		 * Invoked a new ruleset arrived.
		 *
		 * Resets the model.
		 */
		void onLoadRuleSet(wxCommandEvent &);

		/**
		 * Invoked, if the ruleset has changed.
		 *
		 * Reloads the table.
		 */
		void update(Subject *);

		/**
		 * Invoked, if the ruleset has changed.
		 *
		 * Reloads the table.
		 */
		void updateDelete(Subject *);

		/**
		 * Removes all data from the table.
		 */
		void clearTable(void);

		/**
		 * Fires an insert-message.
		 *
		 * So the assigned view can react on such a situation.
		 */
		void fireRowsInserted(int, int);

		/**
		 * Fires a remove-message.
		 *
		 * So the assigned view can react on such a situation.
		 */
		void fireRowsRemoved(int, int);
};

#endif	/* _SIMPLEOVERVIEWTABLE_H_ */
