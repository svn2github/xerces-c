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

#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMNode.hpp>

#include "DOMDocumentImpl.hpp"
#include "DOMTextImpl.hpp"
#include "DOMCharacterDataImpl.hpp"
#include "DOMChildNode.hpp"


#include <assert.h>
#include "DOMRangeImpl.hpp"

class DOMDocument;

DOMTextImpl::DOMTextImpl(DOMDocument *ownerDoc, const XMLCh *dat)
    : fNode(ownerDoc), fCharacterData(ownerDoc, dat)
{
    fNode.setIsLeafNode(true);
};

DOMTextImpl::DOMTextImpl(const DOMTextImpl &other, bool deep)
    : fNode(other.fNode), fCharacterData(other.fCharacterData)
{
    fNode.setIsLeafNode(true);
};

DOMTextImpl::~DOMTextImpl()
{
};


DOMNode *DOMTextImpl::cloneNode(bool deep) const
{
    return new (getOwnerDocument()) DOMTextImpl(*this, deep);
};


const XMLCh * DOMTextImpl::getNodeName() const {
    static const XMLCh gtext[] = {chPound, chLatin_t, chLatin_e, chLatin_x, chLatin_t, chNull};
    return gtext;
}

short DOMTextImpl::getNodeType() const {
    return DOMNode::TEXT_NODE;
};


DOMText *DOMTextImpl::splitText(DOMSize_t offset)
{
    if (fNode.isReadOnly())
    {
        throw DOMException(
            DOMException::NO_MODIFICATION_ALLOWED_ERR, 0);
    }
    DOMSize_t len = XMLString::stringLen(fCharacterData.fData);
    if (offset > len || offset < 0)
        throw DOMException(DOMException::INDEX_SIZE_ERR, 0);

    DOMText *newText =
                getOwnerDocument()->createTextNode(
                        this->substringData(offset, len - offset));

    DOMNode *parent = getParentNode();
    if (parent != 0)
        parent->insertBefore(newText, getNextSibling());

    XMLCh *wData = (XMLCh *)(fCharacterData.fData);  // Cast off const.
    wData[offset] = 0;                               //  revisit - could change a string that
                                                     //     application code has.  Do we want to do this?


    if (this->getOwnerDocument() != 0) {
        Ranges* ranges = ((DOMDocumentImpl *)this->getOwnerDocument())->getRanges();
        if (ranges != 0) {
            DOMSize_t sz = ranges->size();
            if (sz != 0) {
                for (DOMSize_t i =0; i<sz; i++) {
                    ranges->elementAt(i)->updateSplitInfo( this, newText, offset);
                }
            }
        }
    }

    return newText;
};


bool DOMTextImpl::isIgnorableWhitespace() const
{
    return fNode.ignorableWhitespace();
}



void DOMTextImpl::setIgnorableWhitespace(bool ignorable)
{
    fNode.ignorableWhitespace(ignorable);
}


//
//  Delegation functions
//
           DOMNode          *DOMTextImpl::appendChild(DOMNode *newChild)        {return fNode.appendChild (newChild); };
           DOMNamedNodeMap  *DOMTextImpl::getAttributes() const 			        {return fNode.getAttributes (); };
           DOMNodeList      *DOMTextImpl::getChildNodes() const 			        {return fNode.getChildNodes (); };
           DOMNode          *DOMTextImpl::getFirstChild() const 			        {return fNode.getFirstChild (); };
           DOMNode          *DOMTextImpl::getLastChild() const 		            {return fNode.getLastChild (); };
     const XMLCh              *DOMTextImpl::getLocalName() const                    {return fNode.getLocalName (); };
     const XMLCh              *DOMTextImpl::getNamespaceURI() const                 {return fNode.getNamespaceURI (); };
           DOMNode          *DOMTextImpl::getNextSibling() const                  {return fChild.getNextSibling (); };
     const XMLCh              *DOMTextImpl::getNodeValue() const                    {return fCharacterData.getNodeValue (); };
           DOMDocument      *DOMTextImpl::getOwnerDocument() const                {return fNode.getOwnerDocument (); };
     const XMLCh              *DOMTextImpl::getPrefix() const                       {return fNode.getPrefix (); };
           DOMNode          *DOMTextImpl::getParentNode() const                   {return fChild.getParentNode (this); };
           DOMNode          *DOMTextImpl::getPreviousSibling() const              {return fChild.getPreviousSibling (this); };
           bool                DOMTextImpl::hasChildNodes() const                   {return fNode.hasChildNodes (); };
           DOMNode          *DOMTextImpl::insertBefore(DOMNode *newChild, DOMNode *refChild)
                                                                                    {return fNode.insertBefore (newChild, refChild); };
           void                DOMTextImpl::normalize()                             {fNode.normalize (); };
           DOMNode          *DOMTextImpl::removeChild(DOMNode *oldChild)        {return fNode.removeChild (oldChild); };
           DOMNode          *DOMTextImpl::replaceChild(DOMNode *newChild, DOMNode *oldChild)
                                                                                    {return fNode.replaceChild (newChild, oldChild); };
           bool                DOMTextImpl::isSupported(const XMLCh *feature, const XMLCh *version) const
                                                                                    {return fNode.isSupported (feature, version); };
           void                DOMTextImpl::setPrefix(const XMLCh  *prefix)         {fNode.setPrefix(prefix); };
           bool                DOMTextImpl::hasAttributes() const                   {return fNode.hasAttributes(); };


//
//   Delegation of CharacerData functions.
//


     const XMLCh * DOMTextImpl::getData() const                  {return fCharacterData.getData();};
     DOMSize_t  DOMTextImpl::getLength() const                {return fCharacterData.getLength();};
     const XMLCh * DOMTextImpl::substringData(DOMSize_t offset, DOMSize_t count) const
                                                                {return fCharacterData.substringData(this, offset, count);};
     void          DOMTextImpl::appendData(const XMLCh *arg)     {fCharacterData.appendData(this, arg);};
     void          DOMTextImpl::insertData(DOMSize_t offset, const  XMLCh *arg)
                                                                {fCharacterData.insertData(this, offset, arg);};
     void          DOMTextImpl::deleteData(DOMSize_t offset, DOMSize_t count)
                                                                {fCharacterData.deleteData(this, offset, count);};
     void          DOMTextImpl::replaceData(DOMSize_t offset, DOMSize_t count, const XMLCh *arg)
                                                                {fCharacterData.replaceData(this, offset, count, arg);};
     void          DOMTextImpl::setData(const XMLCh *data)       {fCharacterData.setData(this, data);};
     void          DOMTextImpl::setNodeValue(const XMLCh  *nodeValue)   {fCharacterData.setNodeValue (this, nodeValue); };

