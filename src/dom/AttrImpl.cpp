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
 * $Id$
 *
 * <p><b>WARNING</b>: Some of the code here is partially duplicated in
 * ParentNode, be careful to keep these two classes in sync!
 */

#include "AttrImpl.hpp"
#include "DOM_DOMException.hpp"
#include "DocumentImpl.hpp"
#include "TextImpl.hpp"
#include "ElementImpl.hpp"
#include "DStringPool.hpp"
#include "NodeIDMap.hpp"
#include "RangeImpl.hpp"

/*
 * The handling of the value field being either the first child node (a
 * ChildNode*) or directly the value (a DOMString) is rather tricky. In the
 * DOMString case we need to get the field in the right type so that the
 * compiler is happy and the appropriate operator gets called. This is
 * essential for the reference counts of the DOMStrings involved to be updated
 * as due.
 * This is consistently achieved by taking the address of the value field and
 * changing it into a DOMString*, and then dereferencing it to get a DOMString.
 * The typical piece of code is:
 * DOMString *x = (DOMString *)&value;
 *  ... use of *x which is the DOMString ...
 */

AttrImpl::AttrImpl(DocumentImpl *ownerDoc, const DOMString &aName) 
    : NodeImpl (ownerDoc)
{
    name = aName.clone();
    isSpecified(true);
    hasStringValue(true);
    value = null;
};

AttrImpl::AttrImpl(const AttrImpl &other, bool deep)
    : NodeImpl(other)
{
    name = other.name.clone();
	
    isSpecified(other.isSpecified());

    /* We must initialize the void* value to null in *all* cases. Failing to do
     * so would cause, in case of assignment to a DOMString later, its content
     * to be derefenced as a DOMString, which would lead the ref count code to
     * be called on something that is not actually a DOMString... Really bad
     * things would then happen!!!
     */
    value = null;
    hasStringValue(other.hasStringValue());

    if (other.isIdAttr())
    {
        isIdAttr(true);
        this->getOwnerDocument()->getNodeIDMap()->add(this);
    }
    
    // take care of case where there are kids
    if (!hasStringValue()) {
        cloneChildren(other);
    }
    else {
        // get the address of the value field of this as a DOMString*
        DOMString *x = (DOMString*) &value;
        // and the address of the value field of other as a DOMString*
        DOMString *y = (DOMString*) &(other.value);
        // We can now safely do the cloning and assignement, both operands
        // being a DOMString their ref counts will be updated appropriately
        *x = y->clone();
    }
};


AttrImpl::~AttrImpl() {
    if (hasStringValue()) {
        // if value is a DOMString we must make sure its ref count is updated.
        // this is achieved by changing the address of the value field into a
        // DOMString* and setting the value field to null
        DOMString *x = (DOMString *) &value;
        *x = null;
    }
}


// create a real Text node as child if we don't have one yet
void AttrImpl::makeChildNode() {
    if (hasStringValue()) {
        if (value != null) {
            // change the address of the value field into a DOMString*
            DOMString *x = (DOMString *) &value;
            // create a Text node passing the DOMString it points to
            TextImpl *text =
              (TextImpl *) getOwnerDocument()->createTextNode(*x);
            // get the DOMString ref count to be updated by setting the value
            // field to null
            *x = null;
            // finally reassign the value to the node address
            value = text;
            text->isFirstChild(true);
            text->previousSibling = text;
            text->ownerNode = this;
            text->isOwned(true);
        }
        hasStringValue(false);
    }
}

NodeImpl * AttrImpl::cloneNode(bool deep) 
{
    return new AttrImpl(*this, deep);
};


DOMString AttrImpl::getNodeName() {
    return name;
};


short AttrImpl::getNodeType() {
    return DOM_Node::ATTRIBUTE_NODE;
};


DOMString AttrImpl::getName()
{
    return name;
};


DOMString AttrImpl::getNodeValue()
{
    return getValue();
};


bool AttrImpl::getSpecified() 
{
    return isSpecified();
};




DOMString AttrImpl::getValue() 
{
    if (value == null) {
        return 0; // return "";
    }
    if (hasStringValue()) {
        // change value into a DOMString*
        DOMString *x = (DOMString *) &value;
        // return the DOMString it points to
        return *x;
    }
    ChildNode *firstChild = (ChildNode *) value;
    ChildNode *node = firstChild->nextSibling;
    if (node == null) {
        return firstChild->getNodeValue().clone();
    }
    int             length = 0;
    for (node = firstChild; node != null; node = node->nextSibling)
        length += node->getNodeValue().length();
    
    DOMString retString;
    retString.reserve(length);
    for (node = firstChild; node != null; node = node->nextSibling)
    {
        retString.appendData(node->getNodeValue());
    };
    
    return retString;
};


bool AttrImpl::isAttrImpl()
{
    return true;
};


void AttrImpl::setNodeValue(const DOMString &val)
{
    setValue(val);
};



void AttrImpl::setSpecified(bool arg)
{
    isSpecified(arg);
};



void AttrImpl::setValue(const DOMString &val)
{
    if (isReadOnly())
    {
        throw DOM_DOMException
        (
            DOM_DOMException::NO_MODIFICATION_ALLOWED_ERR, null
        );
    }
    
    //  If this attribute was of type ID and in the map, take it out,
    //    then put it back in with the new name.  For now, we don't worry
    //    about what happens if the new name conflicts
    //
    if (isIdAttr())
        this->getOwnerDocument()->getNodeIDMap()->remove(this);

    if (!hasStringValue() && value != null) {
        NodeImpl *kid;
        while ((kid = (ChildNode *) value) != null) { // Remove existing kids
            removeChild(kid);
            if (kid->nodeRefCount == 0)
                NodeImpl::deleteIf(kid);
        }
    }

    if (val != null) {
        // directly store the string as the value by changing the value field
        // into a DOMString
        DOMString *x = (DOMString *) &value;
        *x = val.clone();
        hasStringValue(true);
    }
    isSpecified(true);
    changed();
    
    if (isIdAttr())
        this->getOwnerDocument()->getNodeIDMap()->add(this);

};



DOMString AttrImpl::toString()
{
    DOMString retString;
    
    retString.appendData(name);
    retString.appendData(DOMString("=\""));
    retString.appendData(getValue());
    retString.appendData(DOMString("\""));
    return retString;
}


//Introduced in DOM Level 2

ElementImpl *AttrImpl::getOwnerElement()
{
    // if we have an owner, ownerNode is our ownerElement, otherwise it's
    // our ownerDocument and we don't have an ownerElement
    return (ElementImpl *) (isOwned() ? ownerNode : null);
}


//internal use by parser only
void AttrImpl::setOwnerElement(ElementImpl *ownerElem)
{
    ownerNode = ownerElem;
    isOwned(false);
}


// ParentNode stuff

void AttrImpl::cloneChildren(const NodeImpl &other) {
  //    for (NodeImpl *mykid = other.getFirstChild(); 
    for (NodeImpl *mykid = ((NodeImpl&)other).getFirstChild(); 
         mykid != null; 
         mykid = mykid->getNextSibling()) {
        this->appendChild(mykid->cloneNode(true));
    }
}

NodeListImpl *AttrImpl::getChildNodes() {
    return this;
}


NodeImpl * AttrImpl::getFirstChild() {
    makeChildNode();
    return (ChildNode *) value;
}


NodeImpl * AttrImpl::getLastChild() {
    return lastChild();
}

ChildNode * AttrImpl::lastChild() {
    // last child is stored as the previous sibling of first child
    makeChildNode();
    return value != null ? ((ChildNode *) value)->previousSibling : null;
}

void AttrImpl::lastChild(ChildNode *node) {
    // store lastChild as previous sibling of first child
    if (value != null) {
        ((ChildNode *) value)->previousSibling = node;
    }
}

unsigned int AttrImpl::getLength() {
    if (hasStringValue()) {
        return 1;
    }
    ChildNode *node = (ChildNode *) value;
    int length = 0;
    while (node != null) {
        length++;
        node = node->nextSibling;
    }
    return length;
}

bool AttrImpl::hasChildNodes()
{ 
    return value != null;
}; 



NodeImpl *AttrImpl::insertBefore(NodeImpl *newChild, NodeImpl *refChild) {
    if (isReadOnly())
        throw DOM_DOMException(
        DOM_DOMException::NO_MODIFICATION_ALLOWED_ERR, null);
    
    if (newChild->getOwnerDocument() != getOwnerDocument())
        throw DOM_DOMException(DOM_DOMException::WRONG_DOCUMENT_ERR, null);
    
    // Convert to internal type, to avoid repeated casting  
    ChildNode * newInternal= (ChildNode *)newChild;
    
    // Prevent cycles in the tree
    // newChild cannot be ancestor of this Node, and actually cannot be this
    bool treeSafe=true;
    for(NodeImpl *a=this;
        treeSafe && a!=null;
        a=a->getParentNode())
        treeSafe=(newInternal!=a);
    if(!treeSafe)
        throw DOM_DOMException(DOM_DOMException::HIERARCHY_REQUEST_ERR,null);
    
    // refChild must in fact be a child of this node (or null)
    if (refChild!=null && refChild->getParentNode() != this)
        throw DOM_DOMException(DOM_DOMException::NOT_FOUND_ERR,null);
    
    // refChild cannot be same as newChild
    if(refChild==newInternal)
        throw DOM_DOMException(DOM_DOMException::HIERARCHY_REQUEST_ERR,null);

    if (newInternal->isDocumentFragmentImpl())
    {
        // SLOW BUT SAFE: We could insert the whole subtree without
        // juggling so many next/previous pointers. (Wipe out the
        // parent's child-list, patch the parent pointers, set the
        // ends of the list.) But we know some subclasses have special-
        // case behavior they add to insertBefore(), so we don't risk it.
        // This approch also takes fewer bytecodes.
        
        // NOTE: If one of the children is not a legal child of this
        // node, throw HIERARCHY_REQUEST_ERR before _any_ of the children
        // have been transferred. (Alternative behaviors would be to
        // reparent up to the first failure point or reparent all those
        // which are acceptable to the target node, neither of which is
        // as robust. PR-DOM-0818 isn't entirely clear on which it
        // recommends?????
        
        // No need to check kids for right-document; if they weren't,
        // they wouldn't be kids of that DocFrag.
        for(NodeImpl *kid=newInternal->getFirstChild(); // Prescan
        kid!=null;
        kid=kid->getNextSibling())
        {
            if (!DocumentImpl::isKidOK(this, kid))
              throw DOM_DOMException(DOM_DOMException::HIERARCHY_REQUEST_ERR,null);
        }                       
        while(newInternal->hasChildNodes())     // Move
            insertBefore(newInternal->getFirstChild(),refChild);
    }
    
    else if (!DocumentImpl::isKidOK(this, newInternal))
        throw DOM_DOMException(DOM_DOMException::HIERARCHY_REQUEST_ERR,null);
    
    else {
        makeChildNode(); // make sure we have a node and not a string

        NodeImpl *oldparent=newInternal->getParentNode();
        if(oldparent!=null) {
            oldparent->removeChild(newInternal);
        }

        // Convert to internal type, to avoid repeated casting
        ChildNode *refInternal = (ChildNode *)refChild;

        // Attach up
        newInternal->ownerNode = this;
        newInternal->isOwned(true);
        
        // Attach before and after
        // Note: firstChild.previousSibling == lastChild!!
        ChildNode *firstChild = (ChildNode *) value;
        if (firstChild == null) {
            // this our first and only child
            value = newInternal; // firstChild = newInternal
            newInternal->isFirstChild(true);
            newInternal->previousSibling = newInternal;
        } else {
            if (refInternal == null) {
                // this is an append
                ChildNode *lastChild = firstChild->previousSibling;
                lastChild->nextSibling = newInternal;
                newInternal->previousSibling = lastChild;
                firstChild->previousSibling = newInternal;
            } else {
                // this is an insert
                if (refChild == firstChild) {
                    // at the head of the list
                    firstChild->isFirstChild(false);
                    newInternal->nextSibling = firstChild;
                    newInternal->previousSibling =
                        firstChild->previousSibling;
                    firstChild->previousSibling = newInternal;
                    value = newInternal; // firstChild = newInternal;
                    newInternal->isFirstChild(true);
                } else {
                    // somewhere in the middle
                    ChildNode *prev = refInternal->previousSibling;
                    newInternal->nextSibling = refInternal;
                    prev->nextSibling = newInternal;
                    refInternal->previousSibling = newInternal;
                    newInternal->previousSibling = prev;
                }
            }
        }

        changed();

        if (this->getOwnerDocument() != null) {
            typedef RefVectorOf<RangeImpl> RangeImpls;
            RangeImpls* ranges = this->getOwnerDocument()->getRanges();
            if ( ranges != null) {
                unsigned int sz = ranges->size();
                for (unsigned int i =0; i<sz; i++) {
                    ranges->elementAt(i)->updateRangeForInsertedNode(newInternal);
                }
            }
        }
    }

    return newInternal;
}
  
  
NodeImpl *AttrImpl::item(unsigned int index) {

    if (hasStringValue()) {
        if (index != 0 || value == null) {
            return null;
        }
        else {
            makeChildNode();
            return (NodeImpl *) value;
        }
    }
    ChildNode *nodeListNode = (ChildNode *) value;
    for (unsigned int nodeListIndex = 0; 
         nodeListIndex < index && nodeListNode != null; 
         nodeListIndex++) {
        nodeListNode = nodeListNode->nextSibling;
    }
    return nodeListNode;
}
  
  
NodeImpl *AttrImpl::removeChild(NodeImpl *oldChild) {
    if (isReadOnly())
        throw DOM_DOMException(
        DOM_DOMException::NO_MODIFICATION_ALLOWED_ERR, null);
    
    if (hasStringValue()
        // we don't have any child per say so it can't be one of them!
        || (oldChild != null && oldChild->getParentNode() != this))
        throw DOM_DOMException(DOM_DOMException::NOT_FOUND_ERR, null);
    
    // fix other ranges for change before deleting the node
    if (this->getOwnerDocument() !=  null) {
        typedef RefVectorOf<RangeImpl> RangeImpls;
        RangeImpls* ranges = this->getOwnerDocument()->getRanges();
        if (ranges != null) {
            unsigned int sz = ranges->size();
            if (sz != 0) {
                for (unsigned int i =0; i<sz; i++) {
                    if (ranges->elementAt(i) != null) 
                        ranges->elementAt(i)->updateRangeForDeletedNode(oldChild);
                }
            }
        }
    }
    
    ChildNode * oldInternal = (ChildNode *) oldChild;
    
    // Patch linked list around oldChild
    // Note: lastChild == firstChild->previousSibling
    if (oldInternal == value) {
        // removing first child
        oldInternal->isFirstChild(false);
        value = oldInternal->nextSibling; // firstChild = oldInternal->nextSibling
        ChildNode *firstChild = (ChildNode *) value;
        if (firstChild != null) {
            firstChild->isFirstChild(true);
            firstChild->previousSibling = oldInternal->previousSibling;
        }
    } else {
        ChildNode *prev = oldInternal->previousSibling;
        ChildNode *next = oldInternal->nextSibling;
        prev->nextSibling = next;
        if (next == null) {
            // removing last child
            ChildNode *firstChild = (ChildNode *) value;
            firstChild->previousSibling = prev;
        } else {
            // removing some other child in the middle
            next->previousSibling = prev;
        }
    }

    // Remove oldInternal's references to tree
    oldInternal->ownerNode = getOwnerDocument();
    oldInternal->isOwned(false);
    oldInternal->nextSibling = null;
    oldInternal->previousSibling = null;

    changed();

    return oldInternal;
};


NodeImpl *AttrImpl::replaceChild(NodeImpl *newChild, NodeImpl *oldChild) {
    insertBefore(newChild, oldChild);
    // changed() already done.
    return removeChild(oldChild);
}
  

void AttrImpl::setReadOnly(bool readOnl, bool deep) {
    NodeImpl::setReadOnly(readOnl, deep);
      
    if (deep) {
        if (hasStringValue()) {
            return;
        }
        // Recursively set kids
        for (ChildNode *mykid = (ChildNode *) value; 
             mykid != null; 
             mykid = mykid->nextSibling)
            if(! (mykid->isEntityReference()))
                mykid->setReadOnly(readOnl,true);
    }
}
  
  
//Introduced in DOM Level 2
  
void AttrImpl::normalize()
{
    if (hasStringValue()) {
        return;
    }
    ChildNode *kid, *next;
    for (kid = (ChildNode *) value; kid != null; kid = next)
    {
        next = kid->nextSibling;
        
        // If kid and next are both Text nodes (but _not_ CDATASection,
        // which is a subclass of Text), they can be merged.
        if (next != null && 
            kid->isTextImpl()   && !(kid->isCDATASectionImpl())  && 
            next->isTextImpl()  && !(next->isCDATASectionImpl()) )
        {
            ((TextImpl *) kid)->appendData(((TextImpl *) next)->getData());
            removeChild(next);
            if (next->nodeRefCount == 0)
                deleteIf(next);
            next = kid; // Don't advance; there might be another.
        }
        
        // Otherwise it might be an Element, which is handled recursively  
        else
            if (kid->isElementImpl())
                kid->normalize();
    };
    
    // changed() will have occurred when the removeChild() was done,
    // so does not have to be reissued.
};
