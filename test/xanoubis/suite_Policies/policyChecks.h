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

#ifndef _POLICYCHECKS_H_
#define _POLICYCHECKS_H_

/*
 * Helper macros for verifing values of policies
 */

#define FAIL_IFZERO(ptr, msg)	fail_if(ptr == NULL, msg)

#define CHECK_POLICY_MODIFIED(policy, value) \
	do { \
		bool _isModified = policy->isModified(); \
		\
		policy->clearModified(); \
		if (_isModified != value) { \
			fail("policy->isModified() = %d, expected: %d", \
			    _isModified, value); \
		} \
	} while (0)

/*
 * Binaries
 */
#define CHECK_POLICY_GETBINARYCOUNT(policy, value)			\
	do {								\
		unsigned int _count = policy->getBinaryCount();		\
									\
		if (_count != value) {					\
			fail("getBinaryCount() = %d, expected: %d.",	\
			    _count, value);				\
		}							\
	} while (0)

#define CHECK_POLICY_GETBINARYNAME(policy, index, value)		\
	do {								\
		wxString _getName = policy->getBinaryName(index);	\
									\
		if (_getName.Cmp(value) != 0) {				\
			fail("getBinaryName(%d) = %ls, expected: %ls",	\
			    index, _getName.c_str(), value.c_str());	\
		}							\
	} while (0)

#define CHECK_POLICY_GETHASHTYPENO(policy, index, value)		\
	do {								\
		int _getTypeNo = policy->getHashTypeNo(index);		\
									\
		if (_getTypeNo != value) {				\
			fail("getHashTypeNo(%d) = %d, expected: %d",	\
			    index, _getTypeNo, value);			\
		}							\
	} while (0)

/*
 * Log
 */
#define CHECK_POLICY_GETLOGNO(policy, value)				\
	do {								\
		int _getLogNo = policy->getLogNo();			\
									\
		if (_getLogNo != value) {				\
			fail("getLogNo() = %d, expected: %d",		\
			    _getLogNo, value);				\
		}							\
	} while (0)

#define CHECK_POLICY_GETLOGNAME(policy, value)				\
	do {								\
		wxString _getLogName = policy->getLogName();		\
									\
		if (_getLogName.Cmp(value) != 0) {			\
			fail("getLogName() = %ls, expected: %ls",	\
			    _getLogName.c_str(), value.c_str());	\
		}							\
	} while (0)

/*
 * Action
 */
#define CHECK_POLICY_GETACTIONNO(policy, value)				\
	do {								\
		int _getActionNo = policy->getActionNo();		\
									\
		if (_getActionNo != value) {				\
			fail("getActionNo() = %d, expected: %d",	\
			    _getActionNo, value);			\
		}							\
	} while (0)

#define CHECK_POLICY_GETACTIONNAME(policy, value)			\
	do {								\
		wxString _getActionName = policy->getActionName();	\
									\
		if (_getActionName.Cmp(value) != 0) {			\
			fail("getActionName() = %ls, expected: %ls",	\
			    _getActionName.c_str(), value.c_str());	\
		}							\
	} while (0)

/*
 * Capability type
 */
#define CHECK_POLICY_GETCAPABILITYTYPENO(policy, value)			\
	do {								\
		int _getCapabilityNo = policy->getCapabilityTypeNo();	\
									\
		if (_getCapabilityNo != value) {			\
			fail("getCapabilityNo() = %d, expected: %d",	\
			    _getCapabilityNo, value);			\
		}							\
	} while (0)

#define CHECK_POLICY_GETCAPABILITYTYPENAME(policy, value)		       \
	do {								       \
		wxString _getCapabilityName = policy->getCapabilityTypeName(); \
									       \
		if (_getCapabilityName.Cmp(value) != 0) {		       \
			fail("getCapabilityName() = %ls, expected: %ls",       \
			    _getCapabilityName.c_str(), value.c_str());	       \
		}							       \
	} while (0)

/*
 * Direction
 */
#define CHECK_POLICY_GETDIRECTIONNO(policy, value)			\
	do {								\
		int _getDirectionNo = policy->getDirectionNo();		\
									\
		if (_getDirectionNo != value) {				\
			fail("getDirectionNo() = %d, expected: %d",	\
			    _getDirectionNo, value);			\
		}							\
	} while (0)

#define CHECK_POLICY_GETDIRECTIONNAME(policy, value)			 \
	do {								 \
		wxString _getDirectionName = policy->getDirectionName(); \
									 \
		if (_getDirectionName.Cmp(value) != 0) {		 \
			fail("getDirectionName() = %ls, expected: %ls",	 \
			    _getDirectionName.c_str(), value.c_str());	 \
		}							 \
	} while (0)

/*
 * Protocol
 */
#define CHECK_POLICY_GETPROTOCOLNO(policy, value)			\
	do {								\
		int _getProtocolNo = policy->getProtocolNo();		\
									\
		if (_getProtocolNo != value) {				\
			fail("getProtocolNo() = %d, expected: %d",	\
			    _getProtocolNo, value);			\
		}							\
	} while (0)

#define CHECK_POLICY_GETPROTOCOLNAME(policy, value)			\
	do {								\
		wxString _getProtocolName = policy->getProtocolName();	\
									\
		if (_getProtocolName.Cmp(value) != 0) {			\
			fail("getProtocolName() = %ls, expected: %ls",	\
			    _getProtocolName.c_str(), value.c_str());	\
		}							\
	} while (0)

/*
 * AddrFamily
 */
#define CHECK_POLICY_GETADDRFAMILYNO(policy, value)			\
	do {								\
		int _getAddrFamilyNo = policy->getAddrFamilyNo();	\
									\
		if (_getAddrFamilyNo != value) {			\
			fail("getAddrFamilyNo() = %d, expected: %d",	\
			    _getAddrFamilyNo, value);			\
		}							\
	} while (0)

#define CHECK_POLICY_GETADDRFAMILYNAME(policy, value)			   \
	do {								   \
		wxString _getAddrFamilyName = policy->getAddrFamilyName(); \
									   \
		if (_getAddrFamilyName.Cmp(value) != 0) {		   \
			fail("getAddrFamilyName() = %ls, expected: %ls",   \
			    _getAddrFamilyName.c_str(), value.c_str());	   \
		}							   \
	} while (0)

/*
 * Context type
 */
#define CHECK_POLICY_GETCONTEXTTYPENO(policy, value)			\
	do {								\
		int _getContextNo = policy->getContextTypeNo();		\
									\
		if (_getContextNo != value) {				\
			fail("getContextNo() = %d, expected: %d",	\
			    _getContextNo, value);			\
		}							\
	} while (0)

#define CHECK_POLICY_GETCONTEXTTYPENAME(policy, value)			 \
	do {								 \
		wxString _getContextName = policy->getContextTypeName(); \
									 \
		if (_getContextName.Cmp(value) != 0) {			 \
			fail("getContextName() = %ls, expected: %ls",	 \
			    _getContextName.c_str(), value.c_str());	 \
		}							 \
	} while (0)

/*
 * Path
 */
#define CHECK_POLICY_GETPATH(policy, value)				\
	do {								\
		wxString _getPath = policy->getPath();			\
									\
		if (_getPath.Cmp(value) != 0) {				\
			fail("getPath() = %ls, expected: %ls",		\
			    _getPath.c_str(), value.c_str());		\
		}							\
	} while (0)

/*
 * Subject
 */
#define CHECK_POLICY_GETSUBJECTTYPENO(policy, value)			\
	do {								\
		int _getTypeNo = policy->getSubjectTypeNo();		\
									\
		if (_getTypeNo != value) {				\
			fail("getSubjectTypeNo() = %d, expected: %d",	\
			    _getTypeNo, value);				\
		}							\
	} while (0)

#define CHECK_POLICY_GETSUBJECTNAME(policy, value)			\
	do {								\
		wxString _getSubjectName = policy->getSubjectName();	\
									\
		if (_getSubjectName.Cmp(value) != 0) {			\
			fail("getSubjectName() = %ls, expected: %ls",	\
			    _getSubjectName.c_str(), value.c_str());	\
		}							\
	} while (0)

/*
 * Valid
 */
#define CHECK_POLICY_GETVALIDACTION(policy, valueNo, valueName)		    \
	do {								    \
		int _getActionNo = policy->getValidActionNo();		    \
		wxString _getActionName = policy->getValidActionName();	    \
									    \
		if ((_getActionNo != valueNo) &&			    \
		    (_getActionName.Cmp(valueName) != 0)) {		    \
			fail("Valid action = %d / %ls, expected: %d / %ls", \
			    _getActionNo, _getActionName.c_str(),	    \
			    valueNo, valueName.c_str());		    \
		}							    \
	} while (0)

#define CHECK_POLICY_GETVALIDLOG(policy, valueNo, valueName)		   \
	do {								   \
		int _getLogNo = policy->getValidLogNo();		   \
		wxString _getLogName = policy->getValidLogName();	   \
									   \
		if ((_getLogNo != valueNo) &&				   \
		    (_getLogName.Cmp(valueName) != 0)) {		   \
			fail("Valid log() = %d / %ls, expected: %d / %ls", \
			    _getLogNo, _getLogName.c_str(),		   \
			    valueNo, valueName.c_str());		   \
		}							   \
	} while (0)

/*
 * Invalid
 */
#define CHECK_POLICY_GETINVALIDACTION(policy, valueNo, valueName)	      \
	do {								      \
		int _getActionNo = policy->getInvalidActionNo();	      \
		wxString _getActionName = policy->getInvalidActionName();     \
									      \
		if ((_getActionNo != valueNo) &&			      \
		    (_getActionName.Cmp(valueName) != 0)) {		      \
			fail("Invalid action = %d / %ls, expected: %d / %ls", \
			    _getActionNo, _getActionName.c_str(),	      \
			    valueNo, valueName.c_str());		      \
		}							      \
	} while (0)

#define CHECK_POLICY_GETINVALIDLOG(policy, valueNo, valueName)		     \
	do {								     \
		int _getLogNo = policy->getInvalidLogNo();		     \
		wxString _getLogName = policy->getInvalidLogName();	     \
									     \
		if ((_getLogNo != valueNo) &&				     \
		    (_getLogName.Cmp(valueName) != 0)) {		     \
			fail("Invalid log() = %d / %ls, expected: %d / %ls", \
			    _getLogNo, _getLogName.c_str(),		     \
			    valueNo, valueName.c_str());		     \
		}							     \
	} while (0)

/*
 * Unknown
 */
#define CHECK_POLICY_GETUNKNOWNACTION(policy, valueNo, valueName)	      \
	do {								      \
		int _getActionNo = policy->getUnknownActionNo();	      \
		wxString _getActionName = policy->getUnknownActionName();     \
									      \
		if ((_getActionNo != valueNo) &&			      \
		    (_getActionName.Cmp(valueName) != 0)) {		      \
			fail("Unknown action = %d / %ls, expected: %d / %ls", \
			    _getActionNo, _getActionName.c_str(),	      \
			    valueNo, valueName.c_str());		      \
		}							      \
	} while (0)

#define CHECK_POLICY_GETUNKNOWNLOG(policy, valueNo, valueName)		     \
	do {								     \
		int _getLogNo = policy->getUnknownLogNo();		     \
		wxString _getLogName = policy->getUnknownLogName();	     \
									     \
		if ((_getLogNo != valueNo) &&				     \
		    (_getLogName.Cmp(valueName) != 0)) {		     \
			fail("Unknown log() = %d / %ls, expected: %d / %ls", \
			    _getLogNo, _getLogName.c_str(),		     \
			    valueNo, valueName.c_str());		     \
		}							     \
	} while (0)

/*
 * Mask
 */
#define CHECK_POLICY_GETMASKNO(policy, value)				\
	do {								\
		int _getMaskNo = policy->getAccessMaskNo();		\
									\
		if (_getMaskNo != value) {				\
			fail("getAccessMaskNo() = %d, expected: %d",	\
			    _getMaskNo, value);				\
		}							\
	} while (0)

#define CHECK_POLICY_GETMASKNAME(policy, value)				 \
	do {								 \
		wxString _getMaskName = policy->getAccessMaskName();	 \
									 \
		if (_getMaskName.Cmp(value) != 0) {			 \
			fail("getAccessMaskName() = %ls, expected: %ls", \
			    _getMaskName.c_str(), value.c_str());	 \
		}							 \
	} while (0)

/*
 * Observer
 */
#define CHECK_OBSERVER_NOTIFIED(observer, value)			  \
	do {								  \
		bool _isNotified = observer->isNotified();		  \
									  \
		if (_isNotified != value) {				  \
			fail("observer->isNotified() = %d, expected: %d", \
			    _isNotified, value);			  \
		}							  \
	} while(0)

#endif	/* _POLICYCHECKS_H_ */
