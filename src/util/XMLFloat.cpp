/*
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2001 The Apache Software Foundation.  All rights
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
 * originally based on software copyright (c) 2001, International
 * Business Machines, Inc., http://www.ibm.com .  For more information
 * on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */

/*
 * $Id$
 * $Log$
 * Revision 1.5  2001/08/14 22:10:20  peiyongz
 * new exception message added
 *
 * Revision 1.4  2001/07/31 17:38:16  peiyongz
 * Fix: memory leak by static (boundry) objects
 *
 * Revision 1.3  2001/07/31 13:48:29  peiyongz
 * fValue removed
 *
 * Revision 1.2  2001/07/27 20:43:53  peiyongz
 * copy ctor: to check for special types.
 *
 * Revision 1.1  2001/07/26 20:41:37  peiyongz
 * XMLFloat
 *
 *
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <util/XMLFloat.hpp>
#include <util/PlatformUtils.hpp>
#include <util/XMLDeleterFor.hpp>
#include <util/XMLString.hpp>
#include <util/XMLUniDefs.hpp>
#include <util/NumberFormatException.hpp>
#include <util/TransService.hpp>
#include <util/Janitor.hpp>
#include <util/XMLUni.hpp>
#include <math.h>

//---------
// TODO:
//  
//   Double is supposed to hold 64 bit number and currently
//   we have implemented the checking on boundary values, such
//   the maximum/minimum positive/negative number.
//
//   We also need to do roundup/roundoff for extra bit in the 
//   lexical representation.
//   In order to do that, we need to store them in a canonical form,
//   like this.
//   [-]?[1-9].[0-9]+[E|e][-]?[0-9]+
//   
//   Then we can impose the limit of length after the '.' before 'E'.
//   
//---------

// ---------------------------------------------------------------------------
//  class data member
// ---------------------------------------------------------------------------
bool XMLFloat::isInitialized = false;

// ---------------------------------------------------------------------------
//  local data member
// ---------------------------------------------------------------------------
static const int BUF_LEN = 64;
static XMLCh value1[BUF_LEN+1];

// from <FLOAT.h>
//
//FLT_MAX Maximum representable floating-point number
//3.402823466e+38
//3.4028 23466 e+38
//
static const XMLCh FLT_MAX_POSITIVE[] =
{
    chDigit_3, chPeriod,  chDigit_4, chDigit_0, chDigit_2, chDigit_8,
    chDigit_2, chDigit_3, chDigit_4, chDigit_6, chDigit_6,         
    chLatin_E, chPlus,    chDigit_3, chDigit_8, chNull        
};

//
//FLT_MIN Minimum positive value. 
//1.175494351e-38
//1.1754 94351 e-38
//
static const XMLCh FLT_MIN_POSITIVE[] =
{
    chDigit_1, chPeriod,  chDigit_1, chDigit_7, chDigit_5, chDigit_4,
    chDigit_9, chDigit_4, chDigit_3, chDigit_5, chDigit_1, 
    chLatin_E, chDash,    chDigit_3, chDigit_8, chNull    
};

//
// Deduced from FLT_MAX_POSITIVE
//3.402823466e+38
//
static const XMLCh FLT_MAX_NEGATIVE[] =
{
    chDash,
    chDigit_3, chPeriod,  chDigit_4, chDigit_0, chDigit_2, chDigit_8,
    chDigit_2, chDigit_3, chDigit_4, chDigit_6, chDigit_6,         
    chLatin_E, chPlus,    chDigit_3, chDigit_8, chNull        
};

//
// Deduced from FLT_MIN_POSITIVE
//1.175494351e-38
//
static const XMLCh FLT_MIN_NEGATIVE[] = 
{
    chDash,
    chDigit_1, chPeriod,  chDigit_1, chDigit_7, chDigit_5, chDigit_4,
    chDigit_9, chDigit_4, chDigit_3, chDigit_5, chDigit_1, 
    chLatin_E, chDash,    chDigit_3, chDigit_8, chNull    
};

//
// maxNegativeValue < minNegativeValue < 0 < minPositiveValue < maxPositiveValue
// They are all "Inclusive value"
//

static XMLFloat*  maxNegativeValue;
static XMLFloat*  minNegativeValue; 
static XMLFloat*  minPositiveValue; 
static XMLFloat*  maxPositiveValue;

/***
 *   Algo:
 *
 *   . Check for special cases
 *   . Construct fMantissa, fExponent (optional)
 *   .   Checking boundary
 *   .
***/
XMLFloat::XMLFloat(const XMLCh* const strValue)
:fMantissa(0)
,fExponent(0)
,fType(Normal)
{
    try
    {
        init(strValue);
    }
    catch (XMLException&)
    {
        cleanUp();
        throw;
    }

}

void XMLFloat::init(const XMLCh* const strValue)
{
    if ((!strValue) || (!*strValue))
        ThrowXML(NumberFormatException, XMLExcepts::XMLNUM_emptyString);

    XMLCh* tmpStrValue = XMLString::replicate(strValue);
    ArrayJanitor<XMLCh> janTmpName(tmpStrValue);
    XMLString::trim(tmpStrValue);

    if (XMLString::compareString(tmpStrValue, XMLUni::fgNegINFString) == 0)
    {
        fType = NegINF;
        return;
    }
    else if (XMLString::compareString(tmpStrValue, XMLUni::fgNegZeroString) == 0)
    {
        fType = NegZero;
        return;
    }
    else if (XMLString::compareString(tmpStrValue, XMLUni::fgPosZeroString) == 0)
    {
        fType = PosZero;
        return;
    }
    else if (XMLString::compareString(tmpStrValue, XMLUni::fgPosINFString) == 0)
    {
        fType = PosINF;
        return;
    }
    else if (XMLString::compareString(tmpStrValue, XMLUni::fgNaNString) == 0)
    {
        fType = NaN;
        return;
    }

    //
    // Normal case
    //
    int ePos = 0;
    int tmpStrLen = XMLString::stringLen(tmpStrValue);

    if ((( ePos = XMLString::indexOf(tmpStrValue, chLatin_E)) != -1 ) ||
        (( ePos = XMLString::indexOf(tmpStrValue, chLatin_e)) != -1 )  )
    {
        XMLCh* tmpMantissa = new XMLCh [ePos+1];
        XMLString::subString(tmpMantissa, tmpStrValue, 0, ePos);
        ArrayJanitor<XMLCh> janMantissa(tmpMantissa);
        fMantissa = new XMLBigDecimal(tmpMantissa);

        //
        // 1234.56E78
        // 
        if ( ePos < tmpStrLen - 1)
        {
            XMLCh* tmpExponent = new XMLCh [tmpStrLen - ePos];
            XMLString::subString(tmpExponent, tmpStrValue, ePos + 1, tmpStrLen);
            ArrayJanitor<XMLCh> janExponent(tmpExponent);
            fExponent = new XMLBigInteger(tmpExponent);
        }
        //
        // 1234.56E
        // 
        else
        {        
            fExponent = new XMLBigInteger(XMLUni::fgZeroString);
        }

    }
    else
    {
        fMantissa = new XMLBigDecimal(tmpStrValue);
        fExponent = new XMLBigInteger(XMLUni::fgZeroString);
    }

    checkBoundary(tmpStrValue);
}

//
//
void XMLFloat::checkBoundary(const XMLCh* const strValue)
{
    if (!isInitialized)
    {
        isInitialized = true;  // set first to avoid recursion

        maxNegativeValue = new XMLFloat(FLT_MAX_NEGATIVE);        
        XMLPlatformUtils::registerLazyData
        (
            new XMLDeleterFor<XMLFloat>(maxNegativeValue)
        );        

        minNegativeValue = new XMLFloat(FLT_MIN_NEGATIVE); 
        XMLPlatformUtils::registerLazyData
        (
            new XMLDeleterFor<XMLFloat>(minNegativeValue)
        );        

        minPositiveValue = new XMLFloat(FLT_MIN_POSITIVE); 
        XMLPlatformUtils::registerLazyData
        (
            new XMLDeleterFor<XMLFloat>(minPositiveValue)
        );        

        maxPositiveValue = new XMLFloat(FLT_MAX_POSITIVE);
        XMLPlatformUtils::registerLazyData
        (
            new XMLDeleterFor<XMLFloat>(maxPositiveValue)
        );        
    }

    //
    // by-pass boundary check for boundary value itself
    //
    if (( XMLString::compareString(strValue, FLT_MAX_NEGATIVE) == 0 ) ||
        ( XMLString::compareString(strValue, FLT_MIN_NEGATIVE) == 0 ) ||
        ( XMLString::compareString(strValue, FLT_MIN_POSITIVE) == 0 ) ||
        ( XMLString::compareString(strValue, FLT_MAX_POSITIVE) == 0 )  )
        return;

    //  error: this < maxNegativeValue 
    if ( compareValues(this, maxNegativeValue) == -1 )
    {
        ThrowXML2(NumberFormatException
                , XMLExcepts::XMLNUM_DBL_FLT_maxNeg
                , strValue
                , FLT_MAX_NEGATIVE);
    }

    //  error: this > maxPositiveValue
    if ( compareValues(this, maxPositiveValue) ==  1 )     
    {
        ThrowXML2(NumberFormatException
                , XMLExcepts::XMLNUM_DBL_FLT_maxPos
                , strValue
                , FLT_MAX_POSITIVE);
    }

    //  error: minNegativeValue < this < minPositiveValue
    //  value is not be representable
    if  (( compareValues(this, minNegativeValue) ==  1 ) &&
         ( compareValues(this, minPositiveValue) == -1 )  )
    {
        ThrowXML3(NumberFormatException
                , XMLExcepts::XMLNUM_DBL_FLT_minNegPos
                , strValue
                , FLT_MIN_NEGATIVE
                , FLT_MIN_POSITIVE);
    }

}

XMLFloat::XMLFloat(const XMLFloat& toCopy)
:fMantissa(0)
,fExponent(0)
,fType(Normal)
{
    if (!toCopy.isSpecialValue())
    {
        fMantissa = new XMLBigDecimal(*(toCopy.fMantissa));
        fExponent = new XMLBigInteger(*(toCopy.fExponent));
    }

    fType  = toCopy.fType;
}

//
// Add the 'E' as necessary
// The caller needs to de-allocate the memory allocated by this function
// Deallocate the memory allocated by XMLBigInteger
//
XMLCh*  XMLFloat::toString() const
{
    switch (fType) 
    {
    case NegINF:
        return XMLString::replicate(XMLUni::fgNegINFString);

    case NegZero:
        return XMLString::replicate(XMLUni::fgNegZeroString);

    case PosZero:
        return XMLString::replicate(XMLUni::fgPosZeroString);

    case PosINF:
        return XMLString::replicate(XMLUni::fgPosINFString);

    case NaN:
        return XMLString::replicate(XMLUni::fgNaNString);

    case Normal:

        XMLCh *ret_mantissa;
        ret_mantissa = fMantissa->toString();

        if (fExponent)
        {
            XMLCh *ret_exponent = fExponent->toString();
            XMLCh *ret_val = new XMLCh [ XMLString::stringLen(ret_mantissa) + 
                                         XMLString::stringLen(ret_exponent) + 2];
            *ret_val = 0;
            XMLString::catString(ret_val, ret_mantissa);
            XMLString::catString(ret_val, XMLUni::fgEString);
            XMLString::catString(ret_val, ret_exponent);

            delete [] ret_mantissa;
            delete [] ret_exponent;
            return ret_val;
        }
        else
            return ret_mantissa;

        break;
    default:
        XMLString::binToText(fType, value1, 16, 10);
        ThrowXML1(NumberFormatException
                , XMLExcepts::XMLNUM_DBL_FLT_InvalidType
                , value1);
    };

    return (XMLCh*) 0;
}

//
//
//
int XMLFloat::compareValues(const XMLFloat* const lValue
                           , const XMLFloat* const rValue)
{
    //
    // case#1: lValue normal
    //         rValue normal
    //
    if ((!lValue->isSpecialValue()) &&
        (!rValue->isSpecialValue())  )
    {
        //
        // if we use fValue to compare two 
        // sequences "12.3456E4" and "1234.56E2",
        // they are _NOT_ the same. so we ask
        // BigDecimal to compare.
        //
        XMLBigDecimal ldv = XMLBigDecimal(*(lValue->fMantissa), lValue->fExponent->intValue());
        XMLBigDecimal rdv = XMLBigDecimal(*(rValue->fMantissa), rValue->fExponent->intValue());

        return XMLBigDecimal::compareValues(&ldv, &rdv);
    }
    //
    // case#2: lValue special
    //         rValue special
    //
    else
    if ((lValue->isSpecialValue()) &&
        (rValue->isSpecialValue())  )
    {
        if (lValue->fType == rValue->fType)
            return 0;
        else
            return (lValue->fType > rValue->fType ? 1 : -1);
    }
    //
    // case#3: lValue special
    //         rValue normal
    //
    else
    if ((lValue->isSpecialValue()) &&
        (!rValue->isSpecialValue())  )
    {
        return compareSpecial(lValue, rValue);
    }
    //
    // case#4: lValue normal
    //         rValue special
    //
    else
    {
        return (-1) * compareSpecial(rValue, lValue);
    }

    return 0;
}

int XMLFloat::compareSpecial(const XMLFloat* const specialValue
                            , const XMLFloat* const normalValue)
{
    switch (specialValue->fType) 
    {
    case NegINF:
        return -1;

    case NegZero:
    case PosZero:
        return (normalValue->getSign() > 0 ? -1 : 1);

    case PosINF:
        return 1;

    case NaN:
        return 1;

    default:
        XMLString::binToText(specialValue->fType, value1, 16, 10);
        ThrowXML1(NumberFormatException
                , XMLExcepts::XMLNUM_DBL_FLT_InvalidType
                , value1);
        //internal error
    }
}