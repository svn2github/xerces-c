/*
 * The Apache Software License, Version 1.1
 * 
 * Copyright (c) 1999 The Apache Software Foundation.  All rights 
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

/**
 * $Log$
 * Revision 1.1  1999/11/09 01:07:01  twl
 * Initial revision
 *
 * Revision 1.6  1999/11/08 20:45:30  rahul
 * Swat for adding in Product name and CVS comment log variable.
 *
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------

#ifndef APP_NO_THREADS
#include    <pthread.h>
#endif

#include    <unistd.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <errno.h>
#include    <libgen.h>
#include    <sys/timeb.h>
#include    <string.h>
#include    <util/PlatformUtils.hpp>
#include    <util/RuntimeException.hpp>
#include    <util/Janitor.hpp>
#include    <util/Mutexes.hpp>
#include    <util/XMLString.hpp>
#include    <util/XMLUni.hpp>

#if defined (XML_USE_ICU_TRANSCODER)
    #include <util/Transcoders/ICU/ICUTransService.hpp>
#else	// use native transcoder
    #include <util/Transcoders/Iconv/IconvTransService.hpp>
#endif

#if defined (XML_USE_ICU_MESSAGELOADER)
    #include <util/MsgLoaders/ICU/ICUMsgLoader.hpp>
#elif defined (XML_USE_ICONV_MESSAGELOADER)
    #include <util/MsgLoaders/MsgCatalog/MsgCatalogLoader.hpp>
#else	// use In-memory message loader
    #include <util/MsgLoaders/InMemory/InMemMsgLoader.hpp>   //hint for the user to include this file.
#endif

// ---------------------------------------------------------------------------
//  Local Methods
// ---------------------------------------------------------------------------

static void WriteCharStr( FILE* stream, const char* const toWrite)
{
    if (fputs(toWrite, stream) == EOF)
    {
        ThrowXML(XMLPlatformUtilsException,
				 XML4CExcepts::Strm_StdErrWriteFailure);
    }
}

static void WriteUStrStdErr( const XMLCh* const toWrite)
{
    char* tmpVal = XMLString::transcode(toWrite);
    ArrayJanitor<char> janText(tmpVal);
    if (fputs(tmpVal, stderr) == EOF)
    {
       ThrowXML(XMLPlatformUtilsException,
				XML4CExcepts::Strm_StdErrWriteFailure);
   }
}

static void WriteUStrStdOut( const XMLCh* const toWrite)
 {
    char* tmpVal = XMLString::transcode(toWrite);
    ArrayJanitor<char> janText(tmpVal);
    if (fputs(tmpVal, stdout) == EOF)
    {
        ThrowXML(XMLPlatformUtilsException,
				 XML4CExcepts::Strm_StdOutWriteFailure);
    }
}

XMLNetAccessor* XMLPlatformUtils::makeNetAccessor()
{
    return 0;
}

// ---------------------------------------------------------------------------
//  XMLPlatformUtils: Platform init method
// ---------------------------------------------------------------------------
static XMLMutex atomicOpsMutex;


void XMLPlatformUtils::platformInit()
{
    //
    // The atomicOps mutex needs to be created early.
    // Normally, mutexes are created on first use, but there is a
    // circular dependency between compareAndExchange() and
    // mutex creation that must be broken.
    atomicOpsMutex.fHandle = XMLPlatformUtils::makeMutex();

    // Here you would also set the fgLibLocation global variable
    // XMLPlatformUtils::fgLibLocation is the variable to be set

    static const char *sharedLibEnvVar = "LD_LIBRARY_PATH";
    static const char * libraryPath = 0;

    char libName[256];

    // Construct the library name from the global variables
    strcpy(libName, XML4C_DLLName);
    strcat(libName, gXML4CVersionStr);
    strcat(libName, ".so");

    char* libEnvVar = getenv(sharedLibEnvVar);
    char* libPath = NULL;

    if (libEnvVar == NULL)
    {
        panic( XMLPlatformUtils::Panic_NoTransService );
    }

    //
    // Its necessary to create a copy because strtok() modifies the
    // string as it returns tokens. We don't want to modify the string
    // returned to by getenv().

    libPath = new char[strlen(libEnvVar) + 1];
    strcpy(libPath, libEnvVar);

    // First do the searching process for the first directory listing
    char*  allPaths = libPath;
    char*  libPathName;

    while ((libPathName = strtok(allPaths, ":")) != NULL)
    {
        FILE*  dummyFptr = 0;
        allPaths = 0;

        char* libfile = new char[strlen(libPathName) + strlen(libName) + 2];
        strcpy(libfile, libPathName);
        strcat(libfile, "/");
        strcat(libfile, libName);

        dummyFptr = (FILE *) fopen(libfile, "rb");
        delete [] libfile;
        if (dummyFptr != NULL)
        {
            fclose(dummyFptr);
            libraryPath = new char[strlen(libPathName)+1];
            strcpy((char *) libraryPath, libPathName);
            break;
        }

    }

    delete libPath;

    XMLPlatformUtils::fgLibLocation = libraryPath;

    if (XMLPlatformUtils::fgLibLocation == NULL)
    {
        panic( XMLPlatformUtils::Panic_NoTransService );
    }
}


// ---------------------------------------------------------------------------
//  XMLPlatformUtils: Private Static Methods
// ---------------------------------------------------------------------------

//
//  This method is called by the platform independent part of this class
//  when client code asks to have one of the supported message sets loaded.
//  In our case, we use the ICU based message loader mechanism.
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
        panic( XMLPlatformUtils::Panic_NoDefTranscoder );
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

#if defined (XML_USE_ICU_TRANSCODER)
{
    //
    //  We need to figure out the path to the Intl converter files.
    //

    static const char * xml4cIntlDirEnvVar = "ICU_DATA";
    static const char * sharedLibEnvVar    = "LD_LIBRARY_PATH";
    static const char * intlPath = 0;

    char* envVal = getenv(xml4cIntlDirEnvVar);
    //check if environment variable is set
    if (envVal != NULL) // We have found an environment variable
    {
        // Store this string in the static member
        unsigned int pathLen = strlen(envVal);
        intlPath = new char[pathLen + 2];

        strcpy((char *) intlPath, envVal);
        if (envVal[pathLen - 1] != '/')
        {
            strcat((char *) intlPath, "/");
        }

        ICUTransService::setICUPath(intlPath);
        if (intlPath != NULL) delete intlPath;

        return new ICUTransService;
    }

    //
    //  If we did not find the environment var, so lets try to go the auto
    //  search route.
    //
    char libName[256];
    strcpy(libName, XML4C_DLLName);
    strcat(libName, gXML4CVersionStr);
    strcat(libName, ".so");

    char* libEnvVar = getenv(sharedLibEnvVar);
    char* libPath = NULL;

    if (libEnvVar == NULL)
    {
        panic( XMLPlatformUtils::Panic_NoTransService );
    }

    //
    // Its necessary to create a copy because strtok() modifies the
    // string as it returns tokens. We don't want to modify the string
    // returned to by getenv().
    //

    libPath = new char[strlen(libEnvVar) + 1];
    strcpy(libPath, libEnvVar);

    // First do the searching process for the first directory listing
    char*  allPaths = libPath;
    char*  libPathName;

    while ((libPathName = strtok(allPaths, ":")) != NULL)
    {
        FILE*  dummyFptr = 0;
        allPaths = 0;

        char* libfile = new char[strlen(libPathName) + strlen(libName) + 2];
        strcpy(libfile, libPathName);
        strcat(libfile, "/");
        strcat(libfile, libName);

        dummyFptr = (FILE *) fopen(libfile, "rb");
        delete [] libfile;
        if (dummyFptr != NULL)
        {
            fclose(dummyFptr);
            intlPath = new char[strlen(libPathName)+ strlen("/icu/data/")+1];
            strcpy((char *) intlPath, libPathName);
            strcat((char *) intlPath, "/icu/data/");
            break;
        }

    } // while

    delete libPath;

    ICUTransService::setICUPath(intlPath);

    if (intlPath == NULL)
    {
        panic( XMLPlatformUtils::Panic_NoTransService );
    }
    if (intlPath != NULL) delete intlPath;

    return new ICUTransService;
}
#elif defined (XML_USE_ICONV_TRANSCODER)
{
    return new IconvTransService;
}
#else
{
    return new IconvTransService;
}
#endif


// ---------------------------------------------------------------------------
//  XMLPlatformUtils: The panic method
// ---------------------------------------------------------------------------
void XMLPlatformUtils::panic(const PanicReasons reason)
{
    //
    //  We just print a message and exit
    //
    printf("The XML4C system could not be initialized.\n");
    printf("The most likely reason for this failure is the inability to find\n");
    printf("the international encoding files. By default, the encoding files\n");
    printf("have the extension .cnv and exist in a directory icu/data relative\n");
    printf("to the XML4C shared library. If you have kept the converter files\n");
    printf("in a different location, you need to set up an environment variable\n");
    printf("called ICU_DATA which directly points to the directory where the\n");
    printf("encoding files are kept.\n");

    exit(-1);
}

// ---------------------------------------------------------------------------
//  XMLPlatformUtils: File Methods
// ---------------------------------------------------------------------------
unsigned int XMLPlatformUtils::curFilePos(FileHandle theFile)
{
    // Get the current position
    int curPos = ftell( (FILE*)theFile);
    if (curPos == -1)
        ThrowXML(XMLPlatformUtilsException, XML4CExcepts::File_CouldNotGetSize);

    return (unsigned int)curPos;
}

void XMLPlatformUtils::closeFile(FileHandle theFile)
{
    if (fclose((FILE*)theFile))
        ThrowXML(XMLPlatformUtilsException, XML4CExcepts::File_CouldNotCloseFile);
}

unsigned int XMLPlatformUtils::fileSize(FileHandle theFile)
{
    // Get the current position
    long  int curPos = ftell((FILE*)theFile);
    if (curPos == -1)
        ThrowXML(XMLPlatformUtilsException, XML4CExcepts::File_CouldNotGetCurPos);

    // Seek to the end and save that value for return
     if (fseek( (FILE*)theFile, 0, SEEK_END) )
        ThrowXML(XMLPlatformUtilsException, XML4CExcepts::File_CouldNotSeekToEnd);

    long int retVal = ftell( (FILE*)theFile);
    if (retVal == -1)
        ThrowXML(XMLPlatformUtilsException, XML4CExcepts::File_CouldNotSeekToEnd);

    // And put the pointer back
    if (fseek( (FILE*)theFile, curPos, SEEK_SET) )
        ThrowXML(XMLPlatformUtilsException, XML4CExcepts::File_CouldNotSeekToPos);

    return (unsigned int)retVal;
}

FileHandle XMLPlatformUtils::openFile(const unsigned short* const fileName)
{
    const char* tmpFileName = XMLString::transcode(fileName);
    ArrayJanitor<char> janText((char*)tmpFileName);
    FileHandle retVal = (FILE*)fopen( tmpFileName , "rb" );

    if (retVal == NULL)
        return 0;
    return retVal;
}

unsigned int
XMLPlatformUtils::readFileBuffer(  FileHandle      theFile
                                , const unsigned int    toRead
                                , XMLByte* const  toFill)
{
    size_t noOfItemsRead = fread( (void*) toFill, 1, toRead, (FILE*)theFile);

    if(ferror((FILE*)theFile))
    {
        ThrowXML(XMLPlatformUtilsException, XML4CExcepts::File_CouldNotReadFromFile);
    }

    return (unsigned int)noOfItemsRead;
}


void XMLPlatformUtils::resetFile(FileHandle theFile)
{
    // Seek to the start of the file
    if (fseek((FILE*)theFile, 0, SEEK_SET) )
        ThrowXML(XMLPlatformUtilsException, XML4CExcepts::File_CouldNotResetFile);
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

XMLCh* XMLPlatformUtils::getBasePath(const XMLCh* const srcPath)
{

    //
    //  NOTE: THe path provided has always already been opened successfully,
    //  so we know that its not some pathological freaky path. It comes in
    //  in native format, and goes out as Unicode always
    //
    char* newSrc = XMLString::transcode(srcPath);
    ArrayJanitor<char> janText(newSrc);

    // Use a local buffer that is big enough for the largest legal path
     char* tmpPath = dirname((char*)newSrc);
    if (!tmpPath)
    {
        ThrowXML(XMLPlatformUtilsException, XML4CExcepts::File_CouldNotGetBasePathName);
    }

    char* newXMLString = new char [strlen(tmpPath) +2];
    ArrayJanitor<char> newJanitor(newXMLString);
    strcpy(newXMLString, tmpPath);
        strcat(newXMLString , "/");
    // Return a copy of the path, in Unicode format
    return XMLString::transcode(newXMLString);
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

// -----------------------------------------------------------------------
//  Standard out/error support
// -----------------------------------------------------------------------

void XMLPlatformUtils::writeToStdErr(const char* const toWrite)
{
    WriteCharStr(stderr, toWrite);
}
void XMLPlatformUtils::writeToStdErr(const XMLCh* const toWrite)
{
    WriteUStrStdErr(toWrite);
}
void XMLPlatformUtils::writeToStdOut(const XMLCh* const toWrite)
{
    WriteUStrStdOut(toWrite);
}
void XMLPlatformUtils::writeToStdOut(const char* const toWrite)
{
    WriteCharStr(stdout, toWrite);
}

// -----------------------------------------------------------------------
//  Mutex methods
// -----------------------------------------------------------------------

#ifndef APP_NO_THREADS
void XMLPlatformUtils::closeMutex(void* const mtxHandle)
{
    if (mtxHandle == NULL)
        return;
    if (pthread_mutex_destroy( (pthread_mutex_t*)mtxHandle))
    {
        ThrowXML(XMLPlatformUtilsException, XML4CExcepts::Mutex_CouldNotDestroy);
    }
    if ((pthread_mutex_t*)mtxHandle)
        delete mtxHandle;
}

void XMLPlatformUtils::lockMutex(void* const mtxHandle)
{
    if (mtxHandle == NULL)
        return;
    if (pthread_mutex_lock( (pthread_mutex_t*)mtxHandle))
    {
        ThrowXML(XMLPlatformUtilsException, XML4CExcepts::Mutex_CouldNotLock);
    }
}

void* XMLPlatformUtils::makeMutex()
{
    pthread_mutex_t* mutex = new pthread_mutex_t;

    if (pthread_mutex_init(mutex, NULL))
    {
        ThrowXML(XMLPlatformUtilsException, XML4CExcepts::Mutex_CouldNotCreate);
    }
    return (void*)(mutex);
}

void XMLPlatformUtils::unlockMutex(void* const mtxHandle)
{
    if (mtxHandle == NULL)
        return;
    if (pthread_mutex_unlock( (pthread_mutex_t*)mtxHandle))
    {
        ThrowXML(XMLPlatformUtilsException, XML4CExcepts::Mutex_CouldNotUnlock);
    }
}

#else // #ifndef APP_NO_THREADS

void XMLPlatformUtils::closeMutex(void* const mtxHandle)
{
}

void XMLPlatformUtils::lockMutex(void* const mtxHandle)
{
}

void* XMLPlatformUtils::makeMutex()
{
        return 0;
}

void XMLPlatformUtils::unlockMutex(void* const mtxHandle)
{
}

#endif // APP_NO_THREADS

// -----------------------------------------------------------------------
//  Miscellaneous synchronization methods
// -----------------------------------------------------------------------
//atomic system calls in Solaris is only restricted to kernel libraries
//So, to make operations thread safe we implement static mutex and lock
//the atomic operations. It makes the process slow but what's the alternative!

void* XMLPlatformUtils::compareAndSwap ( void**      toFill ,
                    const void* const newValue ,
                    const void* const toCompare)
{
    //return ((void*)cas32( (uint32_t*)toFill,  (uint32_t)toCompare, (uint32_t)newValue) );
    // the below calls are temporarily made till the above functions are part of user library
    // Currently its supported only in the kernel mode

    lockMutex(&atomicOpsMutex);

    void *retVal = *toFill;
    if (*toFill == toCompare)
              *toFill = (void *)newValue;

    unlockMutex(&atomicOpsMutex);

    return retVal;
}

int XMLPlatformUtils::atomicIncrement(int &location)
{
    //return (int)atomic_add_32_nv( (uint32_t*)&location, 1);
    XMLMutexLock localLock(&atomicOpsMutex);

    return ++location;
}

int XMLPlatformUtils::atomicDecrement(int &location)
{
    //return (int)atomic_add_32_nv( (uint32_t*)&location, -1);
    XMLMutexLock localLock(&atomicOpsMutex);

    return --location;
}

FileHandle XMLPlatformUtils::openStdInHandle()
{
        return (FileHandle)fdopen(dup(0), "rb");
}

