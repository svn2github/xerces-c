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

/**
 * $Log$
 * Revision 1.2  2000/04/05 00:20:16  roddey
 * More updates for the low level formatted output support
 *
 * Revision 1.1  2000/03/28 19:43:17  roddey
 * Fixes for signed/unsigned warnings. New work for two way transcoding
 * stuff.
 *
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <util/PlatformUtils.hpp>
#include <util/XMLString.hpp>
#include <util/TransService.hpp>
#include <util/TranscodingException.hpp>
#include <util/XMLExceptMsgs.hpp>
#include <framework/XMLFormatter.hpp>
#include <memory.h>


// ---------------------------------------------------------------------------
//  Local data
//
//  gXXXRef
//      These are hard coded versions of the char refs we put out for the
//      standard char refs.
//
//  gEscapeChars
//      For each style of escape, we have a list of the chars that must
//      be escaped for that style. The first null hit in each list indicates
//      no more valid entries in that list. The first entry is a dummy for
//      the NoEscapes style.
// ---------------------------------------------------------------------------
static const XMLCh  gAmpRef[] =
{
    chAmpersand, chLatin_a, chLatin_m, chLatin_p, chSemiColon, chNull
};

static const XMLCh  gAposRef[] =
{
    chAmpersand, chLatin_a, chLatin_p, chLatin_o, chLatin_s, chSemiColon, chNull
};

static const XMLCh  gGTRef[] =
{
    chAmpersand, chLatin_g, chLatin_t, chSemiColon, chNull
};

static const XMLCh  gLTRef[] =
{
    chAmpersand, chLatin_l, chLatin_t, chSemiColon, chNull
};

static const XMLCh  gQuoteRef[] =
{
    chAmpersand, chLatin_q, chLatin_u, chLatin_o, chLatin_t, chSemiColon, chNull
};

static const unsigned int kEscapeCount = 6;
static const XMLCh gEscapeChars[XMLFormatter::EscapeFlags_Count][kEscapeCount] =
{
        { chNull      , chNull       , chNull        , chNull       , chNull        , chNull }
    ,   { chAmpersand , chCloseAngle , chDoubleQuote , chOpenAngle  , chSingleQuote , chNull }
    ,   { chAmpersand , chOpenAngle  , chDoubleQuote , chNull       , chNull        , chNull }
    ,   { chAmpersand , chOpenAngle  , chNull        , chNull       , chNull        , chNull }
};


// ---------------------------------------------------------------------------
//  Local methods
// ---------------------------------------------------------------------------
static inline bool inEscapeList(const   XMLFormatter::EscapeFlags escStyle
                                , const XMLCh                     toCheck)
{
    const XMLCh* escList = gEscapeChars[escStyle];
    while (*escList)
    {
        if (*escList++ == toCheck)
            return true;
    }
    return false;
}


// ---------------------------------------------------------------------------
//  XMLFormatter: Constructors and Destructor
// ---------------------------------------------------------------------------
XMLFormatter::XMLFormatter( const   char* const             outEncoding
                            ,       XMLFormatTarget* const  target
                            , const EscapeFlags             escapeFlags
                            , const UnRepFlags              unrepFlags) :

    fEscapeFlags(escapeFlags)
    , fOutEncoding(0)
    , fTarget(target)
    , fUnRepFlags(unrepFlags)
    , fXCoder(0)

    , fAposRef(0)
    , fAmpRef(0)
    , fGTRef(0)
    , fLTRef(0)
    , fQuoteRef(0)
{
    // Transcode the encoding string
    fOutEncoding = XMLString::transcode(outEncoding);

    // Try to create a transcoder for this encoding
    XMLTransService::Codes resCode;
    fXCoder = XMLPlatformUtils::fgTransService->makeNewTranscoderFor
    (
        fOutEncoding
        , resCode
        , kTmpBufSize
    );

    if (!fXCoder)
    {
        delete [] fOutEncoding;
        fOutEncoding = 0;
        ThrowXML1
        (
            TranscodingException
            , XMLExcepts::Trans_CantCreateCvtrFor
            , outEncoding
        );
    }
}


XMLFormatter::XMLFormatter( const   XMLCh* const            outEncoding
                            ,       XMLFormatTarget* const  target
                            , const EscapeFlags             escapeFlags
                            , const UnRepFlags              unrepFlags) :
    fEscapeFlags(escapeFlags)
    , fOutEncoding(0)
    , fTarget(target)
    , fUnRepFlags(unrepFlags)
    , fXCoder(0)

    , fAposRef(0)
    , fAmpRef(0)
    , fGTRef(0)
    , fLTRef(0)
    , fQuoteRef(0)
{
    // Copy the encoding string
    fOutEncoding = XMLString::replicate(outEncoding);

    // Try to create a transcoder for this encoding
    XMLTransService::Codes resCode;
    fXCoder = XMLPlatformUtils::fgTransService->makeNewTranscoderFor
    (
        fOutEncoding
        , resCode
        , kTmpBufSize
    );

    if (!fXCoder)
    {
        delete [] fOutEncoding;
        fOutEncoding = 0;
        ThrowXML1
        (
            TranscodingException
            , XMLExcepts::Trans_CantCreateCvtrFor
            , outEncoding
        );
    }
}

XMLFormatter::~XMLFormatter()
{
    delete fAposRef;
    delete fAmpRef;
    delete fGTRef;
    delete fLTRef;
    delete fQuoteRef;

    delete [] fOutEncoding;
    delete fXCoder;

    // We DO NOT own the target object!
}


// ---------------------------------------------------------------------------
//  XMLFormatter: Formatting methods
// ---------------------------------------------------------------------------
void
XMLFormatter::formatBuf(const   XMLCh* const    toFormat
                        , const unsigned int    count
                        , const EscapeFlags     escapeFlags
                        , const UnRepFlags      unrepFlags)
{
    //
    //  Figure out the actual escape flag value. If the parameter is not
    //  the default, then take it. Else take the current default.
    //
    const EscapeFlags actualEsc = (escapeFlags == DefaultEscape)
                                ? fEscapeFlags : escapeFlags;

    //
    //  If we don't have any escape flags set, then we can do the most
    //  efficient loop, else we have to do it the hard way.
    //
    const XMLCh*    srcPtr = toFormat;
    const XMLCh*    endPtr = toFormat + count;
    unsigned int    charsEaten;
    if (actualEsc == NoEscapes)
    {
        //
        //  Just do a whole buffer at a time into the temp buffer, cap it
        //  off, and send it to the target.
        //
        while (srcPtr < endPtr)
        {
            const unsigned int srcCount = endPtr - srcPtr;
            const unsigned srcChars = srcCount > kTmpBufSize ?
                                      kTmpBufSize : srcCount;

            const unsigned int outBytes = fXCoder->transcodeTo
            (
                srcPtr
                , srcChars
                , fTmpBuf
                , kTmpBufSize
                , charsEaten
                , XMLTranscoder::UnRep_Throw
            );

            #if XML_DEBUG
            if ((outBytes > kTmpBufSize)
            ||  (charsEaten > srcCount))
            {
                // <TBD> The transcoder is freakin out maaaannn
            }
            #endif

            // If we get any bytes out, then write them
            if (outBytes)
            {
                fTmpBuf[outBytes] = 0;
                fTarget->writeChars(fTmpBuf);
            }

            // And bump up our pointer
            srcPtr += charsEaten;
        }
    }
     else
    {
        //
        //  This one just escapes the standard set of XML defined character
        //  refs: apos, amp, lt, gt, and quot.
        //
        //  For now, just whimp out and do it the simple but slow way in
        //  order to get this concept out for evaluation. Come back later
        //  and spiff it up.
        //
        while (srcPtr < endPtr)
        {
            //
            //  Run a temp pointer up until we hit a character that we have
            //  to escape. Then we can convert all the chars between our
            //  current source pointer and here all at once.
            //
            const XMLCh* tmpPtr = srcPtr;
            while (!inEscapeList(actualEsc, *tmpPtr) && (tmpPtr < endPtr))
                tmpPtr++;

            //
            //  If we got any chars, then lets convert them and write them
            //  out.
            //
            if (tmpPtr > srcPtr)
            {
                const unsigned int srcCount = tmpPtr - srcPtr;
                const unsigned srcChars = srcCount > kTmpBufSize ?
                                          kTmpBufSize : srcCount;

                const unsigned int outBytes = fXCoder->transcodeTo
                (
                    srcPtr
                    , srcChars
                    , fTmpBuf
                    , kTmpBufSize
                    , charsEaten
                    , XMLTranscoder::UnRep_Throw
                );

                #if XML_DEBUG
                if ((outBytes > kTmpBufSize)
                ||  (charsEaten > srcCount))
                {
                    // <TBD> The transcoder is freakin out maaaannn
                }
                #endif

                // If we get any bytes out, then write them
                if (outBytes)
                {
                    fTmpBuf[outBytes] = 0;
                    fTarget->writeChars(fTmpBuf);
                }

                // And bump up our pointer
                srcPtr += charsEaten;
            }
             else if (tmpPtr < endPtr)
            {
                //
                //  Ok, so we've hit a char that must be escaped. So loop
                //  until we hit the end or a non-escaped char and put out
                //  char refs for these.
                //
                bool done = false;
                while ((srcPtr < endPtr) && !done)
                {
                    switch(*srcPtr)
                    {
                        case chAmpersand :
                            fTarget->writeChars(getAmpRef());
                            break;

                        case chSingleQuote :
                            fTarget->writeChars(getAposRef());
                            break;

                        case chDoubleQuote :
                            fTarget->writeChars(getQuoteRef());
                            break;

                        case chCloseAngle :
                            fTarget->writeChars(getGTRef());
                            break;

                        case chOpenAngle :
                            fTarget->writeChars(getLTRef());
                            break;

                        default:
                            done = true;
                            break;
                    }

                    if (!done)
                        srcPtr++;
                }
            }
        }
    }
}

XMLFormatter& XMLFormatter::operator<<(const XMLCh* const toFormat)
{
    const unsigned int len = XMLString::stringLen(toFormat);
    formatBuf(toFormat, len);
    return *this;
}

XMLFormatter& XMLFormatter::operator<<(const XMLCh toFormat)
{
    // Make a temp string format that
    XMLCh szTmp[2];
    szTmp[0] = toFormat;
    szTmp[1] = 0;

    formatBuf(szTmp, 1);
    return *this;
}



// ---------------------------------------------------------------------------
//  XMLFormatter: Private helper methosd
// ---------------------------------------------------------------------------
const XMLByte* XMLFormatter::getAposRef()
{
    if (fAposRef)
        return fAposRef;

    unsigned int charsEaten;
    const unsigned int outBytes = fXCoder->transcodeTo
    (
        gAposRef
        , XMLString::stringLen(gAposRef)
        , fTmpBuf
        , kTmpBufSize
        , charsEaten
        , XMLTranscoder::UnRep_Throw
    );
    fTmpBuf[outBytes] = 0;
   
    ((XMLFormatter*)this)->fAposRef = new XMLByte[outBytes + 1];
    memcpy(fAposRef, fTmpBuf, outBytes + 1);
    return fAposRef;
}

const XMLByte* XMLFormatter::getAmpRef()
{
    if (fAmpRef)
        return fAmpRef;

    unsigned int charsEaten;
    const unsigned int outBytes = fXCoder->transcodeTo
    (
        gAmpRef
        , XMLString::stringLen(gAmpRef)
        , fTmpBuf
        , kTmpBufSize
        , charsEaten
        , XMLTranscoder::UnRep_Throw
    );
    fTmpBuf[outBytes] = 0;
   
    ((XMLFormatter*)this)->fAmpRef = new XMLByte[outBytes + 1];
    memcpy(fAmpRef, fTmpBuf, outBytes + 1);
    return fAmpRef;
}

const XMLByte* XMLFormatter::getGTRef()
{
    if (fGTRef)
        return fGTRef;

    unsigned int charsEaten;
    const unsigned int outBytes = fXCoder->transcodeTo
    (
        gGTRef
        , XMLString::stringLen(gGTRef)
        , fTmpBuf
        , kTmpBufSize
        , charsEaten
        , XMLTranscoder::UnRep_Throw
    );
    fTmpBuf[outBytes] = 0;
   
    ((XMLFormatter*)this)->fGTRef = new XMLByte[outBytes + 1];
    memcpy(fGTRef, fTmpBuf, outBytes + 1);
    return fGTRef;
}

const XMLByte* XMLFormatter::getLTRef()
{
    if (fLTRef)
        return fLTRef;

    unsigned int charsEaten;
    const unsigned int outBytes = fXCoder->transcodeTo
    (
        gLTRef
        , XMLString::stringLen(gLTRef)
        , fTmpBuf
        , kTmpBufSize
        , charsEaten
        , XMLTranscoder::UnRep_Throw
    );
    fTmpBuf[outBytes] = 0;
   
    ((XMLFormatter*)this)->fLTRef = new XMLByte[outBytes + 1];
    memcpy(fLTRef, fTmpBuf, outBytes + 1);
    return fLTRef;
}

const XMLByte* XMLFormatter::getQuoteRef()
{
    if (fQuoteRef)
        return fQuoteRef;

    unsigned int charsEaten;
    const unsigned int outBytes = fXCoder->transcodeTo
    (
        gQuoteRef
        , XMLString::stringLen(gQuoteRef)
        , fTmpBuf
        , kTmpBufSize
        , charsEaten
        , XMLTranscoder::UnRep_Throw
    );
    fTmpBuf[outBytes] = 0;
   
    ((XMLFormatter*)this)->fQuoteRef = new XMLByte[outBytes + 1];
    memcpy(fQuoteRef, fTmpBuf, outBytes + 1);
    return fQuoteRef;
}
