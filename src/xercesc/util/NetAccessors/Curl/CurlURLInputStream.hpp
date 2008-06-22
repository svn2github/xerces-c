/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
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

#if !defined(XERCESC_INCLUDE_GUARD_CURLURLINPUTSTREAM_HPP)
#define XERCESC_INCLUDE_GUARD_CURLURLINPUTSTREAM_HPP

#include <curl.h>
#include <multi.h>
#include <easy.h>

#include <xercesc/util/XMLURL.hpp>
#include <xercesc/util/XMLExceptMsgs.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/BinInputStream.hpp>
#include <xercesc/util/XMLNetAccessor.hpp>

XERCES_CPP_NAMESPACE_BEGIN

//
// This class implements the BinInputStream interface specified by the XML
// parser.
//

class XMLUTIL_EXPORT CurlURLInputStream : public BinInputStream
{
public :
    CurlURLInputStream(const XMLURL&  urlSource, const XMLNetHTTPInfo* httpInfo=0);
    ~CurlURLInputStream();

    virtual XMLFilePos curPos() const;
    virtual XMLSize_t readBytes
    (
                XMLByte* const  toFill
        , const XMLSize_t       maxToRead
    );

    virtual const XMLCh *getContentType() const;

private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    CurlURLInputStream(const CurlURLInputStream&);
    CurlURLInputStream& operator=(const CurlURLInputStream&);
    
    static size_t staticWriteCallback(char *buffer,
                                      size_t size,
                                      size_t nitems,
                                      void *outstream);
    size_t writeCallback(			  char *buffer,
                                      size_t size,
                                      size_t nitems);
    static size_t staticHeaderCallback(void *ptr,
                                       size_t size,
                                       size_t nmemb,
                                       void *stream);
    size_t headerCallback(             char *buffer,
                                       size_t size,
                                       size_t nitems);


    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fSocket
    //      The socket representing the connection to the remote file.
    //  fBytesProcessed
    //      Its a rolling count of the number of bytes processed off this
    //      input stream.
    //  fBuffer
    //      Holds the http header, plus the first part of the actual
    //      data.  Filled at the time the stream is opened, data goes
    //      out to user in response to readBytes().
    //  fBufferPos, fBufferEnd
    //      Pointers into fBuffer, showing start and end+1 of content
    //      that readBytes must return.
    // -----------------------------------------------------------------------
	
    CURLM*				fMulti;
    CURL*				fEasy;
    
    MemoryManager*      fMemoryManager;
    
    XMLURL				fURLSource;
    ArrayJanitor<char>	fURL;
    
    unsigned long       fTotalBytesRead;
    XMLByte*			fWritePtr;
    unsigned long		fBytesRead;
    unsigned long		fBytesToRead;
    bool				fDataAvailable;
    
    // Overflow buffer for when curl writes more data to us
    // than we've asked for.
    XMLByte				fBuffer[CURL_MAX_WRITE_SIZE];
    XMLByte*			fBufferHeadPtr;
    XMLByte*			fBufferTailPtr;

    XMLCh *             fContentType;
    
}; // CurlURLInputStream


inline XMLFilePos
CurlURLInputStream::curPos() const
{
    return fTotalBytesRead;
}

XERCES_CPP_NAMESPACE_END

#endif // CURLURLINPUTSTREAM_HPP

