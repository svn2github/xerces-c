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
 * Revision 1.2  2002/02/14 15:17:31  peiyongz
 * getEnumString()
 *
 * Revision 1.1.1.1  2002/02/01 22:22:42  peiyongz
 * sane_include
 *
 * Revision 1.5  2001/10/02 18:59:29  peiyongz
 * Invalid_Facet_Tag to display the tag name
 *
 * Revision 1.4  2001/09/20 13:11:42  knoaman
 * Regx  + misc. fixes
 *
 * Revision 1.3  2001/08/21 18:42:54  peiyongz
 * Bugzilla# 2816: cleanUp() declared with external linkage and called
 *                          before defined as inline
 *
 * Revision 1.2  2001/08/14 22:11:56  peiyongz
 * new exception message added
 *
 * Revision 1.1  2001/07/13 14:10:40  peiyongz
 * UnionDTV
 *
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/datatype/UnionDatatypeValidator.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeFacetException.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeValueException.hpp>
#include <xercesc/util/NumberFormatException.hpp>

static const int BUF_LEN = 64;
static XMLCh value1[BUF_LEN+1];
static XMLCh value2[BUF_LEN+1];

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
UnionDatatypeValidator::UnionDatatypeValidator()
:DatatypeValidator(0, 0, 0, DatatypeValidator::Union)
,fEnumerationInherited(false)
,fEnumeration(0)
,fMemberTypeValidators(0)
{}

UnionDatatypeValidator::~UnionDatatypeValidator()
{
    cleanUp();
}

UnionDatatypeValidator::UnionDatatypeValidator(
                        RefVectorOf<DatatypeValidator>* const memberTypeValidators
                      , const int                                         finalSet)
:DatatypeValidator(0, 0, finalSet, DatatypeValidator::Union)
,fEnumerationInherited(false)
,fEnumeration(0)
,fMemberTypeValidators(0)
{
    if ( !memberTypeValidators )
    {
        ThrowXML(InvalidDatatypeFacetException
               , XMLExcepts::FACET_Union_Null_memberTypeValidators);
    }

    // no pattern, no enumeration
    fMemberTypeValidators = memberTypeValidators;
}

UnionDatatypeValidator::UnionDatatypeValidator(
                          DatatypeValidator*            const baseValidator
                        , RefHashTableOf<KVStringPair>* const facets
                        , RefVectorOf<XMLCh>*           const enums
                        , const int                           finalSet)
:DatatypeValidator(baseValidator, facets, finalSet, DatatypeValidator::Union)
,fEnumerationInherited(false)
,fEnumeration(0)
,fMemberTypeValidators(0)
{
    //
    // baseValidator another UnionDTV from which, 
    // this UnionDTV is derived by restriction.
    // it shall be not null
    //
    if (!baseValidator)
    {
        ThrowXML(InvalidDatatypeFacetException
               , XMLExcepts::FACET_Union_Null_baseValidator);
    }

    if (baseValidator->getType() != DatatypeValidator::Union)
    {
        XMLString::binToText(baseValidator->getType(), value1, BUF_LEN, 10);
        ThrowXML1(InvalidDatatypeFacetException
                , XMLExcepts::FACET_Union_invalid_baseValidatorType
                , value1);
    }

    try
    {
        init(baseValidator, facets, enums);
    }
    catch (...)
    {
        cleanUp();
        throw;
    }
}

void UnionDatatypeValidator::init(DatatypeValidator*            const baseValidator
                                , RefHashTableOf<KVStringPair>* const facets
                                , RefVectorOf<XMLCh>*           const enums)
{
    if (enums)
        setEnumeration(enums, false);

    // Set Facets if any defined
    if (facets)
    {
        XMLCh* key;
        XMLCh* value;
        RefHashTableOfEnumerator<KVStringPair> e(facets);

        while (e.hasMoreElements())
        {
            KVStringPair pair = e.nextElement();
            key = pair.getKey();
            value = pair.getValue();

            if (XMLString::compareString(key, SchemaSymbols::fgELT_PATTERN)==0)
            {
                setPattern(value);
                if (getPattern())
                    setFacetsDefined(DatatypeValidator::FACET_PATTERN);
                // do not construct regex until needed
            }
            else
            {
                 ThrowXML1(InvalidDatatypeFacetException
                         , XMLExcepts::FACET_Invalid_Tag
                         , key);
            }
        }//while

        /***
           Schema constraint: Part I -- self checking
        ***/
        // Nil

        /***
           Schema constraint: Part II base vs derived checking
        ***/
        // check 4.3.5.c0 must: enumeration values from the value space of base
        if ( ((getFacetsDefined() & DatatypeValidator::FACET_ENUMERATION) != 0) &&
            (getEnumeration() !=0))
        {
            int i = 0;
            int enumLength = getEnumeration()->size();
            try
            {
                for ( ; i < enumLength; i++)
                {
                    // ask parent do a complete check
                    //
                    // enum need NOT be passed this->checkContent()
                    // since there are no other facets for Union, parent
                    // checking is good enough.
                    //
                    baseValidator->validate(getEnumeration()->elementAt(i));

                }
            }

            catch ( XMLException& )
            {
                ThrowXML1(InvalidDatatypeFacetException
                            , XMLExcepts::FACET_enum_base
                            , getEnumeration()->elementAt(i));
            }
        }

    }// End of Facet setting

    /***
        Inherit facets from base.facets

        The reason of this inheriting (or copying values) is to ease
        schema constraint checking, so that we need NOT trace back to our
        very first base validator in the hierachy. Instead, we are pretty
        sure checking against immediate base validator is enough.  
    ***/
   
    UnionDatatypeValidator *pBaseValidator = (UnionDatatypeValidator*) baseValidator;

    // inherit enumeration
    if (((pBaseValidator->getFacetsDefined() & DatatypeValidator::FACET_ENUMERATION) !=0) &&
        ((getFacetsDefined() & DatatypeValidator::FACET_ENUMERATION) == 0))
    {
        setEnumeration(pBaseValidator->getEnumeration(), true);
    }

}

//
// 1) the bottom level UnionDTV would check against 
//        pattern and enumeration as well
// 2) each UnionDTV(s) above the bottom level UnionDTV and
//        below the native UnionDTV (the top level DTV)
//        would check against pattern only.
// 3) the natvie Union DTV (the top level DTV) would invoke
//        memberTypeValidator to validate
//
void UnionDatatypeValidator::checkContent(const XMLCh* const content, bool asBase)
{
    DatatypeValidator* bv = getBaseValidator();
    if (bv)
        ((UnionDatatypeValidator*)bv)->checkContent(content, true);
    else
    {   // 3) native union type
        // check content against each member type validator in Union
        // report an error only in case content is not valid against all member datatypes.
        //
        bool memTypeValid = false;
        for ( unsigned int i = 0; i < fMemberTypeValidators->size(); ++i )
        {  
            if ( memTypeValid ) 
                break;

            try 
            {
                fMemberTypeValidators->elementAt(i)->validate(content);
                memTypeValid = true;
            }
            catch (XMLException&)
            {
                //absorbed
            }  
        } // for

        if ( !memTypeValid ) 
        {
            ThrowXML1(InvalidDatatypeValueException
                    , XMLExcepts::VALUE_no_match_memberType
                    , content);
            //( "Content '"+content+"' does not match any union types" );  
        }
    }

    // 1) and 2). we check pattern first
    if ( (getFacetsDefined() & DatatypeValidator::FACET_PATTERN ) != 0 )
    {
        // lazy construction
        if (getRegex() == 0) 
        {
            try {
                setRegex(new RegularExpression(getPattern(), SchemaSymbols::fgRegEx_XOption));
            }
            catch (XMLException &e)
            {
                ThrowXML1(InvalidDatatypeValueException, XMLExcepts::RethrowError, e.getMessage());
            }
        }

        if (getRegex()->matches(content) == false)
        {
            ThrowXML2(InvalidDatatypeValueException
                    , XMLExcepts::VALUE_NotMatch_Pattern
                    , content
                    , getPattern());
        }
    }

    // if this is a base validator, we only need to check pattern facet
    // all other facet were inherited by the derived type
    if (asBase)
        return;

    if ((getFacetsDefined() & DatatypeValidator::FACET_ENUMERATION) != 0 &&
        (getEnumeration() != 0))
    {

        // If the content match (compare equal) any enumeration with 
        // any of the member types, it is considerd valid.
        //
        RefVectorOf<DatatypeValidator>* memberDTV = getMemberTypeValidators();      
        RefVectorOf<XMLCh>* tmpEnum = getEnumeration();
        unsigned int memberTypeNumber = memberDTV->size();
        unsigned int enumLength = tmpEnum->size();

        for ( unsigned int memberIndex = 0; memberIndex < memberTypeNumber; ++memberIndex)
        {           
            for ( unsigned int enumIndex = 0; enumIndex < enumLength; ++enumIndex)
            {  
                try 
                {
                    if (memberDTV->elementAt(memberIndex)->compare(content, tmpEnum->elementAt(enumIndex)) == 0)
                        return;
                }
                catch (XMLException&)
                {
                    //absorbed
                }  
            } // for enumIndex
        } // for memberIndex

        ThrowXML1(InvalidDatatypeValueException, XMLExcepts::VALUE_NotIn_Enumeration, content);

    } // enumeration

}

//
//
//
int UnionDatatypeValidator::compare(const XMLCh* const lValue
                                  , const XMLCh* const rValue)
{
    RefVectorOf<DatatypeValidator>* memberDTV = getMemberTypeValidators();      
    unsigned int memberTypeNumber = memberDTV->size();

    for ( unsigned int memberIndex = 0; memberIndex < memberTypeNumber; ++memberIndex)
    {
        if (memberDTV->elementAt(memberIndex)->compare(lValue, rValue) ==0)
            return  0;
    }

    //REVISIT: what does it mean for UNION1 to be <less than> or <greater than> UNION2 ?    
    // As long as -1 or +1 indicates an unequality, return either of them is ok.
    return -1;        
}

const RefVectorOf<XMLCh>* UnionDatatypeValidator::getEnumString() const
{
	return getEnumeration();
}
/**
  * End of file UnionDatatypeValidator.cpp
  */
