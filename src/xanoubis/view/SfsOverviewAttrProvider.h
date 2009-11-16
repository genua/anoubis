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

#ifndef _SFSOVERVIEWATTRPROVIDER_H_
#define _SFSOVERVIEWATTRPROVIDER_H_

#include <wx/grid.h>

/**
 * Attribute provider for the <i>Sfs Simple Overview</i>-views.
 *
 * An attribute determines the look&feel of a cell of the grid. An attribute
 * provider is responsible for providing such attributes. The attributes
 * of the <i>Simple Overview</i> colourizes the grid.
 */
class SfsOverviewAttrProvider : public wxGridCellAttrProvider
{
	public:
		/**
		 * Constructs a SfsOverviewAttrProvider.
		 *
		 * @param table Source table contains data to be displayed
		 */
		SfsOverviewAttrProvider(void);

		/**
		 * D'tor.
		 */
		~SfsOverviewAttrProvider(void);

		/**
		 * Returns attributes for the given cell.
		 *
		 * This method is part of the wxGridCellAttrProvider-interface
		 * and is used by wxGrid to get information how to display a
		 * cell of the grid.
		 *
		 * @param row Row of cell
		 * @param col Column of cell
		 * @param kind Kind of cell
		 * @return Attribute of the requested cell
		 * @see wxGridCellAttrProvider
		 */
		wxGridCellAttr *GetAttr(int, int,
		    wxGridCellAttr::wxAttrKind) const;

	private:
		/**
		 * Template attribute for an even filter-column.
		 */
		wxGridCellAttr *attrFilterEven_;

		/**
		 * Prepares an attribute.
		 *
		 * If the first attribute "in" is NULL, the second attribute
		 * "template" is used. If "in" is not NULL, the properties of
		 * the template are copied into the first attribute.
		 *
		 * @param in Input attribute. Can be NULL
		 * @param tpl Template attribute. Is used if the source
		 *            template is NULL.
		 * @return The resulting attribute
		 */
		wxGridCellAttr *setupAttr(wxGridCellAttr *,
		    wxGridCellAttr *) const;
};

#endif	/* _SFSOVERVIEWATTRPROVIDER_H_ */
