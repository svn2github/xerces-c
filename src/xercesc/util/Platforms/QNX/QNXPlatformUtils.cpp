/*
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2003 The Apache Software Foundation.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Xerces" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact apache\@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation, and was
 * originally based on software copyright (c) 1999, International
 * Business Machines, Inc., http://www.ibm.com .  For more information
 * on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */

/*
 * $Id$
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <atomic.h>
#include <pthread.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/RuntimeException.hpp>
#include <xercesc/util/XMLExceptMsgs.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLUni.hpp>


//
//  These control which transcoding service is used by the QNX version.
//
#if defined (XML_USE_ICU_TRANSCODER)
    #include <xercesc/util/Transcoders/ICU/ICUTransService.hpp>
#elif defined (XML_USE_GNU_TRANSCODER)
    #include <xercesc/util/Transcoders/IconvGNU/IconvGNUTransService.hpp>
#else
    #error A transcoding service must be chosen
#endif

//
//  These control which message loading service is used by the QNX version.
//
#if defined(XML_USE_ICU_MESSAGELOADER)
    #include <xercesc/util/MsgLoaders/ICU/ICUMsgLoader.hpp>
#else
    #include <xercesc/util/MsgLoaders/InMemory/InMemMsgLoader.hpp>
#endif

//
//  These control which network access service is used by the QNX version.
//
#if defined (XML_USE_NETACCESSOR_LIBWWW)
    #include <xercesc/util/NetAccessors/libWWW/LibWWWNetAccessor.hpp>
#else
    #include <xercesc/util/NetAccessors/Socket/SocketNetAccessor.hpp>
#endif

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  XMLPlatformUtils: The panic method
// ---------------------------------------------------------------------------
void XMLPlatformUtils::panic(const PanicHandler::PanicReasons reason)
{
    fgUserPanicHandler? fgUserPanicHandler->panic(reason) : fgDefaultPanicHandler->panic(reason);	
}



// ---------------------------------------------------------------------------
//  XMLPlatformUtils: File Methods
// ---------------------------------------------------------------------------

unsigned int XMLPlatformUtils::curFilePos(FileHandle theFile)
{
    int curPos = ftell(theFile);
    if (curPos == -1) {
        ThrowXML(XMLPlatformUtilsException, XMLExcepts::File_CouldNotGetSize);
    }
    return (unsigned int)curPos;
}

void XMLPlatformUtils::closeFile(FileHandle theFile)
{
    if (fclose(theFile)) {
        ThrowXML(XMLPlatformUtilsException, XMLExcepts::File_CouldNotCloseFile);
    }
}

unsigned int XMLPlatformUtils::fileSize(FileHandle theFile)
{
    struct stat sbuf;
    if( fstat( fileno(theFile), &sbuf ) ) {
        ThrowXML(XMLPlatformUtilsException, XMLExcepts::File_CouldNotGetSize);
    }
    return sbuf.st_size;
}

FileHandle XMLPlatformUtils::openFile(const char* const fileName)
{
    return fopen( fileName, "rb" );
}

FileHandle XMLPlatformUtils::openFile(const XMLCh* const fileName)
{
    const char* tmpFileName = XMLString::transcode(fileName);
    ArrayJanitor<char> janText((char*)tmpFileName);
    return openFile( tmpFileName );
}

FileHandle XMLPlatformUtils::openFileToWrite(const char* const fileName)
{
    return fopen( fileName, "wb" );
}

FileHandle XMLPlatformUtils::openFileToWrite(const XMLCh* const fileName)
{
    const char* tmpFileName = XMLString::transcode(fileName);
    ArrayJanitor<char> janText((char*)tmpFileName);
    return openFileToWrite(tmpFileName);
}

FileHandle XMLPlatformUtils::openStdInHandle()
{
    return fdopen( dup(STDIN_FILENO), "rb" );
}


unsigned int
XMLPlatformUtils::readFileBuffer(       FileHandle      theFile
                                , const unsigned int    toRead
                                ,       XMLByte* const  toFill)
{
    unsigned long bytesRead = 0;
    bytesRead = fread( toFill, 1, toRead, theFile );
    if (ferror(theFile)) {
        ThrowXML(XMLPlatformUtilsException, XMLExcepts::File_CouldNotReadFromFile);
    }
    return (unsigned int)bytesRead;
}


void
XMLPlatformUtils::writeBufferToFile( FileHandle     const  theFile
                                   , long                  toWrite
                                   , const XMLByte* const  toFlush)
{
    unsigned long bytesWritten = 0;
    bytesWritten = fwrite( toFlush, 1, toWrite, theFile );
    if( bytesWritten != toWrite ) {
        ThrowXML(XMLPlatformUtilsException, XMLExcepts::File_CouldNotWriteToFile);
    }
    return;
}

void XMLPlatformUtils::resetFile(FileHandle theFile)
{
    if (fseek(theFile, 0, SEEK_SET)) {
        ThrowXML(XMLPlatformUtilsException, XMLExcepts::File_CouldNotResetFile);
    }
}


// ---------------------------------------------------------------------------
//  XMLPlatformUtils: File system methods
// ---------------------------------------------------------------------------
XMLCh* XMLPlatformUtils::getFullPath(const XMLCh* const srcPath)
{
    //
    //  NOTE: THe path provided has always already been opened successfully,
    //  so we know that its not some pathological freaky path. It comes in
    //  in native format, and goes out as Unicode always
    //
    char* newSrc = XMLString::transcode(srcPath);
    ArrayJanitor<char> janText(newSrc);
    char absPath[PATH_MAX];

    char* retPath = realpath(newSrc, &absPath[0]);

    if (!retPath) {
        ThrowXML(XMLPlatformUtilsException, XMLExcepts::File_CouldNotGetBasePathName);
    }

    return XMLString::transcode(absPath);
}


bool XMLPlatformUtils::isRelative(const XMLCh* const toCheck)
{
    if (!toCheck[0] || toCheck[0] == XMLCh('/'))
        return false;
    return true;
}

XMLCh* XMLPlatformUtils::getCurrentDirectory()
{
    char  *tempDir = getcwd(NULL, PATH_MAX+1);
    XMLCh *curDir = tempDir ? XMLString::transcode(tempDir) : 0;
    free(tempDir);
    return curDir;
}

inline bool XMLPlatformUtils::isAnySlash(XMLCh c) 
{
    return ( chBackSlash == c || chForwardSlash == c);
}

// ---------------------------------------------------------------------------
//  XMLPlatformUtils: Timing Methods
// ---------------------------------------------------------------------------
unsigned long XMLPlatformUtils::getCurrentMillis()
{
    timeb aTime;
    ftime(&aTime);
    return (unsigned long)(aTime.time*1000 + aTime.millitm);
}



// ---------------------------------------------------------------------------
//  Mutex methods
// ---------------------------------------------------------------------------

void* XMLPlatformUtils::makeMutex()
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init( &attr );
    pthread_mutexattr_setrecursive( &attr, PTHREAD_RECURSIVE_ENABLE );
    pthread_mutex_t *mutex = new pthread_mutex_t;
    if( pthread_mutex_init( mutex, &attr ) != EOK ) {
        delete mutex;
        ThrowXML(XMLPlatformUtilsException, XMLExcepts::Mutex_CouldNotCreate);
    }
    pthread_mutexattr_destroy( &attr );
    return mutex;
}

void XMLPlatformUtils::closeMutex(void* const mtxHandle)
{
    if( mtxHandle == NULL || pthread_mutex_destroy( (pthread_mutex_t *)mtxHandle ) != EOK ) {
        ThrowXML(XMLPlatformUtilsException, XMLExcepts::Mutex_CouldNotClose);
    }
    delete mtxHandle;
}


void XMLPlatformUtils::lockMutex(void* const mtxHandle)
{
    if( mtxHandle == NULL || pthread_mutex_lock( (pthread_mutex_t *)mtxHandle ) != EOK ) {
        ThrowXML(XMLPlatformUtilsException, XMLExcepts::Mutex_CouldNotLock);
    }
}


void XMLPlatformUtils::unlockMutex(void* const mtxHandle)
{
    if( mtxHandle == NULL || pthread_mutex_unlock( (pthread_mutex_t *)mtxHandle ) != EOK ) {
        ThrowXML(XMLPlatformUtilsException, XMLExcepts::Mutex_CouldNotLock);
    }
}



// ---------------------------------------------------------------------------
//  Miscellaneous synchronization methods
// ---------------------------------------------------------------------------
void*
XMLPlatformUtils::compareAndSwap(       void**      toFill
                                , const void* const newValue
                                , const void* const toCompare)
{
    //
    // Undocumented function pulled in by pthread.h.  Uses CPU specific
    // inline assembly routines for doing an atomic compare and exchange
    // operation.
    //
    return (void *)_smp_cmpxchg( (volatile unsigned *)toFill, (unsigned)toCompare, (unsigned)newValue );
}


// ---------------------------------------------------------------------------
//  Atomic increment and decrement methods
// ---------------------------------------------------------------------------
int XMLPlatformUtils::atomicIncrement(int &location)
{
    atomic_add( &(volatile unsigned &)location, 1 );
    return location;
}


int XMLPlatformUtils::atomicDecrement(int &location)
{
    atomic_sub( &(volatile unsigned &)location, 1 );
    return location;
}



// ---------------------------------------------------------------------------
//  XMLPlatformUtils: Private Static Methods
// ---------------------------------------------------------------------------

//
//  This method is called by the platform independent part of this class
//  during initialization. We have to create the type of net accessor that
//  we want to use. If none, then just return zero.
//
XMLNetAccessor* XMLPlatformUtils::makeNetAccessor()
{
#if defined (XML_USE_NETACCESSOR_LIBWWW)
    return new LibWWWNetAccessor();
#else 
    return new SocketNetAccessor();
#endif
}


//
//  This method is called by the platform independent part of this class
//  when client code asks to have one of the supported message sets loaded.
//  In our case, we use the ICU based message loader mechanism.
//
XMLMsgLoader* XMLPlatformUtils::loadAMsgSet(const XMLCh* const msgDomain)
{
#if defined (XML_USE_ICU_MESSAGELOADER)
    return new ICUMsgLoader(msgDomain);
#else
    return new InMemMsgLoader(msgDomain);
#endif
}


//
//  This method is called very early in the bootstrapping process. This guy
//  must create a transcoding service and return it. It cannot use any string
//  methods, any transcoding services, throw any exceptions, etc... It just
//  makes a transcoding service and returns it, or returns zero on failure.
//
XMLTransService* XMLPlatformUtils::makeTransService()
{
#if defined (XML_USE_ICU_TRANSCODER)
    return new ICUTransService;
#elif defined (XML_USE_GNU_TRANSCODER)
    return new IconvGNUTransService;
#else
    #error You must provide a transcoding service implementation
#endif
}


void XMLPlatformUtils::platformInit()
{
}


void XMLPlatformUtils::platformTerm()
{
}

#include <xercesc/util/LogicalPath.c>

XERCES_CPP_NAMESPACE_END
