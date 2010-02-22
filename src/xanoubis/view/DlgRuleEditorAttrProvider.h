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

#ifndef _DLGRULEEDITORATTRPROVIDER_H_
#define _DLGRULEEDITORATTRPROVIDER_H_

#include <wx/grid.h>

#include "AnTable.h"
/**
 * Attribute provider for the grids within the RuleEditor.
 *
 * An attribute determines the look&feel of a cell of the grid. An attribute
 * provider is responsible for providing such attributes.
 * This provider cause admin policies of a user to appear as grey writing
 * on white ground.
 */
class DlgRuleEditorAttrProvider : public wxGridCellAttrProvider
{
	public:
		/**
		 * Constructor.
		 * @param[in] 1st The table (data model).
		 */
		DlgRuleEditorAttrProvider(AnTable *);

		/**
		 * Destructor.
		 */
		~DlgRuleEditorAttrProvider(void);

		/**
		 * Returns attributes for the given cell.
		 * This method is part of the wxGridCellAttrProvider-interface
		 * and is used by wxGrid to get information how to display a
		 * cell of the grid.
		 * @param[in] 1st Row address of cell.
		 * @param[in] 2nd Column address of cell.
		 * @param[in] 3rd Kind of cell.
		 * @return Attribute of the requested cell
		 * @see wxGridCellAttrProvider
		 */
		wxGridCellAttr *GetAttr(int, int,
		    wxGridCellAttr::wxAttrKind) const;

	private:
		/**
		 * Store the associated table.
		 */
		AnTable *table_;

		/**
		 * Template attribute for admin policies
		 */
		wxGridCellAttr *adminTemplateAttr_;//attrFilterEven_;

		/**
		 * Prepares an attribute.
		 * Setup the first attribute with settings of the template.
		 * Or adjust reference count and retrun the template.
		 * @param[in] 1st Input attribute. Can be NULL.
		 * @return The resulting attribute
		 */
		wxGridCellAttr *setupAttr(wxGridCellAttr *) const;
};

#endif	/* _DLGRULEEDITORATTRPROVIDER_H_ */
