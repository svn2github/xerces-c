/*
 * Copyright 1999-2001,2004 The Apache Software Foundation.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
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
 * $Log$
 * Revision 1.6  2004/09/08 13:56:22  peiyongz
 * Apache License Version 2.0
 *
 * Revision 1.5  2003/09/25 22:23:25  peiyongz
 * Implementation of Serialization/Deserialization
 *
 * Revision 1.4  2003/05/16 06:01:52  knoaman
 * Partial implementation of the configurable memory manager.
 *
 * Revision 1.3  2003/05/15 19:04:35  knoaman
 * Partial implementation of the configurable memory manager.
 *
 * Revision 1.2  2002/11/04 15:22:04  tng
 * C++ Namespace Support.
 *
 * Revision 1.1.1.1  2002/02/01 22:22:10  peiyongz
 * sane_include
 *
 * Revision 1.5  2001/05/11 13:26:27  tng
 * Copyright update.
 *
 * Revision 1.4  2001/01/15 21:26:33  tng
 * Performance Patches by David Bertoni.
 *
 * Details: (see xerces-c-dev mailing Jan 14)
 * XMLRecognizer.cpp: the internal encoding string XMLUni::fgXMLChEncodingString
 * was going through this function numerous times.  As a result, the top hot-spot
 * for the parse was _wcsicmp().  The real problem is that the Microsofts wide string
 * functions are unbelievably slow.  For things like encodings, it might be
 * better to use a special comparison function that only considers a-z and
 * A-Z as characters with case.  This works since the character set for
 * encodings is limit to printable ASCII characters.
 *
 *  XMLScanner2.cpp: This also has some case-sensitive vs. insensitive compares.
 * They are also much faster.  The other tweak is to only make a copy of an attribute
 * string if it needs to be split.  And then, the strategy is to try to use a
 * stack-based buffer, rather than a dynamically-allocated one.
 *
 * SAX2XMLReaderImpl.cpp: Again, more case-sensitive vs. insensitive comparisons.
 *
 * KVStringPair.cpp & hpp: By storing the size of the allocation, the storage can
 * likely be re-used many times, cutting down on dynamic memory allocations.
 *
 * XMLString.hpp: a more efficient implementation of stringLen().
 *
 * DTDValidator.cpp: another case of using a stack-based buffer when possible
 *
 * These patches made a big difference in parse time in some of our test
 * files, especially the ones are very attribute-heavy.
 *
 * Revision 1.3  2000/03/02 19:54:40  roddey
 * This checkin includes many changes done while waiting for the
 * 1.1.0 code to be finished. I can't list them all here, but a list is
 * available elsewhere.
 *
 * Revision 1.2  2000/02/06 07:48:02  rahulj
 * Year 2K copyright swat.
 *
 * Revision 1.1.1.1  1999/11/09 01:04:33  twl
 * Initial checkin
 *
 * Revision 1.2  1999/11/08 20:45:08  rahul
 * Swat for adding in Product name and CVS comment log variable.
 *
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/KVStringPair.hpp>
#include <xercesc/util/XMLString.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  KVStringPair: Constructors and Destructor
// ---------------------------------------------------------------------------
KVStringPair::KVStringPair(MemoryManager* const manager) :

    fMemoryManager(manager)
    , fKey(0)
    , fKeyAllocSize(0)
    , fValue(0)
    , fValueAllocSize(0)
{
}

KVStringPair::KVStringPair(const XMLCh* const key,
                           const XMLCh* const value,
                           MemoryManager* const manager) :

    fMemoryManager(manager)
    , fKey(0)
    , fKeyAllocSize(0)
    , fValue(0)
    , fValueAllocSize(0)
{
   set(key, value);
}

KVStringPair::KVStringPair(const KVStringPair& toCopy) :

    fMemoryManager(toCopy.fMemoryManager)
    , fKey(0)
    , fKeyAllocSize(0)
    , fValue(0)
    , fValueAllocSize(0)
{
   set(toCopy.fKey, toCopy.fValue);
}

KVStringPair::~KVStringPair()
{
    fMemoryManager->deallocate(fKey); //delete [] fKey;
    fMemoryManager->deallocate(fValue); //delete [] fValue;
}


// ---------------------------------------------------------------------------
//  KVStringPair: Setters
// ---------------------------------------------------------------------------
void KVStringPair::setKey(const XMLCh* const newKey)
{
    const unsigned int  len = XMLString::stringLen(newKey);

    if (len >= fKeyAllocSize)
    {
        fMemoryManager->deallocate(fKey); //delete [] fKey;
        fKeyAllocSize = len + 1;
        fKey = (XMLCh*) fMemoryManager->allocate(fKeyAllocSize * sizeof(XMLCh)); //new XMLCh[fKeyAllocSize];
    }

    XMLString::copyString(fKey, newKey);
}

void KVStringPair::setValue(const XMLCh* const newValue)
{
    const unsigned int  len = XMLString::stringLen(newValue);

    if (len >= fValueAllocSize)
    {
        fMemoryManager->deallocate(fValue); //delete [] fValue;
        fValueAllocSize = len + 1;
        fValue = (XMLCh*) fMemoryManager->allocate(fValueAllocSize * sizeof(XMLCh)); //new XMLCh[fValueAllocSize];
    }

    XMLString::copyString(fValue, newValue);
}

void KVStringPair::set(  const   XMLCh* const    newKey
                                , const XMLCh* const    newValue)
{
    setKey(newKey);
    setValue(newValue);
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(KVStringPair)

void KVStringPair::serialize(XSerializeEngine& serEng)
{

    if (serEng.isStoring())
    {

        serEng.writeString(fKey,   fKeyAllocSize,   XSerializeEngine::toWriteBufferLen);
        serEng.writeString(fValue, fValueAllocSize, XSerializeEngine::toWriteBufferLen);
    }
    else
    {
        int dataLen = 0;
        serEng.readString(fKey,   (int&)fKeyAllocSize,   dataLen, XSerializeEngine::toReadBufferLen);
        serEng.readString(fValue, (int&)fValueAllocSize, dataLen, XSerializeEngine::toReadBufferLen);
    }

}

XERCES_CPP_NAMESPACE_END
