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
 * $Log$
 * Revision 1.7  2001/04/19 18:17:30  tng
 * Schema: SchemaValidator update, and use QName in Content Model
 *
 * Revision 1.6  2001/03/21 21:56:27  tng
 * Schema: Add Schema Grammar, Schema Validator, and split the DTDValidator into DTDValidator, DTDScanner, and DTDGrammar.
 *
 * Revision 1.5  2001/03/21 19:29:51  tng
 * Schema: Content Model Updates, by Pei Yong Zhang.
 *
 * Revision 1.4  2001/02/27 18:32:31  tng
 * Schema: Use XMLElementDecl instead of DTDElementDecl in Content Model.
 *
 * Revision 1.3  2001/02/27 14:48:51  tng
 * Schema: Add CMAny and ContentLeafNameTypeVector, by Pei Yong Zhang
 *
 * Revision 1.2  2001/02/16 14:58:57  tng
 * Schema: Update Makefile, configure files, project files, and include path in
 * certain cpp files because of the move of the common Content Model files.  By Pei Yong Zhang.
 *
 * Revision 1.1  2001/02/16 14:17:29  tng
 * Schema: Move the common Content Model files that are shared by DTD
 * and schema from 'DTD' folder to 'common' folder.  By Pei Yong Zhang.
 *
 * Revision 1.5  2000/03/28 19:43:25  roddey
 * Fixes for signed/unsigned warnings. New work for two way transcoding
 * stuff.
 *
 * Revision 1.4  2000/03/08 23:52:34  roddey
 * Got rid of the use of -1 to represent an invalid transition state,
 * and just created a const value that is unsigned. This should make
 * some compilers happier.
 *
 * Revision 1.3  2000/03/02 19:55:38  roddey
 * This checkin includes many changes done while waiting for the
 * 1.1.0 code to be finished. I can't list them all here, but a list is
 * available elsewhere.
 *
 * Revision 1.2  2000/02/09 21:42:37  abagchi
 * Copyright swatswat
 *
 * Revision 1.1.1.1  1999/11/09 01:03:17  twl
 * Initial checkin
 *
 * Revision 1.2  1999/11/08 20:45:38  rahul
 * Swat for adding in Product name and CVS comment log variable.
 *
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <util/RuntimeException.hpp>
#include <framework/XMLElementDecl.hpp>
#include <validators/common/CMAny.hpp>
#include <validators/common/CMBinaryOp.hpp>
#include <validators/common/CMLeaf.hpp>
#include <validators/common/CMUnaryOp.hpp>
#include <validators/common/DFAContentModel.hpp>
#include <validators/common/ContentSpecNode.hpp>
#include <validators/common/Grammar.hpp>
#include <util/RefHashTableOf.hpp>

// ---------------------------------------------------------------------------
//  Local static data
//
//  gInvalidTrans
//      This value represents an invalid transition in each line of the
//      transition table.
//
//  gEOCFakeId
//  gEpsilonFakeId
//      We have to put in a couple of special CMLeaf nodes to represent
//      special values, using fake element ids that we know won't conflict
//      with real element ids.
//
//
// ---------------------------------------------------------------------------
static const unsigned int   gInvalidTrans   = 0xFFFFFFFF;
static const unsigned int   gEOCFakeId      = 0xFFFFFFF1;
static const unsigned int   gEpsilonFakeId  = 0xFFFFFFF2;

// ---------------------------------------------------------------------------
//  DFAContentModel: Constructors and Destructor
// ---------------------------------------------------------------------------
DFAContentModel::DFAContentModel(const bool            dtd
                               , XMLElementDecl* const elemDecl) :

    fElemMap(0)
	 , fElemMapType(0)
    , fElemMapSize(0)
    , fEmptyOk(false)
    , fEOCPos(0)
    , fFinalStateFlags(0)
    , fFollowList(0)
    , fHeadNode(0)
    , fLeafCount(0)
    , fLeafList(0)
    , fTransTable(0)
    , fTransTableSize(0)
    , fDTD(dtd)
    , fLeafNameTypeVector(0)
{
    // And build the DFA data structures
    buildDFA(elemDecl->getContentSpec());
}

DFAContentModel::~DFAContentModel()
{
    //
    //  Clean up all the stuff that is not just temporary representation
    //  data that was cleaned up after building the DFA.
    //
    delete [] fFinalStateFlags;

    unsigned index;
    for (index = 0; index < fTransTableSize; index++)
        delete [] fTransTable[index];
    delete [] fTransTable;

    delete [] fElemMap;
    delete [] fElemMapType;
}


// ---------------------------------------------------------------------------
//  DFAContentModel: Implementation of the ContentModel virtual interface
// ---------------------------------------------------------------------------
int
DFAContentModel::validateContent( QName** const        children
                                , const unsigned int   childCount
                                , const unsigned int   emptyNamespaceId) const
{
    //
    //  If there are no children, then either we fail on the 0th element
    //  or we return success. It depends upon whether this content model
    //  accepts empty content, which we determined earlier.
    //
    if (!childCount)
    {
        // success -1
        return fEmptyOk ? -1 : 0;
    }

    //
    //  Lets loop through the children in the array and move our way
    //  through the states. Note that we use the fElemMap array to map
    //  an element index to a state index.
    //
    unsigned int curState = 0;
    unsigned int nextState = 0;
    unsigned int childIndex = 0;
    for (; childIndex < childCount; childIndex++)
    {
        // Get the current element index out
        const QName* curElem = children[childIndex];

        // Look up this child in our element map
        unsigned int elemIndex = 0;
        for (; elemIndex < fElemMapSize; elemIndex++)
        {
            const QName* inElem  = fElemMap[elemIndex];
            if (fDTD) {
                if (!XMLString::compareString(inElem->getRawName(), curElem->getRawName())) {
                    nextState = fTransTable[curState][elemIndex];
                    if (nextState != gInvalidTrans)
                        break;
                }
            }
            else {
                ContentSpecNode::NodeTypes type = fElemMapType[elemIndex];
                if (type == ContentSpecNode::Leaf)
                {
                    if ((inElem->getURI() == curElem->getURI()) &&
                    (!XMLString::compareString(inElem->getLocalPart(), curElem->getLocalPart()))) {
                        nextState = fTransTable[curState][elemIndex];
                        if (nextState != gInvalidTrans)
                            break;
                    }
                }
                else if ((type & 0x0f)== ContentSpecNode::Any)
                {
                    unsigned int uri = inElem->getURI();
                    if ((uri == emptyNamespaceId) || (uri == curElem->getURI())) {
                        nextState = fTransTable[curState][elemIndex];
                        if (nextState != gInvalidTrans)
                            break;
                    }
                }
                else if ((type & 0x0f) == ContentSpecNode::Any_Local)
                {
                    if (curElem->getURI() == emptyNamespaceId)
                    {
                        nextState = fTransTable[curState][elemIndex];
                        if (nextState != gInvalidTrans)
                            break;
                    }
                }
                else if ((type & 0x0f) == ContentSpecNode::Any_Other)
                {
                    if (inElem->getURI() != curElem->getURI()) {
                        nextState = fTransTable[curState][elemIndex];
                        if (nextState != gInvalidTrans)
                            break;
                    }
                }
            }
        }//for elemIndex

        // If "nextState" is -1, we found a match, but the transition is invalid
        if (nextState == gInvalidTrans)
            return childIndex;

        // If we didn't find it, then obviously not valid
        if (elemIndex == fElemMapSize)
            return childIndex;

        curState = nextState;
        nextState = 0;

    }//for childIndex

    //
    //  We transitioned all the way through the input list. However, that
    //  does not mean that we ended in a final state. So check whether
    //  our ending state is a final state.
    //
    if (!fFinalStateFlags[curState])
        return childIndex;

    //success
    return -1;
}


// ---------------------------------------------------------------------------
//  DFAContentModel: Private helper methods
// ---------------------------------------------------------------------------
void DFAContentModel::buildDFA(ContentSpecNode* const curNode)
{
    unsigned int index;


    //
    //  The first step we need to take is to rewrite the content model using
    //  our CMNode objects, and in the process get rid of any repetition short
    //  cuts, converting them into '*' style repetitions or getting rid of
    //  repetitions altogether.
    //
    //  The conversions done are:
    //
    //  x+ -> (x|x*)
    //  x? -> (x|epsilon)
    //
    //  This is a relatively complex scenario. What is happening is that we
    //  create a top level binary node of which the special EOC value is set
    //  as the right side node. The the left side is set to the rewritten
    //  syntax tree. The source is the original content model info from the
    //  decl pool. The rewrite is done by buildSyntaxTree() which recurses the
    //  decl pool's content of the element and builds a new tree in the
    //  process.
    //
    //  Note that, during this operation, we set each non-epsilon leaf node's
    //  DFA state position and count the number of such leafs, which is left
    //  in the fLeafCount member.
    //
    QName* qname = new QName(XMLUni::fgZeroLenString, XMLUni::fgZeroLenString, gEOCFakeId);
    CMLeaf* nodeEOC = new CMLeaf(qname);
    CMNode* nodeOrgContent = buildSyntaxTree(curNode);
    fHeadNode = new CMBinaryOp
    (
        ContentSpecNode::Sequence
        , nodeOrgContent
        , nodeEOC
    );

    //
    //  And handle specially the EOC node, which also must be numbered and
    //  counted as a non-epsilon leaf node. It could not be handled in the
    //  above tree build because it was created before all that started. We
    //  save the EOC position since its used during the DFA building loop.
    //
    fEOCPos = fLeafCount;
    nodeEOC->setPosition(fLeafCount++);

    //
    //  Ok, so now we have to iterate the new tree and do a little more work
    //  now that we know the leaf count. One thing we need to do is to
    //  calculate the first and last position sets of each node. This is
    //  cached away in each of the nodes.
    //
    //  Along the way we also set the leaf count in each node as the maximum
    //  state count. They must know this in order to create their first/last
    //  position sets.
    //
    //  We also need to build an array of references to the non-epsilon
    //  leaf nodes. Since we iterate here the same way as we did during the
    //  initial tree build (which built their position numbers, we will put
    //  them in the array according to their position values.
    //
    fLeafList = new CMLeaf*[fLeafCount];
    fLeafListType = new ContentSpecNode::NodeTypes[fLeafCount];
    postTreeBuildInit(fHeadNode, 0);

    //
    //  And, moving onward... We now need to build the follow position sets
    //  for all the nodes. So we allocate an array of pointers to state sets,
    //  one for each leaf node (i.e. each significant DFA position.)
    //
    fFollowList = new CMStateSet*[fLeafCount];
    for (index = 0; index < fLeafCount; index++)
        fFollowList[index] = new CMStateSet(fLeafCount);
    calcFollowList(fHeadNode);

    //
    //  Check to see whether this content model can handle an empty content,
    //  which is something we need to optimize by looking now before we
    //  throw away the info that would tell us that.
    //
    //  If the left node of the head (the top level of the original content)
    //  is nullable, then its true.
    //
    fEmptyOk = nodeOrgContent->isNullable();

    //
    //  And finally the big push... Now we build the DFA using all the states
    //  and the tree we've built up. First we set up the various data
    //  structures we are going to use while we do this.
    //
    //  First of all we need an array of unique element ids in our content
    //  model. For each transition table entry, we need a set of contiguous
    //  indices to represent the transitions for a particular input element.
    //  So we need to a zero based range of indexes that map to element types.
    //  This element map provides that mapping.
    //
    fElemMap = new QName*[fLeafCount];
    fElemMapType = new ContentSpecNode::NodeTypes[fLeafCount];
    fElemMapSize = 0;
    for (unsigned int outIndex = 0; outIndex < fLeafCount; outIndex++)
    {
        fElemMap[outIndex] = new QName();

        if ( (fLeafListType[outIndex] & 0x0f) != ContentSpecNode::Leaf )
            if (!fLeafNameTypeVector)
                fLeafNameTypeVector = new ContentLeafNameTypeVector();

        // Get the current leaf's element index
        const QName* element = fLeafList[outIndex]->getElement();

        // See if the current leaf node's element index is in the list
        unsigned int inIndex = 0;
        for (; inIndex < fElemMapSize; inIndex++)
        {
            const QName* inElem = fElemMap[inIndex];
            if (fDTD) {
                if (!XMLString::compareString(inElem->getRawName(), element->getRawName())) {
                    break;
                }
            }
            else {
                if ((inElem->getURI() == element->getURI()) &&
                    (!XMLString::compareString(inElem->getLocalPart(), element->getLocalPart()))) {
                    break;
                }
            }
        }

        // If it was not in the list, then add it and bump the map size
        if (inIndex == fElemMapSize)
        {
            fElemMap[fElemMapSize]->setValues(*element);
            fElemMapType[fElemMapSize] = fLeafListType[outIndex];
            ++fElemMapSize;
        }
    }

    // set up the fLeafNameTypeVector object if there is one.
    if (fLeafNameTypeVector) {
        fLeafNameTypeVector->setValues(fElemMap, fElemMapType, fElemMapSize);
    }

    /***
     * Optimization(Jan, 2001); We sort fLeafList according to
     * elemIndex which is *uniquely* associated to each leaf.
     * We are *assuming* that each element appears in at least one leaf.
     **/
    // don't forget to delete it
    unsigned int *fLeafSorter = new unsigned int[fLeafCount + fElemMapSize];
    unsigned int fSortCount = 0;

    for (unsigned int elemIndex = 0; elemIndex < fElemMapSize; elemIndex++)
    {
        for (unsigned int leafIndex = 0; leafIndex < fLeafCount; leafIndex++)
        {
            const QName* leaf = fLeafList[leafIndex]->getElement();
            const int leafType = fLeafListType[leafIndex];
            const QName* element = fElemMap[elemIndex];
            if (fDTD) {
                if (!XMLString::compareString(leaf->getRawName(), element->getRawName())) {
                    fLeafSorter[fSortCount++] = leafIndex;
                }
            }
            else {
                if ((leaf->getURI() == element->getURI()) &&
                    (!XMLString::compareString(leaf->getLocalPart(), element->getLocalPart()))) {
                    fLeafSorter[fSortCount++] = leafIndex;
                }
            }
        }
        fLeafSorter[fSortCount++] = -1;
    }
    //
    //  Next lets create some arrays, some that that hold transient info
    //  during the DFA build and some that are permament. These are kind of
    //  sticky since we cannot know how big they will get, but we don't want
    //  to use any collection type classes because of performance.
    //
    //  Basically they will probably be about fLeafCount*2 on average, but can
    //  be as large as 2^(fLeafCount*2), worst case. So we start with
    //  fLeafCount*4 as a middle ground. This will be very unlikely to ever
    //  have to expand though, it if does, the overhead will be somewhat ugly.
    //
    unsigned int curArraySize = fLeafCount * 4;
    const CMStateSet** statesToDo = new const CMStateSet*[curArraySize];
    fFinalStateFlags = new bool[curArraySize];
    fTransTable = new unsigned int*[curArraySize];

    //
    //  Ok we start with the initial set as the first pos set of the head node
    //  (which is the seq node that holds the content model and the EOC node.)
    //
    const CMStateSet* setT = new CMStateSet(fHeadNode->getFirstPos());

    //
    //  Init our two state flags. Basically the unmarked state counter is
    //  always chasing the current state counter. When it catches up, that
    //  means we made a pass through that did not add any new states to the
    //  lists, at which time we are done. We could have used a expanding array
    //  of flags which we used to mark off states as we complete them, but
    //  this is easier though less readable maybe.
    //
    unsigned int unmarkedState = 0;
    unsigned int curState = 0;

    //
    //  Init the first transition table entry, and put the initial state
    //  into the states to do list, then bump the current state.
    //
    fTransTable[curState] = makeDefStateList();
    statesToDo[curState] = setT;
    curState++;

    //
    // RefHashTableOf<unsigned int> stateTable(curArraySize, false);
    // In XML4J, a hash table is created, with CMStateSet as key and
    // the related index value as data.
    // We don't do HashTable searching here 'cause the performance
    // gain is NOT so clear.

    //
    //  Ok, almost done with the algorithm from hell... We now enter the
    //  loop where we go until the states done counter catches up with
    //  the states to do counter.
    //
    CMStateSet* newSet = 0;
    while (unmarkedState < curState)
    {
        //
        //  Get the next unmarked state out of the list of states to do.
        //  And get the associated transition table entry.
        //
        setT = statesToDo[unmarkedState];
        unsigned int* transEntry = fTransTable[unmarkedState];

        // Mark this one final if it contains the EOC state
        fFinalStateFlags[unmarkedState] = setT->getBit(fEOCPos);

        // Bump up the unmarked state count, marking this state done
        unmarkedState++;

        // Optimization(Jan, 2001)
        unsigned int sorterIndex = 0;
        // Optimization(Jan, 2001)

        // Loop through each possible input symbol in the element map
        for (unsigned int elemIndex = 0; elemIndex < fElemMapSize; elemIndex++)
        {
            //
            //  Build up a set of states which is the union of all of the
            //  follow sets of DFA positions that are in the current state. If
            //  we gave away the new set last time through then create a new
            //  one. Otherwise, zero out the existing one.
            //
            if (!newSet)
                newSet = new CMStateSet(fLeafCount);
            else
                newSet->zeroBits();

#ifdef OBSOLETED
            for (unsigned int leafIndex = 0; leafIndex < fLeafCount; leafIndex++)
            {
                // If this leaf index (DFA position) is in the current set...
                if (setT->getBit(leafIndex))
                {
                    //
                    //  If this leaf is the current input symbol, then we want
                    //  to add its follow list to the set of states to transition
                    //  to from the current state.
                    //
                    const QName* leaf = fLeafList[leafIndex]->getElement();
                    const QName* element = fElemMap[elemIndex];
                    if (fDTD) {
                        if (!XMLString::compareString(leaf->getRawName(), element->getRawName())) {
                            *newSet |= *fFollowList[leafIndex];
                        }
                    }
                    else {
                        if ((leaf->getURI() == element->getURI()) &&
                            (!XMLString::compareString(leaf->getLocalPart(), element->getLocalPart())) {
                            *newSet |= *fFollowList[leafIndex];
                        }
                    }
                }
            } // for leafIndex
#endif
            // Optimization(Jan, 2001)
            unsigned int leafIndex = fLeafSorter[sorterIndex++];

            while (leafIndex != -1)
            {
                // If this leaf index (DFA position) is in the current set...
                if (setT->getBit(leafIndex))
                {
                    //
                    //  If this leaf is the current input symbol, then we
                    //  want to add its follow list to the set of states to
                    //  transition to from the current state.
                    //
                    *newSet |= *fFollowList[leafIndex];
                }
                leafIndex = fLeafSorter[sorterIndex++];
            } // while (leafIndex != -1)

            //
            //  If this new set is not empty, then see if its in the list
            //  of states to do. If not, then add it.
            //
            if (!newSet->isEmpty())
            {
                //
                //  Search the 'states to do' list to see if this new
                //  state set is already in there.
                //
                unsigned int stateIndex = 0;
                for (; stateIndex < curState; stateIndex++)
                {
                    if (*statesToDo[stateIndex] == *newSet)
                        break;
                }

                // If we did not find it, then add it
                if (stateIndex == curState)
                {
                    //
                    //  Put this new state into the states to do and init
                    //  a new entry at the same index in the transition
                    //  table.
                    //
                    statesToDo[curState] = newSet;
                    fTransTable[curState] = makeDefStateList();

                    // We now have a new state to do so bump the count
                    curState++;

                    //
                    //  Null out the new set to indicate we adopted it. This
                    //  will cause the creation of a new set on the next time
                    //  around the loop.
                    //
                    newSet = 0;
                }

                //
                //  Now set this state in the transition table's entry for this
                //  element (using its index), with the DFA state we will move
                //  to from the current state when we see this input element.
                //
                transEntry[elemIndex] = stateIndex;

                // Expand the arrays if we're full
                if (curState == curArraySize)
                {
                    //
                    //  Yikes, we overflowed the initial array size, so we've
                    //  got to expand all of these arrays. So adjust up the
                    //  size by 50% and allocate new arrays.
                    //
                    const unsigned int newSize = (unsigned int)(curArraySize * 1.5);
                    const CMStateSet** newToDo = new const CMStateSet*[newSize];
                    bool* newFinalFlags = new bool[newSize];
                    unsigned int** newTransTable = new unsigned int*[newSize];

                    // Copy over all of the existing content
                    for (unsigned int expIndex = 0; expIndex < curArraySize; expIndex++)
                    {
                        newToDo[expIndex] = statesToDo[expIndex];
                        newFinalFlags[expIndex] = fFinalStateFlags[expIndex];
                        newTransTable[expIndex] = fTransTable[expIndex];
                    }

                    // Clean up the old stuff
                    delete [] statesToDo;
                    delete [] fFinalStateFlags;
                    delete [] fTransTable;

                    // Store the new array size and pointers
                    curArraySize = newSize;
                    statesToDo = newToDo;
                    fFinalStateFlags = newFinalFlags;
                    fTransTable = newTransTable;
                } //if (curState == curArraySize)
            } //if (!newSet->isEmpty())
        } // for elemIndex
    } //while

    // Store the current state count in the trans table size
    fTransTableSize = curState;

    // If the last temp set was not stored, then clean it up
    if (newSet)
        delete newSet;

    //
    //  Now we can clean up all of the temporary data that was needed during
    //  DFA build.
    //
    delete fHeadNode;
    fHeadNode = 0;

    for (index = 0; index < fLeafCount; index++)
        delete fFollowList[index];

    for (index = 0; index < curState; index++)
        delete (CMStateSet*)statesToDo[index];

    delete [] fLeafList;
    delete [] fFollowList;
    delete [] statesToDo;

	delete [] fLeafSorter;
}


CMNode* DFAContentModel::buildSyntaxTree(ContentSpecNode* const curNode)
{
    // Initialize a return node pointer
    CMNode* retNode = 0;

    // Get the spec type of the passed node
    const ContentSpecNode::NodeTypes curType = curNode->getType();

    if ((curType & 0x0f) == ContentSpecNode::Any)
    {
        retNode = new CMAny(curType, curNode->getElement()->getURI(), fLeafCount++);
    }
    else if ((curType & 0x0f) == ContentSpecNode::Any_Other)
    {
        retNode = new CMAny(curType, curNode->getElement()->getURI(), fLeafCount++);
    }
    else if ((curType & 0x0f) == ContentSpecNode::Any_Local)
    {
        retNode = new CMAny(curType, 0, fLeafCount++);
    }
    else if (curType == ContentSpecNode::Leaf)
    {
        //
        //  Create a new leaf node, and pass it the current leaf count, which
        //  is its DFA state position. Bump the leaf count after storing it.
        //  This makes the positions zero based since we store first and then
        //  increment.
        //
        retNode = new CMLeaf(curNode->getElement(), fLeafCount++);
    }
     else
    {
        //
        //  Its not a leaf, so we have to recurse its left and maybe right
        //  nodes. Save both values before we recurse and trash the node.
        //
        ContentSpecNode* leftNode = curNode->getFirst();
        ContentSpecNode* rightNode = curNode->getSecond();

        if ((curType == ContentSpecNode::Choice)
        ||   (curType == ContentSpecNode::Sequence))
        {
            //
            //  Recurse on both children, and return a binary op node with the
            //  two created sub nodes as its children. The node type is the
            //  same type as the source.
            //
            CMNode* newLeft = buildSyntaxTree(leftNode);
            CMNode* newRight = buildSyntaxTree(rightNode);
            retNode = new CMBinaryOp(curType, newLeft, newRight);
        }
         else if (curType == ContentSpecNode::ZeroOrMore
               || curType == ContentSpecNode::ZeroOrOne
               || curType == ContentSpecNode::OneOrMore)
        {
            // This one is fine as is, just change to our form
            retNode = new CMUnaryOp(curType, buildSyntaxTree(leftNode));
        }
         else
        {
            ThrowXML(RuntimeException, XMLExcepts::CM_UnknownCMSpecType);
        }
    }
    return retNode;
}


void DFAContentModel::calcFollowList(CMNode* const curNode)
{
    // Get the spec type of the passed node
    const ContentSpecNode::NodeTypes curType = curNode->getType();

    if (curType == ContentSpecNode::Choice)
    {
        // Just recurse
        calcFollowList(((CMBinaryOp*)curNode)->getLeft());
        calcFollowList(((CMBinaryOp*)curNode)->getRight());
    }
     else if (curType == ContentSpecNode::Sequence)
    {
        // Recurse before we process this node
        calcFollowList(((CMBinaryOp*)curNode)->getLeft());
        calcFollowList(((CMBinaryOp*)curNode)->getRight());

        //
        //  Now handle our level. We use our left child's last pos set and our
        //  right child's first pos set, so get them now for convenience.
        //
        const CMStateSet& last  = ((CMBinaryOp*)curNode)->getLeft()->getLastPos();
        const CMStateSet& first = ((CMBinaryOp*)curNode)->getRight()->getFirstPos();

        //
        //  Now, for every position which is in our left child's last set
        //  add all of the states in our right child's first set to the
        //  follow set for that position.
        //
        for (unsigned int index = 0; index < fLeafCount; index++)
        {
            if (last.getBit(index))
                *fFollowList[index] |= first;
        }
    }
     else if (curType == ContentSpecNode::ZeroOrMore ||
		      curType == ContentSpecNode::OneOrMore   )
    {
        // Recurse first
        calcFollowList(((CMUnaryOp*)curNode)->getChild());

        //
        //  Now handle our level. We use our own first and last position
        //  sets, so get them up front.
        //
        const CMStateSet& first = curNode->getFirstPos();
        const CMStateSet& last  = curNode->getLastPos();

        //
        //  For every position which is in our last position set, add all
        //  of our first position states to the follow set for that
        //  position.
        //
        for (unsigned int index = 0; index < fLeafCount; index++)
        {
            if (last.getBit(index))
                *fFollowList[index] |= first;
        }
    }
     else if (curType == ContentSpecNode::ZeroOrOne)
    {
        // Recurse only
        calcFollowList(((CMUnaryOp*)curNode)->getChild());
        //ThrowXML1(RuntimeException, XMLExcepts::CM_NotValidForSpecType, "CalcFollowList");
    }
}


//
//  gInvalidTrans is used to represent bad transitions in the transition table
//  entry for each state. So each entry is initialized to that value. This
//  method creates a new entry and initializes it.
//
unsigned int* DFAContentModel::makeDefStateList() const
{
    unsigned int* retArray = new unsigned int[fElemMapSize];
    for (unsigned int index = 0; index < fElemMapSize; index++)
        retArray[index] = gInvalidTrans;
    return retArray;
}


int DFAContentModel::postTreeBuildInit(         CMNode* const   nodeCur
                                        , const unsigned int    curIndex)
{
    // Set the maximum states on this node
    nodeCur->setMaxStates(fLeafCount);

    // Get the spec type of the passed node
    const ContentSpecNode::NodeTypes curType = nodeCur->getType();

    // Get a copy of the index we can modify
    unsigned int newIndex = curIndex;

    // Recurse as required
    if ( ((curType & 0x0f) == ContentSpecNode::Any)       ||
         ((curType & 0x0f) == ContentSpecNode::Any_Local) ||
         ((curType & 0x0f) == ContentSpecNode::Any_Other)  )
    {
        QName* qname = new QName(XMLUni::fgZeroLenString, XMLUni::fgZeroLenString, ((CMAny*) nodeCur)->getURI());

        fLeafList[newIndex] = new CMLeaf(qname, ((CMAny*)nodeCur)->getPosition());
        fLeafListType[newIndex] = curType;
        ++newIndex;
    }
    else if ((curType == ContentSpecNode::Choice)
         ||  (curType == ContentSpecNode::Sequence))
    {
        newIndex = postTreeBuildInit(((CMBinaryOp*)nodeCur)->getLeft(), newIndex);
        newIndex = postTreeBuildInit(((CMBinaryOp*)nodeCur)->getRight(), newIndex);
    }
    else if (curType == ContentSpecNode::ZeroOrMore ||
             curType == ContentSpecNode::ZeroOrOne  ||
             curType == ContentSpecNode::OneOrMore)
    {
        newIndex = postTreeBuildInit(((CMUnaryOp*)nodeCur)->getChild(), newIndex);
    }
    else if (curType == ContentSpecNode::Leaf)
    {
        //
        //  Put this node in the leaf list at the current index if its
        //  a non-epsilon leaf.
        //
        if (((CMLeaf*)nodeCur)->getElement()->getURI() != gEpsilonFakeId)
        {
            fLeafList[newIndex] = (CMLeaf*)nodeCur;
            fLeafListType[newIndex] = ContentSpecNode::Leaf;
            ++newIndex;
        }
    }
    else
    {
        ThrowXML(RuntimeException, XMLExcepts::CM_UnknownCMSpecType);
    }
    return newIndex;
}

int
DFAContentModel::validateContentSpecial( QName** const       children
                                       , const unsigned int  childCount
                                       , const unsigned int  emptyNamespaceId) const
{
	return 0;
};

ContentLeafNameTypeVector* DFAContentModel::getContentLeafNameTypeVector() const
{
   //later change it to return the data member
	return fLeafNameTypeVector;
};

