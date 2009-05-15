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

#ifndef _SINGLETON_H_
#define _SINGLETON_H_

/**
 * Design pattern: Singleton.
 *
 * Using this pattern ensures the existence of only one instance of
 * an object. Use this to avoid global variables and please ensure
 * the correct usage of this pattern by implementing these items:
 *   o The constructor of a derived class must call the singleton
 *	constructor to ensure a proper setup.
 *
 *   o You have to set the singleton class as friend of the derived class.
 *
 *   o You have to include the source file Singleton.cpp in your source file.
 *
 *   o Please see the following examples:
 * @code
 *	Test.h:
 *
 *	#include "Singleton.h"
 *
 *	class Test : public Singleton<Test>
 *	{
 *		public:
 *			~Test(void);
 *			static Test* instance(void);
 *			void xyz(void);
 *
 *		protected:
 *			Test(void);
 *
 *		friend class Singleton<Test>;
 *	};
 * @endcode
 * @code
 *	Test.cpp:
 *
 *	#include "Singeton.cpp"
 *
 *	Test *
 *	Test::instance(void) {
 *		return Singleton<Test>::instance();
 *	}
 *
 *	Test::Test(void) : Singleton<Test>() {
 *		...
 *	}
 * @endcode
 *
 *   o For usage of such a derived class just type:
 *	Test::instance()->xyz();
 */
template <typename T>
class Singleton
{
	public:
		/**
		 * Destructor of a singleton.
		 * This will destroy the object of this singleton. Use with
		 * care!
		 * @param None
		 * @return Nothing.
		 */
		virtual ~Singleton(void);

		/**
		 * Get the object of this singleton.
		 * This will return the object. If it does not already
		 * exist, it's been created.
		 * @param None
		 * @return A pointer to the object.
		 */
		static T* instance(void);

	protected:
		/**
		 * Constructor of a singleton.
		 * This is protected so it can't be called "accidentally".
		 * @param None
		 * @return Nothing.
		 */
		Singleton(void);

	private:
		static T* instance_;	/**< Keep the related object here. */
};

#endif	/* _SINGLETON_H_ */
