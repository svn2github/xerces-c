#ifndef RangeImpl_HEADER_GUARD_
#define RangeImpl_HEADER_GUARD_
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



#include "DOM_Node.hpp"
#include "RefCountedImpl.hpp"
#include "DOM_Range.hpp"
#include "DOM_Text.hpp"
#include "DOM_Document.hpp"
#include <util/RefVectorOf.hpp>


typedef RefVectorOf<RangeImpl> Ranges;
 
class CDOM_EXPORT RangeImpl : public RefCountedImpl {
public:    
    //c'tor
    RangeImpl(DOM_Document doc);
    RangeImpl(const RangeImpl& other);

    //d'tor
    ~RangeImpl();

    //referencing related functions
    virtual void                unreferenced();
    

    //getter functions
    DOM_Node&    getStartContainer();
    unsigned    int getStartOffset();
    DOM_Node&    getEndContainer();
    unsigned int getEndOffset();
    
    
    
    void        collapse(bool toStart);
    bool        getCollapsed();

    void        setStartBefore(DOM_Node& node);
    void        setStartAfter(DOM_Node& node);
    void        setEndBefore(DOM_Node& node);
    void        setEndAfter(DOM_Node& node);

    void        setStart(DOM_Node& node, unsigned int offset);
    void        setEnd(DOM_Node& node, unsigned int offset);

    void        selectNode(DOM_Node& node);
    void        selectNodeContents(DOM_Node& node);
   
    short       compareBoundaryPoints(DOM_Range::CompareHow how, RangeImpl* range);
   
    void        detach();
   
    void        deleteContents();
    
    RangeImpl*  cloneRange();
    DOMString   toString();    

    DOM_Document getDocument();
    void        surroundContents(DOM_Node& node);
    DOM_DocumentFragment extractContents();
    DOM_DocumentFragment cloneContents();
    void        insertNode(DOM_Node& newNode);
    const DOM_Node&    getCommonAncestorContainer();

    // functions to inform all existing valid ranges about a change
    void updateSplitInfo(TextImpl* oldNode, TextImpl* startNode);
    void updateRangeForInsertedNode(NodeImpl* node);
    void receiveReplacedText(NodeImpl* node);
    void updateRangeForDeletedText(DOM_Node& node, unsigned int offset, int count);
    void updateRangeForDeletedNode(NodeImpl* node);

private:
    enum TraversalType {
        EXTRACT_CONTENTS = 1,
        CLONE_CONTENTS   = 2
    };

    enum TraversePoint {
        BEFORE  = -1,    
        START   = 0,
        AFTER   = 1
    };

    //setter functions
    void        setStartContainer(const DOM_Node& node); 
    void        setStartOffset(unsigned int offset) ;
    void        setEndContainer(const DOM_Node& node);
    void        setEndOffset(unsigned int offset) ;
    void        setCommonAncestorContainer(const DOM_Node& node) ;

    //misc functions
    void        validateNode(DOM_Node& node);
    bool        isValidAncestorType(DOM_Node& node);
    void        checkIndex(DOM_Node& node, unsigned int offset);
    static bool isAncestorOf(const DOM_Node& a, const DOM_Node& b);
    
    unsigned short indexOf(const DOM_Node& child, const DOM_Node& parent);

    DOM_Node    commonAncestorOf(DOM_Node& pointA, DOM_Node& pointB);
    DOM_Node    nextNode(const DOM_Node& node, bool visitChildren);
    DOM_DocumentFragment traverseContents(TraversalType type);
    void        checkReadOnly(DOM_Node& start, DOM_Node& end, 
                    unsigned int starOffset, unsigned int endOffset);
    void        recurseTreeAndCheck(DOM_Node& start, DOM_Node& end);
    DOM_Node    removeChild(DOM_Node& parent, DOM_Node& child);

   
    //private data 
    DOM_Node        fStartContainer;
    unsigned int    fStartOffset;
    DOM_Node        fEndContainer;
    unsigned int    fEndOffset;
    bool            fCollapsed;
    DOM_Node        fCommonAncestorContainer;
    DOM_Document    fDocument;
    bool            fDetached;
    
    DOM_Node        fRemoveChild;
    
};

#endif
