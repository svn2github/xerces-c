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
 * Revision 1.1  1999/11/09 01:06:08  twl
 * Initial revision
 *
 * Revision 1.3  1999/11/08 20:45:34  rahul
 * Swat for adding in Product name and CVS comment log variable.
 *
 */

#ifndef ICUTRANSSERVICE_HPP
#define ICUTRANSSERVICE_HPP

#include <util/Mutexes.hpp>
#include <util/TransService.hpp>

struct UConverter;

class XMLUTIL_EXPORT ICUTransService : public XMLTransService
{
public :
    // -----------------------------------------------------------------------
    //  Public, static methods
    // -----------------------------------------------------------------------
    static void setICUPath(const char* const pathToSet);


    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    ICUTransService();
    ~ICUTransService();


    // -----------------------------------------------------------------------
    //  Implementation of the virtual transcoding service API
    // -----------------------------------------------------------------------
    virtual int compareIString
    (
        const   XMLCh* const    comp1
        , const XMLCh* const    comp2
    );

    virtual int compareNIString
    (
        const   XMLCh* const    comp1
        , const XMLCh* const    comp2
        , const unsigned int    maxChars
    );

    virtual bool isSpace(const XMLCh toCheck) const;

    virtual XMLTranscoder* makeNewDefTranscoder();

    virtual XMLTranscoder* makeNewTranscoderFor
    (
        const   XMLCh* const            encodingName
        ,       XMLTransService::Codes& resValue
        , const unsigned int            blockSize
    );


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    ICUTransService(const ICUTransService&);
    void operator=(const ICUTransService&);
};



class XMLUTIL_EXPORT ICUTranscoder : public XMLTranscoder
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    ICUTranscoder(UConverter* const toAdopt, const unsigned int blockSize);
    ~ICUTranscoder();


    // -----------------------------------------------------------------------
    //  Implementation of the virtual transcoder interface
    // -----------------------------------------------------------------------
    virtual unsigned int calcRequiredSize(const char* const srcText);

    virtual unsigned int calcRequiredSize(const XMLCh* const srcText);

    virtual XMLCh transcodeOne
    (
        const   char* const     srcData
        , const unsigned int    srcBytes
        ,       unsigned int&   bytesEaten
    );

    virtual char* transcode(const XMLCh* const toTranscode);

    virtual bool transcode
    (
        const   XMLCh* const    toTranscode
        ,       char* const     toFill
        , const unsigned int    maxChars
    );

    virtual XMLCh* transcode(const char* const toTranscode);

    virtual bool transcode
    (
        const   char* const             toTranscode
        ,       XMLCh* const            toFill
        , const unsigned int            maxChars
    );

    virtual unsigned int transcodeXML
    (
        const   char* const             srcData
        , const unsigned int            srcCount
        ,       XMLCh* const            toFill
        , const unsigned int            maxChars
        ,       unsigned int&           bytesEaten
    );


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    ICUTranscoder();
    ICUTranscoder(const ICUTranscoder&);
    void operator=(const ICUTranscoder&);


    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fCharOfsBuf
    //      When we do the XML transcode call, the ICU stuff takes an array
    //      of ints that it fills with the size of each MB char as it xlats
    //      it. Its converted to another form for the caller, but this is used
    //      to get it from ICU. We don't want allocate it over and over so its
    //      done during ctor and left around.
    //
    //  fConverter
    //      This is a pointer to the ICU converter that this transcoder
    //      uses.
    //
    //  fMutex
    //      We have to synchronize threaded calls to the converter.
    // -----------------------------------------------------------------------
    long*           fCharOfsBuf;
    UConverter*     fConverter;
    XMLMutex        fMutex;
};

#endif
