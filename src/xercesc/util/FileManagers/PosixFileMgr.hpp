/*
 * Copyright 1999-2000,2004-2005 The Apache Software Foundation.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * $Id$
 */

#ifndef POSIXFILEMGR_HPP
#define POSIXFILEMGR_HPP

#include <xercesc/util/XMLFileMgr.hpp>

XERCES_CPP_NAMESPACE_BEGIN

//	Concrete implementation of XMLFileMgr, implementing
//	file access on posix compatible systems.
class PosixFileMgr : public XMLFileMgr
{
    public:
        PosixFileMgr();
        virtual ~PosixFileMgr();

		// File access
        virtual FileHandle	open(const XMLCh* path, bool toWrite, MemoryManager* const manager);
        virtual FileHandle	open(const char* path, bool toWrite, MemoryManager* const manager);
        virtual FileHandle	openStdIn(MemoryManager* const manager);
        
        virtual void		close(FileHandle f, MemoryManager* const manager);
        virtual void		reset(FileHandle f, MemoryManager* const manager);

        virtual XMLFilePos	curPos(FileHandle f, MemoryManager* const manager);
        virtual XMLFilePos	size(FileHandle f, MemoryManager* const manager);

        virtual size_t		read(FileHandle f, size_t byteCount, XMLByte* buffer, MemoryManager* const manager);
        virtual void		write(FileHandle f, size_t byteCount, const XMLByte* buffer, MemoryManager* const manager);
        
        // Ancillary path handling routines
        virtual XMLCh*		getFullPath(const XMLCh* const srcPath, MemoryManager* const manager);
        virtual XMLCh*		getCurrentDirectory(MemoryManager* const manager);
        virtual bool		isRelative(const XMLCh* const toCheck, MemoryManager* const manager);
};

XERCES_CPP_NAMESPACE_END

#endif

