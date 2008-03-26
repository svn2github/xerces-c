/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * $Id$
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMLOCATORIMPL_HPP)
#define XERCESC_INCLUDE_GUARD_DOMLOCATORIMPL_HPP

#include <xercesc/dom/DOMLocator.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
  * Introduced in DOM Level 3
  *
  * Implementation of a DOMLocator interface.
  *
  * @see DOMLocator#DOMLocator
  */

class CDOM_EXPORT DOMLocatorImpl : public DOMLocator
{
public:
    /** @name Constructors and Destructor */
    //@{

    /** Constructor */
    DOMLocatorImpl();

    DOMLocatorImpl
    (
        const unsigned long lineNum
        , const unsigned long columnNum
        , DOMNode* const errorNode
        , const XMLCh* const uri
        , const XMLSSize_t offset = -1
        , const XMLSSize_t utf16Offset = -1
    );

    /** Desctructor */
    virtual ~DOMLocatorImpl();

    //@}

    // DOMLocator interface
    virtual unsigned long getLineNumber() const;
    virtual unsigned long getColumnNumber() const;
    virtual XMLFilePos getByteOffset() const;
    virtual XMLFilePos getUtf16Offset() const;
    virtual DOMNode* getRelatedNode() const;
    virtual const XMLCh* getURI() const;

    // Setter functions
    void setLineNumber(const unsigned long lineNumber);
    void setColumnNumber(const unsigned long columnNumber);
    void setByteOffset(const XMLFilePos offset);
    void setUtf16Offset(const XMLFilePos offset);
    void setRelatedNode(DOMNode* const errorNode);
    void setURI(const XMLCh* const uri);


private :
    /* Unimplemented constructors and operators */

    /* Copy constructor */
    DOMLocatorImpl(const DOMLocatorImpl&);

    /* Assignment operator */
    DOMLocatorImpl& operator=(const DOMLocatorImpl&);

protected:
    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fLineNum
    //  fColumnNum
    //      Track line/column number of where the error occured
    //
    //  fByteOffset
    //      Track byte offset in the input source where the error
    //      occured
    //
    //  fUtf16Offset
    //      Track character offset in the input source where the error
    //      occured
    //
    //  fRelatedNode
    //      Current node where the error occured
    //
    //  fURI
    //      The uri where the error occured
    // -----------------------------------------------------------------------
    unsigned long   fLineNum;
    unsigned long   fColumnNum;
    XMLFilePos      fByteOffset;
    XMLFilePos      fUtf16Offset;
    DOMNode*        fRelatedNode;
    const XMLCh*    fURI;
};


// ---------------------------------------------------------------------------
//  DOMLocatorImpl: Getter methods
// ---------------------------------------------------------------------------
inline unsigned long DOMLocatorImpl::getLineNumber() const
{
    return fLineNum;
}

inline unsigned long DOMLocatorImpl::getColumnNumber() const
{
    return fColumnNum;
}

inline XMLFilePos DOMLocatorImpl::getByteOffset() const
{
    return fByteOffset;
}

inline XMLFilePos DOMLocatorImpl::getUtf16Offset() const
{
    return fUtf16Offset;
}

inline DOMNode* DOMLocatorImpl::getRelatedNode() const
{
    return fRelatedNode;
}

inline const XMLCh* DOMLocatorImpl::getURI() const
{
    return fURI;
}


// ---------------------------------------------------------------------------
//  DOMLocatorImpl: Setter methods
// ---------------------------------------------------------------------------
inline void DOMLocatorImpl::setLineNumber(const unsigned long lineNumber)
{
    fLineNum = lineNumber;
}

inline void DOMLocatorImpl::setColumnNumber(const unsigned long columnNumber)
{
    fColumnNum = columnNumber;
}

inline void DOMLocatorImpl::setByteOffset(const XMLFilePos offset)
{
    fByteOffset = offset;
}

inline void DOMLocatorImpl::setUtf16Offset(const XMLFilePos offset)
{
    fUtf16Offset = offset;
}

inline void DOMLocatorImpl::setRelatedNode(DOMNode* const errorNode)
{
    fRelatedNode = errorNode;
}

inline void DOMLocatorImpl::setURI(const XMLCh* const uri)
{
    fURI = uri;
}

XERCES_CPP_NAMESPACE_END

#endif
