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

#include "DOMRangeImpl.hpp"
#include "DOMDocumentImpl.hpp"
#include "DOMDocumentFragmentImpl.hpp"
#include "DOMCasts.hpp"

#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMRangeException.hpp>
#include <xercesc/dom/DOMText.hpp>

#include <xercesc/framework/XMLBuffer.hpp>

//---------------------
// C'tor and D'tor
//---------------------

DOMRangeImpl::DOMRangeImpl(DOMDocument* doc)

    :   fDocument(doc),
        fStartContainer(doc),
        fStartOffset(0),
        fEndContainer(doc),
        fEndOffset(0),
        fDetached(false),
        fCollapsed(true),
        fRemoveChild(0)
{
}

DOMRangeImpl::DOMRangeImpl(const DOMRangeImpl& other)
{
    fDocument = other.fDocument;
    fStartContainer = other.fStartContainer;
    fStartOffset = other.fStartOffset;
    fEndContainer = other.fEndContainer;
    fEndOffset = other.fEndOffset;
    fDetached = other.fDetached;
    fCollapsed = other.fCollapsed;
    fRemoveChild = other.fRemoveChild;
}

DOMRangeImpl::~DOMRangeImpl()
{
}


//-------------------------------
// Public getter functions
//-------------------------------


DOMNode* DOMRangeImpl::getStartContainer() const
{
    return fStartContainer;
}

DOMSize_t DOMRangeImpl::getStartOffset() const
{
    return fStartOffset;
}

DOMNode* DOMRangeImpl::getEndContainer() const
{
    return fEndContainer;
}

DOMSize_t DOMRangeImpl::getEndOffset() const
{
    return fEndOffset;
}



bool DOMRangeImpl::getCollapsed() const
{
    if (fDetached)
    {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0);
    }

    return ((fStartContainer == fEndContainer)
             && (fStartOffset == fEndOffset));
}

//-------------------------------
// Public getter functions
//-------------------------------

void DOMRangeImpl::setStartContainer(const DOMNode* node)
{
    if (fDetached)
    {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0);
    }

    fStartContainer = (DOMNode*) node;
}

void DOMRangeImpl::setStartOffset(DOMSize_t offset)
{
    if (fDetached)
    {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0);
    }

    fStartOffset = offset;
}

void DOMRangeImpl::setEndContainer(const DOMNode* node)
{
    if (fDetached)
    {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0);
    }

    fEndContainer = (DOMNode*) node;

}

void DOMRangeImpl::setEndOffset(DOMSize_t offset)
{
    if (fDetached)
    {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0);
    }

    fEndOffset = offset;
}

void DOMRangeImpl::setStart(const DOMNode* refNode, DOMSize_t offset)
{
    validateNode(refNode);
    checkIndex(refNode, offset);

    fStartContainer = (DOMNode*) refNode;
    fStartOffset    = offset;

    if ((fDocument != refNode->getOwnerDocument() )
        && (refNode->getOwnerDocument() != 0) )
    {
        fDocument = refNode->getOwnerDocument();
        collapse(true);
    }

    //compare the start and end boundary point
    //collapse if start point is after the end point
    if(compareBoundaryPoints(DOMRange::END_TO_START, this) == 1)
        collapse(true); //collapse the range positions to start
    else
        fCollapsed = false;
}

void DOMRangeImpl::setEnd(const DOMNode* refNode, DOMSize_t offset)
{
    validateNode(refNode);
    checkIndex(refNode, offset);

    fEndContainer   = (DOMNode*) refNode;
    fEndOffset      = offset;

    if ((fDocument != refNode->getOwnerDocument() )
        && (refNode->getOwnerDocument() != 0) )
    {
        fDocument = refNode->getOwnerDocument();
        collapse(false);
    }

    //compare the start and end boundary point
    //collapse if start point is after the end point
    if(compareBoundaryPoints(DOMRange::END_TO_START, this) == 1)
        collapse(false); //collapse the range positions to end
    else
        fCollapsed = false;
}

void DOMRangeImpl::setStartBefore(const DOMNode* refNode)
{
    if( fDetached) {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0);
    }
    if ( !hasLegalRootContainer(refNode) || !isLegalContainedNode(refNode)) {
        throw DOMRangeException(
            DOMRangeException::INVALID_NODE_TYPE_ERR, 0);
    }

    fStartContainer = refNode->getParentNode();
   DOMSize_t i = 0;
    for (DOMNode* n = (DOMNode*) refNode; n!=0; n = n->getPreviousSibling()) {
        i++;
    }
    if (i == 0)
        fStartOffset = 0;
    else
        fStartOffset = i-1;

    if ((fDocument != refNode->getOwnerDocument())
        && (refNode->getOwnerDocument() != 0) )
    {
        fDocument = refNode->getOwnerDocument();
        collapse(true);
    }

    //compare the start and end boundary point
    //collapse if start point is after the end point
    if(compareBoundaryPoints(DOMRange::END_TO_START, this) == 1)
        collapse(true); //collapse the range positions to start
    else
        fCollapsed = false;
}

void DOMRangeImpl::setStartAfter(const DOMNode* refNode)
{
    if( fDetached) {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0);
    }
    if ( !hasLegalRootContainer(refNode) || !isLegalContainedNode(refNode)) {
        throw DOMRangeException(
            DOMRangeException::INVALID_NODE_TYPE_ERR, 0);
    }

    fStartContainer = refNode->getParentNode();
    DOMSize_t i = 0;
    for (DOMNode* n = (DOMNode*) refNode; n!=0; n = n->getPreviousSibling()) {
        i++;
    }

    fStartOffset = i;

    if ((fDocument != refNode->getOwnerDocument() )
        && (refNode->getOwnerDocument() != 0) )
    {
        fDocument = refNode->getOwnerDocument();
        collapse(true);
    }

    //compare the start and end boundary point
    //collapse if start point is after the end point
    if(compareBoundaryPoints(DOMRange::END_TO_START, this) == 1)
        collapse(true); //collapse the range positions to start
    else
        fCollapsed = false;
}

void DOMRangeImpl::setEndBefore(const DOMNode* refNode)
{
    if( fDetached) {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0);
    }
    if ( !hasLegalRootContainer(refNode) || !isLegalContainedNode(refNode)) {
        throw DOMRangeException(
            DOMRangeException::INVALID_NODE_TYPE_ERR, 0);
    }

    fEndContainer = refNode->getParentNode();
    DOMSize_t i = 0;
    for (DOMNode* n = (DOMNode*) refNode; n!=0; n = n->getPreviousSibling(), i++) ;

    if (i< 1)
        fEndOffset = 0;
    else
        fEndOffset = i-1;

    if ((fDocument != refNode->getOwnerDocument() )
        && (refNode->getOwnerDocument() != 0) )
    {
        fDocument = refNode->getOwnerDocument();
        collapse(true);
    }

    //compare the start and end boundary point
    //collapse if start point is after the end point
    if(compareBoundaryPoints(DOMRange::END_TO_START, this) == 1)
        collapse(false); //collapse the range positions to end
    else
        fCollapsed = false;
}

void DOMRangeImpl::setEndAfter(const DOMNode* refNode)
{
    if( fDetached) {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0);
    }
    if ( !hasLegalRootContainer(refNode) || !isLegalContainedNode(refNode)) {
        throw DOMRangeException(
            DOMRangeException::INVALID_NODE_TYPE_ERR, 0);
    }

    fEndContainer = refNode->getParentNode();
    DOMSize_t i = 0;
    for (DOMNode* n = (DOMNode*) refNode; n!=0; n = n->getPreviousSibling(), i++) ;

    if (i ==0)
        fEndOffset = 0;
    else
        fEndOffset = i;

    if ((fDocument != refNode->getOwnerDocument() )
        && (refNode->getOwnerDocument() != 0) )
    {
        fDocument = refNode->getOwnerDocument();
        collapse(true);
    }

    //compare the start and end boundary point
    //collapse if start point is after the end point
    if(compareBoundaryPoints(DOMRange::END_TO_START, this) == 1)
        collapse(false); //collapse the range positions to end
    else
        fCollapsed = false;
}
//-------------------------------
// Public Misc. functions
//-------------------------------
void DOMRangeImpl::detach()
{
    if( fDetached) {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0);
    }

    fDetached = true;

    //0ify nodes
    fStartContainer = 0;
    fStartOffset    = 0;
    fEndContainer   = 0;
    fEndOffset      = 0;
    fCollapsed      = true;

    fRemoveChild    = 0;
}

void DOMRangeImpl::collapse(bool toStart)
{
    if( fDetached) {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0);
    }

    if (toStart) {
        fEndContainer = fStartContainer;
        fEndOffset = fStartOffset;
    } else {
        fStartContainer = fEndContainer;
        fStartOffset = fEndOffset;
    }
    fCollapsed = true;
}

void DOMRangeImpl::selectNode(const DOMNode* refNode)
{
    validateNode(refNode);
    if ( !isLegalContainedNode(refNode)) {
        throw DOMRangeException(
            DOMRangeException::INVALID_NODE_TYPE_ERR, 0);
    }
    //First check for the text type node
    if (refNode->getNodeType() ==  DOMNode::TEXT_NODE)
    {
        //The node itself is the container.
        fStartContainer = (DOMNode*) refNode;
        fEndContainer   = (DOMNode*) refNode;

        //Select all the contents of the node
        fStartOffset = 0;
        fEndOffset = ((DOMText *)refNode)->getLength();
        return;
    }

    DOMNode* parent = refNode->getParentNode();
    if (parent != 0 ) // REVIST: what to do if it IS 0?
    {
        fStartContainer = parent;
        fEndContainer = parent;

        DOMSize_t i = 0;
        for (DOMNode* n = parent->getFirstChild(); n!=0, n!=refNode; n = n->getNextSibling()) {
            i++;
        }

        fStartOffset = i;
        fEndOffset = fStartOffset+1;
    }
}

void DOMRangeImpl::selectNodeContents(const DOMNode* node)
{
    validateNode(node);

    fStartContainer = (DOMNode*) node;
    fEndContainer = (DOMNode*) node;

    fStartOffset = 0;
    if (node->getNodeType() == DOMNode::TEXT_NODE ) {
        fEndOffset = ((DOMText *)node)->getLength();
        return;
    }

    DOMNode* first = node->getFirstChild();
    if (first == 0) {
        fEndOffset = 0;
        return;
    }
    DOMSize_t i = 0;
    for (DOMNode* n = first; n!=0; n = n->getNextSibling()) {
        i++;
    }
    fEndOffset = i;
}

void DOMRangeImpl::surroundContents(DOMNode* newParent)
{
    if (newParent==0) return;

    //check for elimination criteria
    if( fDetached) {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0);
    }

    if (newParent->getOwnerDocument() !=fDocument) {
        throw DOMException(
            DOMException::WRONG_DOCUMENT_ERR, 0);
    }

    int type = newParent->getNodeType();
    if ( !isLegalContainedNode(newParent)
        || type == DOMNode::DOCUMENT_TYPE_NODE)
    {
        throw DOMRangeException(
            DOMRangeException::INVALID_NODE_TYPE_ERR, 0);
    }

    DOMNode* root = (DOMNode*) getCommonAncestorContainer();

    DOMNode* realStart = fStartContainer;
    DOMNode* realEnd = fEndContainer;

    if (fStartContainer->getNodeType() == DOMNode::TEXT_NODE) {
        realStart = fStartContainer->getParentNode();
    }
    if (fEndContainer->getNodeType() == DOMNode::TEXT_NODE) {
        realEnd = fEndContainer->getParentNode();
    }

    if (realStart != realEnd) {
        throw DOMRangeException(
            DOMRangeException::BAD_BOUNDARYPOINTS_ERR, 0);
    }

    DOMDocumentFragment* frag = (DOMDocumentFragment*) extractContents();
    insertNode(newParent);
    newParent->appendChild(frag);
    selectNode(newParent);
}


short DOMRangeImpl::compareBoundaryPoints(DOMRange::CompareHow how, const DOMRange* srcRange) const
{
    if (fDocument != ((DOMRangeImpl*)srcRange)->fDocument) {
        throw DOMException(
            DOMException::WRONG_DOCUMENT_ERR, 0);
    }
    if( fDetached) {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0);
    }

    DOMNode* pointA;
    DOMNode* pointB;
    int offsetA, offsetB;

    switch (how)
    {
    case (DOMRange::START_TO_START) :
        pointB = srcRange->getStartContainer();
        pointA = fStartContainer;
        offsetB = srcRange->getStartOffset();
        offsetA = fStartOffset;
        break;
    case (DOMRange::START_TO_END) :
        pointB = srcRange->getStartContainer();
        pointA = fEndContainer;
        offsetB = srcRange->getStartOffset();
        offsetA = fEndOffset;
        break;
    case (DOMRange::END_TO_START) :
        pointB = srcRange->getEndContainer();
        pointA = fStartContainer;
        offsetB = srcRange->getEndOffset();
        offsetA = fStartOffset;
        break;
    case (DOMRange::END_TO_END) :
        pointB = srcRange->getEndContainer();
        pointA = fEndContainer;
        offsetB = srcRange->getEndOffset();
        offsetA = fEndOffset;
        break;
    }

    // case 1: same container
    if (pointA == pointB) {
        if (offsetA < offsetB) return -1; //A before B
        if (offsetA == offsetB) return 0; //A equal to B
        return 1; // A after B
    }
    // case 2: Child C of container A is ancestor of B
    for (DOMNode* node = pointA->getFirstChild(); node != 0; node=node->getNextSibling()) {
        if (isAncestorOf(node, pointB)) {
            int index = indexOf(node, pointA);
            if (offsetA <=  index) return -1;
            return 1;
        }
    }
    // case 3: Child C of container B is ancestor of A
    for (DOMNode* nd = pointB->getFirstChild(); nd != 0; nd=nd->getNextSibling()) {
        if (isAncestorOf(nd, pointA)) {
            int index = indexOf(nd, pointB);
            if (index < offsetB ) return -1;
            return 1; //B strictly before A
        }
    }

    // case 4: preorder traversal of context tree.
    DOMNode* ancestor = (DOMNode*) commonAncestorOf(pointA, pointB);
    DOMNode* current = ancestor;

    do {
        if (current == pointA) return -1;
        if (current == pointB) return 1;
        current = nextNode(current, true);
    }
    while (current!=0 && current!=ancestor);

    return -2; // this should never happen
}


void DOMRangeImpl:: deleteContents()
{
    traverseContents(DELETE_CONTENTS);
}

DOMDocumentFragment* DOMRangeImpl::extractContents()
{
    checkReadOnly(fStartContainer, fEndContainer, fStartOffset, fEndOffset);
    return traverseContents(EXTRACT_CONTENTS);
}

DOMDocumentFragment* DOMRangeImpl::cloneContents() const
{
    // cast off const.
    return ((DOMRangeImpl *)this)->traverseContents(CLONE_CONTENTS);
}


void DOMRangeImpl::insertNode(DOMNode* newNode)
{
    if (newNode == 0) return; //don't have to do anything

    for (DOMNode* aNode = fStartContainer; aNode!=0; aNode = aNode->getParentNode()) {
        if (castToNodeImpl(newNode)->isReadOnly()) {
        throw DOMException(
            DOMException::NO_MODIFICATION_ALLOWED_ERR, 0);
    }
    }

    if (fDocument != newNode->getOwnerDocument()) {
        throw DOMException(
            DOMException::WRONG_DOCUMENT_ERR, 0);
    }

    // Prevent cycles in the tree.
    //isKidOK() is not checked here as its taken care by insertBefore() function
    if (isAncestorOf( newNode, fStartContainer)) {
        throw DOMException(
            DOMException::HIERARCHY_REQUEST_ERR, 0);
    }

    if( fDetached) {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0);
    }

    int type = newNode->getNodeType();
    if (type == DOMNode::ATTRIBUTE_NODE
        || type == DOMNode::ENTITY_NODE
        || type == DOMNode::NOTATION_NODE
        || type == DOMNode::DOCUMENT_NODE)
    {
        throw DOMRangeException(
            DOMRangeException::INVALID_NODE_TYPE_ERR, 0);
    }


    DOMNode* parent;
    DOMNode* next;

    if (fStartContainer->getNodeType() == DOMNode::TEXT_NODE) {

        //set 'parent' and 'next' here
        parent = fStartContainer->getParentNode();

        //split the text nodes
       if (fStartOffset > 0)
            ((DOMText*)fStartContainer)->splitText(fStartOffset);

        //update the new start information later. After inserting the first newNode
        if (fStartOffset == 0)
            next = fStartContainer;
        else
            next = fStartContainer->getNextSibling();

    } // end of text handling
    else {
        parent = fStartContainer;

        next = fStartContainer->getFirstChild();
        for(DOMSize_t i = 0; (i < fStartOffset) && (next != 0); i++) {
            next=next->getNextSibling();
        }
    }

    if (parent != 0) {
        if (next != 0)
            parent->insertBefore(newNode, next);
        else
            parent->appendChild(newNode);
    }
}

DOMRange* DOMRangeImpl::cloneRange() const
{
    if( fDetached) {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0);
    }

    DOMRange* range = fDocument->createRange();
    range->setStart(fStartContainer, fStartOffset);
    range->setEnd(fEndContainer, fEndOffset);

    return range;
}

const XMLCh* DOMRangeImpl::toString() const
{
    if( fDetached) {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0);
    }

    DOMNode* node = fStartContainer;
    DOMNode* stopNode = fEndContainer;

    XMLBuffer retStringBuf;
    if ( (fStartContainer->getNodeType() == DOMNode::TEXT_NODE)
        || (fStartContainer->getNodeType() == DOMNode::CDATA_SECTION_NODE) ) {
        if (fStartContainer == fEndContainer) {
            if (fEndOffset == fStartOffset) {
                return XMLUni::fgZeroLenString;
            }
            else {

                XMLCh* tempString;
                XMLCh temp[4000];
                if ((fEndOffset-fStartOffset) >= 3999)
                    tempString = new XMLCh[fEndOffset-fStartOffset+1];
                else
                    tempString = temp;

                XMLString::subString(tempString, fStartContainer->getNodeValue(), fStartOffset, fEndOffset);
                const XMLCh* retString = ((DOMDocumentImpl *)fDocument)->getPooledString(tempString);

                if ((fEndOffset-fStartOffset) >= 3999)
                    delete[] tempString;

                return retString;
            }

        } else {
            DOMSize_t length = XMLString::stringLen(fStartContainer->getNodeValue());
            if (length != fStartOffset) {

                XMLCh* tempString;
                XMLCh temp[4000];
                if ((length - fStartOffset) >= 3999)
                    tempString = new XMLCh[length - fStartOffset+1];
                else
                    tempString = temp;

                XMLString::subString(tempString, fStartContainer->getNodeValue(), fStartOffset, length);
                retStringBuf.append(tempString);

                if ((length - fStartOffset) >= 3999)
                    delete[] tempString;
            }

            node = nextNode(node, true);
        }
    }else { //fStartContainer is not a TextNode
        node=node->getFirstChild();
        if (fStartOffset>0) { //find a first node within a range, specified by fStartOffset
            DOMSize_t counter = 0;
            while (counter<fStartOffset && node!=0) {
                node=node->getNextSibling();
                counter++;
            }
        }
        if (node == 0) {
            node = nextNode(fStartContainer,false);
        }
    }

    if ( fEndContainer->getNodeType()!= DOMNode::TEXT_NODE &&
        fEndContainer->getNodeType()!= DOMNode::CDATA_SECTION_NODE ){
        int i=fEndOffset;
        stopNode = fEndContainer->getFirstChild();
        while( i>0 && stopNode!=0 ){
            --i;
            stopNode = stopNode->getNextSibling();
        }
        if ( stopNode == 0 )
            stopNode = nextNode( fEndContainer, false );
    }

    while (node != stopNode) {  //look into all kids of the Range
        if (node == 0) break;
        if (node->getNodeType() == DOMNode::TEXT_NODE
            ||  node->getNodeType() == DOMNode::CDATA_SECTION_NODE) {
            retStringBuf.append(node->getNodeValue());
        }
        node = nextNode(node, true);
    }

    if (fEndContainer->getNodeType() == DOMNode::TEXT_NODE
        || fEndContainer->getNodeType() == DOMNode::CDATA_SECTION_NODE) {

        if (fEndOffset != 0) {

            XMLCh* tempString;
            XMLCh temp[4000];
            if (fEndOffset >= 3999)
                tempString = new XMLCh[fEndOffset+1];
            else
                tempString = temp;

            XMLString::subString(tempString, fEndContainer->getNodeValue(), 0, fEndOffset);
            retStringBuf.append(tempString);

            if (fEndOffset >= 3999)
                delete[] tempString;
        }
    }
    return ((DOMDocumentImpl *)fDocument)->getPooledString(retStringBuf.getRawBuffer());
}

DOMDocument* DOMRangeImpl::getDocument()
{
    return fDocument;
}

const DOMNode* DOMRangeImpl::getCommonAncestorContainer() const
{
     return commonAncestorOf(fStartContainer, fEndContainer);

}

//---------------------
//private functions
//---------------------

bool DOMRangeImpl::isValidAncestorType(const DOMNode* node) const
{
    for (DOMNode* aNode = (DOMNode*) node; aNode!=0; aNode = aNode->getParentNode()) {
        short type = aNode->getNodeType();
        if ( type == DOMNode::ENTITY_NODE
            || type == DOMNode::NOTATION_NODE
            || type == DOMNode::DOCUMENT_TYPE_NODE)
            return false;
    }
    return true;
}

bool DOMRangeImpl::isAncestorOf(const DOMNode* a, const DOMNode* b) {
    for (DOMNode* node = (DOMNode*) b; node != 0; node=node->getParentNode()) {
        if  (node == a) return true;
    }
    return false;
}

bool DOMRangeImpl::hasLegalRootContainer(const DOMNode* node) const {
    if ( node==0 )
        return false;

    DOMNode* rootContainer = (DOMNode*)node;
    for (; rootContainer->getParentNode()!=0; rootContainer = rootContainer->getParentNode())
        ;

    switch( rootContainer->getNodeType() ) {
        case DOMNode::ATTRIBUTE_NODE:
        case DOMNode::DOCUMENT_NODE:
        case DOMNode::DOCUMENT_FRAGMENT_NODE:
        return true;
    }
    return false;
}

bool DOMRangeImpl::isLegalContainedNode(const DOMNode* node ) const {
   if ( node==0 )
       return false;
   switch( node->getNodeType() )
   {
       case DOMNode::DOCUMENT_NODE:
       case DOMNode::DOCUMENT_FRAGMENT_NODE:
       case DOMNode::ATTRIBUTE_NODE:
       case DOMNode::ENTITY_NODE:
       case DOMNode::NOTATION_NODE:
       return false;
   }
   return true;
}

DOMSize_t DOMRangeImpl::indexOf(const DOMNode* child, const DOMNode* parent) const
{
    DOMSize_t i = 0;
    if (child->getParentNode() != parent) return (DOMSize_t)-1;
    for(DOMNode* node = child->getPreviousSibling(); node!= 0; node=node->getPreviousSibling()) {
        i++;
    }
    return i;
}

void DOMRangeImpl::validateNode(const DOMNode* node) const
{
    if( fDetached) {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0);
    }

    if ( !isValidAncestorType(node)) {
        throw DOMRangeException(
            DOMRangeException::INVALID_NODE_TYPE_ERR, 0);
    }
}


const DOMNode* DOMRangeImpl::commonAncestorOf(const DOMNode* pointA, const DOMNode* pointB) const
{
    if (fDetached)
            throw DOMException(DOMException::INVALID_STATE_ERR, 0);

    if (pointA->getOwnerDocument() != pointB->getOwnerDocument())
        throw DOMException( DOMException::WRONG_DOCUMENT_ERR, 0 );

    //if the containers are same then it itself is its common ancestor.
    if (pointA == pointB)
        return pointA;

    typedef RefVectorOf<DOMNode> VectorNodes;
    VectorNodes startV(1, false);
    DOMNode* node;

    for (node=fStartContainer; node != 0; node=node->getParentNode())
    {
        startV.addElement(node);
    }
    VectorNodes endV(1, false);
    for (node=fEndContainer; node != 0; node=node->getParentNode())
    {
        endV.addElement(node);
    }

    int s = startV.size()-1;
    int e = endV.size()-1;

    DOMNode* commonAncestor;

    while (s>=0 && e>=0) {
        if (startV.elementAt(s) == endV.elementAt(e)) {
            commonAncestor = startV.elementAt(s);
        }
        else  break;
        --s;
        --e;
    }

    return commonAncestor;
}

void DOMRangeImpl::checkIndex(const DOMNode* node, DOMSize_t offset) const
{
    if (offset < 0) {
        throw DOMException( DOMException::INDEX_SIZE_ERR, 0 );
    }

    short type = node->getNodeType();

    if((type == DOMNode::TEXT_NODE
        || type == DOMNode::CDATA_SECTION_NODE
        || type == DOMNode::COMMENT_NODE
        || type == DOMNode::PROCESSING_INSTRUCTION_NODE)) {
        if (offset > XMLString::stringLen(node->getNodeValue()))
            throw DOMException( DOMException::INDEX_SIZE_ERR, 0 );
        else  return;
    }

    DOMNode* child = node->getFirstChild();
    DOMSize_t i = 0;
    for (; child != 0; i++) {
        child = child->getNextSibling();
    }
    if (i < offset) {
        throw DOMException( DOMException::INDEX_SIZE_ERR, 0 );
    }

}

DOMNode* DOMRangeImpl::nextNode(const DOMNode* node, bool visitChildren) const
{

    if (node == 0) return 0;

    DOMNode* result;
    if (visitChildren) {
        result = node->getFirstChild();
        if (result != 0) {
            return result;
        }
    }

    // if hasSibling, return sibling
    result = node->getNextSibling();
    if (result != 0) {
        return result;
    }


    // return parent's 1st sibling.
    DOMNode* parent = node->getParentNode();


    while ( (parent != 0) && (parent != fDocument) )
    {
        result = parent->getNextSibling();
        if (result != 0) {
            return result;
        } else {
            parent = parent->getParentNode();
            if (parent == fEndContainer) return parent;

        }

    }
    // end of list, return 0
    return 0;
}


/** This is the master routine invoked to visit the nodes
*   selected by this range.  For each such node, different
*   actions are taken depending on the value of the TraversalType argument.
*/
DOMDocumentFragment* DOMRangeImpl::traverseContents(TraversalType how)
{
    if (fDetached)
            throw DOMException(DOMException::INVALID_STATE_ERR, 0);

    if (fStartContainer == 0 || fEndContainer == 0) {
        return 0; // REVIST: Throw exception?
    }

    /* Traversal is accomplished by first determining the
       relationship between the endpoints of the range.
       For each of four significant relationships, we will
       delegate the traversal call to a method that
       can make appropriate assumptions.
    */

    // case 1: same container
    if ( fStartContainer == fEndContainer )
        return traverseSameContainer( how );

    // case 2: Child C of start container is ancestor of end container
    for (DOMNode* node = fStartContainer->getFirstChild(); node != 0; node=node->getNextSibling()) {
        if (isAncestorOf(node, fEndContainer))
            return traverseCommonStartContainer( node, how );
    }

    // case 3: Child C of end container  is ancestor of start container
    for (DOMNode* nd = fEndContainer->getFirstChild(); nd != 0; nd=nd->getNextSibling()) {
        if (isAncestorOf(nd, fStartContainer))
             return traverseCommonEndContainer( nd, how );
        }

    // case 4: preorder traversal of context tree.
    // There is a common ancestor container.  Find the
    // ancestor siblings that are children of that container.
    DOMNode* ancestor = (DOMNode*)commonAncestorOf(fStartContainer, fEndContainer);
    return traverseCommonAncestors( ancestor, ancestor, how );
    }

/**
 * Visits the nodes selected by this range when we know
 * a-priori that the start and end containers are the same.
 *
 */
DOMDocumentFragment* DOMRangeImpl::traverseSameContainer( int how )
{
    DOMDocumentFragment* frag = 0;
    if ( how!=DELETE_CONTENTS)
        frag = fDocument->createDocumentFragment();

    // If selection is empty, just return the fragment
    if ( fStartOffset==fEndOffset )
            return frag;

    DOMNode* current = fStartContainer;
    DOMNode* cloneCurrent = 0;

    // Text node needs special case handling
    if ( fStartContainer->getNodeType()== DOMNode::TEXT_NODE )
    {
        cloneCurrent = fStartContainer->cloneNode(false);
        if (fEndOffset == fStartOffset) {
            cloneCurrent->setNodeValue(XMLUni::fgZeroLenString);
        }
        else {
            XMLCh* tempString;
            XMLCh temp[4000];
            if (fEndOffset >= 3999)
                tempString = new XMLCh[fEndOffset+1];
            else
                tempString = temp;

            XMLString::subString(tempString, cloneCurrent->getNodeValue(), fStartOffset, fEndOffset);
            cloneCurrent->setNodeValue(((DOMDocumentImpl *)fDocument)->getPooledString(tempString));

            if (fEndOffset >= 3999)
                delete[] tempString;
        }

        // set the original text node to its new value
        if ( how != CLONE_CONTENTS )
            ((DOMText*)fStartContainer)->deleteData(fStartOffset, fEndOffset-fStartOffset);
        if ( how != DELETE_CONTENTS)
            frag->appendChild(cloneCurrent);
    }
    else {
        // Copy nodes between the start/end offsets.
        DOMNode* n = getSelectedNode( fStartContainer, fStartOffset );
        int cnt = fEndOffset - fStartOffset;
        while( cnt > 0 )
        {
            DOMNode* sibling = n->getNextSibling();
            DOMNode* xferNode = traverseFullySelected( n, how );
            if ( frag!=0 )
                frag->appendChild( xferNode );
            --cnt;
            n = sibling;
            }
    }

    // Nothing is partially selected, so collapse to start point
    if ( how != CLONE_CONTENTS )
            collapse(true);
    return frag;
}

/**
 * Visits the nodes selected by this range when we know
 * a-priori that the start and end containers are not the
 * same, but the start container is an ancestor of the end container
 *
 */
DOMDocumentFragment* DOMRangeImpl::traverseCommonStartContainer( DOMNode*endAncestor, int how )
{
    DOMDocumentFragment* frag = 0;
    if ( how!=DELETE_CONTENTS)
        frag = fDocument->createDocumentFragment();
    DOMNode*n = traverseRightBoundary( endAncestor, how );
    if ( frag!=0 )
        frag->appendChild( n );

    int endIdx = indexOf( endAncestor, fStartContainer );
    int cnt = endIdx - fStartOffset;
    if ( cnt <=0 )
    {
        // Collapse to just before the endAncestor, which
        // is partially selected.
        if ( how != CLONE_CONTENTS )
        {
            setEndBefore( endAncestor );
            collapse( false );
        }
        return frag;
    }

    n = endAncestor->getPreviousSibling();
    while( cnt > 0 )
    {
        DOMNode* sibling = n->getPreviousSibling();
        DOMNode* xferNode = traverseFullySelected( n, how );
        if ( frag!=0 )
            frag->insertBefore( xferNode, frag->getFirstChild() );
        --cnt;
        n = sibling;
    }
    // Collapse to just before the endAncestor, which
    // is partially selected.
    if ( how != CLONE_CONTENTS )
    {
        setEndBefore( endAncestor );
        collapse( false );
    }
    return frag;
}

/**
 * Visits the nodes selected by this range when we know
 * a-priori that the start and end containers are not the
 * same, but the end container is an ancestor of the start container
 *
 */
DOMDocumentFragment* DOMRangeImpl::traverseCommonEndContainer( DOMNode*startAncestor, int how )
{
    DOMDocumentFragment* frag = 0;
    if ( how!=DELETE_CONTENTS)
        frag = fDocument->createDocumentFragment();
    DOMNode* n = traverseLeftBoundary( startAncestor, how );
    if ( frag!=0 )
        frag->appendChild( n );
    int startIdx = indexOf( startAncestor, fEndContainer );
    ++startIdx;  // Because we already traversed it....

    int cnt = fEndOffset - startIdx;
    n = startAncestor->getNextSibling();
    while( cnt > 0 )
    {
        DOMNode* sibling = n->getNextSibling();
        DOMNode* xferNode = traverseFullySelected( n, how );
        if ( frag!=0 )
            frag->appendChild( xferNode );
        --cnt;
        n = sibling;
    }

    if ( how != CLONE_CONTENTS )
    {
        setStartAfter( startAncestor );
        collapse( true );
    }

    return frag;
}

/**
 * Visits the nodes selected by this range when we know
 * a-priori that the start and end containers are not
 * the same, and we also know that neither the start
 * nor end container is an ancestor of the other.
 */
DOMDocumentFragment* DOMRangeImpl::traverseCommonAncestors( DOMNode*startAncestor, DOMNode*endAncestor, int how )
{
    DOMDocumentFragment* frag = 0;
    if ( how!=DELETE_CONTENTS)
        frag = fDocument->createDocumentFragment();

    DOMNode*n = traverseLeftBoundary( startAncestor, how );
    if ( frag!=0 )
        frag->appendChild( n );

    DOMNode*commonParent = startAncestor->getParentNode();
    int startOffset = indexOf( startAncestor, commonParent );
    int endOffset = indexOf( endAncestor, commonParent );
    ++startOffset;

    int cnt = endOffset - startOffset;
    DOMNode* sibling = startAncestor->getNextSibling();

    while( cnt > 0 )
    {
        DOMNode* nextSibling = sibling->getNextSibling();
        n = traverseFullySelected( sibling, how );
        if ( frag!=0 )
            frag->appendChild( n );
        sibling = nextSibling;
        --cnt;
    }

    n = traverseRightBoundary( endAncestor, how );
    if ( frag!=0 )
        frag->appendChild( n );

    if ( how != CLONE_CONTENTS )
    {
        setStartAfter( startAncestor );
        collapse( true );
    }
    return frag;
}

/**
 * Traverses the "right boundary" of this range and
 * operates on each "boundary node" according to the
 * how parameter.  It is a-priori assumed
 * by this method that the right boundary does
 * not contain the range's start container.
 *
 * A "right boundary" is best visualized by thinking
 * of a sample tree:
 *                 A
 *                /|\
 *               / | \
 *              /  |  \
 *             B   C   D
 *            /|\     /|\
 *           E F G   H I J
 *
 * Imagine first a range that begins between the
 * "E" and "F" nodes and ends between the
 * "I" and "J" nodes.  The start container is
 * "B" and the end container is "D".  Given this setup,
 * the following applies:
 *
 * Partially Selected Nodes: B, D<br>
 * Fully Selected Nodes: F, G, C, H, I
 *
 * The "right boundary" is the highest subtree node
 * that contains the ending container.  The root of
 * this subtree is always partially selected.
 *
 * In this example, the nodes that are traversed
 * as "right boundary" nodes are: H, I, and D.
 *
 */
DOMNode*DOMRangeImpl::traverseRightBoundary( DOMNode*root, int how )
{
    DOMNode*next = getSelectedNode( fEndContainer, fEndOffset-1 );
    bool isFullySelected = ( next!=fEndContainer );

    if ( next==root )
        return traverseNode( next, isFullySelected, false, how );

    DOMNode*parent = next->getParentNode();
    DOMNode*clonedParent = traverseNode( parent, false, false, how );

    while( parent!=0 )
    {
        while( next!=0 )
        {
            DOMNode* prevSibling = next->getPreviousSibling();
            DOMNode* clonedChild =
                traverseNode( next, isFullySelected, false, how );
            if ( how!=DELETE_CONTENTS )
            {
                clonedParent->insertBefore(
                    clonedChild,
                    clonedParent->getFirstChild()
                );
            }
            isFullySelected = true;
            next = prevSibling;
        }
        if ( parent==root )
            return clonedParent;

        next = parent->getPreviousSibling();
        parent = parent->getParentNode();
        DOMNode* clonedGrandParent = traverseNode( parent, false, false, how );
        if ( how!=DELETE_CONTENTS )
            clonedGrandParent->appendChild( clonedParent );
        clonedParent = clonedGrandParent;

    }

    // should never occur
    return 0;
}

/**
 * Traverses the "left boundary" of this range and
 * operates on each "boundary node" according to the
 * how parameter.  It is a-priori assumed
 * by this method that the left boundary does
 * not contain the range's end container.
 *
 * A "left boundary" is best visualized by thinking
 * of a sample tree:
 *
 *                 A
 *                /|\
 *               / | \
 *              /  |  \
 *             B   C   D
 *            /|\     /|\
 *           E F G   H I J
 *
 * Imagine first a range that begins between the
 * "E" and "F" nodes and ends between the
 * "I" and "J" nodes.  The start container is
 * "B" and the end container is "D".  Given this setup,
 * the following applies:
 *
 * Partially Selected Nodes: B, D<br>
 * Fully Selected Nodes: F, G, C, H, I
 *
 * The "left boundary" is the highest subtree node
 * that contains the starting container.  The root of
 * this subtree is always partially selected.
 *
 * In this example, the nodes that are traversed
 * as "left boundary" nodes are: F, G, and B.
 *
 */
DOMNode*DOMRangeImpl::traverseLeftBoundary( DOMNode*root, int how )
{
    DOMNode*next = getSelectedNode( getStartContainer(), getStartOffset() );
    bool isFullySelected = ( next!=getStartContainer() );

    if ( next==root )
        return traverseNode( next, isFullySelected, true, how );

    DOMNode* parent = next->getParentNode();
    DOMNode* clonedParent = traverseNode( parent, false, true, how );

    while( parent!=0 )
    {
        while( next!=0 )
        {
            DOMNode* nextSibling = next->getNextSibling();
            DOMNode* clonedChild =
                traverseNode( next, isFullySelected, true, how );
            if ( how!=DELETE_CONTENTS )
                clonedParent->appendChild(clonedChild);
            isFullySelected = true;
            next = nextSibling;
        }
        if ( parent==root )
            return clonedParent;

        next = parent->getNextSibling();
        parent = parent->getParentNode();
        DOMNode* clonedGrandParent = traverseNode( parent, false, true, how );
        if ( how!=DELETE_CONTENTS )
            clonedGrandParent->appendChild( clonedParent );
        clonedParent = clonedGrandParent;

    }

    // should never occur
    return 0;

}

/**
 * Utility method for traversing a single node.
 * Does not properly handle a text node containing both the
 * start and end offsets.  Such nodes should
 * have been previously detected and been routed to traverseTextNode.
 *
 */
DOMNode*DOMRangeImpl::traverseNode( DOMNode* n, bool isFullySelected, bool isLeft, int how )
{
    if ( isFullySelected )
        return traverseFullySelected( n, how );
    if ( n->getNodeType()== DOMNode::TEXT_NODE )
        return traverseTextNode( n, isLeft, how );
    return traversePartiallySelected( n, how );
}

/**
 * Utility method for traversing a single node when
 * we know a-priori that the node if fully
 * selected.
 *
 */
DOMNode*DOMRangeImpl::traverseFullySelected( DOMNode* n, int how )
{
    switch( how )
    {
    case CLONE_CONTENTS:
        return n->cloneNode( true );
    case EXTRACT_CONTENTS:
        if ( n->getNodeType()== DOMNode::DOCUMENT_TYPE_NODE )
        {
            throw DOMException(
                DOMException::HIERARCHY_REQUEST_ERR, 0);
        }
        return n;
    case DELETE_CONTENTS:
        n->getParentNode()->removeChild(n);
        return 0;
    }
    return 0;
}

/**
 * Utility method for traversing a single node when
 * we know a-priori that the node if partially
 * selected and is not a text node.
 *
 */
DOMNode*DOMRangeImpl::traversePartiallySelected( DOMNode*n, int how )
{
    switch( how )
    {
    case DELETE_CONTENTS:
        return 0;
    case CLONE_CONTENTS:
    case EXTRACT_CONTENTS:
        return n->cloneNode( false );
    }
    return 0;
}

/**
 * Utility method for traversing a text node that we know
 * a-priori to be on a left or right boundary of the range.
 * This method does not properly handle text nodes that contain
 * both the start and end points of the range.
 *
 */
DOMNode*DOMRangeImpl::traverseTextNode( DOMNode*n, bool isLeft, int how )
{
    const XMLCh* txtValue = n->getNodeValue();

    if ( isLeft )
    {
        int startLen = XMLString::stringLen(fStartContainer->getNodeValue());
        int offset = getStartOffset();

        if (offset == 0) {
            if ( how != CLONE_CONTENTS )
                n->setNodeValue(XMLUni::fgZeroLenString);
        }
        else {
            XMLCh* oldNodeValue;
            XMLCh oldTemp[4000];

            if (offset >= 3999)  {
                oldNodeValue = new XMLCh[offset+1];
            }
            else {
                oldNodeValue = oldTemp;
            }
            XMLString::subString(oldNodeValue, txtValue, 0, offset);

            if ( how != CLONE_CONTENTS )
                n->setNodeValue( ((DOMDocumentImpl *)fDocument)->getPooledString(oldNodeValue) );

            if (offset>= 3999)
                delete[] oldNodeValue;
        }

        if ( how==DELETE_CONTENTS )
            return 0;

        DOMNode* newNode = n->cloneNode( false );

        if (startLen == offset) {
            newNode->setNodeValue(XMLUni::fgZeroLenString);
        }
        else {
            XMLCh* newNodeValue;
            XMLCh newTemp[4000];

            if (offset >= 3999)  {
                newNodeValue = new XMLCh[offset+1];
            }
            else {
                newNodeValue = newTemp;
            }
            XMLString::subString(newNodeValue, txtValue, offset, startLen);
            newNode->setNodeValue( ((DOMDocumentImpl *)fDocument)->getPooledString(newNodeValue) );

            if (offset>= 3999)
                delete[] newNodeValue;

        }
        return newNode;
    }
    else
    {
        int endLen = XMLString::stringLen(fEndContainer->getNodeValue());
        int offset = getEndOffset();

        if (endLen == offset) {
            if ( how != CLONE_CONTENTS )
                n->setNodeValue(XMLUni::fgZeroLenString);
        }
        else {
            XMLCh* oldNodeValue;
            XMLCh oldTemp[4000];

            if (offset >= 3999)  {
                oldNodeValue = new XMLCh[offset+1];
            }
            else {
                oldNodeValue = oldTemp;
            }
            XMLString::subString(oldNodeValue, txtValue, offset, endLen);

            if ( how != CLONE_CONTENTS )
                n->setNodeValue( ((DOMDocumentImpl *)fDocument)->getPooledString(oldNodeValue) );

            if (offset>= 3999)
                delete[] oldNodeValue;
        }

        if ( how==DELETE_CONTENTS )
            return 0;

        DOMNode* newNode = n->cloneNode( false );

        if (offset == 0) {
            newNode->setNodeValue(XMLUni::fgZeroLenString);
        }
        else {
            XMLCh* newNodeValue;
            XMLCh newTemp[4000];

            if (offset >= 3999)  {
                newNodeValue = new XMLCh[offset+1];
            }
            else {
                newNodeValue = newTemp;
            }
            XMLString::subString(newNodeValue, txtValue, 0, offset);
            newNode->setNodeValue( ((DOMDocumentImpl *)fDocument)->getPooledString(newNodeValue) );

            if (offset>= 3999)
                delete[] newNodeValue;

        }
        return newNode;
    }
}

/**
 * Utility method to retrieve a child node by index.  This method
 * assumes the caller is trying to find out which node is
 * selected by the given index.  Note that if the index is
 * greater than the number of children, this implies that the
 * first node selected is the parent node itself.
 *
 */
DOMNode*DOMRangeImpl::getSelectedNode( DOMNode*container, int offset )
{
    if ( container->getNodeType() == DOMNode::TEXT_NODE )
        return container;

    // This case is an important convenience for
    // traverseRightBoundary()
    if ( offset<0 )
        return container;

    DOMNode*child = container->getFirstChild();
    while( child!=0 && offset > 0 )
    {
        --offset;
        child = child->getNextSibling();
    }
    if ( child!=0 )
        return child;
    return container;
}

void DOMRangeImpl::checkReadOnly(DOMNode* start, DOMNode* end,
                              DOMSize_t startOffset, DOMSize_t endOffset)
{
    if ((start == 0) || (end == 0) ) return;
    //if both start and end are text check and return
    if (start->getNodeType() == DOMNode::TEXT_NODE) {
        if (castToNodeImpl(start)->isReadOnly()) {
            throw DOMException(
                DOMException::NO_MODIFICATION_ALLOWED_ERR, 0);
        }
        if (start == end)
            return;
    }
    //set the start and end nodes to check
    DOMNode*sNode = start->getFirstChild();
    for(DOMSize_t i = 0; i<startOffset; i++)
        sNode = sNode->getNextSibling();

    DOMNode* eNode;
    if (end->getNodeType() == DOMNode::TEXT_NODE) {
        eNode = end; //need to check only till this node
    }
    else { //need to check all the kids that fall before the end offset value
        eNode = end->getFirstChild();
        for (DOMSize_t i = 0; i<endOffset-1; i++)
            eNode = eNode->getNextSibling();
    }
    //recursivly search if any node is readonly
    recurseTreeAndCheck(sNode, eNode);
}

void DOMRangeImpl::recurseTreeAndCheck(DOMNode* start, DOMNode* end)
{
    for(DOMNode* node=start; node != 0 && node !=end; node=node->getNextSibling())
    {
        if (castToNodeImpl(node)->isReadOnly()) {
            throw DOMException(
                DOMException::NO_MODIFICATION_ALLOWED_ERR, 0);
        }

        if (node->hasChildNodes()) {
            node = node->getFirstChild();
            recurseTreeAndCheck(node, end);
        }
    }
}


DOMNode*DOMRangeImpl::removeChild(DOMNode* parent, DOMNode* child)
{
    fRemoveChild = child; //only a precaution measure not to update this range data before removal
    DOMNode*n = parent->removeChild(child);
    fRemoveChild = 0;
    return n;
}


//
// Mutation functions
//


/* This function is called from DOM.
*  The  text has already beeen replaced.
*  Fix-up any offsets.
*/
void DOMRangeImpl::receiveReplacedText(DOMNode* node)
{
    if (node == 0) return;

    if (node == fStartContainer
        && fStartContainer->getNodeType() == DOMNode::TEXT_NODE) {
        fStartOffset = 0;
    }
    if (node == fEndContainer
        && fEndContainer->getNodeType() == DOMNode::TEXT_NODE) {
        fEndOffset = 0;
    }
}


/** This function is called from DOM.
*  The  text has already beeen inserted.
*  Fix-up any offsets.
*/
void DOMRangeImpl::updateRangeForDeletedText(DOMNode* node, DOMSize_t offset, int count)
{
    if (node == 0) return;

    if (node == fStartContainer
        && fStartContainer->getNodeType() == DOMNode::TEXT_NODE) {
        if (fStartOffset > offset+count) {
            fStartOffset = fStartOffset-count;
        } else if (fStartOffset > offset) {
            fStartOffset = offset;
        }
    }
    if (node == fEndContainer
        && fEndContainer->getNodeType() == DOMNode::TEXT_NODE) {
        if (fEndOffset > offset+count) {
            fEndOffset = fEndOffset-count;
        } else if (fEndOffset > offset) {
            fEndOffset = offset;
        }
    }
}



/** This function must be called by the DOM _BEFORE_
*  a node is deleted, because at that time it is
*  connected in the DOM tree, which we depend on.
*/
void DOMRangeImpl::updateRangeForDeletedNode(DOMNode* node)
{

    if (node == 0) return;
    if (fRemoveChild == node) return;

    if (node->getParentNode() == fStartContainer) {
        DOMSize_t index = indexOf(node, fStartContainer);
        if ( fStartOffset > index) {
            fStartOffset--;
        }
    }

    if (node->getParentNode() == fEndContainer) {
        DOMSize_t index = indexOf(node, fEndContainer);
        if ( fEndOffset > index) {
            fEndOffset--;
        }
    }

    if (node->getParentNode() != fStartContainer
        ||  node->getParentNode() != fEndContainer) {
        if (isAncestorOf(node, fStartContainer)) {
            DOMNode* tpNode = node->getParentNode();
            setStartContainer( tpNode );
            fStartOffset = indexOf( node, tpNode);
        }
        if (isAncestorOf(node, fEndContainer)) {
            DOMNode* tpNode = node->getParentNode();
            setEndContainer( tpNode );
            fEndOffset = indexOf( node, tpNode);
        }
    }

}

void DOMRangeImpl::updateRangeForInsertedNode(DOMNode* node) {
    if (node == 0) return;

    if (node->getParentNode() == fStartContainer) {
        DOMSize_t index = indexOf(node, fStartContainer);
        if (index < fStartOffset) {
            fStartOffset++;
        }
    }

    if (node->getParentNode() == fEndContainer) {
        DOMSize_t index = indexOf(node, fEndContainer);
        if (index < fEndOffset) {
            fEndOffset++;
        }
    }
}


void DOMRangeImpl::updateSplitInfo(DOMText* oldNode, DOMText* startNode, DOMSize_t offset)
{
    if (startNode == 0) return;

    if (fStartContainer == oldNode && fStartOffset > offset) {
          fStartOffset = fStartOffset - offset;
        fStartContainer = startNode;
    }

    if (fEndContainer == oldNode && fEndOffset > offset) {
            fEndContainer = startNode;
       fEndOffset = fEndOffset - offset;
    }
}



