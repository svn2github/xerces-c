/*
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 1999-2000 The Apache Software Foundation.  All rights
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
 * $Log$
 * Revision 1.2  2002/05/01 21:13:32  tng
 * [Bug 7265] UnixWare port broken in platformTerm() since rev 1.10.
 *
 * Revision 1.1.1.1  2002/02/01 22:22:26  peiyongz
 * sane_include
 *
 * Revision 1.11  2001/10/25 15:20:31  tng
 * Need to guard with NO_APP_THREADS when destroying the mutex.
 *
 * Revision 1.10  2001/10/23 23:13:09  peiyongz
 * [Bug#880] patch to PlatformUtils:init()/term() and related. from Mark Weaver
 *
 * Revision 1.9  2001/02/09 14:40:02  tng
 * Update support for SCO UnixWare 7 (gcc).  Tested under UnixWare 7.1.1 with gcc version 2.95.2 19991024 (release) with gmake 3.79.1.  Updated by Martin Kalen.
 *
 * Revision 1.8  2000/07/25 22:30:54  aruna1
 * Char definitions in XMLUni moved to XMLUniDefs
 *
 * Revision 1.7  2000/03/24 00:13:34  aruna1
 * Platform initialization taken care for both threaded and non-threaded environment
 *
 * Revision 1.6  2000/03/02 21:10:39  abagchi
 * Added empty function platformTerm()
 *
 * Revision 1.5  2000/03/02 20:42:42  abagchi
 * Fixed typo in XMLExcepts
 *
 * Revision 1.4  2000/03/02 19:55:33  roddey
 * This checkin includes many changes done while waiting for the
 * 1.1.0 code to be finished. I can't list them all here, but a list is
 * available elsewhere.
 *
 * Revision 1.3  2000/02/06 07:48:31  rahulj
 * Year 2K copyright swat.
 *
 * Revision 1.2  2000/01/25 22:33:13  aruna1
 * Updated panic information
 *
 * Revision 1.1  1999/12/17 22:24:03  rahulj
 * Added missing UnixWare files to the repository.
 *
 * Created by Ron Record (rr@sco.com) based on SolarisPlatformUtils.cpp
 * 13-Nov-1999
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------

#if !defined (APP_NO_THREADS)
#include    <pthread.h>
#endif // APP_NO_THREADS


#include    <unistd.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <errno.h>
#include    <libgen.h>
#include    <sys/timeb.h>
#include    <limits.h>
#include    <xercesc/util/PlatformUtils.hpp>
#include    <xercesc/util/RuntimeException.hpp>
#include    <xercesc/util/Janitor.hpp>
#include    <xercesc/util/Mutexes.hpp>
#include    <xercesc/util/XMLString.hpp>
#include    <xercesc/util/XMLUniDefs.hpp>
#include    <xercesc/util/XMLUni.hpp>

#if defined (XML_USE_ICU_TRANSCODER)
    #include <xercesc/util/Transcoders/ICU/ICUTransService.hpp>
#elif defined (XML_USE_NATIVE_TRANSCODER)
    #include <xercesc/util/Transcoders/Iconv/IconvTransService.hpp>
#else
    #error Transcoding service unknown by UnixWarePlatformUtils.cpp
#endif


#if defined(XML_USE_ICU_MESSAGELOADER)
    #include <xercesc/util/MsgLoaders/ICU/ICUMsgLoader.hpp>
#elif defined (XML_USE_ICONV_MESSAGELOADER)
    #include <xercesc/util/MsgLoaders/MsgCatalog/MsgCatalogLoader.hpp>
#elif defined (XML_USE_INMEM_MESSAGELOADER)
    #include <xercesc/util/MsgLoaders/InMemory/InMemMsgLoader.hpp>
#else
    #error Message loading service unknown by UnixWarePlatformUtils.cpp
#endif


#if defined (XML_USE_NETACCESSOR_LIBWWW)
    #include <xercesc/util/NetAccessors/libWWW/LibWWWNetAccessor.hpp>
#elif defined (XML_USE_NETACCESSOR_SOCKET)
    #include <xercesc/util/NetAccessors/Socket/SocketNetAccessor.hpp>
#endif


// ---------------------------------------------------------------------------
//  Local Methods
// ---------------------------------------------------------------------------
static void WriteCharStr( FILE* stream, const char* const toWrite)
{
    if (fputs(toWrite, stream) == EOF)
    {
        ThrowXML(XMLPlatformUtilsException,
                 XMLExcepts::Strm_StdErrWriteFailure);
    }
}

static void WriteUStrStdErr( const XMLCh* const toWrite)
{
    char* tmpVal = XMLString::transcode(toWrite);
    ArrayJanitor<char> janText(tmpVal);
    if (fputs(tmpVal, stderr) == EOF)
    {
        ThrowXML(XMLPlatformUtilsException,
                 XMLExcepts::Strm_StdErrWriteFailure);
    }
}

static void WriteUStrStdOut( const XMLCh* const toWrite)
{
    char* tmpVal = XMLString::transcode(toWrite);
    ArrayJanitor<char> janText(tmpVal);
    if (fputs(tmpVal, stdout) == EOF)
    {
        ThrowXML(XMLPlatformUtilsException,
                 XMLExcepts::Strm_StdOutWriteFailure);
    }
}

XMLNetAccessor* XMLPlatformUtils::makeNetAccessor()
{
#if defined (XML_USE_NETACCESSOR_LIBWWW)
    return new LibWWWNetAccessor();
#elif defined (XML_USE_NETACCESSOR_SOCKET)
    return new SocketNetAccessor();
#else
    return 0;
#endif
}



// ---------------------------------------------------------------------------
//  XMLPlatformUtils: Private Static Methods
// ---------------------------------------------------------------------------

//
//  This method is called by the platform independent part of this class
//  when client code asks to have one of the supported message sets loaded.
//

XMLMsgLoader* XMLPlatformUtils::loadAMsgSet(const XMLCh* const msgDomain)
{
    XMLMsgLoader* retVal;
    try
    {
#if defined (XML_USE_ICU_MESSAGELOADER)
        retVal = new ICUMsgLoader(msgDomain);
#elif defined (XML_USE_ICONV_MESSAGELOADER)
        retVal = new MsgCatalogLoader(msgDomain);
#else
        retVal = new InMemMsgLoader(msgDomain);
#endif
    }
    catch(...)
    {
        panic(XMLPlatformUtils::Panic_CantLoadMsgDomain);
    }
    return retVal;
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

#elif defined (XML_USE_ICONV_TRANSCODER)

    return new IconvTransService;

#else // Use Native transcoding service

    return new IconvTransService;

#endif
}


// ---------------------------------------------------------------------------
//  XMLPlatformUtils: The panic method
// ---------------------------------------------------------------------------
void XMLPlatformUtils::panic(const PanicReasons reason)
{
	 const char* reasonStr = "Unknown reason";
    if (reason == Panic_NoTransService)
        reasonStr = "Could not load a transcoding service";
    else if (reason == Panic_NoDefTranscoder)
        reasonStr = "Could not load a local code page transcoder";
    else if (reason == Panic_CantFindLib)
        reasonStr = "Could not find the xerces-c DLL";
    else if (reason == Panic_UnknownMsgDomain)
        reasonStr = "Unknown message domain";
    else if (reason == Panic_CantLoadMsgDomain)
        reasonStr = "Cannot load message domain";
    else if (reason == Panic_SynchronizationErr)
        reasonStr = "Cannot synchronize system or mutex";
    else if (reason == Panic_SystemInit)
        reasonStr = "Cannot initialize the system or mutex";

    fprintf(stderr, "%s\n", reasonStr);

    exit(-1);
}

// ---------------------------------------------------------------------------
//  XMLPlatformUtils: File Methods
// ---------------------------------------------------------------------------
unsigned int XMLPlatformUtils::curFilePos(FileHandle theFile)
{
    // Get the current position
    int curPos = ftell((FILE*) theFile);
    if (curPos == -1)
        ThrowXML(XMLPlatformUtilsException,
                 XMLExcepts::File_CouldNotGetSize);

    return (unsigned int)curPos;
}

void XMLPlatformUtils::closeFile(FileHandle theFile)
{
    if (fclose((FILE*) theFile))
        ThrowXML(XMLPlatformUtilsException,
                 XMLExcepts::File_CouldNotCloseFile);
}

unsigned int XMLPlatformUtils::fileSize(FileHandle theFile)
{
    // Get the current position
    long  int curPos = ftell((FILE*) theFile);
    if (curPos == -1)
        ThrowXML(XMLPlatformUtilsException,
                 XMLExcepts::File_CouldNotGetCurPos);

    // Seek to the end and save that value for return
	if (fseek((FILE*) theFile, 0, SEEK_END))
        ThrowXML(XMLPlatformUtilsException,
                 XMLExcepts::File_CouldNotSeekToEnd);

    long int retVal = ftell((FILE*) theFile);
    if (retVal == -1)
        ThrowXML(XMLPlatformUtilsException,
                 XMLExcepts::File_CouldNotSeekToEnd);

    // And put the pointer back
    if (fseek((FILE*) theFile, curPos, SEEK_SET))
        ThrowXML(XMLPlatformUtilsException,
                 XMLExcepts::File_CouldNotSeekToPos);

    return (unsigned int)retVal;
}

FileHandle XMLPlatformUtils::openFile(const XMLCh* const fileName)
{
    const char* tmpFileName = XMLString::transcode(fileName);
    ArrayJanitor<char> janText((char*)tmpFileName);
    FileHandle retVal = (FILE*)fopen( tmpFileName , "rb" );

    if (retVal == NULL)
        return 0;
    return retVal;
}

FileHandle XMLPlatformUtils::openFile(const char* const fileName)
{
    FileHandle retVal = (FILE*)fopen( fileName , "rb" );

    if (retVal == NULL)
        return 0;
    return retVal;
}

FileHandle XMLPlatformUtils::openStdInHandle()
{
	return (FileHandle)fdopen(dup(0), "rb");
}

unsigned int
XMLPlatformUtils::readFileBuffer( FileHandle          theFile
                               , const unsigned int      toRead
                               , XMLByte* const          toFill)
{
    size_t noOfItemsRead = fread((void*) toFill, 1, toRead, (FILE*) theFile);

    if(ferror((FILE*) theFile))
    {
        ThrowXML(XMLPlatformUtilsException,
                 XMLExcepts::File_CouldNotReadFromFile);
    }

    return (unsigned int) noOfItemsRead;
}


void XMLPlatformUtils::resetFile(FileHandle theFile)
{
    // Seek to the start of the file
    if (fseek((FILE*) theFile, 0, SEEK_SET))
        ThrowXML(XMLPlatformUtilsException,
                 XMLExcepts::File_CouldNotResetFile);
}


XMLCh* XMLPlatformUtils::weavePaths(const   XMLCh* const    basePath
                                    , const XMLCh* const    relativePath)

{
    // Create a buffer as large as both parts and empty it
    XMLCh* tmpBuf = new XMLCh[XMLString::stringLen(basePath)
                              + XMLString::stringLen(relativePath)
                              + 2];
    *tmpBuf = 0;

    //
    //  If we have no base path, then just take the relative path as
    //  is.
    //
    if (!basePath)
    {
        XMLString::copyString(tmpBuf, relativePath);
        return tmpBuf;
    }

    if (!*basePath)
    {
        XMLString::copyString(tmpBuf, relativePath);
        return tmpBuf;
    }

    const XMLCh* basePtr = basePath + (XMLString::stringLen(basePath) - 1);
    if ((*basePtr != chForwardSlash)
    &&  (*basePtr != chBackSlash))
    {
        while ((basePtr >= basePath)
        &&     ((*basePtr != chForwardSlash) && (*basePtr != chBackSlash)))
        {
            basePtr--;
        }
    }

    // There is no relevant base path, so just take the relative part
    if (basePtr < basePath)
    {
        XMLString::copyString(tmpBuf, relativePath);
        return tmpBuf;
    }

    // After this, make sure the buffer gets handled if we exit early
    ArrayJanitor<XMLCh> janBuf(tmpBuf);

    //
    //  We have some path part, so we need to check to see if we ahve to
    //  weave any of the parts together.
    //
    const XMLCh* pathPtr = relativePath;
    while (true)
    {
        // If it does not start with some period, then we are done
        if (*pathPtr != chPeriod)
            break;

        unsigned int periodCount = 1;
        pathPtr++;
        if (*pathPtr == chPeriod)
        {
            pathPtr++;
            periodCount++;
        }

        // Has to be followed by a \ or / or the null to mean anything
        if ((*pathPtr != chForwardSlash) && (*pathPtr != chBackSlash)
        &&  *pathPtr)
        {
            break;
        }
        if (*pathPtr)
            pathPtr++;

        // If its one period, just eat it, else move backwards in the base
        if (periodCount == 2)
        {
            basePtr--;
            while ((basePtr >= basePath)
            &&     ((*basePtr != chForwardSlash) && (*basePtr != chBackSlash)))
            {
                basePtr--;
            }

            // The base cannot provide enough levels, so its in error/
            if (basePtr < basePath)
                ThrowXML(XMLPlatformUtilsException,
                         XMLExcepts::File_BasePathUnderflow);
        }
    }

    // Copy the base part up to the base pointer
    XMLCh* bufPtr = tmpBuf;
    const XMLCh* tmpPtr = basePath;
    while (tmpPtr <= basePtr)
        *bufPtr++ = *tmpPtr++;

    // And then copy on the rest of our path
    XMLString::copyString(bufPtr, pathPtr);

    // Orphan the buffer and return it
    janBuf.orphan();
    return tmpBuf;
}

XMLCh* XMLPlatformUtils::getFullPath(const XMLCh* const srcPath)
{
    //
    //  NOTE: The path provided has always already been opened successfully,
    //  so we know that its not some pathological freaky path. It comes in
    //  in native format, and goes out as Unicode always
    //
    char* newSrc = XMLString::transcode(srcPath);
    ArrayJanitor<char> janText(newSrc);

    // Use a local buffer that is big enough for the largest legal path
    char *absPath = new char[PATH_MAX];
    ArrayJanitor<char> janText2(absPath);
    //get the absolute path
    char* retPath = realpath(newSrc, absPath);

    if (!retPath)
    {
        ThrowXML(XMLPlatformUtilsException,
                 XMLExcepts::File_CouldNotGetBasePathName);
    }
    return XMLString::transcode(absPath);
}

bool XMLPlatformUtils::isRelative(const XMLCh* const toCheck)
{
    // Check for pathological case of empty path
    if (!toCheck[0])
        return false;

    //
    //  If it starts with a slash, then it cannot be relative. This covers
    //  both something like "\Test\File.xml" and an NT Lan type remote path
    //  that starts with a node like "\\MyNode\Test\File.xml".
    //
    if (toCheck[0] == XMLCh('/'))
        return false;

    // Else assume its a relative path
    return true;
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


// -----------------------------------------------------------------------
//  Mutex methods
// -----------------------------------------------------------------------

#if !defined (APP_NO_THREADS)

// ---------------------------------------------------------------------------
//  XMLPlatformUtils: Platform init method
// ---------------------------------------------------------------------------
static pthread_mutex_t* gAtomicOpMutex =0 ;

void XMLPlatformUtils::platformInit()
{
    //
    // The gAtomicOpMutex mutex needs to be created
    // because compareAndSwap, atomicIncrement and atomicDecrement
    // does not have the atomic system calls for usage
    // Normally, mutexes are created on first use, but there is a
    // circular dependency between compareAndExchange() and
    // mutex creation that must be broken.

    gAtomicOpMutex = new pthread_mutex_t;

    if (pthread_mutex_init(gAtomicOpMutex, NULL))
        panic( XMLPlatformUtils::Panic_SystemInit );
}

class  RecursiveMutex
{
public:
    pthread_mutex_t   mutex;
    int               recursionCount;
    pthread_t         tid;

    RecursiveMutex() {
		if (pthread_mutex_init(&mutex, NULL))
			ThrowXML(XMLPlatformUtilsException, XMLExcepts::Mutex_CouldNotCreate);
		recursionCount = 0;
		tid = 0;
	};

    ~RecursiveMutex() {
		if (pthread_mutex_destroy(&mutex))
			ThrowXML(XMLPlatformUtilsException, XMLExcepts::Mutex_CouldNotDestroy);
	};

	void lock()      {
		if (pthread_equal(tid, pthread_self()))
		{
			recursionCount++;
			return;
		}
		if (pthread_mutex_lock(&mutex) != 0)
			ThrowXML(XMLPlatformUtilsException, XMLExcepts::Mutex_CouldNotLock);
		tid = pthread_self();
		recursionCount = 1;
	};


	void unlock()    {
		if (--recursionCount > 0)
			return;

		if (pthread_mutex_unlock(&mutex) != 0)
			ThrowXML(XMLPlatformUtilsException, XMLExcepts::Mutex_CouldNotUnlock);
		tid = 0;
	};
};

void* XMLPlatformUtils::makeMutex()
{
    return new RecursiveMutex;
};


void XMLPlatformUtils::closeMutex(void* const mtxHandle)
{
    if (mtxHandle == NULL)
        return;
    RecursiveMutex *rm = (RecursiveMutex *)mtxHandle;
    delete rm;
};


void XMLPlatformUtils::lockMutex(void* const mtxHandle)
{
    if (mtxHandle == NULL)
        return;
    RecursiveMutex *rm = (RecursiveMutex *)mtxHandle;
    rm->lock();
}

void XMLPlatformUtils::unlockMutex(void* const mtxHandle)
{
    if (mtxHandle == NULL)
        return;
    RecursiveMutex *rm = (RecursiveMutex *)mtxHandle;
    rm->unlock();
}

// -----------------------------------------------------------------------
//  Miscellaneous synchronization methods
// -----------------------------------------------------------------------
//atomic system calls in UnixWare is only restricted to kernel libraries
//So, to make operations thread safe we implement static mutex and lock
//the atomic operations. It makes the process slow but what's the alternative!

void* XMLPlatformUtils::compareAndSwap ( void**      toFill ,
                    const void* const newValue ,
                    const void* const toCompare)
{
    if (pthread_mutex_lock( gAtomicOpMutex))
        panic(XMLPlatformUtils::Panic_SynchronizationErr);

    void *retVal = *toFill;
    if (*toFill == toCompare)
              *toFill = (void *)newValue;

    if (pthread_mutex_unlock( gAtomicOpMutex))
        panic(XMLPlatformUtils::Panic_SynchronizationErr);


    return retVal;
}

int XMLPlatformUtils::atomicIncrement(int &location)
{
    if (pthread_mutex_lock( gAtomicOpMutex))
        panic(XMLPlatformUtils::Panic_SynchronizationErr);

    int tmp = ++location;

    if (pthread_mutex_unlock( gAtomicOpMutex))
        panic(XMLPlatformUtils::Panic_SynchronizationErr);

    return tmp;
}

int XMLPlatformUtils::atomicDecrement(int &location)
{
    if (pthread_mutex_lock( gAtomicOpMutex))
        panic(XMLPlatformUtils::Panic_SynchronizationErr);

    int tmp = --location;

    if (pthread_mutex_unlock( gAtomicOpMutex))
        panic(XMLPlatformUtils::Panic_SynchronizationErr);

    return tmp;
}

#else // #if !defined (APP_NO_THREADS)

void XMLPlatformUtils::platformInit()
{
}

void* XMLPlatformUtils::makeMutex()
{
	return 0;
}

void XMLPlatformUtils::closeMutex(void* const mtxHandle)
{
}

void XMLPlatformUtils::lockMutex(void* const mtxHandle)
{
}

void XMLPlatformUtils::unlockMutex(void* const mtxHandle)
{
}

void* XMLPlatformUtils::compareAndSwap ( void**      toFill,
                                   const void* const newValue,
                                   const void* const toCompare)
{
    void *retVal = *toFill;
    if (*toFill == toCompare)
       *toFill = (void *)newValue;
    return retVal;
}

int XMLPlatformUtils::atomicIncrement(int &location)
{
    return ++location;
}

int XMLPlatformUtils::atomicDecrement(int &location)
{
    return --location;
}

#endif // APP_NO_THREADS



void XMLPlatformUtils::platformTerm()
{
#if !defined (APP_NO_THREADS)
    // delete the mutex we created
	pthread_mutex_destroy(gAtomicOpMutex);
    delete gAtomicOpMutex;
	gAtomicOpMutex = 0;
#endif
}

