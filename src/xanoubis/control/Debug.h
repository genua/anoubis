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

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <wx/string.h>

/**
 * Debug Messages
 *
 * The Debug level controlls the amount of output to the console.
 * The lower the level less output will be given to the console.
 * The print function gets an level which represent the priority level
 * of the Debug Message.
 */
class Debug
{
	public:
		/**
		 * Enumeration of possible debugging-levels.
		 */
		enum Level {
			FATAL = 0,	/*!< A fatal (non recoverable) error
					     occured. A message is printed to
					     stderr and to syslog. The
					     application is terminated. */
			ERR,		/*!< A recoverable error occured. The
					     message is send to the log-viewer
					     and to syslog. */
			WARN,		/*!< A warning. The message is send to
					     the log-viewer and to syslog. */
			INFO,		/*!< An information message. The
					     message is send to the
					     log-viewer. */
			CHAT,		/*!< Prints messages exchanged between
					     daemon and client */
			TRACE		/*!< An arbitrary trace-message. */
		};

		/**
		 * Initializes the debugging-utility.
		 *
		 * This method needs to be called before the first debug-entry
		 * is created.
		 *
		 * @return Nothing
		 */
		static void initialize(void);

		/**
		 * Uninitializes the debugging-utility.
		 *
		 * This method needs to be called before the application
		 * exists.
		 *
		 * @return Nothing
		 */
		static void uninitialize(void);

		/**
		 * Returns the debugging-level.
		 * @return Debugging-level
		 */
		static Level getLevel(void);

		//@{
		/**
		 * Set Debug level
		 * Set the level of debug output to the console
		 * the higher the level, the more output should be
		 * created
		 * @param 1st level of debug output
		 * @return Nothing.
		 */
		static void setLevel(Level);
		static void setLevel(long);
		//@}

		/**
		 * Check Debug level
		 * Check the level of debug output to the console
		 * @param 1st level of debug output
		 * @return true if the output would be logged, false otherwise.
		 */
		static bool checkLevel(Level);

		/**
		 * Debugs a fatal error.
		 *
		 * The error is not recoverable. The message is printed to
		 * stderr and to syslog. The application is terminated.
		 *
		 * @param 1st Message format
		 * @param 2nd...n Arguments for the message
		 * @return Nothing
		 */
		static void fatal(const wxChar *, ...);

		/**
		 * Debugs a recoverable error.
		 *
		 * The message is send to the log-viewer and to syslog.
		 *
		 * @param 1st Message format
		 * @param 2nd...n Arguments for the message
		 * @return Nothing
		 */
		static void err(const wxChar *, ...);

		/**
		 * Debugs a warning message.
		 *
		 * The message is send to the log-viewer and to syslog.
		 *
		 * @param 1st Message format
		 * @param 2nd...n Arguments for the message
		 * @return Nothing
		 */
		static void warn(const wxChar *, ...);

		/**
		 * Debugs an information message.
		 *
		 * The message is send to the log-viewer.
		 *
		 * @param 1st Message format
		 * @param 2nd...n Arguments for the message
		 * @return Nothing
		 */
		static void info(const wxChar *, ...);

		/**
		 * Debugs messages exchanged between daemon and client.
		 *
		 * @param 1st Message format
		 * @param 2nd...n Arguments for the message
		 * @return Nothing
		 */
		static void chat(const wxChar *, ...);

		/**
		 * Debugs an arbitrary trace-message.
		 *
		 * @param 1st Message format
		 * @param 2nd...n Arguments for the message
		 * @return Nothing
		 */
		static void trace(const wxChar *, ...);

	private:
		Debug(void) {}
		Debug(const Debug &) {}
		~Debug(void) {}

		static Level level_;	/**< Level of Debug ouput */
};

#endif	/* _DEBUG_H_ */
