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
 * $Log$
 * Revision 1.4  2001/08/21 18:42:54  peiyongz
 * Bugzilla# 2816: cleanUp() declared with external linkage and called
 *                          before defined as inline
 *
 * Revision 1.2  2001/07/24 21:23:40  tng
 * Schema: Use DatatypeValidator for ID/IDREF/ENTITY/ENTITIES/NOTATION.
 *
 * Revision 1.1  2001/07/13 14:10:40  peiyongz
 * UnionDTV
 *
 */

#if !defined(UNION_DATATYPEVALIDATOR_HPP)
#define UNION_DATATYPEVALIDATOR_HPP

#include <validators/datatype/DatatypeValidator.hpp>
#include <validators/schema/SchemaSymbols.hpp>

class VALIDATORS_EXPORT UnionDatatypeValidator : public DatatypeValidator
{
public:

    // -----------------------------------------------------------------------
    //  Public ctor/dtor
    // -----------------------------------------------------------------------
	/** @name Constructor. */
    //@{

    UnionDatatypeValidator();

    //
    // constructor for native Union datatype validator
    // <simpleType name="nativeUnion">
    //      <union   memberTypes="member1 member2 ...">
    // </simpleType>
    //
    UnionDatatypeValidator(RefVectorOf<DatatypeValidator>* const memberTypeValidators
                         , const int                             finalSet);

    //
    // constructor for derived Union datatype validator
    // <simpleType name="derivedUnion">
    //      <restriction base="nativeUnion">
    //          <pattern     value="patter_value"/>
    //          <enumeartion value="enum_value"/>
    //      </restriction>
    // </simpleType>
    //
    UnionDatatypeValidator(DatatypeValidator*            const baseValidator
                         , RefHashTableOf<KVStringPair>* const facets
                         , RefVectorOf<XMLCh>*           const enums
                         , const int                           finalSet);

    virtual ~UnionDatatypeValidator();

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
    DatatypeValidator* newInstance(DatatypeValidator*            const baseValidator
                                 , RefHashTableOf<KVStringPair>* const facets
                                 , RefVectorOf<XMLCh>*           const enums
                                 , const int                           finalSet);

    RefVectorOf<DatatypeValidator>* getMemberTypeValidators() const;

private:

    void checkContent(const XMLCh* const content, bool asBase);

    void init(DatatypeValidator*            const baseValidator
            , RefHashTableOf<KVStringPair>* const facets
            , RefVectorOf<XMLCh>*           const enums);

    void cleanUp();

    RefVectorOf<XMLCh>*  getEnumeration() const;

    void                 setEnumeration(RefVectorOf<XMLCh>*, bool);


    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fEnumeration
    //      we own it.
    //
    //  fMemberTypeValidators
    //      we own it.
    //
    // -----------------------------------------------------------------------
     bool                 fEnumerationInherited;
     RefVectorOf<XMLCh>*  fEnumeration;
     RefVectorOf<DatatypeValidator>*  fMemberTypeValidators;
};

inline DatatypeValidator* UnionDatatypeValidator::newInstance(
                                      DatatypeValidator*            const baseValidator
                                    , RefHashTableOf<KVStringPair>* const facets
                                    , RefVectorOf<XMLCh>*           const enums
                                    , const int                           finalSet)
{
    return (DatatypeValidator*) new UnionDatatypeValidator(baseValidator, facets, enums, finalSet);
}

inline void UnionDatatypeValidator::validate( const XMLCh* const content)
{
    checkContent(content, false);
}

inline void UnionDatatypeValidator::cleanUp()
{
    //~RefVectorOf will delete all adopted elements
    if (fEnumeration && !fEnumerationInherited)
        delete fEnumeration;

    if (fMemberTypeValidators)
        delete fMemberTypeValidators;
}

inline RefVectorOf<XMLCh>* UnionDatatypeValidator:: getEnumeration() const
{
    return fEnumeration;
}

inline void UnionDatatypeValidator::setEnumeration(RefVectorOf<XMLCh>* enums
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

//
// get the native UnionDTV's fMemberTypeValidators
//
inline
RefVectorOf<DatatypeValidator>* UnionDatatypeValidator::getMemberTypeValidators() const
{
    UnionDatatypeValidator* thisdv = (UnionDatatypeValidator*)this; // cast away constness

    while (thisdv->getBaseValidator())
        thisdv = (UnionDatatypeValidator*) thisdv->getBaseValidator();

    return thisdv->fMemberTypeValidators;
}
/**
  * End of file UnionDatatypeValidator.hpp
  */
#endif
