#ifndef IDDocumentImpl_HEADER_GUARD_
#define IDDocumentImpl_HEADER_GUARD_

/*
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2001 The Apache Software Foundation.  All rights
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

//
//  This file is part of the internal implementation of the C++ XML DOM.
//  It should NOT be included or used directly by application programs.
//
//  Applications should include the file <dom/DOM.hpp> for the entire
//  DOM API, or IDOM_*.hpp for individual DOM classes, where the class
//  name is substituded for the *.
//

#include <string.h>
#include <xercesc/util/XercesDefs.hpp>
#include "xercesc/util/RefVectorOf.hpp"
#include "xercesc/util/RefHashTableOf.hpp"

#include "IDOM_Document.hpp"
#include "IDNodeImpl.hpp"
#include "IDParentNode.hpp"
#include "IDDeepNodeListPool.hpp"


class IDAttrImpl;
class IDCDATASectionImpl;
class IDCommentImpl;
class IDDeepNodeListImpl;
class IDDocumentFragmentImpl;
class IDDocumentTypeImpl;
class IDDStringPool;
class IDElementImpl;
class IDEntityImpl;
class IDEntityReferenceImpl;
class IDNodeImpl;
class IDNotationImpl;
class IDProcessingInstructionImpl;
class IDTextImpl;
class IDNodeIteratorImpl;
class IDTreeWalkerImpl;
class IDOM_NodeFilter;
class IDNodeFilterImpl;
class IDOM_DOMImplementation;
class IDNodeIDMap;
class IDRangeImpl;
class IDParentNode;
class IDStringPool;

typedef RefVectorOf<IDNodeIteratorImpl> NodeIterators;
typedef RefVectorOf<IDTreeWalkerImpl>   TreeWalkers;
typedef RefVectorOf<IDRangeImpl>        Ranges;


class CDOM_EXPORT IDDocumentImpl: public IDOM_Document {
public:
    // -----------------------------------------------------------------------
    //  Private data types
    // -----------------------------------------------------------------------


    IDNodeImpl                  fNode;           // Implements common node functionality.
    IDParentNode                fParent;         // Implements common parent node functionality

    IDOM_DocumentType           *fDocType;
    IDOM_Element                *fDocElement;
    IDStringPool                *fNamePool;
    IDNodeIDMap                 *fNodeIDMap;     // for use by GetElementsById().

    NodeIterators               *fIterators;
    TreeWalkers                 *fTreeWalkers;
    Ranges                      *fRanges;

    RefHashTableOf<void>        *fUserData;
    int                          fChanges;

    bool errorChecking;                          // Bypass error checking.


    // Per-Document heap Variables.
    //   The heap consists of one or more biggish blocks which are
    //   sub-allocated for individual allocations of nodes, strings, etc.
    //   The big blocks form a linked list, allowing them to be located for deletion.
    //
    //   There is no provision for deleting suballocated blocks, other than
    //     deleting the entire heap when the document is deleted.
    //
    //   There is no header on individual sub-allocated blocks.
    //   The header on big blocks consists only of a single back pointer to
    //    the previously allocated big block (our linked list of big blocks)
    //
    //
    //   idom_revisit - this heap should be encapsulated into its own
    //                  class, rather than hanging naked on Document.
    //
    void                        *fCurrentBlock;
    char                        *fFreePtr;
    unsigned int                 fFreeBytesRemaining;




    friend class IDNodeImpl;
    friend class IDNodeIteratorImpl;
    friend class IDTreeWalkerImpl;
    friend class IDRangeImpl;
    friend class IDDOMParser;



    void setDocumentType(IDOM_DocumentType *doctype);



public:
    IDDocumentImpl();
    IDDocumentImpl(const XMLCh      *namespaceURI,	     //DOM Level 2
                   const XMLCh      *qualifiedName,
                   IDOM_DocumentType *doctype);
    virtual ~IDDocumentImpl();

    // Add all functions that are pure virutal in IDOM_NODE
    IDOM_NODE_FUNCTIONS;

    virtual IDOM_Attr             *createAttribute(const XMLCh *name);
    virtual IDOM_CDATASection     *createCDATASection(const XMLCh *data);
    virtual IDOM_Comment          *createComment(const XMLCh *data);
    virtual IDOM_DocumentFragment *createDocumentFragment();
    virtual IDOM_DocumentType     *createDocumentType(const XMLCh *name);
    virtual IDOM_DocumentType     *createDocumentType(const XMLCh *qName,
                                                    const XMLCh *publicId,
                                                    const XMLCh *systemId);
    virtual IDOM_Element          *createElement(const XMLCh * tagName);
    virtual IDOM_Element          *createElementNoCheck(const XMLCh *tagName);
    virtual IDOM_Entity           *createEntity(const XMLCh * name);
    virtual IDOM_EntityReference  *createEntityReference(const XMLCh * name);
    virtual IDOM_Notation         *createNotation(const XMLCh * name);
    virtual IDOM_ProcessingInstruction *createProcessingInstruction(const XMLCh * target, const XMLCh * data);
    virtual IDOM_Text             *createTextNode(const XMLCh * data);
    virtual IDOM_DocumentType     *getDoctype() const;
    virtual IDOM_Element          *getDocumentElement() const;
    virtual IDOM_NodeList         *getElementsByTagName(const XMLCh * tagname) const;
    virtual IDOM_DOMImplementation *getImplementation() const;
    static  bool                  isXMLName(const XMLCh * s);
    virtual IDOM_NodeIterator     *createNodeIterator(IDOM_Node *root,
                                                      unsigned long whatToShow,
                                                      IDOM_NodeFilter* filter,
                                                      bool entityReferenceExpansion);
    virtual IDOM_TreeWalker       *createTreeWalker(IDOM_Node *root,
                                                    unsigned long whatToShow,
                                                    IDOM_NodeFilter* filter,
                                                    bool entityReferenceExpansion);

    virtual void*                  getUserData() const;
    virtual void                   setUserData(void* value);
    virtual IDOM_Range            *createRange();
    virtual Ranges*                getRanges() const;  //non-standard api
    virtual void                   removeRange(IDRangeImpl* range); //non-standard api


    // helper functions to prevent storing userdata pointers on every node.
    virtual void  setUserData(IDOM_Node* n, void* data);
    virtual void* getUserData(const IDOM_Node* n) const;

    //
    // Functions to keep track of document mutations, so that node list chached
    //   information can be invalidated.  One global changes counter per document.
    //
    virtual void                 changed();
    virtual int                  changes() const;

    /**
     * Sets whether the DOM implementation performs error checking
     * upon operations. Turning off error checking only affects
     * the following DOM checks:
     * <ul>
     * <li>Checking strings to make sure that all characters are
     *     legal XML characters
     * <li>Hierarchy checking such as allowed children, checks for
     *     cycles, etc.
     * </ul>
     * <p>
     * Turning off error checking does <em>not</em> turn off the
     * following checks:
     * <ul>
     * <li>Read only checks
     * <li>Checks related to DOM events
     * </ul>
     */
    inline void setErrorChecking(bool check) {
        errorChecking = check;
    }

    /**
     * Returns true if the DOM implementation performs error checking.
     */
    inline bool getErrorChecking() {
        return errorChecking;
    }

    //Introduced in DOM Level 2
    virtual IDOM_Node            *importNode(IDOM_Node *source, bool deep);
    virtual IDOM_Element         *createElementNS(const XMLCh *namespaceURI,
	                                             const XMLCh *qualifiedName);
    virtual IDOM_Attr            *createAttributeNS(const XMLCh *namespaceURI,
	                                             const XMLCh *qualifiedName);
    virtual IDOM_NodeList        *getElementsByTagNameNS(const XMLCh *namespaceURI,
	                                              const XMLCh *localName) const;
    virtual IDOM_Element         *getElementById(const XMLCh *elementId) const;


    //Return the index > 0 of ':' in the given qualified name qName="prefix:localName".
    //Return 0 if there is no ':', or -1 if qName is malformed such as ":abcd".
    static  int                 indexofQualifiedName(const XMLCh * qName);
    static  bool                isKidOK(IDOM_Node *parent, IDOM_Node *child);

    inline IDNodeIDMap *          getNodeIDMap() {return fNodeIDMap;};


    //
    // Memory Management Functions.  All memory is allocated by and owned by
    //                               a document, and is not recovered until the
    //                               document itself is deleted.
    //

    void        * allocate(size_t amount);
    XMLCh       * cloneString(const XMLCh *src);
    const XMLCh * getPooledString(const XMLCh *src);
    void          deleteHeap();


    // Factory methods for getting/creating node lists.
    // Because nothing is ever deleted, the implementation caches and recycles
    //  previously used instances of IDDeepNodeList
    //
    IDOM_NodeList *getDeepNodeList(const IDOM_Node *rootNode, const XMLCh *tagName);
    IDOM_NodeList *getDeepNodeList(const IDOM_Node *rootNode,	//DOM Level 2
			                            const XMLCh *namespaceURI,
                                     const XMLCh *localName);

private:
    IDDeepNodeListPool<IDDeepNodeListImpl> *fNodeListPool;
};

//
//  Operator new.  Global overloaded version, lets any object be allocated on
//                 the heap owned by a document.
//
inline void * operator new(size_t amt, IDOM_Document *doc)
{
    // idom_revist.  Probably should be a checked cast.
    void *p = ((IDDocumentImpl *)doc)->allocate(amt);
    return p;
}

#endif
