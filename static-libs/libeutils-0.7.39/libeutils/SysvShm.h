/*

    libeutils - http://www.excito.com/

    SysvShm.h - this file is part of libeutils.

    Copyright (C) 2009 Tor Krill <tor@excito.com>

    libeutils is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    libeutils is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    version 2 along with libeutils; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.

    $Id$
*/
#ifndef SYSVSHM_H_
#define SYSVSHM_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "SysvIPC.h"

namespace EUtils {

class SysvShm: public EUtils::SysvIPC {
private:
	int shmid;
	void* data;
public:
	SysvShm(size_t size, const string& path, int token=123);
	void* operator*();
	void* Value();
	void Remove();
	virtual ~SysvShm();
};

}

#endif /* SYSVSHM_H_ */
