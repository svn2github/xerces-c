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

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMException.hpp>

#include "DOMAttrImpl.hpp"
#include "DOMStringPool.hpp"
#include "DOMDocumentImpl.hpp"
#include "DOMCasts.hpp"
#include "DOMNodeIDMap.hpp"


DOMAttrImpl::DOMAttrImpl(DOMDocument *ownerDoc, const XMLCh *aName)
    : fNode(ownerDoc), fParent (ownerDoc)
{
    DOMDocumentImpl *docImpl = (DOMDocumentImpl *)ownerDoc;
    fName = docImpl->getPooledString(aName);
    fNode.isSpecified(true);
};

DOMAttrImpl::DOMAttrImpl(const DOMAttrImpl &other, bool deep)
    : fNode(other.fNode), fParent (other.fParent)
{
    fName = other.fName;

    if (other.fNode.isSpecified())
        fNode.isSpecified(true);
    else
        fNode.isSpecified(false);

    if (other.fNode.isIdAttr())
    {
        fNode.isIdAttr(true);
        DOMDocumentImpl *doc = (DOMDocumentImpl *)this->getOwnerDocument();
        doc->getNodeIDMap()->add(this);
    }

    fParent.cloneChildren(&other);
};


DOMAttrImpl::~DOMAttrImpl() {
};


DOMNode * DOMAttrImpl::cloneNode(bool deep) const
{
    DOMNode* newNode = new (this->getOwnerDocument(), DOMDocumentImpl::ATTR_OBJECT) DOMAttrImpl(*this, deep);
    fNode.callUserDataHandlers(DOMUserDataHandler::NODE_CLONED, this, newNode);
    return newNode;
};


const XMLCh * DOMAttrImpl::getNodeName()  const{
    return fName;
};


short DOMAttrImpl::getNodeType() const {
    return DOMNode::ATTRIBUTE_NODE;
};


const XMLCh * DOMAttrImpl::getName() const {
    return fName;
};


const XMLCh * DOMAttrImpl::getNodeValue() const
{
    return getValue();
};


bool DOMAttrImpl::getSpecified() const
{
    return fNode.isSpecified();
};




const XMLCh * DOMAttrImpl::getValue() const
{
    if (fParent.fFirstChild == 0) {
        return XMLUni::fgZeroLenString; // return "";
    }
    DOMNode *node = castToChildImpl(fParent.fFirstChild)->nextSibling;
    if (node == 0) {
        return fParent.fFirstChild->getNodeValue();
    }
    int             length = 0;
    for (node = fParent.fFirstChild; node != 0; node = castToChildImpl(node)->nextSibling)
        length += XMLString::stringLen(node->getNodeValue());

    // revisit - Come up with some way of not allocating a new string each
    //                time we do this.  But it's not an immediate pressing problem,
    //                becuase we only allocate a new string when we have attribute
    //                values that contain entity reference nodes.  And the parser
    //                does not ever produce such a thing.

    //XMLCh * retString = new (this->getOwnerDocument()) XMLCh[length+1];
    length = sizeof(XMLCh) * (length+1);
    length = (length % 4) + length;
    XMLCh * retString = (XMLCh*) ((DOMDocumentImpl *)this->getOwnerDocument())->allocate(length);
    retString[0] = 0;
    for (node = fParent.fFirstChild; node != 0; node = castToChildImpl(node)->nextSibling)
    {
        XMLString::catString(retString, node->getNodeValue());
    };

    return retString;
};



void DOMAttrImpl::setNodeValue(const XMLCh *val)
{
    setValue(val);
};



void DOMAttrImpl::setSpecified(bool arg)
{
    fNode.isSpecified(arg);
};



void DOMAttrImpl::setValue(const XMLCh *val)
{
    if (fNode.isReadOnly())
    {
        throw DOMException (
            DOMException::NO_MODIFICATION_ALLOWED_ERR, 0);
    }

    //  If this attribute was of type ID and in the map, take it out,
    //    then put it back in with the new name.  For now, we don't worry
    //    about what happens if the new name conflicts
    //
    DOMDocumentImpl *doc = (DOMDocumentImpl *)getOwnerDocument();
    if (fNode.isIdAttr())
        doc->getNodeIDMap()->remove(this);

    DOMNode *kid;
    while ((kid = fParent.fFirstChild) != 0)         // Remove existing kids
    {
        DOMNode* node = removeChild(kid);
        if (node)
            node->release();
    }

    if (val != 0)              // Create and add the new one
        appendChild(doc->createTextNode(val));
    fNode.isSpecified(true);
    fParent.changed();

    if (fNode.isIdAttr())
        doc->getNodeIDMap()->add(this);

};




//Introduced in DOM Level 2

DOMElement *DOMAttrImpl::getOwnerElement() const
{
    // if we have an owner, ownerNode is our ownerElement, otherwise it's
    // our ownerDocument and we don't have an ownerElement
    return (DOMElement *) (fNode.isOwned() ? fNode.fOwnerNode : 0);
}


//internal use by parser only
void DOMAttrImpl::setOwnerElement(DOMElement *ownerElem)
{
    fNode.fOwnerNode = ownerElem;
    // revisit.  Is this backwards?  isOwned(true)?
    fNode.isOwned(false);
}

void DOMAttrImpl::release()
{
    if (fNode.isOwned() && !fNode.isToBeReleased())
        throw DOMException(DOMException::INVALID_ACCESS_ERR,0);

    DOMDocumentImpl* doc = (DOMDocumentImpl*) getOwnerDocument();
    if (doc) {
        fParent.release();
        doc->release(this, DOMDocumentImpl::ATTR_OBJECT);
    }
    else {
        // shouldn't reach here
        throw DOMException(DOMException::INVALID_ACCESS_ERR,0);
    }
}


           DOMNode*         DOMAttrImpl::appendChild(DOMNode *newChild)          {return fParent.appendChild (newChild); };
           DOMNamedNodeMap* DOMAttrImpl::getAttributes() const                   {return fNode.getAttributes (); };
           DOMNodeList*     DOMAttrImpl::getChildNodes() const                   {return fParent.getChildNodes (); };
           DOMNode*         DOMAttrImpl::getFirstChild() const                   {return fParent.getFirstChild (); };
           DOMNode*         DOMAttrImpl::getLastChild() const                    {return fParent.getLastChild (); };
     const XMLCh*           DOMAttrImpl::getLocalName() const                    {return fNode.getLocalName (); };
     const XMLCh*           DOMAttrImpl::getNamespaceURI() const                 {return fNode.getNamespaceURI (); };
           DOMNode*         DOMAttrImpl::getNextSibling() const                  {return fNode.getNextSibling (); };
           DOMDocument*     DOMAttrImpl::getOwnerDocument() const                {return fNode.getOwnerDocument (); };
     const XMLCh*           DOMAttrImpl::getPrefix() const                       {return fNode.getPrefix (); };
           DOMNode*         DOMAttrImpl::getParentNode() const                   {return fNode.getParentNode (); };
           DOMNode*         DOMAttrImpl::getPreviousSibling() const              {return fNode.getPreviousSibling (); };
           bool             DOMAttrImpl::hasChildNodes() const                   {return fParent.hasChildNodes (); };
           DOMNode*         DOMAttrImpl::insertBefore(DOMNode *newChild, DOMNode *refChild)
                                                                                 {return fParent.insertBefore (newChild, refChild); };
           void             DOMAttrImpl::normalize()                             {fParent.normalize (); };
           DOMNode*         DOMAttrImpl::removeChild(DOMNode *oldChild)          {return fParent.removeChild (oldChild); };
           DOMNode*         DOMAttrImpl::replaceChild(DOMNode *newChild, DOMNode *oldChild)
                                                                                 {return fParent.replaceChild (newChild, oldChild); };
           bool             DOMAttrImpl::isSupported(const XMLCh *feature, const XMLCh *version) const
                                                                                 {return fNode.isSupported (feature, version); };
           void             DOMAttrImpl::setPrefix(const XMLCh  *prefix)         {fNode.setPrefix(prefix); };
           bool             DOMAttrImpl::hasAttributes() const                   {return fNode.hasAttributes(); };
           bool             DOMAttrImpl::isSameNode(const DOMNode* other)        {return fNode.isSameNode(other); };
           bool             DOMAttrImpl::isEqualNode(const DOMNode* arg)         {return fParent.isEqualNode(arg); };
           void*            DOMAttrImpl::setUserData(const XMLCh* key, void* data, DOMUserDataHandler* handler)
                                                                                 {return fNode.setUserData(key, data, handler); };
           void*            DOMAttrImpl::getUserData(const XMLCh* key) const     {return fNode.getUserData(key); };



