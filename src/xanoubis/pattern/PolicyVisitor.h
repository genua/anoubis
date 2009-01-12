/*
 * Copyright (c) 2008 GeNUA mbH <info@genua.de>
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

#ifndef _POLICYVISITOR_H_
#define _POLICYVISITOR_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//#include "Policy.h"
//#include "AppPolicy.h"
//#include "FilterPolicy.h"

#include "AlfAppPolicy.h"
#include "AlfCapabilityFilterPolicy.h"
#include "AlfFilterPolicy.h"
#include "ContextAppPolicy.h"
#include "ContextFilterPolicy.h"
#include "DefaultFilterPolicy.h"
#include "SbAccessFilterPolicy.h"
#include "SbAppPolicy.h"
#include "SfsAppPolicy.h"
#include "SfsFilterPolicy.h"

/**
 * Design pattern: Visitor.
 *
 * This is the common way of separating an algorithm from an object structure
 * upon which it operates. Use this as base class for new algorithms on
 * policies.
 */
class PolicyVisitor
{
	public:
		/**
		 * Constructor for PolicyVisitor.
		 * @param None.
		 * @return Nothing.
		 */
		PolicyVisitor(void);

		/**
		 * Destructor for PolicyVisitor.
		 * Because for abstract classes the destructor has to be
		 * virtual, we have to declare it. (The default/implicite
		 * destructor is not virtual and causes warnings).
		 * @param None.
		 * @return Nothing.
		 */
		virtual ~PolicyVisitor(void);

		/**
		 * Set the propagation.
		 * If the propagation is 'false', only one policy will been
		 * visited. Set this to 'true' (the default), the visitor
		 * will walk down the object structure.
		 * @param 1st The new propagation flag.
		 * @return Nothing.
		 */
		void setPropagation(bool);

		/**
		 * Get propagation flag.
		 * Use this to decide if the visitor is given to the next
		 * object in the structure.
		 * @param None.
		 * @return The propagation flag.
		 */
		bool shallBeenPropagated(void);

		/**
		 * Visit an AlfAppPolicy.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitAlfAppPolicy(AlfAppPolicy *) = 0;

		/**
		 * Visit an AlfCapabilityFilterPolicy.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitAlfCapabilityFilterPolicy(
		    AlfCapabilityFilterPolicy *) = 0;

		/**
		 * Visit an AlfFilterPolicy.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitAlfFilterPolicy(AlfFilterPolicy *) = 0;

		/**
		 * Visit a ContextAppPolicy.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitContextAppPolicy(ContextAppPolicy *) = 0;

		/**
		 * Visit a ContextFilterPolicy.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitContextFilterPolicy(
		    ContextFilterPolicy *) = 0;

		/**
		 * Visit a DefaultFilterPolicy.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitDefaultFilterPolicy(
		    DefaultFilterPolicy *) = 0;

		/**
		 * Visit a SbAccessFilterPolicy.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitSbAccessFilterPolicy(
		    SbAccessFilterPolicy *) = 0;

		/**
		 * Visit a SbAppPolicy.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitSbAppPolicy(SbAppPolicy *) = 0;

		/**
		 * Visit a SfsAppPolicy.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitSfsAppPolicy(SfsAppPolicy *) = 0;

		/**
		 * Visit a SfsFilterPolicy.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitSfsFilterPolicy(SfsFilterPolicy *) = 0;

	private:
		bool	propagate_;	/**< The propagation flag. */
};

#endif	/* _POLICYVISITOR_H_ */
