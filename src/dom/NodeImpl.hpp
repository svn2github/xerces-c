#ifndef NodeImpl_HEADER_GUARD_
#define NodeImpl_HEADER_GUARD_

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

//
//  This file is part of the internal implementation of the C++ XML DOM.
//  It should NOT be included or used directly by application programs.
//
//  Applications should include the file <dom/DOM.hpp> for the entire
//  DOM API, or DOM_*.hpp for individual DOM classes, where the class
//  name is substituded for the *.
//

/**
 * A NodeImpl doesn't have any children, and can therefore only be directly
 * inherited by classes of nodes that never have any, such as Text nodes. For
 * other types, such as Element, classes must inherit from NodeContainer.
 **/

#include <util/XercesDefs.hpp>
#include "NodeListImpl.hpp"
#include "DOMString.hpp"


class NamedNodeMapImpl;
class NodeListImpl;
class DocumentImpl;

//  define 'null' is used extensively in the DOM implementation code, 
//  as a consequence of its Java origins.
//  MSVC 5.0 compiler has problems with overloaded function resolution
//	when using the const int definition.
//
#if defined(XML_CSET)
const int null = 0;
#else
#define null 0
#endif


class CDOM_EXPORT NodeImpl: public NodeListImpl {
public:
    DOMString               name;                   // Name of this node or node type
    DOMString               value;                  // String value (not used in all nodes)
    bool                    readOnly;
    bool                    owned;                  // True if there is a reference to this
                                                    //  node in a named node list.  Applies to
                                                    //  attributes, notations, entities.
    DocumentImpl            *ownerDocument;         // Document this node belongs to
    NodeImpl                *previousSibling;
    NodeImpl                *nextSibling;
    NodeImpl                *parentNode;
    int changes;
    void *userData;

    static int              gLiveNodeImpls;         // Counters for debug & tuning.
    static int              gTotalNodeImpls;

public:
    NodeImpl(DocumentImpl *ownerDocument,
        const DOMString &name,
        const DOMString &initValue);
    NodeImpl(const NodeImpl &other);
    virtual ~NodeImpl();
    
    // Dynamic Cast replacement functions.
    virtual bool isAttrImpl();
    virtual bool isCDATASectionImpl();
    virtual bool isDocumentFragmentImpl();
    virtual bool isDocumentImpl();
    virtual bool isDocumentTypeImpl();
    virtual bool isElementImpl();
    virtual bool isEntityReference();
    virtual bool isTextImpl();

    virtual NodeImpl *appendChild(NodeImpl *newChild);
    virtual void changed();
    virtual NodeImpl * cloneNode(bool deep) = 0;
    static void deleteIf(NodeImpl *thisNode);
    virtual NamedNodeMapImpl * getAttributes();
    virtual NodeListImpl *getChildNodes();
    virtual NodeImpl * getFirstChild();
    virtual NodeImpl * getLastChild();
    virtual unsigned int getLength();
    virtual NodeImpl * getNextSibling();
    virtual DOMString getNodeName();
    virtual short getNodeType() = 0;
    virtual DOMString getNodeValue();
    virtual DocumentImpl * getOwnerDocument();
    virtual NodeImpl * getParentNode();
    virtual NodeImpl*  getPreviousSibling();
    virtual void *getUserData();
    virtual bool        hasChildNodes();
    virtual NodeImpl    *insertBefore(NodeImpl *newChild, NodeImpl *refChild);
    static  bool        isKidOK(NodeImpl *parent, NodeImpl *child);
    virtual NodeImpl    *item(unsigned int index);
    virtual void        referenced();
    virtual NodeImpl    * removeChild(NodeImpl *oldChild);
    virtual NodeImpl    *replaceChild(NodeImpl *newChild, NodeImpl *oldChild);
    virtual void        setNodeValue(const DOMString &value);
    virtual void        setReadOnly(bool readOnly, bool deep);
    virtual void        setUserData(void *value);
    virtual DOMString   toString();
    virtual void        unreferenced();

    //Introduced in DOM Level 2
    virtual void	normalize();
    virtual bool	supports(const DOMString &feature, const DOMString &version);
    virtual DOMString	getNamespaceURI();
    virtual DOMString   getPrefix();
    virtual DOMString   getLocalName();
    virtual void        setPrefix(const DOMString &prefix);

protected:
    //Utility, not part of DOM Level 2 API
    static const DOMString&	mapPrefix(const DOMString &prefix,
	const DOMString &namespaceURI, short nType);
    static DOMString getXmlnsString();
    static DOMString getXmlnsURIString();
    static DOMString getXmlString();
    static DOMString getXmlURIString();
};


#endif
