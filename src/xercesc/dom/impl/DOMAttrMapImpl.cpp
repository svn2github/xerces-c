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

#include "DOMAttrMapImpl.hpp"
#include "DOMAttrImpl.hpp"
#include "DOMNodeImpl.hpp"
#include "DOMElementImpl.hpp"
#include "DOMCasts.hpp"
#include "DOMNodeVector.hpp"

#include <xercesc/dom/DOMAttr.hpp>
#include <xercesc/dom/DOMException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

DOMAttrMapImpl::DOMAttrMapImpl(DOMNode *ownerNod)
{
    this->fOwnerNode=ownerNod;
    this->fNodes = 0;
	hasDefaults(false);
}

DOMAttrMapImpl::DOMAttrMapImpl(DOMNode *ownerNod, const DOMAttrMapImpl *defaults)
{
    this->fOwnerNode=ownerNod;
    this->fNodes = 0;
	hasDefaults(false);
	if (defaults != 0)
	{
		if (defaults->getLength() > 0)
		{
			hasDefaults(true);
			cloneContent(defaults);
		}
	}
}

DOMAttrMapImpl::~DOMAttrMapImpl()
{
}

void DOMAttrMapImpl::cloneContent(const DOMAttrMapImpl *srcmap)
{
    if ((srcmap != 0) && (srcmap->fNodes != 0))
    {
        if (fNodes != 0)
            fNodes->reset();
        else
        {
            XMLSize_t size = srcmap->fNodes->size();
            if(size > 0) {
                DOMDocument *doc = fOwnerNode->getOwnerDocument();
                fNodes = new (doc) DOMNodeVector(doc, size);
            }
        }

        for (XMLSize_t i = 0; i < srcmap->fNodes->size(); i++)
        {
            DOMNode *n = srcmap->fNodes->elementAt(i);
            DOMNode *clone = n->cloneNode(true);
            castToNodeImpl(clone)->isSpecified(castToNodeImpl(n)->isSpecified());
            castToNodeImpl(clone)->fOwnerNode = fOwnerNode;
            castToNodeImpl(clone)->isOwned(true);
            fNodes->addElement(clone);
        }
    }
}

DOMAttrMapImpl *DOMAttrMapImpl::cloneAttrMap(DOMNode *ownerNode_p)
{
	DOMAttrMapImpl *newmap = new (castToNodeImpl(ownerNode_p)->getOwnerDocument()) DOMAttrMapImpl(ownerNode_p);
	newmap->cloneContent(this);
	// newmap->attrDefaults = this->attrDefaults;  // revisit
	return newmap;
}

void DOMAttrMapImpl::setReadOnly(bool readOnl, bool deep)
{
    // this->fReadOnly=readOnl;
    if(deep && fNodes!=0)
    {
        int sz = fNodes->size();
        for (int i=0; i<sz; ++i) {
            castToNodeImpl(fNodes->elementAt(i))->setReadOnly(readOnl, deep);
        }
    }
}

bool DOMAttrMapImpl::readOnly() {
    return castToNodeImpl(fOwnerNode)->isReadOnly();
}

int DOMAttrMapImpl::findNamePoint(const XMLCh *name) const
{

    // Binary search
    int i=0;
    if(fNodes!=0)
    {
        int first=0,last=fNodes->size()-1;

        while(first<=last)
        {
            i=(first+last)/2;
            int test = XMLString::compareString(name, fNodes->elementAt(i)->getNodeName());
            if(test==0)
                return i; // Name found
            else if(test<0)
                last=i-1;
            else
                first=i+1;
        }
        if(first>i) i=first;
    }
    /********************
    // Linear search
    int i = 0;
    if (fNodes != 0)
    for (i = 0; i < fNodes.size(); ++i)
    {
    int test = name.compareTo(((NodeImpl *) (fNodes.elementAt(i))).getNodeName());
    if (test == 0)
    return i;
    else
    if (test < 0)
    {
    break; // Found insertpoint
    }
    }

    *******************/
    return -1 - i; // not-found has to be encoded.
}

DOMNode * DOMAttrMapImpl::getNamedItem(const XMLCh *name) const
{
    int i=findNamePoint(name);
    return (i<0) ? 0 : fNodes->elementAt(i);
}

DOMNode *DOMAttrMapImpl::setNamedItem(DOMNode *arg)
{
    if (arg->getNodeType() != DOMNode::ATTRIBUTE_NODE)
        throw DOMException(DOMException::HIERARCHY_REQUEST_ERR, 0);

    DOMDocument *doc = fOwnerNode->getOwnerDocument();
    DOMNodeImpl *argImpl = castToNodeImpl(arg);
    if(argImpl->getOwnerDocument() != doc)
        throw DOMException(DOMException::WRONG_DOCUMENT_ERR,0);
    if (this->readOnly())
        throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR, 0);
    if ((arg->getNodeType() == DOMNode::ATTRIBUTE_NODE) && argImpl->isOwned() && (argImpl->fOwnerNode != fOwnerNode))
        throw DOMException(DOMException::INUSE_ATTRIBUTE_ERR,0);

    argImpl->fOwnerNode = fOwnerNode;
    argImpl->isOwned(true);
    int i=findNamePoint(arg->getNodeName());
    DOMNode * previous=0;
    if(i>=0)
    {
        previous = fNodes->elementAt(i);
        fNodes->setElementAt(arg,i);
    }
    else
    {
        i=-1-i; // Insert point (may be end of list)
        if(0==fNodes)
        {
            fNodes=new (doc) DOMNodeVector(doc);
        }
        fNodes->insertElementAt(arg,i);
    }
    if (previous != 0) {
        castToNodeImpl(previous)->fOwnerNode = fOwnerNode->getOwnerDocument();
        castToNodeImpl(previous)->isOwned(false);
    }

    return previous;
}

//Introduced in DOM Level 2

int DOMAttrMapImpl::findNamePoint(const XMLCh *namespaceURI,
	const XMLCh *localName) const
{
    if (fNodes == 0)
	return -1;
    // This is a linear search through the same fNodes Vector.
    // The Vector is sorted on the DOM Level 1 nodename.
    // The DOM Level 2 NS keys are namespaceURI and Localname,
    // so we must linear search thru it.
    // In addition, to get this to work with fNodes without any namespace
    // (namespaceURI and localNames are both 0) we then use the nodeName
    // as a secondary key.
    int i, len = fNodes -> size();
    for (i = 0; i < len; ++i) {
        DOMNode *node = fNodes -> elementAt(i);
        const XMLCh * nNamespaceURI = node->getNamespaceURI();
        const XMLCh * nLocalName = node->getLocalName();
        if (!XMLString::equals(nNamespaceURI, namespaceURI))    //URI not match
            continue;
        else {
            if (XMLString::equals(localName, nLocalName)
                ||
                (nLocalName == 0 && XMLString::equals(localName, node->getNodeName())))
                return i;
        }
    }
    return -1;	//not found
}

DOMNode *DOMAttrMapImpl::getNamedItemNS(const XMLCh *namespaceURI,
	const XMLCh *localName) const
{
    int i = findNamePoint(namespaceURI, localName);
    return i < 0 ? 0 : fNodes -> elementAt(i);
}

DOMNode *DOMAttrMapImpl::setNamedItemNS(DOMNode* arg)
{
    if (arg->getNodeType() != DOMNode::ATTRIBUTE_NODE)
        throw DOMException(DOMException::HIERARCHY_REQUEST_ERR, 0);

    DOMDocument *doc = fOwnerNode->getOwnerDocument();
    DOMNodeImpl *argImpl = castToNodeImpl(arg);
    if (argImpl->getOwnerDocument() != doc)
        throw DOMException(DOMException::WRONG_DOCUMENT_ERR,0);
    if (this->readOnly())
        throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR, 0);
    if (argImpl->isOwned())
        throw DOMException(DOMException::INUSE_ATTRIBUTE_ERR,0);

    argImpl->fOwnerNode = fOwnerNode;
    argImpl->isOwned(true);
    int i=findNamePoint(arg->getNamespaceURI(), arg->getLocalName());
    DOMNode *previous=0;
    if(i>=0) {
        previous = fNodes->elementAt(i);
        fNodes->setElementAt(arg,i);
    } else {
        i=findNamePoint(arg->getNodeName()); // Insert point (may be end of list)
        if (i<0)
          i = -1 - i;
        if(0==fNodes)
            fNodes=new (doc) DOMNodeVector(doc);
        fNodes->insertElementAt(arg,i);
    }
    if (previous != 0) {
        castToNodeImpl(previous)->fOwnerNode = fOwnerNode->getOwnerDocument();
        castToNodeImpl(previous)->isOwned(false);
    }

    return previous;
}

DOMNode *DOMAttrMapImpl::removeNamedItem(const XMLCh *name)
{
    if (this->readOnly())
        throw DOMException(
            DOMException::NO_MODIFICATION_ALLOWED_ERR, 0);
    int i=findNamePoint(name);
    DOMNode *removed = 0;

    if(i<0)
        throw DOMException(DOMException::NOT_FOUND_ERR, 0);

    removed = fNodes->elementAt(i);
    fNodes->removeElementAt(i);
    castToNodeImpl(removed)->fOwnerNode = fOwnerNode->getOwnerDocument();
    castToNodeImpl(removed)->isOwned(false);

    // Replace it if it had a default value
    // (DOM spec level 1 - Element Interface)
    if (hasDefaults() && (removed != 0))
    {
        DOMAttrMapImpl* defAttrs = ((DOMElementImpl*)fOwnerNode)->getDefaultAttributes();
        DOMAttr* attr = (DOMAttr*)(defAttrs->getNamedItem(name));
        if (attr != 0)
        {
            DOMAttr* newAttr = (DOMAttr*)attr->cloneNode(true);
            setNamedItem(newAttr);
        }
    }

    return removed;
}

DOMNode *DOMAttrMapImpl::removeNamedItemNS(const XMLCh *namespaceURI, const XMLCh *localName)
{
    if (this->readOnly())
        throw DOMException(
        DOMException::NO_MODIFICATION_ALLOWED_ERR, 0);
    int i = findNamePoint(namespaceURI, localName);
    if (i < 0)
        throw DOMException(DOMException::NOT_FOUND_ERR, 0);

    DOMNode * removed = fNodes -> elementAt(i);
    fNodes -> removeElementAt(i);	//remove n from nodes
    castToNodeImpl(removed)->fOwnerNode = fOwnerNode->getOwnerDocument();
    castToNodeImpl(removed)->isOwned(false);

    // Replace it if it had a default value
    // (DOM spec level 2 - Element Interface)

    if (hasDefaults() && (removed != 0))
    {
        DOMAttrMapImpl* defAttrs = ((DOMElementImpl*)fOwnerNode)->getDefaultAttributes();
        DOMAttr* attr = (DOMAttr*)(defAttrs->getNamedItemNS(namespaceURI, localName));
        if (attr != 0)
        {
            DOMAttr* newAttr = (DOMAttr*)attr->cloneNode(true);
            setNamedItemNS(newAttr);
        }
    }

    return removed;
}

// remove the name using index
// avoid calling findNamePoint again if the index is already known
DOMNode * DOMAttrMapImpl::removeNamedItemAt(XMLSize_t index)
{
    if (this->readOnly())
        throw DOMException(
            DOMException::NO_MODIFICATION_ALLOWED_ERR, 0);

    DOMNode *removed = item(index);
    if(!removed)
        throw DOMException(DOMException::NOT_FOUND_ERR, 0);

    fNodes->removeElementAt(index);
    castToNodeImpl(removed)->fOwnerNode = fOwnerNode->getOwnerDocument();
    castToNodeImpl(removed)->isOwned(false);

    // Replace it if it had a default value
    // (DOM spec level 1 - Element Interface)
    if (hasDefaults() && (removed != 0))
    {
        DOMAttrMapImpl* defAttrs = ((DOMElementImpl*)fOwnerNode)->getDefaultAttributes();

        const XMLCh* localName = removed->getLocalName();
        DOMAttr* attr = 0;
        if (localName)
            attr = (DOMAttr*)(defAttrs->getNamedItemNS(removed->getNamespaceURI(), localName));
        else
            attr = (DOMAttr*)(defAttrs->getNamedItem(((DOMAttr*)removed)->getName()));

        if (attr != 0)
        {
            DOMAttr* newAttr = (DOMAttr*)attr->cloneNode(true);
            setNamedItem(newAttr);
        }
    }

    return removed;
}

/**
 * Get this AttributeMap in sync with the given "defaults" map.
 * @param defaults The default attributes map to sync with.
 */
void DOMAttrMapImpl::reconcileDefaultAttributes(const DOMAttrMapImpl* defaults) {

    // remove any existing default
    XMLSize_t nsize = getLength();
    for (XMLSSize_t i = nsize - 1; i >= 0; i--) {
        DOMAttr* attr = (DOMAttr*)item(i);
        if (!attr->getSpecified()) {
            removeNamedItemAt(i);
        }
    }

    hasDefaults(false);

    // add the new defaults
    if (defaults) {
        hasDefaults(true);

        if (nsize == 0) {
            cloneContent(defaults);
        }
        else {
            XMLSize_t dsize = defaults->getLength();
            for (XMLSize_t n = 0; n < dsize; n++) {
                DOMAttr* attr = (DOMAttr*)defaults->item(n);

                DOMAttr* newAttr = (DOMAttr*)attr->cloneNode(true);
                setNamedItemNS(newAttr);
                DOMAttrImpl* newAttrImpl = (DOMAttrImpl*) newAttr;
                newAttrImpl->setSpecified(false);
            }
        }
    }
} // reconcileDefaults()


/**
 * Move specified attributes from the given map to this one
 */
void DOMAttrMapImpl::moveSpecifiedAttributes(DOMAttrMapImpl* srcmap) {
    XMLSize_t nsize = srcmap->getLength();

    for (XMLSSize_t i = nsize - 1; i >= 0; i--) {
        DOMAttr* attr = (DOMAttr*)srcmap->item(i);
        if (attr->getSpecified()) {
            srcmap->removeNamedItemAt(i);
        }

        if (attr->getLocalName())
            setNamedItemNS(attr);
        else
            setNamedItem(attr);
    }
} // moveSpecifiedAttributes(AttributeMap):void

XMLSize_t DOMAttrMapImpl::getLength() const
{
    return (fNodes != 0) ? fNodes->size() : 0;
}

DOMNode * DOMAttrMapImpl::item(XMLSize_t index) const
{
    return (fNodes != 0 && index < fNodes->size()) ?
        fNodes->elementAt(index) : 0;
}


XERCES_CPP_NAMESPACE_END

