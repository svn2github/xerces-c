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
 * Revision 1.3  1999/12/15 19:59:12  roddey
 * Added new tests and updated tests for new split transcoder architecture.
 *
 * Revision 1.2  1999/12/07 23:11:01  roddey
 * Add in some new tests for transcoders and update the URL tests
 * a bit.
 *
 * Revision 1.1.1.1  1999/11/09 01:02:03  twl
 * Initial checkin
 *
 * Revision 1.2  1999/11/08 20:42:28  rahul
 * Swat for adding in Product name and CVS comment log variable.
 *
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include "CoreTests.hpp"
#include <util/XMLASCIITranscoder.hpp>
#include <util/Janitor.hpp>
#include <util/PlatformUtils.hpp>
#include <util/TransService.hpp>



// ---------------------------------------------------------------------------
//  Local test methods
// ---------------------------------------------------------------------------
static bool testOneTranscoder(          XMLTranscoder&  toTest
                                , const char* const     nativeStr
                                , const unsigned int    nativeBytes
                                , const XMLCh* const    unicodeStr
                                , const unsigned int    unicodeBytes)
{
    // Ask it to do a basic transcoding of a string round trip
    {
        // Do the first pass and compare against the expected result
        XMLCh* firstPass = toTest.transcode(nativeStr);
        if (XMLString::compareString(firstPass, unicodeStr))
        {
            outStrm << "Xcode from native to Unicode failed"
                    << EndLn;
            return false;
        }

        // Now transcode that back to native again
        char* secondPass = toTest.transcode(firstPass);
        if (XMLString::compareString(secondPass, nativeStr))
        {
            outStrm << "Xcode from Unicode to native failed"
                    << EndLn;
            return false;
        }
    }

    // Check the methods that calculate required storage
    {
        if (toTest.calcRequiredSize(nativeStr) != unicodeBytes)
        {
            outStrm << "Calculated size to xcode char string was wrong"
                    << EndLn;
            return false;
        }

        if (toTest.calcRequiredSize(unicodeStr) != nativeBytes)
        {
            outStrm << "Calculated size to xcode Unicode string was wrong"
                    << EndLn;
            return false;
        }
    }

    return true;
}

//
//  This method tests the basic services of the currently loaded transcoding
//  service. This is the stuff that is independent of the source XML
//  encoding, i.e. it just works mostly on XMLCh content.
//
static bool basicTransServiceTests()
{
    static const char* const   testStr1 = "The test string";
    static const XMLCh         testStr2[] = 
    {
            chLatin_T, chLatin_h, chLatin_e, chSpace, chLatin_t, chLatin_e
        ,   chLatin_s, chLatin_t, chSpace, chLatin_s, chLatin_t, chLatin_r
        ,   chLatin_i, chLatin_n, chLatin_g, chNull
    };
    static const XMLCh         testStr3[] = 
    {
            chLatin_T, chLatin_H, chLatin_E, chSpace, chLatin_T, chLatin_E
        ,   chLatin_S, chLatin_T, chSpace, chLatin_S, chLatin_T, chLatin_R
        ,   chLatin_I, chLatin_N, chLatin_G, chNull
    };
    static const XMLCh testWS[] = { chSpace, chHTab, chLF, chCR, chNull };
    static const XMLCh testNWS[] = { chDigit_0, chLatin_A, chPound, chNull };

    const XMLCh* pszTmp;

    // Test the case insensitive comparison
    if (XMLPlatformUtils::fgTransService->compareIString(testStr2, testStr3))
    {
        outStrm << "Case sensitive compare failed" << EndLn;
        return false;
    }

    //
    //  Test the case sensitive leading substring compare. Loop through and
    //  test it for each length possible. This will probe boundary conditions
    //  hopefully.
    //
    const unsigned int testLen = XMLString::stringLen(testStr2);
    for (unsigned int testInd = 0; testInd < testLen; testInd++)
    {
        if (XMLPlatformUtils::fgTransService->compareNIString(testStr2, testStr3, testInd))
        {
            outStrm << "Case sensitive leading compare failed" << EndLn;
            return false;
        }
    }

    //  Check the isSpace API. This is the only generalized character type
    //  check that we need (all others are XML specific and done via XML's
    //  character type tables.)
    //
    pszTmp = testWS;
    while (*pszTmp)
    {
        if (!XMLPlatformUtils::fgTransService->isSpace(*pszTmp))
        {
            outStrm << "isSpace() failed on code point: "
                    << (unsigned int)*pszTmp << EndLn;
            return false;
        }
        pszTmp++;
    }

    pszTmp = testNWS;
    while (*pszTmp)
    {
        if (XMLPlatformUtils::fgTransService->isSpace(*pszTmp))
        {
            outStrm << "isSpace() failed on code point: "
                    << (unsigned int)*pszTmp << EndLn;
            return false;
        }
        pszTmp++;
    }

    return true;
}


//
//  This tests some of the individual transcoders. These transcode between
//  some native encoding and the internal Unicode format.
//
//  We test the transcoders for the intrinsic transcoders here, the ones
//  that are implemented directly in the parser system. We can't test any
//  others because there is no guarantee that any others are supported by
//  any particular plugged in transcoding service.
//
static bool basicTranscoderTests()
{
    static const XMLCh         testStr2[] = 
    {
            chLatin_T, chLatin_h, chLatin_e, chSpace, chLatin_t, chLatin_e
        ,   chLatin_s, chLatin_t, chSpace, chLatin_s, chLatin_t, chLatin_r
        ,   chLatin_i, chLatin_n, chLatin_g, chNull
    };
    bool result;

    // Test the intrinsic ASCII transcoder
    {
        const char* const testStr1 = "The test string";
        XMLASCIITranscoder toTest;
        result = testOneTranscoder
        (
            toTest
            , testStr1
            , XMLString::stringLen(testStr1)
            , testStr2
            , XMLString::stringLen(testStr2)
        );

        if (!result)
            return false;
    }

    // Test the intrinsic UTF-16 transcoder, in little endian
    {
    }

    // Test the intrinsic UTF-16 transcoder, in big endian
    {
    }


    return true;
}


// ---------------------------------------------------------------------------
//  Test entry point
// ---------------------------------------------------------------------------
bool testTranscoders()
{
    outStrm << "----------------------------------\n"
            << "Testing transcoder classes\n"
            << "----------------------------------" << EndLn;

    bool retVal = true;

    try
    {
        // Call other local methods to do specific tests
        outStrm << "Testing basic trans service functions" << EndLn;
        if (!basicTransServiceTests())
        {
            outStrm << "TransService basic tests failed" << EndLn;
            retVal = false;
        }
         else
        {
            outStrm << "TransService basic tests passed" << EndLn;
        }
        outStrm << EndLn;

        outStrm << "Testing basic transcoder functions" << EndLn;
        if (!basicTranscoderTests())
        {
            outStrm << "Transcoder basic tests failed" << EndLn;
            retVal = false;
        }
         else
        {
            outStrm << "Transcoder basic tests passed" << EndLn;
        }
        outStrm << EndLn;
    }

    catch(const XMLException& toCatch)
    {
        outStrm << "  ERROR: Unexpected exception!\n   Msg: "
                << toCatch.getMessage() << EndLn;
        return false;
    }
    return retVal;
}
