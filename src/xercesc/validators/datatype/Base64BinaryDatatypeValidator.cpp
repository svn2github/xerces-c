/*
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2001-2003 The Apache Software Foundation.  All rights
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
 * $Log$
 * Revision 1.6  2003/05/15 18:53:26  knoaman
 * Partial implementation of the configurable memory manager.
 *
 * Revision 1.5  2003/01/27 19:24:17  peiyongz
 * normalize Base64 data before checking against enumeration.
 *
 * Revision 1.4  2003/01/24 23:18:34  peiyongz
 * normalizeEnumeration() added to remove optional ws in Base64 data.
 *
 * Revision 1.3  2002/12/18 14:17:55  gareth
 * Fix to bug #13438. When you eant a vector that calls delete[] on its members you should use RefArrayVectorOf.
 *
 * Revision 1.2  2002/11/04 14:53:28  tng
 * C++ Namespace Support.
 *
 * Revision 1.1.1.1  2002/02/01 22:22:40  peiyongz
 * sane_include
 *
 * Revision 1.12  2001/10/09 20:47:26  peiyongz
 * init(): take 1 arg
 *
 * Revision 1.11  2001/10/02 18:59:29  peiyongz
 * Invalid_Facet_Tag to display the tag name
 *
 * Revision 1.10  2001/09/24 15:33:15  peiyongz
 * DTV Reorganization: virtual methods moved to *.cpp
 *
 * Revision 1.9  2001/09/19 18:49:40  peiyongz
 * DTV reorganization: inherit from AbstractStringVaildator
 *
 * Revision 1.8  2001/08/21 18:42:53  peiyongz
 * Bugzilla# 2816: cleanUp() declared with external linkage and called
 *                          before defined as inline
 *
 * Revision 1.7  2001/06/20 17:56:47  peiyongz
 * support for "fixed" option on constrainning facets
 *
 * Revision 1.6  2001/05/29 19:49:31  tng
 * Schema: Constraint Checking Fix in datatypeValidators.  By Pei Yong Zhang.
 *
 * Revision 1.5  2001/05/28 21:11:17  tng
 * Schema: Various DatatypeValidator fix.  By Pei Yong Zhang
 *
 * Revision 1.4  2001/05/18 13:36:41  tng
 * Schema: Catch RegularExpression exception and NumberFormatException
 *
 * Revision 1.3  2001/05/18 13:23:43  tng
 * Schema: Exception messages in DatatypeValidator.  By Pei Yong Zhang.
 *
 * Revision 1.2  2001/05/17 18:13:46  tng
 * Schema Fix: issue error message when binary data is invalid.
 *
 * Revision 1.1  2001/05/16 15:24:40  tng
 * Schema: Add Base64 and HexBin.  By Pei Yong Zhang.
 *
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/datatype/Base64BinaryDatatypeValidator.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeFacetException.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeValueException.hpp>
#include <xercesc/util/Base64.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
Base64BinaryDatatypeValidator::Base64BinaryDatatypeValidator(MemoryManager* const manager)
:AbstractStringValidator(0, 0, 0, DatatypeValidator::Base64Binary, manager)
{}

Base64BinaryDatatypeValidator::~Base64BinaryDatatypeValidator()
{}

Base64BinaryDatatypeValidator::Base64BinaryDatatypeValidator(
                          DatatypeValidator*            const baseValidator
                        , RefHashTableOf<KVStringPair>* const facets
                        , RefArrayVectorOf<XMLCh>*      const enums
                        , const int                           finalSet
                        , MemoryManager* const                manager)
:AbstractStringValidator(baseValidator, facets, finalSet, DatatypeValidator::Base64Binary, manager)
{
    init(enums);
}

DatatypeValidator* Base64BinaryDatatypeValidator::newInstance
(
      RefHashTableOf<KVStringPair>* const facets
    , RefArrayVectorOf<XMLCh>* const      enums
    , const int                           finalSet
    , MemoryManager* const                manager
)
{
    return (DatatypeValidator*) new (manager) Base64BinaryDatatypeValidator(this, facets, enums, finalSet, manager);
}

// ---------------------------------------------------------------------------
//  Utilities
// ---------------------------------------------------------------------------
void Base64BinaryDatatypeValidator::assignAdditionalFacet( const XMLCh* const key
                                                         , const XMLCh* const)
{
    ThrowXML1(InvalidDatatypeFacetException
            , XMLExcepts::FACET_Invalid_Tag
            , key);
}

void Base64BinaryDatatypeValidator::inheritAdditionalFacet()
{}

void Base64BinaryDatatypeValidator::checkAdditionalFacetConstraints() const
{}

void Base64BinaryDatatypeValidator::checkAdditionalFacet(const XMLCh* const) const
{}

void Base64BinaryDatatypeValidator::checkValueSpace(const XMLCh* const content)
{
    if (getLength(content) <= 0)
    {
        ThrowXML1(InvalidDatatypeValueException
                , XMLExcepts::VALUE_Not_Base64
                , content);
    }
}

int Base64BinaryDatatypeValidator::getLength(const XMLCh* const content) const
{
    return Base64::getDataLength(content);
}

void Base64BinaryDatatypeValidator::normalizeEnumeration()
{

    int enumLength = getEnumeration()->size();
    for ( int i=0; i < enumLength; i++)
    {
        XMLString::removeWS(getEnumeration()->elementAt(i));
    }

}

void Base64BinaryDatatypeValidator::normalizeContent(XMLCh* const content) const
{
    XMLString::removeWS(content);     
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file Base64BinaryDatatypeValidator.cpp
  */
