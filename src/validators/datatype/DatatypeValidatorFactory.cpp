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
 * $Log$
 * Revision 1.4  2001/05/11 13:27:28  tng
 * Copyright update.
 *
 * Revision 1.3  2001/05/09 18:43:40  tng
 * Add StringDatatypeValidator and BooleanDatatypeValidator.  By Pei Yong Zhang.
 *
 * Revision 1.2  2001/05/03 19:17:47  knoaman
 * TraverseSchema Part II.
 *
 * Revision 1.1  2001/03/21 21:39:14  knoaman
 * Schema symbols and Datatype validator part I
 *
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <validators/datatype/DatatypeValidatorFactory.hpp>
#include <validators/schema/SchemaSymbols.hpp>
#include <util/XMLUniDefs.hpp>
#include <util/Janitor.hpp>
#include <validators/datatype/StringDatatypeValidator.hpp>
#include <validators/datatype/BooleanDatatypeValidator.hpp>


// ---------------------------------------------------------------------------
//  DatatypeValidatorFactory: Local const data
// ---------------------------------------------------------------------------
const XMLCh fgTokPattern[] =
{
    chBackSlash, chLatin_c, chPlus, chNull
};

//"([a-zA-Z]{2}|[iI]-[a-zA-Z]+|[xX]-[a-zA-Z]+)(-[a-zA-Z]+)*"
const XMLCh fgLangPattern[] =
{
    chOpenParen, chOpenSquare, chLatin_a, chDash, chLatin_z, chLatin_A, chDash,
    chLatin_Z, chCloseSquare, chOpenCurly, chDigit_2, chCloseCurly, chPipe,
    chOpenSquare, chLatin_i, chLatin_I, chCloseSquare, chDash, chOpenSquare,
    chLatin_a, chDash, chLatin_z, chLatin_A, chDash, chLatin_Z, chCloseSquare,
    chPlus, chPipe, chOpenSquare, chLatin_x, chLatin_X, chCloseSquare, chDash,
    chOpenSquare, chLatin_a, chDash, chLatin_z, chLatin_A, chDash, chLatin_Z,
	chCloseSquare, chPlus, chCloseParen, chOpenParen, chDash, chOpenSquare,
    chLatin_a, chDash, chLatin_z, chLatin_A, chDash, chLatin_Z, chCloseSquare,
    chPlus, chCloseParen, chAsterisk, chNull
};

//"\\i\\c*"
const XMLCh fgNamePattern[] =
{
    chBackSlash, chLatin_i, chBackSlash, chLatin_c, chAsterisk, chNull
};

//"[\\i-[:]][\\c-[:]]*"
const XMLCh fgNCNamePattern[] =
{
    chOpenSquare, chBackSlash, chLatin_i, chDash, chOpenSquare, chColon, chCloseSquare,
    chCloseSquare, chOpenSquare, chBackSlash, chLatin_c, chDash, chOpenSquare,
    chColon, chCloseSquare, chCloseSquare, chAsterisk, chNull
};

const XMLCh fgValueZero[] =
{
    chDigit_0, chNull
};

const XMLCh fgNegOne[] =
{
    chDash, chDigit_1, chNull
};

const XMLCh fgValueOne[] =
{
    chDigit_1, chNull
};

//"9223372036854775807"
const XMLCh fgLongMaxInc[] =
{
    chDigit_9, chDigit_2, chDigit_2, chDigit_3, chDigit_3, chDigit_7, chDigit_2,
    chDigit_0, chDigit_3, chDigit_6, chDigit_8, chDigit_5, chDigit_4, chDigit_7,
    chDigit_7, chDigit_5, chDigit_8, chDigit_0, chDigit_7, chNull
};

//"-9223372036854775808"
const XMLCh fgLongMinInc[] =
{
    chDash, chDigit_9, chDigit_2, chDigit_2, chDigit_3, chDigit_3, chDigit_7,
    chDigit_2, chDigit_0, chDigit_3, chDigit_6, chDigit_8, chDigit_5, chDigit_4,
    chDigit_7, chDigit_7, chDigit_5, chDigit_8, chDigit_0, chDigit_8,  chNull
};

const XMLCh fgIntMaxInc[] =
{
    chDigit_2, chDigit_1, chDigit_4, chDigit_7, chDigit_4, chDigit_8,
    chDigit_3, chDigit_6, chDigit_4, chDigit_7, chNull
};

const XMLCh fgIntMinInc[] =
{
    chDash, chDigit_2, chDigit_1, chDigit_4, chDigit_7, chDigit_4,
    chDigit_8, chDigit_3, chDigit_6, chDigit_4, chDigit_8, chNull
};

const XMLCh fgShortMaxInc[] =
{
    chDigit_3, chDigit_2, chDigit_7, chDigit_6, chDigit_7, chNull
};

const XMLCh fgShortMinInc[] =
{
    chDash, chDigit_3, chDigit_2, chDigit_7, chDigit_6, chDigit_8, chNull
};

const XMLCh fgByteMaxInc[] =
{
    chDigit_1, chDigit_2, chDigit_7, chNull
};

const XMLCh fgByteMinInc[] =
{
    chDash, chDigit_1, chDigit_2, chDigit_8, chNull
};

const XMLCh fgULongMaxInc[] =
{
    chDigit_1, chDigit_8, chDigit_4, chDigit_4, chDigit_6, chDigit_7, chDigit_4,
    chDigit_4, chDigit_0, chDigit_7, chDigit_3, chDigit_7, chDigit_0, chDigit_9,
    chDigit_5, chDigit_5, chDigit_1, chDigit_6, chDigit_1, chDigit_5, chNull
};

const XMLCh fgUIntMaxInc[] =
{
    chDigit_4, chDigit_2, chDigit_9, chDigit_4, chDigit_9, chDigit_6,
    chDigit_7, chDigit_2, chDigit_9, chDigit_5, chNull
};

const XMLCh fgUShortMaxInc[] =
{
    chDigit_6, chDigit_5, chDigit_5, chDigit_3, chDigit_5, chNull
};

const XMLCh fgUByteMaxInc[] =
{
    chDigit_2, chDigit_5, chDigit_5, chNull
};

const XMLCh fgP0Y[] =
{
    chLatin_P, chDigit_0, chLatin_Y, chNull
};

const XMLCh fgP1Y[] =
{
    chLatin_P, chDigit_1, chLatin_Y, chNull
};

const XMLCh fgP100Y[] =
{
    chLatin_P, chDigit_1, chDigit_0, chDigit_0, chLatin_Y, chNull
};

const XMLCh fgPT24H[] =
{
    chLatin_P, chLatin_T, chDigit_2, chDigit_4, chLatin_H, chNull
};

const XMLCh fgP1M[] =
{
    chLatin_P, chDigit_1, chLatin_M, chNull
};

// ---------------------------------------------------------------------------
//  DatatypeValidatorFactory: Constructors and Destructor
// ---------------------------------------------------------------------------
DatatypeValidatorFactory::DatatypeValidatorFactory()
    : fRegistryExpanded(false)
      , fRegistry(0)
{

}

DatatypeValidatorFactory::~DatatypeValidatorFactory()
{
	cleanUp();
}


// ---------------------------------------------------------------------------
//  DatatypeValidatorFactory: Reset methods
// ---------------------------------------------------------------------------
void DatatypeValidatorFactory::resetRegistry() {

    if (fRegistry != 0) {

        fRegistry->removeAll();
		fRegistryExpanded = false;
    }
}


// ---------------------------------------------------------------------------
//  DatatypeValidatorFactory: Registry initialization methods
// ---------------------------------------------------------------------------
void DatatypeValidatorFactory::initializeDTDRegistry()
{
	if (fRegistryExpanded)
		return;

    if (fRegistry == 0) {
        fRegistry = new RefHashTableOf<DatatypeValidator>(109);
    }

	try {

        fRegistry->put((void*) SchemaSymbols::fgDT_STRING,
                       new StringDatatypeValidator());

/*
        fRegistry->put((void*) XMLUni::fgIDString,
                       new IDDatatypeValidator());
        fRegistry->put((void*) XMLUni::fgIDRefString,
                       new IDREFDatatypeValidator());
        fRegistry->put((void*) XMLUni::fgEntityString,
                       new ENTITYDatatypeValidator());
        fRegistry->put((void*) XMLUni::fgNotationString,
                       new NOTATIONDatatypeValidator());
	    */

        // Create 'IDREFS' datatype validator
	    createDatatypeValidator(XMLUni::fgIDRefsString,
                    getDatatypeValidator(XMLUni::fgIDRefString), 0, true);

        // Create 'ENTITIES' datatype validator
        createDatatypeValidator(XMLUni::fgEntitiesString,
		            getDatatypeValidator(XMLUni::fgEntityString), 0, true);

        RefHashTableOf<KVStringPair>* facets = new RefHashTableOf<KVStringPair>(3);
		Janitor<KVStringPairHashTable> janFacets(facets);

        facets->put((void*) SchemaSymbols::fgELT_PATTERN ,
                    new KVStringPair(SchemaSymbols::fgELT_PATTERN,fgTokPattern));
        facets->put((void*) SchemaSymbols::fgELT_WHITESPACE,
                    new KVStringPair(SchemaSymbols::fgELT_WHITESPACE, SchemaSymbols::fgWS_COLLAPSE));

        // Create 'NMTOKEN' datatype validator
        if (createDatatypeValidator(XMLUni::fgNmTokenString,
                    getDatatypeValidator(SchemaSymbols::fgDT_STRING),
                    facets, false) != 0) {
            janFacets.orphan();
        }

        // Create 'NMTOKENS' datatype validator
        createDatatypeValidator(XMLUni::fgNmTokensString,
		            getDatatypeValidator(XMLUni::fgNmTokenString), 0, true);
    }
	catch(...) {
        throw;
    }
}


void DatatypeValidatorFactory::expandRegistryToFullSchemaSet()
{
	if (fRegistryExpanded)
		return;

    if (fRegistry == 0) {
        fRegistry = new RefHashTableOf<DatatypeValidator>(109);
    }

    // Initialize common Schema/DTD Datatype validator set if not initialized
    if (fRegistry->get(SchemaSymbols::fgDT_STRING) == 0) {
        initializeDTDRegistry();
    }

	try {
        fRegistry->put((void*) SchemaSymbols::fgDT_BOOLEAN,
                       new BooleanDatatypeValidator());
/*
        fRegistry->put((void*) SchemaSymbols::fgDT_FLOAT,
                       new FloatDatatypeValidator());
        fRegistry->put((void*) SchemaSymbols::fgDT_DOUBLE,
                       new DoubleDatatypeValidator());
        fRegistry->put((void*) SchemaSymbols::fgDT_DECIMAL,
                       new DecimalDatatypeValidator());
        fRegistry->put((void*) SchemaSymbols::fgDT_HEXBINARY,
                       new HexBinaryDatatypeValidator());
        fRegistry->put((void*) SchemaSymbols::fgDT_BASE64BINARY,
                       new Base64BinaryDatatypeValidator());
        fRegistry->put((void*) SchemaSymbols::fgDT_ANYURI,
                       new AnyURIDatatypeValidator());
        fRegistry->put((void*) SchemaSymbols::fgDT_QNAME,
                       new QNameDatatypeValidator());
        fRegistry->put((void*) SchemaSymbols::fgDT_DURATION,
                       new DurationDatatypeValidator());
        fRegistry->put((void*) SchemaSymbols::fgDT_DAY,
                       new DayDatatypeValidator());
        fRegistry->put((void*) SchemaSymbols::fgDT_TIME,
                       new TimeDatatypeValidator());
        */

        // REVISIT
        // We are creating a lot of Hashtables for the facets of the different
        // validators. It's better to have some kind of a memory pool and ask
        // the pool to give us a new instance of the hashtable.


        // Since the datatype validator is responsible for deleting the
        // facets hashtable, we have the potential of a memory leak if
        // an exception is thrown or we failed to create the datatype
        // validator.
		// We will use a janitor object to take care of any potential memory
        // leaks. After each succesful datatype creation, we will orphan the
        // janitor, and then reset it with the new hashtable.

        // Create 'normalizedString' datatype validator
        RefHashTableOf<KVStringPair>* facets = new RefHashTableOf<KVStringPair>(3);
		Janitor<KVStringPairHashTable> janFacets(facets);

        facets->put((void*) SchemaSymbols::fgELT_WHITESPACE,
                    new KVStringPair(SchemaSymbols::fgELT_WHITESPACE, SchemaSymbols::fgWS_REPLACE));

        if (createDatatypeValidator(SchemaSymbols::fgDT_NORMALIZEDSTRING,
                    getDatatypeValidator(SchemaSymbols::fgDT_STRING),
				    facets, false) != 0) {
            janFacets.orphan();
        }

		// Create 'token' datatype validator
        facets = new RefHashTableOf<KVStringPair>(3);
        janFacets.reset(facets);

        facets->put((void*) SchemaSymbols::fgELT_WHITESPACE,
                    new KVStringPair(SchemaSymbols::fgELT_WHITESPACE, SchemaSymbols::fgWS_COLLAPSE));

        if (createDatatypeValidator(SchemaSymbols::fgDT_TOKEN,
                      getDatatypeValidator(SchemaSymbols::fgDT_NORMALIZEDSTRING),
                      facets, false) != 0) {
            janFacets.orphan();
        }

        // Create 'language' datatype validator
        facets = new RefHashTableOf<KVStringPair>(3);
        janFacets.reset(facets);

        facets->put((void*) SchemaSymbols::fgELT_PATTERN,
                    new KVStringPair(SchemaSymbols::fgELT_PATTERN, fgLangPattern));

        if (createDatatypeValidator(SchemaSymbols::fgDT_LANGUAGE,
                      getDatatypeValidator(SchemaSymbols::fgDT_TOKEN),
                      facets, false ) != 0) {
            janFacets.orphan();
        }

        // Create 'Name' datatype validator
        facets = new RefHashTableOf<KVStringPair>(3);
        janFacets.reset(facets);

        facets->put((void*) SchemaSymbols::fgELT_PATTERN,
                    new KVStringPair(SchemaSymbols::fgELT_PATTERN, fgNamePattern));

        if (createDatatypeValidator(SchemaSymbols::fgDT_NAME,
                      getDatatypeValidator(SchemaSymbols::fgDT_TOKEN),
                      facets, false ) != 0) {
            janFacets.orphan();
        }

        // Create a 'NCName' datatype validator
        facets = new RefHashTableOf<KVStringPair>(3);
        janFacets.reset(facets);

        facets->put((void*) SchemaSymbols::fgELT_PATTERN,
                    new KVStringPair(SchemaSymbols::fgELT_PATTERN, fgNCNamePattern));

        if (createDatatypeValidator(SchemaSymbols::fgDT_NCNAME,
                      getDatatypeValidator(SchemaSymbols::fgDT_TOKEN),
                      facets, false) != 0) {
            janFacets.orphan();
        }

        // Create 'integer' datatype validator
        facets = new RefHashTableOf<KVStringPair>(3);
        janFacets.reset(facets);

        facets->put((void*) SchemaSymbols::fgELT_FRACTIONDIGITS,
                    new KVStringPair(SchemaSymbols::fgELT_FRACTIONDIGITS, fgValueZero));

        if (createDatatypeValidator(SchemaSymbols::fgDT_INTEGER,
                      getDatatypeValidator(SchemaSymbols::fgDT_DECIMAL),
                      facets, false) != 0) {
            janFacets.orphan();
        }

        // Create 'nonPositiveInteger' datatype validator
        facets = new RefHashTableOf<KVStringPair>(3);
        janFacets.reset(facets);

        facets->put((void*) SchemaSymbols::fgELT_MAXINCLUSIVE,
                    new KVStringPair(SchemaSymbols::fgELT_MAXINCLUSIVE, fgValueZero));

        if (createDatatypeValidator(SchemaSymbols::fgDT_NONPOSITIVEINTEGER,
                      getDatatypeValidator(SchemaSymbols::fgDT_INTEGER),
                      facets, false) != 0) {
            janFacets.orphan();
        }

        // Create 'negativeInteger' datatype validator
        facets = new RefHashTableOf<KVStringPair>(3);
        janFacets.reset(facets);

        facets->put((void*) SchemaSymbols::fgELT_MAXINCLUSIVE,
			        new KVStringPair(SchemaSymbols::fgELT_MAXINCLUSIVE,fgNegOne));

        if (createDatatypeValidator(SchemaSymbols::fgDT_NEGATIVEINTEGER,
                      getDatatypeValidator(SchemaSymbols::fgDT_NONPOSITIVEINTEGER),
                      facets, false) != 0) {
            janFacets.orphan();
        }

        // Create 'long' datatype validator
        facets = new RefHashTableOf<KVStringPair>(3);
        janFacets.reset(facets);

        facets->put((void*) SchemaSymbols::fgELT_MAXINCLUSIVE,
                    new KVStringPair(SchemaSymbols::fgELT_MAXINCLUSIVE,fgLongMaxInc));
        facets->put((void*) SchemaSymbols::fgELT_MININCLUSIVE,
                    new KVStringPair(SchemaSymbols::fgELT_MININCLUSIVE,fgLongMinInc));

        if (createDatatypeValidator(SchemaSymbols::fgDT_LONG,
                      getDatatypeValidator(SchemaSymbols::fgDT_INTEGER),
                      facets, false) != 0) {
            janFacets.orphan();
        }

        // Create 'int' datatype validator
        facets = new RefHashTableOf<KVStringPair>(3);
        janFacets.reset(facets);

        facets->put((void*) SchemaSymbols::fgELT_MAXINCLUSIVE,
                    new KVStringPair(SchemaSymbols::fgELT_MAXINCLUSIVE, fgIntMaxInc));
        facets->put((void*) SchemaSymbols::fgELT_MININCLUSIVE,
                    new KVStringPair(SchemaSymbols::fgELT_MININCLUSIVE, fgIntMinInc));

        if (createDatatypeValidator(SchemaSymbols::fgDT_INT,
                      getDatatypeValidator(SchemaSymbols::fgDT_LONG),
                      facets,false) != 0) {
            janFacets.orphan();
        }

        // Create 'short' datatype validator
        facets = new RefHashTableOf<KVStringPair>(3);
        janFacets.reset(facets);

        facets->put((void*) SchemaSymbols::fgELT_MAXINCLUSIVE,
                    new KVStringPair(SchemaSymbols::fgELT_MAXINCLUSIVE, fgShortMaxInc));
        facets->put((void*) SchemaSymbols::fgELT_MININCLUSIVE,
                    new KVStringPair(SchemaSymbols::fgELT_MININCLUSIVE, fgShortMinInc));

        if (createDatatypeValidator(SchemaSymbols::fgDT_SHORT,
                      getDatatypeValidator(SchemaSymbols::fgDT_INT),
                      facets, false) != 0) {
            janFacets.orphan();
        }

        // Create 'byte' datatype validator
        facets = new RefHashTableOf<KVStringPair>(3);
        janFacets.reset(facets);

        facets->put((void*) SchemaSymbols::fgELT_MAXINCLUSIVE,
                    new KVStringPair(SchemaSymbols::fgELT_MAXINCLUSIVE, fgByteMaxInc));
        facets->put((void*) SchemaSymbols::fgELT_MININCLUSIVE,
                    new KVStringPair(SchemaSymbols::fgELT_MININCLUSIVE, fgByteMinInc));

        if (createDatatypeValidator(SchemaSymbols::fgDT_BYTE,
                      getDatatypeValidator(SchemaSymbols::fgDT_SHORT),
                      facets, false) != 0) {
            janFacets.orphan();
        }

        // Create 'nonNegativeInteger' datatype validator
        facets = new RefHashTableOf<KVStringPair>(3);
        janFacets.reset(facets);

        facets->put((void*) SchemaSymbols::fgELT_MININCLUSIVE,
                    new KVStringPair(SchemaSymbols::fgELT_MININCLUSIVE, fgValueZero));

        if (createDatatypeValidator(SchemaSymbols::fgDT_NONNEGATIVEINTEGER,
                      getDatatypeValidator(SchemaSymbols::fgDT_INTEGER),
                      facets, false) != 0) {
            janFacets.orphan();
        }

        // Create 'unsignedLong' datatype validator
        facets = new RefHashTableOf<KVStringPair>(3);
        janFacets.reset(facets);

        facets->put((void*) SchemaSymbols::fgELT_MAXINCLUSIVE,
                    new KVStringPair(SchemaSymbols::fgELT_MAXINCLUSIVE, fgULongMaxInc));

        if (createDatatypeValidator(SchemaSymbols::fgDT_ULONG,
                      getDatatypeValidator(SchemaSymbols::fgDT_NONNEGATIVEINTEGER),
                      facets, false) != 0) {
            janFacets.orphan();
        }

        // Create 'unsignedInt' datatype validator
        facets = new RefHashTableOf<KVStringPair>(3);
        janFacets.reset(facets);

        facets->put((void*) SchemaSymbols::fgELT_MAXINCLUSIVE,
                    new KVStringPair(SchemaSymbols::fgELT_MAXINCLUSIVE, fgUIntMaxInc));

        if (createDatatypeValidator(SchemaSymbols::fgDT_UINT,
                      getDatatypeValidator(SchemaSymbols::fgDT_ULONG),
                      facets, false) != 0) {
            janFacets.orphan();
        }

        // Create 'unsignedShort' datatypeValidator
        facets = new RefHashTableOf<KVStringPair>(3);
        janFacets.reset(facets);

        facets->put((void*) SchemaSymbols::fgELT_MAXINCLUSIVE,
                    new KVStringPair(SchemaSymbols::fgELT_MAXINCLUSIVE, fgUShortMaxInc));

        if (createDatatypeValidator(SchemaSymbols::fgDT_USHORT,
                      getDatatypeValidator(SchemaSymbols::fgDT_UINT),
                      facets, false) != 0) {
            janFacets.orphan();
        }

        // Create 'unsignedByte' datatype validator
        facets = new RefHashTableOf<KVStringPair>(3);
        janFacets.reset(facets);

        facets->put((void*) SchemaSymbols::fgELT_MAXINCLUSIVE,
                    new KVStringPair(SchemaSymbols::fgELT_MAXINCLUSIVE, fgUByteMaxInc));

        if (createDatatypeValidator(SchemaSymbols::fgDT_UBYTE,
                      getDatatypeValidator(SchemaSymbols::fgDT_USHORT),
                      facets, false) != 0) {
            janFacets.orphan();
        }

        // Create 'positiveInteger' datatype validator
        facets = new RefHashTableOf<KVStringPair>(3);
        janFacets.reset(facets);

        facets->put((void*) SchemaSymbols::fgELT_MININCLUSIVE,
                    new KVStringPair(SchemaSymbols::fgELT_MININCLUSIVE, fgValueOne));

        if (createDatatypeValidator(SchemaSymbols::fgDT_POSITIVEINTEGER,
                      getDatatypeValidator(SchemaSymbols::fgDT_NONNEGATIVEINTEGER),
                      facets, false) != 0) {
            janFacets.orphan();
        }

        // REVISIT - Add the remaining datatype validators
        // Create 'dateTime' datatype validator

        // Create 'date' datatype validator

        // Create 'gMonthDay' datatype validator

        // Create 'gYearMonth' datatype validator

        // Create 'gYear' datatype validator

        // Create 'gMonth' datatype validator


        fRegistryExpanded = true;
    }
    catch(...){
        throw;
    }
}

// ---------------------------------------------------------------------------
//  DatatypeValidatorFactory: factory methods
// ---------------------------------------------------------------------------
DatatypeValidator*
DatatypeValidatorFactory::createDatatypeValidator(const XMLCh* const typeName,
		                                          DatatypeValidator* const baseValidator,
                                                  RefHashTableOf<KVStringPair>* const facets,
                                                  const bool derivedByList,
                                                  const int finalSet)
{
	if (baseValidator == 0) {
        return 0;
    }

	DatatypeValidator* datatypeValidator = 0;

    if (derivedByList) {
        //datatypeValidator = new ListDatatypeValidator(baseValidator, facets);
		//datatypeValidator->setDerivedByList(derivedByList);
    }
    else {

        if (baseValidator->getType() != DatatypeValidator::String) {

            KVStringPair* value = facets->get(SchemaSymbols::fgELT_WHITESPACE);

            if (value != 0) {
                facets->removeKey(SchemaSymbols::fgELT_WHITESPACE);
            }
        }

        datatypeValidator = baseValidator->newInstance(baseValidator, facets, finalSet);
    }

    if (datatypeValidator != 0) {
        fRegistry->put((void *)typeName, datatypeValidator);
    }

    return datatypeValidator;
}


DatatypeValidator*
DatatypeValidatorFactory::createDatatypeValidator(const XMLCh* const typeName,
                                                  RefVectorOf<DatatypeValidator>* const validators,
                                                  const int finalSet)
{
    if (validators == 0)
        return 0;

    DatatypeValidator* datatypeValidator = 0;

    //datatypeValidator = new UnionDatatypeValidator(validators, finalSet);

    if (datatypeValidator != 0) {
        fRegistry->put((void*) typeName, datatypeValidator);
    }

    return datatypeValidator;
}

/**
  * End of file DatatypeValidatorFactory.cpp
  */

