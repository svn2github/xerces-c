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
 */

#include "DocumentTypeImpl.hpp"
#include "DOM_Node.hpp"
#include "NamedNodeMapImpl.hpp"
#include "DOM_DOMException.hpp"
#include "DocumentImpl.hpp"


DocumentTypeImpl::DocumentTypeImpl(DocumentImpl *ownerDoc,
                                   const DOMString &dtName) 
    : NodeContainer(ownerDoc),
    publicId(null), systemId(null)	//DOM Level 2
{
    name = dtName.clone();
    entities = new NamedNodeMapImpl(this, null);
    notations= new NamedNodeMapImpl(this, null);
    
    // NON-DOM
    elements = new NamedNodeMapImpl(this, null);
};


//Introduced in DOM Level 2
DocumentTypeImpl::DocumentTypeImpl(const DOMString &qualifiedName,
    const DOMString &fPublicId, const DOMString &fSystemId)
    : NodeContainer(null),
    publicId(fPublicId), systemId(fSystemId)
{
    name = qualifiedName.clone();
    if (DocumentImpl::indexofQualifiedName(qualifiedName) < 0)
        throw DOM_DOMException(DOM_DOMException::NAMESPACE_ERR, null);

    entities = new NamedNodeMapImpl(this, null);
    notations= new NamedNodeMapImpl(this, null);
    
    // NON-DOM
    elements = new NamedNodeMapImpl(this, null);
};


DocumentTypeImpl::DocumentTypeImpl(const DocumentTypeImpl &other, bool deep)
    : NodeContainer(other)
{
    name = other.name.clone();
    if (deep)
        cloneChildren(other);
    entities = other.entities->cloneMap();
    notations= other.notations->cloneMap();
    
    // NON-DOM
    elements = other.elements->cloneMap();

    //DOM Level 2
    publicId = other.publicId.clone();
    systemId = other.systemId.clone();
};


DocumentTypeImpl::~DocumentTypeImpl()
{
    if (entities != null)
    {
        entities->removeAll();
        NamedNodeMapImpl::removeRef(entities);
    }

    if (notations != null)
    {
        notations->removeAll();
        NamedNodeMapImpl::removeRef(notations);
    }

    if (elements != null)
    {
        elements->removeAll();
        NamedNodeMapImpl::removeRef(elements);
    }

};


NodeImpl *DocumentTypeImpl::cloneNode(bool deep)
{
    return new DocumentTypeImpl(*this, deep);
};


DOMString DocumentTypeImpl::getNodeName()
{
    return name;
};


short DocumentTypeImpl::getNodeType() {
    return DOM_Node::DOCUMENT_TYPE_NODE;
};


NamedNodeMapImpl *DocumentTypeImpl::getElements()
{
    return elements;
};


NamedNodeMapImpl *DocumentTypeImpl::getEntities()
{
    return entities;
};


DOMString DocumentTypeImpl::getName()
{
    return name;
};


NamedNodeMapImpl *DocumentTypeImpl::getNotations()
{
    return notations;
};


bool DocumentTypeImpl::isDocumentTypeImpl()
{
    return true;
};


void DocumentTypeImpl::setNodeValue(const DOMString &val)
{
    throw DOM_DOMException(
        DOM_DOMException::NO_MODIFICATION_ALLOWED_ERR, null);
};


void DocumentTypeImpl::setReadOnly(bool readOnl, bool deep)
{
    NodeContainer::setReadOnly(readOnl,deep);
    entities->setReadOnly(readOnl,true);
    notations->setReadOnly(readOnl,true);
};


//Introduced in DOM Level 2

DOMString DocumentTypeImpl::getPublicId()
{
    return publicId;
}


DOMString DocumentTypeImpl::getSystemId()
{
    return systemId;
}


DOMString DocumentTypeImpl::getInternalSubset()
{
    return null;    //not implemented yet
}


void DocumentTypeImpl::setOwnerDocument(DocumentImpl *docImpl)
{
    ownerDocument = docImpl;
    //Note: ownerDoc of entities, notations and elements remain unchanged
    //The DOM APIs does not require a NamedNodeMap to have an owner document
}


/** Export this node to a different document docImpl.
 */
DocumentTypeImpl *DocumentTypeImpl::exportNode(DocumentImpl *docImpl, bool deep)
{
    DocumentTypeImpl *doctype;
    doctype = new DocumentTypeImpl(name, publicId, systemId);
    doctype -> setOwnerDocument(docImpl);
    if (deep) {
	delete doctype -> entities;
	delete doctype -> notations;
	delete doctype -> elements;
	doctype -> entities = entities -> exportNode(doctype);
	doctype -> notations = notations -> exportNode(doctype);
	doctype -> elements = elements -> exportNode(doctype);
    }
    return doctype;
}

