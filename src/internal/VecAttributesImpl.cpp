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
 * Revision 1.1  2000/08/02 18:09:14  jpolast
 * initial checkin: attributes vector needed for
 * Attributes class as defined by sax2 spec
 * 
 *
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <util/Janitor.hpp>
#include <internal/VecAttributesImpl.hpp>


// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
VecAttributesImpl::VecAttributesImpl() :

    fAdopt(false)
    , fCount(0)
    , fVector(0)
	, fValidator(0)
{
}

VecAttributesImpl::~VecAttributesImpl()
{
    //
    //  Note that some compilers can't deal with the fact that the pointer
    //  is to a const object, so we have to cast off the const'ness here!
    //
    if (fAdopt)
        delete (RefVectorOf<XMLAttr>*)fVector;
}


// ---------------------------------------------------------------------------
//  Implementation of the attribute list interface
// ---------------------------------------------------------------------------
unsigned int VecAttributesImpl::getLength() const
{
    return fCount;
}

const XMLCh* VecAttributesImpl::getURI(const unsigned int index) 
{
	// this func really needs to be const, like the rest, not sure how we
	// make it const and re-use the fURIBuffer member variable
    if (index >= fCount)
        ThrowXML(ArrayIndexOutOfBoundsException, XMLExcepts::AttrList_BadIndex);
    fValidator->getURIText(fVector->elementAt(index)->getURIId(), fURIBuffer) ;
	return fURIBuffer.getRawBuffer() ;
}

const XMLCh* VecAttributesImpl::getLocalName(const unsigned int index) const 
{
    if (index >= fCount)
        ThrowXML(ArrayIndexOutOfBoundsException, XMLExcepts::AttrList_BadIndex);
    return fVector->elementAt(index)->getName(); 
}

const XMLCh* VecAttributesImpl::getQName(const unsigned int index) const 
{
    if (index >= fCount)
        ThrowXML(ArrayIndexOutOfBoundsException, XMLExcepts::AttrList_BadIndex);
    return fVector->elementAt(index)->getQName(); 
}

const XMLCh* VecAttributesImpl::getType(const unsigned int index) const
{
    if (index >= fCount)
        ThrowXML(ArrayIndexOutOfBoundsException, XMLExcepts::AttrList_BadIndex);
    return XMLAttDef::getAttTypeString(fVector->elementAt(index)->getType());
}

const XMLCh* VecAttributesImpl::getValue(const unsigned int index) const
{
    if (index >= fCount)
        ThrowXML(ArrayIndexOutOfBoundsException, XMLExcepts::AttrList_BadIndex);
    return fVector->elementAt(index)->getValue();
}

int VecAttributesImpl::getIndex(const XMLCh* const uri, const XMLCh* const localPart ) const 
{
    //
    //  Search the vector for the attribute with the given name and return
    //  its type.
    //
	XMLBuffer uriBuffer ;
    for (unsigned int index = 0; index < fCount; index++)
    {
        const XMLAttr* curElem = fVector->elementAt(index);
		
		fValidator->getURIText(curElem->getURIId(), uriBuffer) ;

        if ( (!XMLString::compareString(curElem->getName(), localPart)) &&
			 (!XMLString::compareString(uriBuffer.getRawBuffer(), uri)) )
			return index ;
    }
    return -1; 
}

int VecAttributesImpl::getIndex(const XMLCh* const QName ) const 
{
    //
    //  Search the vector for the attribute with the given name and return
    //  its type.
    //
    for (unsigned int index = 0; index < fCount; index++)
    {
        const XMLAttr* curElem = fVector->elementAt(index);

        if (!XMLString::compareString(curElem->getQName(), QName))
            return index ;
    }
    return -1; 
}

const XMLCh* VecAttributesImpl::getType(const XMLCh* const uri, const XMLCh* const localPart ) const 
{
	return getType(getIndex(uri, localPart)) ;
}

const XMLCh* VecAttributesImpl::getType(const XMLCh* const QName) const 
{
	return getType(getIndex(QName)) ;
}

const XMLCh* VecAttributesImpl::getValue(const XMLCh* const uri, const XMLCh* const localPart ) const 
{
	return getValue(getIndex(uri, localPart)) ;
}

const XMLCh* VecAttributesImpl::getValue(const XMLCh* const QName) const 
{
	return getValue(getIndex(QName)) ;
}

// ---------------------------------------------------------------------------
//  Setter methods
// ---------------------------------------------------------------------------
void VecAttributesImpl::setVector(const   RefVectorOf<XMLAttr>* const srcVec
                                , const unsigned int                count
								, const XMLValidator * const		validator
                                , const bool                        adopt)
{
    //
    //  Delete the previous vector (if any) if we are adopting. Note that some
    //  compilers can't deal with the fact that the pointer is to a const
    //  object, so we have to cast off the const'ness here!
    //
    if (fAdopt)
        delete (RefVectorOf<XMLAttr>*)fVector;

    fAdopt = adopt;
    fCount = count;
    fVector = srcVec;
	fValidator = validator ;
}

