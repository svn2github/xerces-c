/*
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2001-2002 The Apache Software Foundation.  All rights
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
 */

#include <xercesc/util/XMLUniDefs.hpp>
#include "IDAttrNSImpl.hpp"
#include "IDDocumentImpl.hpp"
#include "IDOM_Document.hpp"
#include "IDOM_DOMException.hpp"

#include "assert.h"

IDAttrNSImpl::IDAttrNSImpl(IDOM_Document *ownerDoc, const XMLCh *nam) :
IDAttrImpl(ownerDoc, nam)
{
    this->fNamespaceURI=0;	//DOM Level 2
    this->fLocalName=0;       //DOM Level 2
    this->fPrefix=0;
}

//Introduced in DOM Level 2
IDAttrNSImpl::IDAttrNSImpl(IDOM_Document *ownerDoc,
                           const XMLCh *namespaceURI,
                           const XMLCh *qualifiedName) :
IDAttrImpl(ownerDoc, qualifiedName)
{
    const XMLCh * xmlns = IDNodeImpl::getXmlnsString();
    const XMLCh * xmlnsURI = IDNodeImpl::getXmlnsURIString();
    this->fName = ((IDDocumentImpl *)ownerDoc)->getPooledString(qualifiedName);

    int index = IDDocumentImpl::indexofQualifiedName(qualifiedName);
    if (index < 0)
        throw IDOM_DOMException(IDOM_DOMException::NAMESPACE_ERR, 0);

    bool xmlnsAlone = false;	//true if attribute name is "xmlns"
    if (index == 0) {	//qualifiedName contains no ':'
        if (XMLString::compareString(this->fName, xmlns) == 0) {
            if (XMLString::compareString(namespaceURI, xmlnsURI) != 0)
                throw IDOM_DOMException(IDOM_DOMException::NAMESPACE_ERR, 0);
            xmlnsAlone = true;
        }
        this -> fPrefix = 0;
        this -> fLocalName = this -> fName;
    } else {	//0 < index < this->name.length()-1
        XMLCh* newName;
        XMLCh temp[4000];
        if (index >= 3999)
            newName = new XMLCh[XMLString::stringLen(qualifiedName)+1];
        else
            newName = temp;

        XMLString::copyNString(newName, fName, index);
        newName[index] = chNull;
        this-> fPrefix = ((IDDocumentImpl *)ownerDoc)->getPooledString(newName);
        this -> fLocalName = ((IDDocumentImpl *)ownerDoc)->getPooledString(fName+index+1);

        if (index >= 3999)
            delete[] newName;
    }

    const XMLCh * URI = xmlnsAlone ?
                xmlnsURI : IDNodeImpl::mapPrefix(fPrefix, namespaceURI, IDOM_Node::ATTRIBUTE_NODE);
    this -> fNamespaceURI = (URI == 0) ? 0 : ((IDDocumentImpl *)ownerDoc)->getPooledString(URI);
};

IDAttrNSImpl::IDAttrNSImpl(const IDAttrNSImpl &other, bool deep) :
IDAttrImpl(other, deep)
{
    this->fNamespaceURI = other.fNamespaceURI;	//DOM Level 2
    this->fLocalName = other.fLocalName;          //DOM Level 2
    this->fPrefix = other.fPrefix;
};

IDOM_Node * IDAttrNSImpl::cloneNode(bool deep) const
{
    return new (getOwnerDocument()) IDAttrNSImpl(*this, deep);
};

const XMLCh * IDAttrNSImpl::getNamespaceURI() const
{
    return fNamespaceURI;
}

const XMLCh * IDAttrNSImpl::getPrefix() const
{
    return fPrefix;
}

const XMLCh * IDAttrNSImpl::getLocalName() const
{
    return fLocalName;
}

void IDAttrNSImpl::setPrefix(const XMLCh *prefix)
{
    const XMLCh * xml = IDNodeImpl::getXmlString();
    const XMLCh * xmlURI = IDNodeImpl::getXmlURIString();
    const XMLCh * xmlns = IDNodeImpl::getXmlnsString();
    const XMLCh * xmlnsURI = IDNodeImpl::getXmlnsURIString();

    if (fNode.isReadOnly())
        throw IDOM_DOMException(IDOM_DOMException::NO_MODIFICATION_ALLOWED_ERR,
        0);
    if (fNamespaceURI == 0 || fNamespaceURI[0] == chNull || XMLString::compareString(fLocalName, xmlns) == 0)
        throw IDOM_DOMException(IDOM_DOMException::NAMESPACE_ERR, 0);

    if (prefix != 0 && !IDDocumentImpl::isXMLName(prefix))
        throw IDOM_DOMException(IDOM_DOMException::INVALID_CHARACTER_ERR,0);

    if (prefix == 0 || prefix[0] == chNull) {
        fName = fLocalName;
        fPrefix = 0;
        return;
    }

    if (XMLString::compareString(prefix, xml) == 0 &&
        XMLString::compareString(fNamespaceURI, xmlURI) != 0 ||
        XMLString::compareString(prefix, xmlns) == 0 &&
        XMLString::compareString(fNamespaceURI, xmlnsURI) != 0)
        throw IDOM_DOMException(IDOM_DOMException::NAMESPACE_ERR, 0);

    if (XMLString::indexOf(prefix, chColon) != -1) {
        throw IDOM_DOMException(IDOM_DOMException::NAMESPACE_ERR, 0);
    }

    this-> fPrefix = ((IDDocumentImpl *)this->getOwnerDocument())->getPooledString(prefix);

    int prefixLen = XMLString::stringLen(prefix);
    int newQualifiedNameLen = prefixLen+1+XMLString::stringLen(fLocalName);
    XMLCh* newName;
    XMLCh temp[4000];
    if (newQualifiedNameLen >= 3999)
        newName = new XMLCh[newQualifiedNameLen];
    else
        newName = temp;

    // newName = prefix + chColon + fLocalName;
    XMLString::copyString(newName, prefix);
    newName[prefixLen] = chColon;
    XMLString::copyString(&newName[prefixLen+1], fLocalName);

    fName = ((IDDocumentImpl *)this->getOwnerDocument())->
                                           getPooledString(newName);

    if (newQualifiedNameLen >= 3999)
        delete[] newName;

}
