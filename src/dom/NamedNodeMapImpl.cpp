/*
 * The Apache Software License, Version 1.1
 * 
 * Copyright (c) 1999 The Apache Software Foundation.  All rights 
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

/**
 * $Log$
 * Revision 1.1  1999/11/09 01:09:11  twl
 * Initial revision
 *
 * Revision 1.3  1999/11/08 20:44:29  rahul
 * Swat for adding in Product name and CVS comment log variable.
 *
 */

#include "NamedNodeMapImpl.hpp"
#include "NodeVector.hpp"
#include "AttrImpl.hpp"
#include "DOM_DOMException.hpp"


int        NamedNodeMapImpl::gLiveNamedNodeMaps  = 0;
int        NamedNodeMapImpl::gTotalNamedNodeMaps = 0;

NamedNodeMapImpl::NamedNodeMapImpl(DocumentImpl *ownerD,NamedNodeMapImpl *defs)
{
    this->ownerDoc=ownerD;
    this->defaults=defs;
    this->nodes = null;
    this->changes = 0;
    lastDefaultsChanges = -1;
    this->readOnly = false;
    this->refCount = 1;
    gLiveNamedNodeMaps++;
    gTotalNamedNodeMaps++;
};



NamedNodeMapImpl::~NamedNodeMapImpl()
{ 
    if (nodes)
    {
        // It is the responsibility of whoever was using the named node
        // map to do any cleanup on the nodes contained in the map
        //  before deleting it.
        delete nodes;
        nodes = 0;
    }
    gLiveNamedNodeMaps--;
};


void NamedNodeMapImpl::addRef(NamedNodeMapImpl *This)
{
    if (This)
        ++This->refCount;
};


NamedNodeMapImpl *NamedNodeMapImpl::cloneMap()
{
    bool deep = true;
    NamedNodeMapImpl *newmap = new NamedNodeMapImpl(ownerDoc, defaults);
    if (nodes != null)
    {
        newmap->nodes = new NodeVector(nodes->size());
        for (int i = 0; i < nodes->size(); ++i)
        {
            NodeImpl *n = nodes->elementAt(i)->cloneNode(deep);
            n->owned = true;
            newmap->nodes->addElement(n);
        }
    }
    newmap->defaults = defaults;
    if (defaults)
        defaults->refCount++;
    
    return newmap;
};


//
//  removeAll - This function removes all elements from a named node map.
//              It is called from the destructors for Elements and DocumentTypes,
//              to remove the contents when the owning Element or DocType goes
//              away.  The empty NamedNodeMap may persist if the user code
//              has a reference to it.
//
//              AH Revist - the empty map should be made read-only, since
//              adding it was logically part of the [Element, DocumentType]
//              that has been deleted, and adding anything new to it would
//              be meaningless, and almost certainly an error.
//
void NamedNodeMapImpl::removeAll()
{
    if (nodes)
    {

        for (int i=nodes->size()-1; i>=0; i--)
        {
            NodeImpl *n = nodes->elementAt(i);
            n->owned = false;
            if (n->nodeRefCount == 0)
                NodeImpl::deleteIf(n);
        }
        delete nodes;        
        nodes = null;  
    }
}



int NamedNodeMapImpl::findNamePoint(const DOMString &name)
{
    reconcileDefaults();
    
    // Binary search
    int i=0;
    if(nodes!=null)
    {
        int first=0,last=nodes->size()-1;
        
        while(first<=last)
        {
            i=(first+last)/2;
            int test = name.strcmp(nodes->elementAt(i)->getNodeName());
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
    if (nodes != null)
    for (i = 0; i < nodes.size(); ++i)
    {
    int test = name.compareTo(((NodeImpl *) (nodes.elementAt(i))).getNodeName());
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
};



int NamedNodeMapImpl::getLength()
{
    reconcileDefaults();
    return (nodes != null) ? nodes->size() : 0;
};



NodeImpl * NamedNodeMapImpl::getNamedItem(const DOMString &name)
{
    int i=findNamePoint(name);
    return (i<0) ? null : (NodeImpl *)(nodes->elementAt(i));
    
    /************   
    reconcileDefaults();
    
      // DO BINARY SEARCH?????
      if (nodes != null)
      for (int i = 0; i < nodes.size(); ++i)
      {
      int test = name.compareTo(((Node) (nodes.elementAt(i))).getNodeName());
      if (test == 0)
      return (Node) (nodes.elementAt(i));
      if (test < 0)
      break; // Past the sort point.
      }
      return null;
    **************/ 
};



NodeImpl * NamedNodeMapImpl::item(int index)
{
    reconcileDefaults();
    return (nodes != null && index < nodes->size()) ?
        (NodeImpl *) (nodes->elementAt(index)) : null;
};



void NamedNodeMapImpl::reconcileDefaults()
{
    if (defaults != null && lastDefaultsChanges != defaults->changes)
    {
        int n = 0, d = 0, nsize = nodes->size(), dsize = defaults->nodes->size();
        AttrImpl * nnode = (nsize == 0) ? null : (AttrImpl *) nodes->elementAt(0);
        AttrImpl * dnode = (dsize == 0) ? null : (AttrImpl *) defaults->nodes->elementAt(0);
        while (n < nsize && d < dsize)
        {
            nnode = (AttrImpl *) nodes->elementAt(n);
            dnode = (AttrImpl *) defaults->nodes->elementAt(d);
            int test = nnode->getNodeName().strcmp( dnode->getNodeName());
            // nnode->getNodeName()->compareTo(dnode->getNodeName());
            
            // Same name and a default -- make sure same value
            if (test == 0 && !nnode->getSpecified())
            {
                nodes->setElementAt(dnode, n);
                // Advance over both, since names in sync
                ++n;
                ++d;
            }
            
            // Different name, new default in table; add it
            else if (test > 0)
            {
                nodes->insertElementAt(dnode, n);
                // Now in sync, so advance over both
                ++n;
                ++d;
            }
            
            // Different name, old default here; remove it.
            else if (!nnode->getSpecified())
            {
                nodes->removeElementAt(n);
                // n didn't advance but represents a different element
            }
            
            // Different name, specified; accept it
            else
                ++n;
        }
        
        // If we ran out of local before default, pick up defaults
        while (d < dsize)
        {
            nodes->addElement(defaults->nodes->elementAt(d));
        }
        lastDefaultsChanges = defaults->changes;
    }
};


//
// removeNamedItem() - Remove the named item, and return it.
//                      The caller must arrange for deletion of the 
//                      returned item if its refcount has gone to zero -
//                      we can't do it here because the caller would
//                      never see the returned node.
//
NodeImpl * NamedNodeMapImpl::removeNamedItem(const DOMString &name)
{
    int i=findNamePoint(name);
    if(i<0)
        throw new DOM_DOMException(DOM_DOMException::NOT_FOUND_ERR, null);
    else
    {
        NodeImpl * n = (NodeImpl *) (nodes->elementAt(i));
        // If there's a default, add it instead
        NodeImpl * d;
	if (defaults != null && (d = defaults->getNamedItem(name)) != null) {
	    if (n->isAttrImpl() && d->isAttrImpl()) {	//DOM Level 2
		d = d->cloneNode(true); //copy d and ownerElement of n
		((AttrImpl *) d)->setOwnerElement(((AttrImpl *) n)->getOwnerElement());
	    }
            nodes->setElementAt(d, i);
	} else
            nodes->removeElementAt(i);
        
        ++changes;
        n->owned = false;
        return n;
    }
    
    /************
    // DO BINARY SEARCH????? 
    // At least take advantage of sort-order for early stop.
    if (nodes != null)
    {
    for (int i = 0; i < nodes.size(); ++i)
    if (name.equals(((Node) (nodes.elementAt(i))).getNodeName()))
    {
    Node n = (Node) (nodes.elementAt(i));
    // If there's a default, add it instead
    Node d;
    if (defaults != null && 
    (d = defaults.getNamedItem(name)) != null)
    nodes.setElementAt(d, i);
    else
    nodes.removeElementAt(i);
    
      ++changes;
      return n;
      }
      }
      throw new DOMExceptionImpl(DOMException.NOT_FOUND_ERR, null);
    **************/ 
	return null;	// just to keep the compiler happy
};



void NamedNodeMapImpl::removeRef(NamedNodeMapImpl *This)
{
    if (This && --This->refCount == 0)
        delete This;
};

//
// setNamedItem()  Put the item into the NamedNodeList by name.
//                  If an item with the same name already was
//                  in the list, replace it.  Return the old
//                  item, if there was one.
//                  Caller is responsible for arranging for
//                  deletion of the old item if its ref count is
//                  zero.
//
NodeImpl * NamedNodeMapImpl::setNamedItem(NodeImpl * arg)
{
    if(arg->getOwnerDocument()!=ownerDoc)
        throw new DOM_DOMException(DOM_DOMException::WRONG_DOCUMENT_ERR,null);
    
    if (arg->owned)
        throw new DOM_DOMException(DOM_DOMException::INUSE_ATTRIBUTE_ERR,null);
    
    arg->owned = true;
    int i=findNamePoint(arg->getNodeName());
    NodeImpl * previous=null;
    if(i>=0)
    {
        previous = nodes->elementAt(i);
        nodes->setElementAt(arg,i);
    }
    else
    {
        i=-1-i; // Insert point (may be end of list)
        if(null==nodes)
            nodes=new NodeVector();
        nodes->insertElementAt(arg,i);
    }
    ++changes;
    if (previous != null)
        previous->owned = false;

    return previous;
    
    /*************************      
    // DO BINARY SEARCH?????
    // Insertion-sort is used to maintain list in lexical order.  That
    // doesn't matter a great deal in normal operation (minor
    // performance boost in getNamedItem), but improves comparison
    // for reconcileDefault() (and equals(), if we add that.)
    if(nodes==null)
    nodes=new Vector();
    for(int i=0; i<nodes.size();++i)
    {
    int test=arg.getNodeName()
    .compareTo( ((Node)(nodes.elementAt(i))) .getNodeName());
    if(test==0)
    {                     // Names match
    Node previous=(Node)nodes.elementAt(i);
    nodes.setElementAt(arg,i);
    ++changes;
    return previous;
    }
    else if(test<0)
    {                     // Mismatch but new goes before old
    nodes.insertElementAt(arg,i);
    ++changes;
    return null;
    }
    }
    nodes.addElement(arg);      // Bigger than all, append to end
    ++changes;
    return null;
    ************/   
};


void NamedNodeMapImpl::setReadOnly(bool readOnl, bool deep)
{
    this->readOnly=readOnl;
    if(deep && nodes!=null)
    {
        //Enumeration e=nodes->elements();
        //while(e->hasMoreElements())
        //      ((NodeImpl)e->nextElement())->setReadOnly(readOnl,deep);
        int sz = nodes->size();
        for (int i=0; i<sz; ++i) {
            nodes->elementAt(i)->setReadOnly(readOnl, deep);
        }
    }
};


//Introduced in DOM Level 2

int NamedNodeMapImpl::findNamePoint(const DOMString &namespaceURI,
	const DOMString &localName)
{
    reconcileDefaults();
    if (nodes == null)
	return -1;
   // Linear search
    int i, len = nodes -> size();
    for (i = 0; i < len; ++i) {
	NodeImpl *node = nodes -> elementAt(i);
	if (! node -> getNamespaceURI().equals(namespaceURI))	//URI not match
	    continue;
	if (node -> getLocalName().equals(localName))	//both match
	    return i;
    }
    return -1;	//not found
}


NodeImpl *NamedNodeMapImpl::getNamedItemNS(const DOMString &namespaceURI,
	const DOMString &localName)
{
    if (namespaceURI == null || namespaceURI.length() == 0)
	return getNamedItem(localName);
    int i = findNamePoint(namespaceURI, localName);
    return i < 0 ? null : nodes -> elementAt(i);
}


// removeNamedItemNS() - Remove the named item, and return it.
//                      The caller must arrange for deletion of the 
//                      returned item if its refcount has gone to zero -
//                      we can't do it here because the caller would
//                      never see the returned node.
NodeImpl *NamedNodeMapImpl::removeNamedItemNS(const DOMString &namespaceURI,
	const DOMString &name)
{
    if (readOnly)
	throw new DOM_DOMException(
	    DOM_DOMException::NO_MODIFICATION_ALLOWED_ERR, null);
    if (namespaceURI == null || namespaceURI.length() == 0)
	return removeNamedItem(name);
    int i = findNamePoint(namespaceURI, name);
    if (i < 0)
	throw new DOM_DOMException(DOM_DOMException::NOT_FOUND_ERR, null);
    NodeImpl * n = nodes -> elementAt(i);   //node to be removed or replaced
    //find if n has a default value defined in DTD, if so replace n in nodes
    //by its corresponding default value node, otherwise remove n from nodes
    NodeImpl * d;
    if (defaults != null && (d = defaults->getNamedItemNS(namespaceURI, name)) != null) {
	if (n->isAttrImpl() && d->isAttrImpl()) {
	    d = d->cloneNode(true); //copy d and ownerElement of n
	    ((AttrImpl *) d)->setOwnerElement(((AttrImpl *) n)->getOwnerElement());
	}
        nodes -> setElementAt(d, i);	//replace n in nodes by d
    } else
        nodes -> removeElementAt(i);	//remove n from nodes
    ++changes;
    n -> owned = false;
    return n;
}
