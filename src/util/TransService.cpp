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


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <util/RefHashTableOf.hpp>
#include <util/XML88591Transcoder.hpp>
#include <util/XMLASCIITranscoder.hpp>
#include <util/XMLEBCDICTranscoder.hpp>
#include <util/XMLIBM1140Transcoder.hpp>
#include <util/XMLUCS4Transcoder.hpp>
#include <util/XMLUTF8Transcoder.hpp>
#include <util/XMLUTF16Transcoder.hpp>
#include <util/XMLUni.hpp>
#include <util/TransENameMap.hpp>



// ---------------------------------------------------------------------------
//  Local, static data
//
//  gMappings
//      This is a hash table of ENameMap objects. It is created and filled
//      in when the platform init calls our initTransService() method.
// ---------------------------------------------------------------------------
static RefHashTableOf<ENameMap>*    gMappings = 0;



// ---------------------------------------------------------------------------
//  XLMTransService: Constructors and destructor
// ---------------------------------------------------------------------------
XMLTransService::XMLTransService()
{
}

XMLTransService::~XMLTransService()
{
}


// ---------------------------------------------------------------------------
//  XLMTranscoder: Non-virtual API
// ---------------------------------------------------------------------------
XMLTranscoder*
XMLTransService::makeNewTranscoderFor(  const   XMLCh* const            encodingName
                                        ,       XMLTransService::Codes& resValue
                                        , const unsigned int            blockSize)
{
    //
    //  First try to find it in our list of mappings to intrinsically
    //  supported encodings. We have to upper case the passed encoding
    //  name because we use a hash table and we stored all our mappings
    //  in all uppercase.
    //
    const unsigned int bufSize = 2048;
    XMLCh upBuf[bufSize + 1];
    if (!XMLString::copyNString(upBuf, encodingName, bufSize))
        return 0;
    XMLString::upperCase(upBuf);
    ENameMap* ourMapping = gMappings->get(upBuf);

    // If we found it, then call the factory method for it
    if (ourMapping)
        return ourMapping->makeNew(blockSize);

    // It wasn't an intrinsic, so pass it on to the trans service
    return makeNewXMLTranscoder(encodingName, resValue, blockSize);
}


// ---------------------------------------------------------------------------
//  XLMTranscoder: Public Destructor
// ---------------------------------------------------------------------------
XMLTranscoder::~XMLTranscoder()
{
    delete [] fEncodingName;
}


// ---------------------------------------------------------------------------
//  XLMTranscoder: Hidden Constructors
// ---------------------------------------------------------------------------
XMLTranscoder::XMLTranscoder(const  XMLCh* const    encodingName
                            , const unsigned int    blockSize) :
    fEncodingName(0)
    , fBlockSize(blockSize)
{
    fEncodingName = XMLString::replicate(encodingName);
}


// ---------------------------------------------------------------------------
//  XLMTranscoder: Protected helpers
// ---------------------------------------------------------------------------
void XMLTranscoder::checkBlockSize(const unsigned int toCheck)
{
    if (toCheck > fBlockSize)
    {
        // <TBD> Throw an exception here
    }
}




// ---------------------------------------------------------------------------
//  XLMLCPTranscoder: Public Destructor
// ---------------------------------------------------------------------------
XMLLCPTranscoder::XMLLCPTranscoder()
{
}


// ---------------------------------------------------------------------------
//  XLMTranscoder: Hidden Constructors
// ---------------------------------------------------------------------------
XMLLCPTranscoder::~XMLLCPTranscoder()
{
}


// ---------------------------------------------------------------------------
//  XLMTranscoder: Hidden Init Method
//
//  This is called by platform utils during startup.
// ---------------------------------------------------------------------------
void XMLTransService::initTransService()
{
    //
    //  Create our hash table that we will fill with mappings. The default
    //  is to adopt the elements, which is fine with us.
    //
    gMappings = new RefHashTableOf<ENameMap>(103);

    //
    //  Add in our mappings for ASCII.
    //
    gMappings->put(new ENameMapFor<XMLASCIITranscoder>(XMLUni::fgUSASCIIEncodingString));
    gMappings->put(new ENameMapFor<XMLASCIITranscoder>(XMLUni::fgUSASCIIEncodingString2));
    gMappings->put(new ENameMapFor<XMLASCIITranscoder>(XMLUni::fgUSASCIIEncodingString3));
    gMappings->put(new ENameMapFor<XMLASCIITranscoder>(XMLUni::fgUSASCIIEncodingString4));

    //
    //  Add in our mappings for UTF-8
    //
    gMappings->put(new ENameMapFor<XMLUTF8Transcoder>(XMLUni::fgUTF8EncodingString));
    gMappings->put(new ENameMapFor<XMLUTF8Transcoder>(XMLUni::fgUTF8EncodingString2));

    //
    //  Add in our mappings for Latin1
    //
    gMappings->put(new ENameMapFor<XML88591Transcoder>(XMLUni::fgISO88591EncodingString));
    gMappings->put(new ENameMapFor<XML88591Transcoder>(XMLUni::fgISO88591EncodingString2));
    gMappings->put(new ENameMapFor<XML88591Transcoder>(XMLUni::fgISO88591EncodingString3));
    gMappings->put(new ENameMapFor<XML88591Transcoder>(XMLUni::fgISO88591EncodingString4));
    gMappings->put(new ENameMapFor<XML88591Transcoder>(XMLUni::fgISO88591EncodingString5));
    gMappings->put(new ENameMapFor<XML88591Transcoder>(XMLUni::fgISO88591EncodingString6));
    gMappings->put(new ENameMapFor<XML88591Transcoder>(XMLUni::fgISO88591EncodingString7));
    gMappings->put(new ENameMapFor<XML88591Transcoder>(XMLUni::fgISO88591EncodingString8));

    //
    //  Add in our mappings for UTF-16 and UCS-4, little endian
    //
    bool swapped = false;

    #if defined(ENDIANMODE_BIG)
    swapped = true;
    #endif
    gMappings->put
    (
        new EEndianNameMapFor<XMLUTF16Transcoder>
        (
            XMLUni::fgUTF16LEncodingString
            , swapped
        )
    );

    gMappings->put
    (
        new EEndianNameMapFor<XMLUTF16Transcoder>
        (
            XMLUni::fgUTF16LEncodingString2
            , swapped
        )
    );

    gMappings->put
    (
        new EEndianNameMapFor<XMLUCS4Transcoder>
        (
            XMLUni::fgUCS4LEncodingString
            , swapped
        )
    );

    gMappings->put
    (
        new EEndianNameMapFor<XMLUCS4Transcoder>
        (
            XMLUni::fgUCS4LEncodingString2
            , swapped
        )
    );

    //
    //  Add in our mappings for UTF-16 and UCS-4, big endian
    //
    swapped = false;
    #if defined(ENDIANMODE_LITTLE)
    swapped = true;
    #endif
    gMappings->put
    (
        new EEndianNameMapFor<XMLUTF16Transcoder>
        (
            XMLUni::fgUTF16BEncodingString
            , swapped
        )
    );

    gMappings->put
    (
        new EEndianNameMapFor<XMLUTF16Transcoder>
        (
            XMLUni::fgUTF16BEncodingString2
            , swapped
        )
    );

    gMappings->put
    (
        new EEndianNameMapFor<XMLUCS4Transcoder>
        (
            XMLUni::fgUCS4BEncodingString
            , swapped
        )
    );

    gMappings->put
    (
        new EEndianNameMapFor<XMLUCS4Transcoder>
        (
            XMLUni::fgUCS4BEncodingString2
            , swapped
        )
    );

    //
    //  Add in our mappings for EBCDIC-US. We map all the default EBCDIC
    //  encodings to our intrinsic encoder, which is IBM-037. And of course
    //  we map the IBM-037 encoding itself here as well.
    //
    gMappings->put(new ENameMapFor<XMLEBCDICTranscoder>(XMLUni::fgIBM037EncodingString));
    gMappings->put(new ENameMapFor<XMLEBCDICTranscoder>(XMLUni::fgIBM037EncodingString2));
    gMappings->put(new ENameMapFor<XMLEBCDICTranscoder>(XMLUni::fgIBM037EncodingString3));
    gMappings->put(new ENameMapFor<XMLEBCDICTranscoder>(XMLUni::fgIBM037EncodingString4));
    gMappings->put(new ENameMapFor<XMLEBCDICTranscoder>(XMLUni::fgIBM037EncodingString5));
    gMappings->put(new ENameMapFor<XMLEBCDICTranscoder>(XMLUni::fgIBM037EncodingString6));

    //
    //  Add in our mappings for the EBCDIC-US with Euro update, i.e. IBM1140
    //  or CPIBM1140.
    //
    gMappings->put(new ENameMapFor<XMLIBM1140Transcoder>(XMLUni::fgIBM1140EncodingString));
    gMappings->put(new ENameMapFor<XMLIBM1140Transcoder>(XMLUni::fgIBM1140EncodingString2));
}
