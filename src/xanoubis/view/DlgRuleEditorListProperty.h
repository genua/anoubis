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

#ifndef _DLGRULEEDITORLISTPROPERTY_H_
#define _DLGRULEEDITORLISTPROPERTY_H_

#include <wx/string.h>

#include "Policy.h"
#include "PolicyRuleSet.h"
#include "AnIconList.h"
#include "AnListClass.h"
#include "AnListProperty.h"

/**
 * Attributre provider for the RuleEditor grids listing the policies.
 * Depending on the assigned type, this property class acts as a different
 * propety and provides differend aspects of the given policy.
 * This is an implementation of the interface defined by AnListProperty.
 */
class DlgRuleEditorListProperty : public AnListProperty
{
	public:
		/**
		 * Enumeration of different types of properties.
		 */
		enum PropertyType {
			/**
			 * APN rule id. Valid on any policy.
			 */
			PROPERTY_ID = 0,
			/**
			 * Policy type. Valid on any policy.
			 */
			PROPERTY_TYPE,
			/**
			 * User name of RuleSet. Valid on app policy.
			 */
			PROPERTY_USER,
			/**
			 * DETAILS flag. Valid on CTX filter.
			 */
			PROPERTY_DETAILS,
			/**
			 * Name of binary. Valid on app policy and
			 * CTX filter.
			 */
			PROPERTY_BINARY,
			/**
			 * Action. Valid on filter policy.
			 */
			PROPERTY_ACTION,
			/**
			 * Log. Valid on filter policy.
			 */
			PROPERTY_LOG,
			/**
			 * Scope of policy. Valid on filter policy.
			 */
			PROPERTY_SCOPE,
			/**
			 * Path within subject. Valid on SFS and SB policies.
			 */
			PROPERTY_PATH,
			/**
			 * Subject. Valid on SFS and SB policies.
			 */
			PROPERTY_SUB,
			/**
			 * Capability. Valid on ALF filter.
			 */
			PROPERTY_CAP,
			/**
			 * Direction. Valid on ALF filter.
			 */
			PROPERTY_DIR,
			/**
			 * Protocol. Valid on ALF filter.
			 */
			PROPERTY_PROT,
			/**
			 * From Host. Valid on ALF filter.
			 */
			PROPERTY_FHOST,
			/**
			 * From Port. Valid on ALF filter.
			 */
			PROPERTY_FPORT,
			/**
			 * To Host. Valid on ALF filter.
			 */
			PROPERTY_THOST,
			/**
			 * To Port. Valid on ALF filter.
			 */
			PROPERTY_TPORT,
			/**
			 * Valid Action. Valid on SFS filter.
			 */
			PROPERTY_VALACT,
			/**
			 * Valid Log. Valid on SFS filter.
			 */
			PROPERTY_VALLOG,
			/**
			 * Invalid Action. Valid on SFS filter.
			 */
			PROPERTY_INVALACT,
			/**
			 * Invalid Log. Valid on SFS filter.
			 */
			PROPERTY_INVALLOG,
			/**
			 * Unknown Action. Valid on SFS filter.
			 */
			PROPERTY_UNKACT,
			/**
			 * Unknown Log. Valid on SFS filter.
			 */
			PROPERTY_UNKLOG,
			/**
			 * Access Mask. Valid on SB filter.
			 */
			PROPERTY_MASK,
		};

		/**
		 * Constructor.
		 * @param[in] 1st Type of property.
		 */
		DlgRuleEditorListProperty(PropertyType);

		/**
		 * Implementation of AnListProperty::getHeader().
		 * Depending of the type, a different header is returned.
		 * @return Header of property.
		 */
		wxString getHeader(void) const;

		/**
		 * Implementation of AnListProperty::getText().
		 * Depending of the type, another text to be displayed is
		 * returned.
		 * @param[in] 1st The policy to be displayed.
		 * @return Text of property.
		 */
		wxString getText(AnListClass *) const;

		/**
		 * Implementation of AnListProperty::getIcon().
		 * ATTENTION - This method exists only to satisfy the interface.
		 * It returns ICON_NONE in any case.
		 * @param[in] 1st The policy to be displayed.
		 * @return Text of property.
		 */
		AnIconList::IconId getIcon(AnListClass *) const;

	private:
		/**
		 * Store type of this property instance.
		 */
		PropertyType type_;

		/**
		 * Examine the different text of the given policy according
		 * to the type of this property.
		 * @param[in] 1st The policy in question.
		 * @return The concerning text depending on the property type.
		 */
		wxString examineAppPolicy(AppPolicy *) const;

		/**
		 * Examine the different text of the given policy according
		 * to the type of this property.
		 * @param[in] 1st The policy in question.
		 * @return The concerning text depending on the property type.
		 */
		wxString examineAlfFilterPolicy(AlfFilterPolicy *) const;

		/**
		 * Examine the different text of the given policy according
		 * to the type of this property.
		 * @param[in] 1st The policy in question.
		 * @return The concerning text depending on the property type.
		 */
		wxString examineAlfCapabilityFilterPolicy(
		    AlfCapabilityFilterPolicy *) const;

		/**
		 * Examine the different text of the given policy according
		 * to the type of this property.
		 * @param[in] 1st The policy in question.
		 * @return The concerning text depending on the property type.
		 */
		wxString examineSfsDefaultFilterPolicy(
		    SfsDefaultFilterPolicy *) const;

		/**
		 * Examine the different text of the given policy according
		 * to the type of this property.
		 * @param[in] 1st The policy in question.
		 * @return The concerning text depending on the property type.
		 */
		wxString examineSfsFilterPolicy(SfsFilterPolicy *) const;

		/**
		 * Examine the different text of the given policy according
		 * to the type of this property.
		 * @param[in] 1st The policy in question.
		 * @return The concerning text depending on the property type.
		 */
		wxString examineDefaultFilterPolicy(
		    DefaultFilterPolicy *) const;

		/**
		 * Examine the different text of the given policy according
		 * to the type of this property.
		 * @param[in] 1st The policy in question.
		 * @return The concerning text depending on the property type.
		 */
		wxString examineContextFilterPolicy(
		    ContextFilterPolicy *) const;

		/**
		 * Examine the different text of the given policy according
		 * to the type of this property.
		 * @param[in] 1st The policy in question.
		 * @return The concerning text depending on the property type.
		 */
		wxString examineSbAccessFilterPolicy(
		    SbAccessFilterPolicy *) const;
};

#endif	/* _DLGRULEEDITORLISTPROPERTY_H_ */
