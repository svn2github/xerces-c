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

#if !defined(BASE64BINARY_DATATYPEVALIDATOR_HPP)
#define BASE64BINARY_DATATYPEVALIDATOR_HPP

#include <validators/datatype/DatatypeValidator.hpp>
#include <validators/schema/SchemaSymbols.hpp>
#include <util/RefVectorOf.hpp>

class VALIDATORS_EXPORT Base64BinaryDatatypeValidator : public DatatypeValidator
{
public:

    // -----------------------------------------------------------------------
    //  Public ctor/dtor
    // -----------------------------------------------------------------------
	/** @name Constructor. */
    //@{

    Base64BinaryDatatypeValidator();

    Base64BinaryDatatypeValidator(DatatypeValidator*            const baseValidator
                                , RefHashTableOf<KVStringPair>* const facets    
                                , RefVectorOf<XMLCh>*           const enums                                
                                , const int                           finalSet);

    virtual ~Base64BinaryDatatypeValidator();

	//@}

    // -----------------------------------------------------------------------
    // Getter methods
    // -----------------------------------------------------------------------
    /** @name Getter Functions */
    //@{


    //@}

    // -----------------------------------------------------------------------
    // Validation methods
    // -----------------------------------------------------------------------
    /** @name Validation Function */
    //@{

    /**
     * validate that a string matches the boolean datatype
     * @param content A string containing the content to be validated
     *
     * @exception throws InvalidDatatypeException if the content is
     * is not valid.
     */

	void validate(const XMLCh* const content);

    //@}

    // -----------------------------------------------------------------------
    // Compare methods
    // -----------------------------------------------------------------------
    /** @name Compare Function */
    //@{

    /**
     * Compare two boolean data types
     * 
     * @param content1
     * @param content2
     * @return 
     */
    int compare(const XMLCh* const, const XMLCh* const);

    //@}

    /**
      * Returns an instance of the base datatype validator class
	  * Used by the DatatypeValidatorFactory.
      */
    DatatypeValidator* newInstance(RefHashTableOf<KVStringPair>* const facets
                                 , RefVectorOf<XMLCh>*           const enums
                                 , const int                           finalSet);


private:

    void checkContent( const XMLCh* const content, bool asBase);

    void init(DatatypeValidator*            const baseValidator
            , RefHashTableOf<KVStringPair>* const facets
            , RefVectorOf<XMLCh>*           const enums);

    void cleanUp();

// -----------------------------------------------------------------------
// Getter methods
// -----------------------------------------------------------------------

    unsigned int         getLength() const;

    unsigned int         getMaxLength() const;

    unsigned int         getMinLength() const;

    RefVectorOf<XMLCh>*  getEnumeration() const;

// -----------------------------------------------------------------------
// Setter methods
// -----------------------------------------------------------------------

    void                 setLength(unsigned int);

    void                 setMaxLength(unsigned int);

    void                 setMinLength(unsigned int);

    void                 setEnumeration(RefVectorOf<XMLCh>*, bool);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  
    //      .
	//		
    // -----------------------------------------------------------------------    
     int                  fLength;
     int                  fMaxLength;  
     int                  fMinLength;  
     bool                 fEnumerationInherited;
     RefVectorOf<XMLCh>*  fEnumeration;

};

// -----------------------------------------------------------------------
// Getter methods
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// Compare methods
// -----------------------------------------------------------------------
inline int Base64BinaryDatatypeValidator::compare(const XMLCh* const lValue
                                                , const XMLCh* const rValue)
{
    return 0;
}

inline DatatypeValidator* Base64BinaryDatatypeValidator::newInstance(
                                      RefHashTableOf<KVStringPair>* const facets
                                    , RefVectorOf<XMLCh>*           const enums
                                    , const int                           finalSet)
{
    return (DatatypeValidator*) new Base64BinaryDatatypeValidator(this, facets, enums, finalSet);
}

inline void Base64BinaryDatatypeValidator::validate( const XMLCh* const content)
{
    checkContent(content, false);
}

inline void Base64BinaryDatatypeValidator::cleanUp()
{
    //~RefVectorOf will delete all adopted elements
    if (fEnumeration && !fEnumerationInherited)
        delete fEnumeration;
}

// -----------------------------------------------------------------------
// Getter methods
// -----------------------------------------------------------------------

inline unsigned int Base64BinaryDatatypeValidator::getLength() const
{
    return fLength;
}

inline unsigned int Base64BinaryDatatypeValidator::getMaxLength() const
{
    return fMaxLength;
}

inline unsigned int Base64BinaryDatatypeValidator::getMinLength() const
{
    return fMinLength;
}

inline RefVectorOf<XMLCh>* Base64BinaryDatatypeValidator:: getEnumeration() const
{
    return fEnumeration;
}

// -----------------------------------------------------------------------
// Setter methods
// -----------------------------------------------------------------------

inline void Base64BinaryDatatypeValidator::setLength(unsigned int newLength)
{
    fLength = newLength;
}

inline void Base64BinaryDatatypeValidator::setMaxLength(unsigned int newMaxLength)
{
    fMaxLength = newMaxLength;
}

inline void Base64BinaryDatatypeValidator::setMinLength(unsigned int newMinLength)
{
    fMinLength = newMinLength;
}

inline void Base64BinaryDatatypeValidator::setEnumeration(RefVectorOf<XMLCh>* enums
                                                        , bool                inherited)
{
    if (enums)
    {
        if (fEnumeration && !fEnumerationInherited)
            delete fEnumeration;

        fEnumeration = enums;
        fEnumerationInherited = inherited;
        setFacetsDefined(DatatypeValidator::FACET_ENUMERATION);
    }
}

/**
  * End of file Base64BinaryDatatypeValidator.hpp
  */
#endif
