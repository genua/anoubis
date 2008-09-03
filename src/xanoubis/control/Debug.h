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

#include <stdio.h>

#include <wx/string.h>
#include "Singleton.cpp"

#define DEBUG_ALERT 1
#define DEBUG_LOG 2
#define DEBUG_STATUS 3

/**
 * Debug Messages
 *
 * The Debug level controlls the amount of output to the console.
 * The lower the level less output will be given to the console.
 * The print function gets an level which represent the priority level
 * of the Debug Message.
 */
class Debug : public Singleton<Debug>
{
	friend class Singleton<Debug>;
	public:
		/**
		 * Deconstrutor of the Debug Messages
		 * Nothing happens till, now.
		 * @param None.
		 * @return Nothing.
		 */
		~Debug(void);

		/**
		 * Instance for singelton
		 * @param None.
		 * @return self.
		 */
		static Debug* instance(void);

		/**
		 * Print function
		 * Prints the debug message to the console
		 * @param 1st Message for debugging
		 * @param 2nd Level of the message
		 * @return Nothing.
		 */
		void	log(wxString, int);

		/**
		 * Set Debug level
		 * Set the level of debug output to the console
		 * the higher the level, the more output should be
		 * created
		 * @param 1st level of debug output
		 * @return Nothing.
		 */
		void	setLevel(int);

	protected:
		/**
		 * Constructor of the Debug Messages
		 * This will set the level of debug output to the console.
		 * @param None.
		 * @reutrn Nothing;
		 */
		Debug(void);

	private:
		int	level_;		/**< Level of Debug ouput */

};

#endif	/* _DEBUG_H_ */
