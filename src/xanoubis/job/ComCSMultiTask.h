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

#ifndef _COMCSMULTITASK_H_
#define _COMCSMULTITASK_H_

#include "ComTask.h"

#include <sys/types.h>
#include <vector>
#include <anoubis_client.h>
#include <anoubis_transaction.h>

/**
 * Common base class of all CSMulti request.
 *
 * This class handles pathname, public keys, csmulti requests and
 * error reporting.
 */
class ComCSMultiTask : public ComTask
{
	public:
		/**
		 * Destructor.
		 */
		~ComCSMultiTask(void);

		/**
		 * Add a path to the request.
		 *
		 * @param 1st The path.
		 * @retrun None.
		 */
		void addPath(const wxString);

		/**
		 * Return the number of paths in this ComCSMultiTask.
		 *
		 * @return The number of paths.
		 */
		unsigned int getPathCount(void) const;

		/**
		 * Return the path associated with an index in the request.
		 *
		 * @param idx the index.
		 * @return The path.
		 */
		wxString getPath(unsigned int idx) const;

		/**
		 * Returns the result of the checksum request for the
		 * path with the given index.
		 *
		 * @param idx The index.
		 * @return The error code. Zero if the checksum was removed.
		 */
		int getChecksumError(unsigned int idx) const;

		/**
		 * Returns the result of the signature requst for the
		 * path with the given index.
		 *
		 * @param idx The index.
		 * @return The error code. Zero if the signature was removed.
		 */
		int getSignatureError(unsigned int idx) const;

		/**
		 * Assign a keyid to the request.
		 *
		 * @param 1st The keyid.
		 * @param 2nd The length of the keyid.
		 * @return True if successful.
		 */
		bool setKeyId(const uint8_t *keyid, unsigned int kidlen);

		/**
		 * Return true if a keyid is assigned to the request.
		 *
		 * @param None.
		 * @return True if there is a keyid.
		 */
		bool haveKeyId(void) const;

	protected:
		/**
		 * Default constructor.
		 */
		ComCSMultiTask(void);

		std::vector<wxString>		 paths_;
		struct anoubis_csmulti_request	*csreq_;
		struct anoubis_csmulti_request	*sigreq_;
		uint8_t				*keyid_;
		unsigned int			 kidlen_;

		/**
		 * Add a new path to a single request. If this fails add
		 * an error request.
		 *
		 * @param 1st The request.
		 * @param 2nd The path.
		 * @return Zero if something was added to the request.
		 *     An error code (ENOMEM) if neither the request nor
		 *     an error request could be added.
		 */
		int addPathToRequest(struct anoubis_csmulti_request *,
		    const char *path);

		/**
		 * Add a new path with checksum/signature data to a single
		 * request. If this fails add an error request.
		 *
		 * @param 1st The requst.
		 * @param 2nd The path.
		 * @param 3rd A pointer to the checksum/signature data.
		 * @param 4th The length of the signature data.
		 * @return Zero if something was added to the request.
		 *     An error code (ENOMEM) if neither the request nor
		 *     an error request could be added.
		 */
		int addPathToRequest(struct anoubis_csmulti_request *,
		    const char *path, uint8_t *, unsigned int);

		/**
		 * Create CSMulti request for checksum and/or signature
		 * operations. The type of the operation is passed as
		 * and argument. It should be a checksum operation. The
		 * corresponding signature operation is derived.
		 *
		 * @param 1st The checksum operation (ADD, GET or DEL)
		 * @return None.
		 */
		void createRequests(int op);

		/**
		 * Implementation of ComTask::done()
		 * This done functions expects anoubis_csmulti_requests
		 * that are prepared in csreq_ and optionally sigreq_.
		 * This requests are sent to the daemon in consecutive
		 * calls to this done function.
		 */
		virtual bool done(void);

	private:
		struct anoubis_transaction	*ta_;
};

#endif	/* _COMCSMULTITASK_H_ */
