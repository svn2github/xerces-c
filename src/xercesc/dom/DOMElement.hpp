#ifndef DOMElement_HEADER_GUARD_
#define DOMElement_HEADER_GUARD_

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

#include <xercesc/util/XercesDefs.hpp>
#include "DOMNode.hpp"

class DOMAttr;
class DOMNodeList;


/**
 * By far the vast majority of objects (apart from text) that authors
 * encounter when traversing a document are <code>DOMElement</code> nodes.
 *
 * Assume the following XML document:&lt;elementExample id="demo"&gt;
 * &lt;subelement1/&gt;
 * &lt;subelement2&gt;&lt;subsubelement/&gt;&lt;/subelement2&gt;
 * &lt;/elementExample&gt;
 * <p>When represented using DOM, the top node is an <code>DOMElement</code> node
 * for "elementExample", which contains two child <code>DOMElement</code> nodes,
 * one for "subelement1" and one for "subelement2". "subelement1" contains no
 * child nodes.
 * <p>Elements may have attributes associated with them; since the
 * <code>DOMElement</code> interface inherits from <code>DOMNode</code>, the generic
 *  <code>DOMNode</code> interface method <code>getAttributes</code> may be used
 * to retrieve the set of all attributes for an element.  There are methods on
 *  the <code>DOMElement</code> interface to retrieve either an <code>DOMAttr</code>
 *  object by name or an attribute value by name. In XML, where an attribute
 * value may contain entity references, an <code>DOMAttr</code> object should be
 * retrieved to examine the possibly fairly complex sub-tree representing the
 * attribute value. On the other hand, in HTML, where all attributes have
 * simple string values, methods to directly access an attribute value can
 * safely be used as a convenience.
 *
 * @since DOM Level 1
 */

class CDOM_EXPORT DOMElement: public DOMNode {
protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{
    DOMElement() {};
    DOMElement(const DOMElement &other) {};
    DOMElement & operator = (const DOMElement &other) {return *this;};
    //@}

public:
    // -----------------------------------------------------------------------
    //  All constructors are hidden, just the destructor is available
    // -----------------------------------------------------------------------
    /** @name Destructor */
    //@{
    /**
     * Destructor
     *
     */
    virtual ~DOMElement() {};
    //@}

    // -----------------------------------------------------------------------
    //  Virtual DOMElement interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 1 */
    //@{
    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    /**
     * The name of the element.
     *
     * For example, in: &lt;elementExample
     * id="demo"&gt;  ... &lt;/elementExample&gt; , <code>tagName</code> has
     * the value <code>"elementExample"</code>. Note that this is
     * case-preserving in XML, as are all of the operations of the DOM.
     * @since DOM Level 1
     */
    virtual const XMLCh *         getTagName() const = 0;

    /**
     * Retrieves an attribute value by name.
     *
     * @param name The name of the attribute to retrieve.
     * @return The <code>DOMAttr</code> value as a string, or the empty  string if
     *   that attribute does not have a specified or default value.
     * @since DOM Level 1
     */
    virtual const XMLCh *         getAttribute(const XMLCh *name) const = 0;

    /**
     * Retrieves an <code>DOMAttr</code> node by name.
     *
     * @param name The name (<CODE>nodeName</CODE>) of the attribute to retrieve.
     * @return The <code>DOMAttr</code> node with the specified name (<CODE>nodeName</CODE>) or
     *   <code>null</code> if there is no such attribute.
     * @since DOM Level 1
     */
    virtual DOMAttr       * getAttributeNode(const XMLCh *name) const = 0;

    /**
     * Returns a <code>DOMNodeList</code> of all descendant elements with a given
     * tag name, in the order in which they would be encountered in a preorder
     * traversal of the <code>DOMElement</code> tree.
     *
     * @param name The name of the tag to match on. The special value "*"
     *   matches all tags.
     * @return A list of matching <code>DOMElement</code> nodes.
     * @since DOM Level 1
     */
    virtual DOMNodeList   * getElementsByTagName(const XMLCh *name) const = 0;

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    /**
     * Adds a new attribute.
     *
     * If an attribute with that name is already present
     * in the element, its value is changed to be that of the value parameter.
     * This value is a simple string, it is not parsed as it is being set. So
     * any markup (such as syntax to be recognized as an entity reference) is
     * treated as literal text, and needs to be appropriately escaped by the
     * implementation when it is written out. In order to assign an attribute
     * value that contains entity references, the user must create an
     * <code>DOMAttr</code> node plus any <code>DOMText</code> and
     * <code>DOMEntityReference</code> nodes, build the appropriate subtree, and
     * use <code>setAttributeNode</code> to assign it as the value of an
     * attribute.
     * @param name The name of the attribute to create or alter.
     * @param value Value to set in string form.
     * @exception DOMException
     *   INVALID_CHARACTER_ERR: Raised if the specified name contains an
     *   illegal character.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
     * @since DOM Level 1
     */
    virtual void             setAttribute(const XMLCh *name,
                                  const XMLCh *value) = 0;
    /**
     * Adds a new attribute.
     *
     * If an attribute with that name (<CODE>nodeName</CODE>) is already present
     * in the element, it is replaced by the new one.
     * @param newAttr The <code>DOMAttr</code> node to add to the attribute list.
     * @return If the <code>newAttr</code> attribute replaces an existing
     *   attribute, the replaced
     *   <code>DOMAttr</code> node is returned, otherwise <code>null</code> is
     *   returned.
     * @exception DOMException
     *   WRONG_DOCUMENT_ERR: Raised if <code>newAttr</code> was created from a
     *   different document than the one that created the element.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
     *   <br>INUSE_ATTRIBUTE_ERR: Raised if <code>newAttr</code> is already an
     *   attribute of another <code>DOMElement</code> object. The DOM user must
     *   explicitly clone <code>DOMAttr</code> nodes to re-use them in other
     *   elements.
     * @since DOM Level 1
     */
    virtual DOMAttr       * setAttributeNode(DOMAttr *newAttr) = 0;

    /**
     * Removes the specified attribute node.
     * If the removed <CODE>DOMAttr</CODE>
     *   has a default value it is immediately replaced. The replacing attribute
     *   has the same namespace URI and local name, as well as the original prefix,
     *   when applicable.
     *
     * @param oldAttr The <code>DOMAttr</code> node to remove from the attribute
     *   list.
     * @return The <code>DOMAttr</code> node that was removed.
     * @exception DOMException
     *   NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
     *   <br>NOT_FOUND_ERR: Raised if <code>oldAttr</code> is not an attribute
     *   of the element.
     * @since DOM Level 1
     */
    virtual DOMAttr       * removeAttributeNode(DOMAttr *oldAttr) = 0;

    /**
     * Removes an attribute by name.
     *
     * If the removed attribute
     *   is known to have a default value, an attribute immediately appears
     *   containing the default value as well as the corresponding namespace URI,
     *   local name, and prefix when applicable.<BR>To remove an attribute by local
     *   name and namespace URI, use the <CODE>removeAttributeNS</CODE> method.
     * @param name The name of the attribute to remove.
     * @exception DOMException
     *   NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
     * @since DOM Level 1
     */
    virtual void              removeAttribute(const XMLCh *name) = 0;
    //@}

    /** @name Functions introduced in DOM Level 2. */
    //@{
    /**
     * Retrieves an attribute value by local name and namespace URI.
     *
     * @param namespaceURI The <em>namespace URI</em> of
     *    the attribute to retrieve.
     * @param localName The <em>local name</em> of the
     *    attribute to retrieve.
     * @return The <code>DOMAttr</code> value as a string, or an <CODE>null</CODE> if
     *    that attribute does not have a specified or default value.
     * @since DOM Level 2
     */
    virtual const XMLCh *         getAttributeNS(const XMLCh *namespaceURI,
                                                 const XMLCh *localName) const = 0;

    /**
     * Adds a new attribute. If an attribute with the same
     * local name and namespace URI is already present on the element, its prefix
     * is changed to be the prefix part of the <CODE>qualifiedName</CODE>, and
     * its value is changed to be the <CODE>value</CODE> parameter. This value is
     * a simple string, it is not parsed as it is being set. So any markup (such
     * as syntax to be recognized as an entity reference) is treated as literal
     * text, and needs to be appropriately escaped by the implementation when it
     * is written out. In order to assign an attribute value that contains entity
     * references, the user must create an <CODE>DOMAttr</CODE>
     * node plus any <CODE>DOMText</CODE> and <CODE>DOMEntityReference</CODE>
     * nodes, build the appropriate subtree, and use
     * <CODE>setAttributeNodeNS</CODE> or <CODE>setAttributeNode</CODE> to assign
     * it as the value of an attribute.
     *
     * @param namespaceURI The <em>namespace URI</em> of
     *    the attribute to create or alter.
     * @param qualifiedName The <em>qualified name</em> of the
     *    attribute to create or alter.
     * @param value The value to set in string form.
     * @exception DOMException
     *   INVALID_CHARACTER_ERR: Raised if the specified qualified name contains an
     *   illegal character.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
     * <br>
     *   NAMESPACE_ERR: Raised if the <CODE>qualifiedName</CODE> is
     *        malformed, if the <CODE>qualifiedName</CODE> has a prefix and the
     *        <CODE>namespaceURI</CODE> is <CODE>null</CODE> or an empty string,
     *        if the <CODE>qualifiedName</CODE> has a prefix that is "xml" and the
     *        <CODE>namespaceURI</CODE> is different from
     *        "http://www.w3.org/XML/1998/namespace", if the
     *        <CODE>qualifiedName</CODE> has a prefix that is "xmlns" and the
     *        <CODE>namespaceURI</CODE> is different from
     *        "http://www.w3.org/2000/xmlns/", or if the
     *        <CODE>qualifiedName</CODE> is "xmlns" and the
     *        <CODE>namespaceURI</CODE> is different from
     *        "http://www.w3.org/2000/xmlns/".
     * @since DOM Level 2
     */
    virtual void             setAttributeNS(const XMLCh *namespaceURI,
                                            const XMLCh *qualifiedName, const XMLCh *value) = 0;

    /**
     * Removes an attribute by local name and namespace URI. If the
     * removed attribute has a default value it is immediately replaced.
     * The replacing attribute has the same namespace URI and local name, as well as
     * the original prefix.
     *
     * @param namespaceURI The <em>namespace URI</em> of
     *    the attribute to remove.
     * @param localName The <em>local name</em> of the
     *    attribute to remove.
     * @exception DOMException
     *   NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
     * @since DOM Level 2
     */
    virtual void              removeAttributeNS(const XMLCh *namespaceURI,
                                                const XMLCh *localName) = 0;

    /**
     * Retrieves an <code>DOMAttr</code> node by local name and namespace URI.
     *
     * @param namespaceURI The <em>namespace URI</em> of
     *    the attribute to retrieve.
     * @param localName The <em>local name</em> of the
     *    attribute to retrieve.
     * @return The <code>DOMAttr</code> node with the specified attribute local
     *    name and namespace URI or <code>null</code> if there is no such attribute.
     * @since DOM Level 2
     */
    virtual DOMAttr      *  getAttributeNodeNS(const XMLCh *namespaceURI,
                                               const XMLCh *localName) const = 0;

    /**
     * Adds a new attribute.
     *
     * If an attribute with that local name and namespace URI is already present
     * in the element, it is replaced by the new one.
     *
     * @param newAttr The <code>DOMAttr</code> node to add to the attribute list.
     * @return If the <code>newAttr</code> attribute replaces an existing
     *    attribute with the same <em>local name</em> and <em>namespace URI</em>,
     *    the replaced <code>DOMAttr</code> node is
     *    returned, otherwise <code>null</code> is returned.
     * @exception DOMException
     *   WRONG_DOCUMENT_ERR: Raised if <code>newAttr</code> was created from a
     *   different document than the one that created the element.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
     *   <br>INUSE_ATTRIBUTE_ERR: Raised if <code>newAttr</code> is already an
     *   attribute of another <code>DOMElement</code> object. The DOM user must
     *   explicitly clone <code>DOMAttr</code> nodes to re-use them in other
     *   elements.
     * @since DOM Level 2
     */
    virtual DOMAttr      *  setAttributeNodeNS(DOMAttr *newAttr) = 0;

    /**
     * Returns a <code>DOMNodeList</code> of all the <code>DOMElement</code>s
     * with a given local name and namespace URI in the order in which they
     * would be encountered in a preorder traversal of the
     * <code>DOMDocument</code> tree, starting from this node.
     *
     * @param namespaceURI The <em>namespace URI</em> of
     *    the elements to match on. The special value "*" matches all
     *    namespaces.
     * @param localName The <em>local name</em> of the
     *    elements to match on. The special value "*" matches all local names.
     * @return A new <code>DOMNodeList</code> object containing all the matched
     *    <code>DOMElement</code>s.
     * @since DOM Level 2
     */
    virtual DOMNodeList   * getElementsByTagNameNS(const XMLCh *namespaceURI,
                                                   const XMLCh *localName) const = 0;

    /**
     * Returns <code>true</code> when an attribute with a given name is
     * specified on this element or has a default value, <code>false</code>
     * otherwise.
     * @param name The name of the attribute to look for.
     * @return <code>true</code> if an attribute with the given name is
     *   specified on this element or has a default value, <code>false</code>
     *    otherwise.
     * @since DOM Level 2
     */
    virtual bool         hasAttribute(const XMLCh *name) const = 0;

    /**
     * Returns <code>true</code> when an attribute with a given local name and
     * namespace URI is specified on this element or has a default value,
     * <code>false</code> otherwise. HTML-only DOM implementations do not
     * need to implement this method.
     * @param namespaceURI The namespace URI of the attribute to look for.
     * @param localName The local name of the attribute to look for.
     * @return <code>true</code> if an attribute with the given local name
     *   and namespace URI is specified or has a default value on this
     *   element, <code>false</code> otherwise.
     * @since DOM Level 2
     */
    virtual bool         hasAttributeNS(const XMLCh *namespaceURI,
                                        const XMLCh *localName) const = 0;
    //@}

};

#endif


