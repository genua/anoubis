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

#ifndef _ANDETAILS_H_
#define _ANDETAILS_H_

#include <wx/panel.h>
#include <wx/statbmp.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/tglbtn.h>
#include <wx/stattext.h>

/**
 * This realizes the Details-Widgets.
 *
 * The Details widget is an ordinary panel, extended by an arrow-icon
 * and a label. A release of the left mouse button on these will toggle
 * the visability of the comprehend content.\n
 * \n
 * \b Usage (with the wxformbuilder):\n
 * - Add a normal wxPanel
 * - Set the property 'window_name' to the value you want to be shown on
 *   the right side of the arrow as description of the containing stuff.
 * - Set the property 'subclass.name' to "AnDetails"
 * - Set the property 'subclass.include' to "AnDetails.h"
 * - Disable the stretching of the panel. (It's Alf-s or the icon with the
 *   square and the arrow on it's top and on the right side.)
 * - Set the property 'wxRAISED_BOARDER'
 * - Fill the panel as normal
 */
class AnDetails : public wxPanel
{
	public:
		/**
		 * Constructor of the details panel.
		 * It has to have the same signature as a ordinary wxPanel,
		 * so wxformbuilder can just exchange wxPanel with AnDetails.
		 */
		AnDetails(wxWindow *parent, wxWindowID id = wxID_ANY,
		    const wxPoint& pos = wxDefaultPosition,
		    const wxSize& size = wxDefaultSize,
		    long style = wxTAB_TRAVERSAL,
		    const wxString& name = wxPanelNameStr);

		/**
		 * Set the (main) sizer of this panel.
		 * This method is an overlay to the original method inherited
		 * from wxPanel (which is unfortuantely not virtual).
		 * @param[in] 1st The new sizer.
		 * @param[in] 2nd Given true here deletes the old sizer.
		 * @return Nothind.
		 */
		void SetSizer(wxSizer *, bool deleteOld = true);

	protected:
		/**
		 * This is the icon shown at the top left corner. It shows
		 * an arrow pointing:\n
		 * - upwards if the content is visible (can closed)
		 * - downwards if the content is not visible (can opended)
		 * An event-handling method onDetailClick() is registered here,
		 * triggered when the left mouse button is released.
		 */
		wxStaticBitmap* detailsIcon_;

		/**
		 * This is the text just right to the icon. It is set to the
		 * value of window_name (set within the properties in the
		 * wxformbuilder).\n
		 * An event-handling method onDetailClick() is registered here,
		 * triggered when the left mouse button is released.
		 */
		wxStaticText* detailsLabel_;

		/**
		 * This is the delimiting line just to the label.
		 */
		wxStaticLine* detailsLine_;

		/**
		 * This holds the arrow image (arrow down).
		 */
		wxIcon *downArrow_;

		/**
		 * This holds the arrow image (arrow right).
		 */
		wxIcon *rightArrow_;

		/**
		 * Event handler for left mouse button releases.
		 * This is called by the arrow-icon and the lable-text.
		 * @param[in] 1st The mouse event.
		 * @return Nothing.
		 */
		virtual void onDetailClick(wxMouseEvent &);

	private:
		/**
		 * This flag is used to store the current status of visability
		 * of the sizer of the content.
		 */
		bool isVisible_;

		/**
		 * Store here the sizer of content. This is used to toggle
		 * the visability.
		 */
		wxSizer *contentSizer_;

		/**
		 * Update icon, tooltips and content's visibility.
		 * @param None.
		 * @return Nothing.
		 */
		void update(void);
};

#endif	/* _ANDETAILS_H_ */
