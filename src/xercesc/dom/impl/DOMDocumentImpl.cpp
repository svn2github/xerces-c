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

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMDocumentType.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/dom/DOMNode.hpp>

#include "DOMDocumentImpl.hpp"
#include "DOMCasts.hpp"
#include "DOMDocumentTypeImpl.hpp"
#include "DOMAttrImpl.hpp"
#include "DOMAttrNSImpl.hpp"
#include "DOMCDATASectionImpl.hpp"
#include "DOMCommentImpl.hpp"
#include "DOMDeepNodeListImpl.hpp"
#include "DOMDocumentFragmentImpl.hpp"
#include "DOMElementImpl.hpp"
#include "XSDElementNSImpl.hpp"
#include "DOMEntityImpl.hpp"
#include "DOMEntityReferenceImpl.hpp"
#include "DOMNamedNodeMapImpl.hpp"
#include "DOMNotationImpl.hpp"
#include "DOMProcessingInstructionImpl.hpp"
#include "DOMTextImpl.hpp"

#include "DOMStringPool.hpp"
#include "DOMTreeWalkerImpl.hpp"
#include "DOMNodeIteratorImpl.hpp"
#include "DOMNodeIDMap.hpp"
#include "DOMRangeImpl.hpp"

#include <xercesc/internal/XMLReader.hpp>
#include <xercesc/util/HashPtr.hpp>


//revisit.  These can go away once all of the include files above are really there.
class DOMTreeWalker;
class DOMNodeFilter;


//
//   Constructors.   Warning - be very careful with the ordering of initialization
//                             of the heap.  Ordering depends on the order of declaration
//                             in the .hpp file, not on the order of initializers here
//                             in the constructor.  The heap declaration can not be
//                             first - fNode and fParent must be first for the casting
//                             functions in DOMCasts to work correctly.  This means that
//                             fNode and fParent constructors used here can not
//                             allocate.
//
DOMDocumentImpl::DOMDocumentImpl()
    : fNode(this),
      fParent(this),
      fCurrentBlock(0),
      fFreePtr(0),
      fFreeBytesRemaining(0),
      fDocType(0),
      fDocElement(0),
      fNamePool(0),
      fIterators(0L),
      fTreeWalkers(0L),
      fNodeIDMap(0),
      fRanges(0),
      fChanges(0),
      fNodeListPool(0),
      fActualEncoding(0),
      fEncoding(0),
      fVersion(0),
      fStandalone(false),
      fDocumentURI(0),
      fUserDataTable(0)
{
    fNamePool    = new (this) DOMStringPool(257, this);
};


//DOM Level 2
DOMDocumentImpl::DOMDocumentImpl(const XMLCh *fNamespaceURI,
                               const XMLCh *qualifiedName,
                               DOMDocumentType *doctype)
    : fNode(this),
      fParent(this),
      fCurrentBlock(0),
      fFreePtr(0),
      fFreeBytesRemaining(0),
      fDocType(0),
      fDocElement(0),
      fNamePool(0),
      fIterators(0L),
      fTreeWalkers(0L),
      fNodeIDMap(0),
      fRanges(0),
      fChanges(0),
      fNodeListPool(0),
      fActualEncoding(0),
      fEncoding(0),
      fVersion(0),
      fStandalone(false),
      fDocumentURI(0),
      fUserDataTable(0)
{
    fNamePool    = new (this) DOMStringPool(257, this);
    try {
        setDocumentType(doctype);
        appendChild(createElementNS(fNamespaceURI, qualifiedName));  //root element
    }
    catch (...) {
        this->deleteHeap();
        throw;
    }
}

void DOMDocumentImpl::setDocumentType(DOMDocumentType *doctype)
{
    if (!doctype)
        return;

    // New doctypes can be created either with the factory methods on DOMImplementation, in
    //   which case ownerDocument will be 0, or with methods on DocumentImpl, in which case
    //   ownerDocument will be set, but the DocType won't yet be a child of the document.
    if (doctype->getOwnerDocument() != 0 && doctype->getOwnerDocument() != this)
        throw DOMException(    //one doctype can belong to only one DOMDocumentImpl
        DOMException::WRONG_DOCUMENT_ERR, 0);

    DOMDocumentTypeImpl* doctypeImpl = (DOMDocumentTypeImpl*) doctype;
    doctypeImpl->setOwnerDocument(this);

    // The doctype can not have any Entities or Notations yet, because they can not
    //   be created except through factory methods on a document.

    // revisit.  What if this doctype is already a child of the document?
    appendChild(doctype);

}

DOMDocumentImpl::~DOMDocumentImpl()
{
    //  Clean up the fNodeListPool
    if (fNodeListPool)
        fNodeListPool->cleanup();

    //  Clean up the RefVector
    if (fIterators)
        fIterators->cleanup();

    if (fTreeWalkers)
        fTreeWalkers->cleanup();

    if (fRanges)
        fRanges->cleanup();

    if (fUserDataTable)
        fUserDataTable->cleanup();

    //  Delete the heap for this document.  This uncerimoniously yanks the storage
    //      out from under all of the nodes in the document.  Destructors are NOT called.
    this->deleteHeap();
};


DOMNode *DOMDocumentImpl::cloneNode(bool deep) const {

    // Note:  the cloned document node goes on the system heap.  All other
    //   nodes added to the new document will go on that document's heap,
    //   but we need to construct the document first, before its heap exists.
    DOMDocumentImpl *newdoc = new DOMDocumentImpl();

    // then the children by _importing_ them
    if (deep)
        for (DOMNode *n = this->getFirstChild(); n != 0; n = n->getNextSibling()) {
            newdoc->appendChild(newdoc->importNode(n, true, true));
    }

    fNode.callUserDataHandlers(DOMUserDataHandler::NODE_CLONED, this, newdoc);
    return newdoc;
};


const XMLCh * DOMDocumentImpl::getNodeName() const {
    static const XMLCh nam[] =  // "#document"
        {chPound, chLatin_d, chLatin_o, chLatin_c, chLatin_u, chLatin_m, chLatin_e, chLatin_n, chLatin_t, 0};
    return nam;
}


short DOMDocumentImpl::getNodeType() const {
    return DOMNode::DOCUMENT_NODE;
};


// even though ownerDocument refers to this in this implementation
// the DOM Level 2 spec says it must be 0, so make it appear so
DOMDocument * DOMDocumentImpl::getOwnerDocument() const {
    return 0;
}


DOMAttr *DOMDocumentImpl::createAttribute(const XMLCh *nam)
{
    if(!isXMLName(nam))
        throw DOMException(DOMException::INVALID_CHARACTER_ERR,0);
    return new (this) DOMAttrImpl(this,nam);
};



DOMCDATASection *DOMDocumentImpl::createCDATASection(const XMLCh *data) {
    return new (this) DOMCDATASectionImpl(this,data);
};



DOMComment *DOMDocumentImpl::createComment(const XMLCh *data)
{
    return new (this) DOMCommentImpl(this, data);
};



DOMDocumentFragment *DOMDocumentImpl::createDocumentFragment()
{
    return new (this) DOMDocumentFragmentImpl(this);
};



DOMDocumentType *DOMDocumentImpl::createDocumentType(const XMLCh *nam)
{
    if (!isXMLName(nam))
        throw DOMException(
        DOMException::INVALID_CHARACTER_ERR, 0);

    return new (this) DOMDocumentTypeImpl(this, nam);
};



DOMDocumentType *
    DOMDocumentImpl::createDocumentType(const XMLCh *qualifiedName,
                                     const XMLCh *publicId,
                                     const XMLCh *systemId)
{
    if (!isXMLName(qualifiedName))
        throw DOMException(
        DOMException::INVALID_CHARACTER_ERR, 0);

    return new (this) DOMDocumentTypeImpl(this, qualifiedName, publicId, systemId);
};



DOMElement *DOMDocumentImpl::createElement(const XMLCh *tagName)
{
    if(!isXMLName(tagName))
        throw DOMException(DOMException::INVALID_CHARACTER_ERR,0);

    return new (this) DOMElementImpl(this,tagName);
};


DOMElement *DOMDocumentImpl::createElementNoCheck(const XMLCh *tagName)
{
    return new (this) DOMElementImpl(this, tagName);
};




DOMEntity *DOMDocumentImpl::createEntity(const XMLCh *nam)
{
    if (!isXMLName(nam))
        throw DOMException(
        DOMException::INVALID_CHARACTER_ERR, 0);

    return new (this) DOMEntityImpl(this, nam);
};



DOMEntityReference *DOMDocumentImpl::createEntityReference(const XMLCh *nam)
{
    if (!isXMLName(nam))
        throw DOMException(
        DOMException::INVALID_CHARACTER_ERR, 0);

    return new (this) DOMEntityReferenceImpl(this, nam);
};



DOMNotation *DOMDocumentImpl::createNotation(const XMLCh *nam)
{
    if (!isXMLName(nam))
        throw DOMException(
        DOMException::INVALID_CHARACTER_ERR, 0);

    return new (this) DOMNotationImpl(this, nam);
};



DOMProcessingInstruction *DOMDocumentImpl::createProcessingInstruction(
                                          const XMLCh *target, const XMLCh *data)
{
    if(!isXMLName(target))
        throw DOMException(DOMException::INVALID_CHARACTER_ERR,0);
    return new (this) DOMProcessingInstructionImpl(this,target,data);
};




DOMText *DOMDocumentImpl::createTextNode(const XMLCh *data)
{
    return new (this) DOMTextImpl(this,data);
};


DOMNodeIterator* DOMDocumentImpl::createNodeIterator (
          DOMNode *root, unsigned long whatToShow, DOMNodeFilter* filter, bool entityReferenceExpansion)
{
    // Create the node iterator implementation object.
    // Add it to the vector of fIterators that must be synchronized when a node is deleted.
    // The vector of fIterators is kept in the "owner document" if there is one. If there isn't one, I assume that root is the
    // owner document.

    DOMNodeIteratorImpl* iter = new (this) DOMNodeIteratorImpl(root, whatToShow, filter, entityReferenceExpansion);
    DOMDocument* doc = root->getOwnerDocument();
    DOMDocumentImpl* impl;

    if (doc != 0) {
        impl = (DOMDocumentImpl *) doc;
    }
    else
        impl = (DOMDocumentImpl *) root;

    if (impl->fIterators == 0L) {
        impl->fIterators = new (this) NodeIterators(1, false);
        impl->fIterators->addElement(iter);
    }

    return iter;
}


DOMTreeWalker* DOMDocumentImpl::createTreeWalker (DOMNode *root, unsigned long whatToShow, DOMNodeFilter* filter, bool entityReferenceExpansion)
{
    // See notes for createNodeIterator...

    DOMTreeWalkerImpl* twi = new (this) DOMTreeWalkerImpl(root, whatToShow, filter, entityReferenceExpansion);
    DOMDocument* doc = root->getOwnerDocument();
    DOMDocumentImpl* impl;

    if ( doc != 0) {
        impl = (DOMDocumentImpl *) doc;
    }
    else
        impl = (DOMDocumentImpl *) root;

    if (impl->fTreeWalkers == 0L) {
        impl->fTreeWalkers = new (this) TreeWalkers(1, false);
        impl->fTreeWalkers->addElement(twi);
    }

    return twi;
}




DOMDocumentType *DOMDocumentImpl::getDoctype() const
{
    return fDocType;
};



DOMElement *DOMDocumentImpl::getDocumentElement() const
{
    return fDocElement;
};



DOMNodeList *DOMDocumentImpl::getElementsByTagName(const XMLCh *tagname) const
{
    // cast off the const of this because we will update the fNodeListPool
    return ((DOMDocumentImpl*)this)->getDeepNodeList(this,tagname);
};


DOMImplementation   *DOMDocumentImpl::getImplementation() const {
    return DOMImplementation::getImplementation();
}


DOMNode *DOMDocumentImpl::insertBefore(DOMNode *newChild, DOMNode *refChild)
{
    // Only one such child permitted
    if(
        (newChild->getNodeType() == DOMNode::ELEMENT_NODE  && fDocElement!=0)
        ||
        (newChild->getNodeType() == DOMNode::DOCUMENT_TYPE_NODE  && fDocType!=0)
        )
        throw DOMException(DOMException::HIERARCHY_REQUEST_ERR,0);

    fParent.insertBefore(newChild,refChild);

    // If insert succeeded, cache the kid appropriately
    if(newChild->getNodeType() == DOMNode::ELEMENT_NODE)
        fDocElement=(DOMElement *)newChild;
    else if(newChild->getNodeType() == DOMNode::DOCUMENT_TYPE_NODE)
        fDocType=(DOMDocumentType *)newChild;

    return newChild;
};



bool DOMDocumentImpl::isXMLName(const XMLCh *s)
{
    // revist.  This function probably already exists in the scanner.
    if (!XMLReader::isFirstNameChar(s[0]))
        return false;

    const XMLCh *p;
    for (p=s+1; *p!=0; p++)
    {
        if (!XMLReader::isNameChar(*p))
            return false;
    }
    return true;
};




DOMNode *DOMDocumentImpl::removeChild(DOMNode *oldChild)
{
    fParent.removeChild(oldChild);

    // If remove succeeded, un-cache the kid appropriately
    if(oldChild->getNodeType() == DOMNode::ELEMENT_NODE)
        fDocElement=0;
    else if(oldChild->getNodeType() == DOMNode::DOCUMENT_TYPE_NODE)
        fDocType=0;

    return oldChild;
};



void DOMDocumentImpl::setNodeValue(const XMLCh *x)
{
    fNode.setNodeValue(x);
};


//Introduced in DOM Level 2
DOMNode *DOMDocumentImpl::importNode(DOMNode *source, bool deep)
{
    return importNode(source, deep, false);
}


DOMElement *DOMDocumentImpl::createElementNS(const XMLCh *fNamespaceURI,
    const XMLCh *qualifiedName)
{
    if(!isXMLName(qualifiedName))
        throw DOMException(DOMException::INVALID_CHARACTER_ERR,0);
    //XMLCh * pooledTagName = this->fNamePool->getPooledString(qualifiedName);
    return new (this) DOMElementNSImpl(this, fNamespaceURI, qualifiedName);
}

DOMElement *DOMDocumentImpl::createElementNS(const XMLCh *fNamespaceURI,
                                              const XMLCh *qualifiedName,
                                              const XMLSSize_t lineNo,
                                              const XMLSSize_t columnNo)
{
    if(!isXMLName(qualifiedName))
        throw DOMException(DOMException::INVALID_CHARACTER_ERR,0);

    return new (this) XSDElementNSImpl(this, fNamespaceURI, qualifiedName, lineNo, columnNo);
}


DOMAttr *DOMDocumentImpl::createAttributeNS(const XMLCh *fNamespaceURI,
    const XMLCh *qualifiedName)
{
    if(!isXMLName(qualifiedName))
        throw DOMException(DOMException::INVALID_CHARACTER_ERR,0);
    return new (this) DOMAttrNSImpl(this, fNamespaceURI, qualifiedName);
}


DOMNodeList *DOMDocumentImpl::getElementsByTagNameNS(const XMLCh *fNamespaceURI,
    const XMLCh *fLocalName)  const
{
    // cast off the const of this because we will update the fNodeListPool
    return ((DOMDocumentImpl*)this)->getDeepNodeList(this, fNamespaceURI, fLocalName);
}


DOMElement *DOMDocumentImpl::getElementById(const XMLCh *elementId) const
{
    if (fNodeIDMap == 0)
        return 0;

    DOMAttr *theAttr = fNodeIDMap->find(elementId);
    if (theAttr == 0)
        return 0;

    return theAttr->getOwnerElement();
}


//Return the index > 0 of ':' in the given qualified name qName="prefix:localName".
//Return 0 if there is no ':', or -1 if qName is malformed such as ":abcd" or "abcd:".
int DOMDocumentImpl::indexofQualifiedName(const XMLCh * qName)
{
    int qNameLen = XMLString::stringLen(qName);
    int index = -1, count = 0;
    for (int i = 0; i < qNameLen; ++i) {
        if (qName[i] == chColon) {
            index = i;
            ++count;    //number of ':' found
        }
    }

    if (qNameLen == 0 || count > 1 || index == 0 || index == qNameLen-1)
        return -1;
    return count == 0 ? 0 : index;
}



DOMRange* DOMDocumentImpl::createRange()
{

    DOMRangeImpl* range = new (this) DOMRangeImpl(this);

    if (fRanges == 0L) {
        fRanges = new (this) Ranges(1, false);
    }
    fRanges->addElement(range);
    return range;
}

Ranges* DOMDocumentImpl::getRanges() const
{
    return fRanges;
}

void DOMDocumentImpl::removeRange(DOMRangeImpl* range)
{
    if (fRanges != 0) {
        XMLSize_t sz = fRanges->size();
        if (sz !=0) {
            for (XMLSize_t i =0; i<sz; i++) {
                if (fRanges->elementAt(i) == range) {
                    fRanges->removeElementAt(i);
                    break;
                }
            }
        }
    }
}

/** Uses the kidOK lookup table to check whether the proposed
    tree structure is legal.

    ????? It feels like there must be a more efficient solution,
    but for the life of me I can't think what it would be.
*/
bool DOMDocumentImpl::isKidOK(DOMNode *parent, DOMNode *child)
{
      static int kidOK[14];

      if (kidOK[DOMNode::ATTRIBUTE_NODE] == 0)
      {
          kidOK[DOMNode::DOCUMENT_NODE] =
              1 << DOMNode::ELEMENT_NODE |
              1 << DOMNode::PROCESSING_INSTRUCTION_NODE |
              1 << DOMNode::COMMENT_NODE |
              1 << DOMNode::DOCUMENT_TYPE_NODE;

          kidOK[DOMNode::DOCUMENT_FRAGMENT_NODE] =
              kidOK[DOMNode::ENTITY_NODE] =
              kidOK[DOMNode::ENTITY_REFERENCE_NODE] =
              kidOK[DOMNode::ELEMENT_NODE] =
              1 << DOMNode::ELEMENT_NODE |
              1 << DOMNode::PROCESSING_INSTRUCTION_NODE |
              1 << DOMNode::COMMENT_NODE |
              1 << DOMNode::TEXT_NODE |
              1 << DOMNode::CDATA_SECTION_NODE |
              1 << DOMNode::ENTITY_REFERENCE_NODE;

          kidOK[DOMNode::ATTRIBUTE_NODE] =
              1 << DOMNode::TEXT_NODE |
              1 << DOMNode::ENTITY_REFERENCE_NODE;

          kidOK[DOMNode::PROCESSING_INSTRUCTION_NODE] =
              kidOK[DOMNode::COMMENT_NODE] =
              kidOK[DOMNode::TEXT_NODE] =
              kidOK[DOMNode::CDATA_SECTION_NODE] =
              kidOK[DOMNode::NOTATION_NODE] =
              0;
      };
      int p=parent->getNodeType();
      int ch = child->getNodeType();
      return (kidOK[p] & 1<<ch) != 0;
}

void            DOMDocumentImpl::changed()
{
    fChanges++;
}


int             DOMDocumentImpl::changes() const{
    return fChanges;
};



//
//    Delegation for functions inherited from DOMNode
//
           DOMNode*         DOMDocumentImpl::appendChild(DOMNode *newChild)          {return insertBefore(newChild, 0); };
           DOMNamedNodeMap* DOMDocumentImpl::getAttributes() const                   {return fNode.getAttributes (); };
           DOMNodeList*     DOMDocumentImpl::getChildNodes() const                   {return fParent.getChildNodes (); };
           DOMNode*         DOMDocumentImpl::getFirstChild() const                   {return fParent.getFirstChild (); };
           DOMNode*         DOMDocumentImpl::getLastChild() const                    {return fParent.getLastChild (); };
     const XMLCh*           DOMDocumentImpl::getLocalName() const                    {return fNode.getLocalName (); };
     const XMLCh*           DOMDocumentImpl::getNamespaceURI() const                 {return fNode.getNamespaceURI (); };
           DOMNode*         DOMDocumentImpl::getNextSibling() const                  {return fNode.getNextSibling (); };
     const XMLCh*           DOMDocumentImpl::getNodeValue() const                    {return fNode.getNodeValue (); };
     const XMLCh*           DOMDocumentImpl::getPrefix() const                       {return fNode.getPrefix (); };
           DOMNode*         DOMDocumentImpl::getParentNode() const                   {return fNode.getParentNode (); };
           DOMNode*         DOMDocumentImpl::getPreviousSibling() const              {return fNode.getPreviousSibling (); };
           bool             DOMDocumentImpl::hasChildNodes() const                   {return fParent.hasChildNodes (); };
           void             DOMDocumentImpl::normalize()                             {fParent.normalize (); };
           DOMNode*         DOMDocumentImpl::replaceChild(DOMNode *newChild, DOMNode *oldChild)
                                                                                     {return fParent.replaceChild (newChild, oldChild); };
           bool             DOMDocumentImpl::isSupported(const XMLCh *feature, const XMLCh *version) const
                                                                                     {return fNode.isSupported (feature, version); };
           void             DOMDocumentImpl::setPrefix(const XMLCh  *prefix)         {fNode.setPrefix(prefix); };
           bool             DOMDocumentImpl::hasAttributes() const                   {return fNode.hasAttributes(); };
           void*            DOMDocumentImpl::setUserData(const XMLCh* key, void* data, DOMUserDataHandler* handler)
                                                                                     {return fNode.setUserData(key, data, handler); };
           void*            DOMDocumentImpl::getUserData(const XMLCh* key) const     {return fNode.getUserData(key); };



//-----------------------------------------------------------------------
//
//  Per Document Heap and Heap Helper functions
//
//        revisit - this stuff should be a class of its own, rather than
//                       just lying around naked in DocumentImpl.
//
//-----------------------------------------------------------------------

XMLCh * DOMDocumentImpl::cloneString(const XMLCh *src)
{
    if (!src) return 0;
    size_t   len = XMLString::stringLen(src);
    len = (len + 1) * sizeof(XMLCh);
    len = (len % 4) + len;
    XMLCh *newStr = (XMLCh *)this->allocate(len);
    XMLString::copyString(newStr, src);
    return newStr;
}


const XMLCh *  DOMDocumentImpl::getPooledString(const XMLCh *src)
{
    return this->fNamePool->getPooledString(src);
}

static const int kHeapAllocSize = 0x10000;    // The chunk size to allocate from the
                                              //   system allocator.

static const int kMaxSubAllocationSize = 4096;   // Any request for more bytes
                                                 //  than this will be handled by
                                                 //  allocating directly with system.

void *         DOMDocumentImpl::allocate(size_t amount)
{

     size_t sizeOfPointer = sizeof(void *);
     if (amount%sizeOfPointer!=0)
       amount = amount + (sizeOfPointer - (amount % sizeOfPointer));

    // If the request is for a largish block, hand it off to the system
    //   allocator.  The block still must be linked into the list of
    //   allocated blocks so that it will be deleted when the time comes.
    if (amount > kMaxSubAllocationSize)
    {
        void* newBlock = 0;
        try {
            newBlock = new char[amount + sizeOfPointer];
        }
        catch (...) {
            ThrowXML(RuntimeException, XMLExcepts::Out_Of_Memory);
        }
        if (!newBlock)
           ThrowXML(RuntimeException, XMLExcepts::Out_Of_Memory);

        if (fCurrentBlock)
        {
            *(void **)newBlock = *(void **)fCurrentBlock;
            *(void **)fCurrentBlock = newBlock;
        }
        else
        {
            fCurrentBlock = newBlock;
            fFreePtr = 0;
            fFreeBytesRemaining = 0;
        }
        void *retPtr = (char *)newBlock + sizeOfPointer;
        return retPtr;
    }


    // It's a normal (sub-allocatable) request.
    if (amount > fFreeBytesRemaining)
    {
        // Request doesn't fit in the current block.
        //   Get a new one from the system allocator.
        void* newBlock = 0;
        try {
            newBlock = new char[kHeapAllocSize];
        }
        catch (...) {
            ThrowXML(RuntimeException, XMLExcepts::Out_Of_Memory);
        }
        if (!newBlock)
           ThrowXML(RuntimeException, XMLExcepts::Out_Of_Memory);

        *(void **)newBlock = fCurrentBlock;
        fCurrentBlock = newBlock;
        fFreePtr = (char *)newBlock + sizeOfPointer;
        fFreeBytesRemaining = kHeapAllocSize - sizeOfPointer;
    }

    void *retPtr = fFreePtr;
    fFreePtr += amount;
    fFreeBytesRemaining -= amount;
    return retPtr;
}


void    DOMDocumentImpl::deleteHeap()
{
    void *block = fCurrentBlock;
    while (fCurrentBlock != 0)
    {
        void *nextBlock = *(void **)fCurrentBlock;
        delete [] fCurrentBlock;
        fCurrentBlock = nextBlock;
    }

}

DOMNodeList *DOMDocumentImpl::getDeepNodeList(const DOMNode *rootNode, const XMLCh *tagName)
{
    if(!fNodeListPool) {
        fNodeListPool = new (this) DOMDeepNodeListPool<DOMDeepNodeListImpl>(109, false);
    }

    DOMDeepNodeListImpl* retList = fNodeListPool->getByKey(rootNode, tagName, 0);
    if (!retList) {
        int id = fNodeListPool->put((void*) rootNode, (XMLCh*) tagName, 0, new (this) DOMDeepNodeListImpl(rootNode, tagName));
        retList = fNodeListPool->getById(id);
    }

    return retList;
}


DOMNodeList *DOMDocumentImpl::getDeepNodeList(const DOMNode *rootNode,     //DOM Level 2
                                                   const XMLCh *namespaceURI,
                                                   const XMLCh *localName)
{
    if(!fNodeListPool) {
        fNodeListPool = new (this) DOMDeepNodeListPool<DOMDeepNodeListImpl>(109, false);
    }

    DOMDeepNodeListImpl* retList = fNodeListPool->getByKey(rootNode, localName, namespaceURI);
    if (!retList) {
        // the pool will adopt the DOMDeepNodeListImpl
        int id = fNodeListPool->put((void*) rootNode, (XMLCh*) localName, (XMLCh*) namespaceURI, new (this) DOMDeepNodeListImpl(rootNode, namespaceURI, localName));
        retList = fNodeListPool->getById(id);
    }

    return retList;
}


//Introduced in DOM Level 3
const XMLCh* DOMDocumentImpl::getActualEncoding() const {
    return fActualEncoding;
}

void DOMDocumentImpl::setActualEncoding(const XMLCh* actualEncoding){
    fActualEncoding = cloneString(actualEncoding);
}

const XMLCh* DOMDocumentImpl::getEncoding() const {
    return fEncoding;
}

void DOMDocumentImpl::setEncoding(const XMLCh* encoding){
    fEncoding = cloneString(encoding);
}

bool DOMDocumentImpl::getStandalone() const{
    return fStandalone;
}

void DOMDocumentImpl::setStandalone(bool standalone){
    fStandalone = standalone;
}

const XMLCh* DOMDocumentImpl::getVersion() const {
    return fVersion;
}

void DOMDocumentImpl::setVersion(const XMLCh* version){
    fVersion = cloneString(version);
}

const XMLCh* DOMDocumentImpl::getDocumentURI() const {
    return fDocumentURI;
}

void DOMDocumentImpl::setDocumentURI(const XMLCh* documentURI){
    fDocumentURI = cloneString(documentURI);
}

//Introduced in DOM Level 2

DOMNode *DOMDocumentImpl::importNode(DOMNode *source, bool deep, bool cloningDoc)
{
    DOMNode *newnode=0;
    bool oldErrorCheckingFlag = errorChecking;

    switch (source->getNodeType())
    {
    case DOMNode::ELEMENT_NODE :
        {
            DOMElement *newelement;
            if (source->getLocalName() == 0)
                newelement = createElement(source->getNodeName());
            else
                newelement = createElementNS(source->getNamespaceURI(),

            source->getNodeName());
            DOMNamedNodeMap *srcattr=source->getAttributes();
            if(srcattr!=0)
                for(XMLSize_t i=0;i<srcattr->getLength();++i)
                {
                    DOMAttr *attr = (DOMAttr *) srcattr->item(i);
                    if (attr -> getSpecified()) { // not a default attribute
                        DOMAttr *nattr = (DOMAttr *) importNode(attr, true, false);
                        if (attr -> getLocalName() == 0)
                            newelement->setAttributeNode(nattr);
                        else
                            newelement->setAttributeNodeNS(nattr);

                        // if the imported attribute is of ID type, register the new node in fNodeIDMap
                        if (castToNodeImpl(attr)->isIdAttr()) {
                            castToNodeImpl(nattr)->isIdAttr(true);
                            if (!fNodeIDMap)
                                 fNodeIDMap = new (this) DOMNodeIDMap(500, this);
                            fNodeIDMap->add((DOMAttr*)nattr);
                        }
                    }
                }
            newnode=newelement;

        }
        break;
    case DOMNode::ATTRIBUTE_NODE :
        if (source->getLocalName() == 0)
            newnode = createAttribute(source->getNodeName());
        else
            newnode = createAttributeNS(source->getNamespaceURI(),
            source->getNodeName());
        deep = true;
        // Kids carry value

        break;
    case DOMNode::TEXT_NODE :
        newnode = createTextNode(source->getNodeValue());
        break;
    case DOMNode::CDATA_SECTION_NODE :
        newnode = createCDATASection(source->getNodeValue());
        break;
    case DOMNode::ENTITY_REFERENCE_NODE :
        {
            DOMEntityReferenceImpl* newentityRef = (DOMEntityReferenceImpl*)createEntityReference(source->getNodeName());
            newnode=newentityRef;
            errorChecking = false;
            newentityRef->setReadOnly(false, true); //allow deep import temporarily
        }
        break;
    case DOMNode::ENTITY_NODE :
        {
            DOMEntity *srcentity=(DOMEntity *)source;
            DOMEntityImpl *newentity = (DOMEntityImpl *)createEntity(source->getNodeName());
            newentity->setPublicId(srcentity->getPublicId());
            newentity->setSystemId(srcentity->getSystemId());
            newentity->setNotationName(srcentity->getNotationName());
            // Kids carry additional value
            newnode=newentity;
            castToNodeImpl(newentity)->setReadOnly(false, true);// allow deep import temporarily
        }
        break;
    case DOMNode::PROCESSING_INSTRUCTION_NODE :
        newnode = createProcessingInstruction(source->getNodeName(),
            source->getNodeValue());
        break;
    case DOMNode::COMMENT_NODE :
        newnode = createComment(source->getNodeValue());
        break;
    case DOMNode::DOCUMENT_TYPE_NODE :
        {
            // unless this is used as part of cloning a Document
            // forbid it for the sake of being compliant to the DOM spec
            if (!cloningDoc)
                throw DOMException(DOMException::NOT_SUPPORTED_ERR, 0);

            DOMDocumentType *srcdoctype = (DOMDocumentType *)source;
            DOMDocumentType *newdoctype = (DOMDocumentType *)
                createDocumentType(srcdoctype->getNodeName(),
                srcdoctype->getPublicId(),
                srcdoctype->getSystemId());
            // Values are on NamedNodeMaps
            DOMNamedNodeMap *smap = srcdoctype->getEntities();
            DOMNamedNodeMap *tmap = newdoctype->getEntities();
            if(smap != 0) {
                for(XMLSize_t i = 0; i < smap->getLength(); i++) {
                    tmap->setNamedItem(importNode(smap->item(i), true, false));
                }
            }
            smap = srcdoctype->getNotations();
            tmap = newdoctype->getNotations();
            if (smap != 0) {
                for(XMLSize_t i = 0; i < smap->getLength(); i++) {
                    tmap->setNamedItem(importNode(smap->item(i), true, false));
                }
            }
            // NOTE: At this time, the DOM definition of DocumentType
            // doesn't cover Elements and their Attributes. domimpl's
            // extentions in that area will not be preserved, even if
            // copying from domimpl to domimpl. We could special-case
            // that here. Arguably we should. Consider. ?????
            newnode = newdoctype;
        }
        break;
    case DOMNode::DOCUMENT_FRAGMENT_NODE :
        newnode = createDocumentFragment();
        // No name, kids carry value
        break;
    case DOMNode::NOTATION_NODE :
        {
            DOMNotation *srcnotation=(DOMNotation *)source;
            DOMNotationImpl *newnotation = (DOMNotationImpl *)createNotation(source->getNodeName());
            newnotation->setPublicId(srcnotation->getPublicId());
            newnotation->setSystemId(srcnotation->getSystemId());
            // Kids carry additional value
            newnode=newnotation;
            // No name, no value
            break;
        }

    case DOMNode::DOCUMENT_NODE : // Document can't be child of Document
    default:                       // Unknown node type
        throw DOMException(DOMException::NOT_SUPPORTED_ERR, 0);
    }

    // If deep, replicate and attach the kids.
    if (deep)
        for (DOMNode *srckid = source->getFirstChild();
             srckid != 0;
             srckid = srckid->getNextSibling())
        {
            newnode->appendChild(importNode(srckid, true, false));
        }

    if (newnode->getNodeType() == DOMNode::ENTITY_REFERENCE_NODE
        || newnode->getNodeType() == DOMNode::ENTITY_NODE) {
        castToNodeImpl(newnode)->setReadOnly(true, true);
        errorChecking = oldErrorCheckingFlag;
    }

    if (!cloningDoc)
        fNode.callUserDataHandlers(DOMUserDataHandler::NODE_IMPORTED, source, newnode);

    return newnode;
}

// user data utility
void* DOMDocumentImpl::setUserData(DOMNodeImpl* n, const XMLCh* key, void* data, DOMUserDataHandler* handler)
{
    void* oldData = 0;
    DOMNode_UserDataTable* node_userDataTable = 0;

    if (!fUserDataTable) {
        // create the table on heap so that it can be cleaned in destructor
        fUserDataTable = new (this) RefHashTableOf<DOMNode_UserDataTable>(29, true, new HashPtr());
    }
    else {
        node_userDataTable = fUserDataTable->get((void*)n);
        DOMUserDataRecord* oldDataRecord = 0;

        if (node_userDataTable) {
            oldDataRecord = node_userDataTable->get((void*)key);

            if (oldDataRecord) {
                oldData = oldDataRecord->getKey();
                node_userDataTable->removeKey((void*)key);
            }
        }
    }

    if (data) {

        // create the DOMNode_UserDataTable if not exists
        // create on the heap and adopted by the hashtable which will delete it upon removal.
        if (!node_userDataTable) {
            node_userDataTable  = new RefHashTableOf<DOMUserDataRecord>(29, true);
            fUserDataTable->put(n, node_userDataTable);
        }

        // clone the key first, and create the DOMUserDataRecord
        // create on the heap and adopted by the hashtable which will delete it upon removal.
        node_userDataTable->put((void*)getPooledString(key), new DOMUserDataRecord(data, handler));
    }
    else {
        if (node_userDataTable->isEmpty())
            n->hasUserData(false);
    }

    return oldData;
}

void* DOMDocumentImpl::getUserData(const DOMNodeImpl* n, const XMLCh* key) const
{
    if (fUserDataTable) {
        DOMNode_UserDataTable*  node_userDataTable = fUserDataTable->get((void*)n);

        if (node_userDataTable) {
            DOMUserDataRecord* dataRecord = node_userDataTable->get((void*)key);
            if (dataRecord)
                return dataRecord->getKey();
        }
    }

    return 0;
}

void DOMDocumentImpl::callUserDataHandlers(const DOMNodeImpl* n, DOMUserDataHandler::DOMOperationType operation, const DOMNode* src, const DOMNode* dst) const
{
    if (fUserDataTable) {
        DOMNode_UserDataTable*  node_userDataTable = fUserDataTable->get((void*)n);

        if (node_userDataTable) {
            RefHashTableOfEnumerator<DOMUserDataRecord> userDataEnum(node_userDataTable);

            // walk through the entire node_userDataTable table
            while (userDataEnum.hasMoreElements()) {
                // get the key
                XMLCh* key = (XMLCh*) userDataEnum.nextElementKey();

                // get the DOMUserDataRecord
                DOMUserDataRecord* userDataRecord = node_userDataTable->get((void*)key);

                // get the handler
                DOMUserDataHandler* handler = userDataRecord->getValue();

                if (handler) {
                    // get the data
                    void* data = userDataRecord->getKey();
                    handler->handle(operation, key, data, src, dst);
                }
            }
        }
    }
}


