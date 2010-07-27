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

#ifndef _SINGLETON_CPP_
#define _SINGLETON_CPP_

#include "Singleton.h"

template <typename T> T* Singleton<T>::instance_ = 0;

template <typename T>
Singleton<T>::Singleton(void)
{
	instance_ = 0;
}

template <typename T>
Singleton<T>::~Singleton(void)
{
	/*
	 * Because the instance_ will point to ourself, we can not call
	 * delete here (deadlock). We reach this point by the destructor
	 * of a derived class.
	 */
	instance_ = 0;
}

template <typename T> T*
Singleton<T>::instance(void)
{
	if (instance_ == 0) {
		instance_ = new T();
	}

	return (instance_);
}

template <typename T> T*
Singleton<T>::existingInstance(void)
{
	return instance_;
}

template <typename T> bool
Singleton<T>::hasInstance(void)
{
	return (instance_ == 0 ? false : true);
}

#endif /* _SINGLETON_CPP_ */
