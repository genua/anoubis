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

#include "AnEvents.h"

DEFINE_LOCAL_EVENT_TYPE(anEVT_LOGVIEWER_SHOW)
DEFINE_LOCAL_EVENT_TYPE(anEVT_RULEEDITOR_SHOW)

/*
 * A connection to the daemon was sucessfully established.
 * This event is intended of been used between the communicator
 * thread and communicator control only.
 * The integer field contains the connectionState enum stating:
 * a successfull connection     => CONNECTION_CONNECTED
 * normal disconnect            => CONNECTION_DISCONNECTED
 * an error (failed to connect) => CONNECTION_FAILED
 */
DEFINE_LOCAL_EVENT_TYPE(anEVT_COM_CONNECTION)

/*
 * The count of open ALERTS is stored in the events int field
 */
DEFINE_LOCAL_EVENT_TYPE(anEVT_OPEN_ALERTS)

/*
 * The count of open ESCALATIONS is stored in the events int field
 */
DEFINE_LOCAL_EVENT_TYPE(anEVT_OPEN_ESCALATIONS)

/*
 * A connection to the daemon was sucessfully established.
 * The remote station is transmitted as client data.
 */
DEFINE_LOCAL_EVENT_TYPE(anEVT_COM_REMOTESTATION)

/*
 * A notification was received. The client data points to the
 * corresponding client protocol message.
 */
DEFINE_LOCAL_EVENT_TYPE(anEVT_COM_NOTIFYRECEIVED)

/*
 * Transports a notification as client object.
 * A notification will been stored and deleted by ModAnoubis.
 * _ANY_ receiver of this evnet has to call event.Skip().
 */
DEFINE_LOCAL_EVENT_TYPE(anEVT_ADD_NOTIFICATION)
