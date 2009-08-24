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

#if !defined(XERCESC_INCLUDE_GUARD_CMSTATESET_HPP)
#define XERCESC_INCLUDE_GUARD_CMSTATESET_HPP

//  DESCRIPTION:
//
//  This class is a specialized bitset class for the content model code of
//  the validator. It assumes that its never called with two objects of
//  different bit counts, and that bit sets smaller than a threshold are far
//  and away the most common. So it can be a lot more optimized than a general
//  purpose utility bitset class
//

#include <xercesc/util/ArrayIndexOutOfBoundsException.hpp>
#include <xercesc/util/RuntimeException.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/MemoryManager.hpp>
#include <string.h>

XERCES_CPP_NAMESPACE_BEGIN

class CMStateSetEnumerator;

#define CMSTATE_CACHED_INT32_SIZE  4

#define CMSTATE_BITFIELD_CHUNK  1024
#define CMSTATE_BITFIELD_INT32_SIZE (1024 / 32)

struct CMDynamicBuffer
{
    //  fArraySize
    //      This indicates the number of elements of the fBitArray vector
    //
    //  fBitArray
    //      A vector of arrays of XMLInt32; each array is allocated on demand
    //      if a bit needs to be set in that range
    //
    //  fMemoryManager
    //      The memory manager used to allocate and deallocate memory
    //
    XMLSize_t    fArraySize;
    XMLInt32**      fBitArray;
    MemoryManager*  fMemoryManager;
};

class CMStateSet : public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    CMStateSet( const XMLSize_t bitCount
              , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager) :

        fBitCount(bitCount)
        , fDynamicBuffer(0)
    {
        //
        //  See if we need to allocate the byte array or whether we can live
        //  within the cached bit high performance scheme.
        //
        if (fBitCount > (CMSTATE_CACHED_INT32_SIZE * 32))
        {
            fDynamicBuffer = (CMDynamicBuffer*)manager->allocate(sizeof(CMDynamicBuffer));
            fDynamicBuffer->fMemoryManager = manager;
            // allocate an array of vectors, each one containing CMSTATE_BITFIELD_CHUNK bits
            fDynamicBuffer->fArraySize = fBitCount / CMSTATE_BITFIELD_CHUNK;
            if (fBitCount % CMSTATE_BITFIELD_CHUNK)
                fDynamicBuffer->fArraySize++;
            fDynamicBuffer->fBitArray = (XMLInt32**) fDynamicBuffer->fMemoryManager->allocate(fDynamicBuffer->fArraySize*sizeof(XMLInt32*));
            for(XMLSize_t index = 0; index < fDynamicBuffer->fArraySize; index++)
                fDynamicBuffer->fBitArray[index]=NULL;
        }
        else
        {
            for (XMLSize_t index = 0; index < CMSTATE_CACHED_INT32_SIZE; index++)
                fBits[index] = 0;
        }
    }

    CMStateSet(const CMStateSet& toCopy) :
        XMemory(toCopy)
      , fBitCount(toCopy.fBitCount)
      , fDynamicBuffer(0)
    {
        //
        //  See if we need to allocate the byte array or whether we can live
        //  within the cahced bit high performance scheme.
        //
        if (fBitCount > (CMSTATE_CACHED_INT32_SIZE * 32))
        {
            fDynamicBuffer = (CMDynamicBuffer*) toCopy.fDynamicBuffer->fMemoryManager->allocate(sizeof(CMDynamicBuffer));
            fDynamicBuffer->fMemoryManager = toCopy.fDynamicBuffer->fMemoryManager;
            fDynamicBuffer->fArraySize = fBitCount / CMSTATE_BITFIELD_CHUNK;
            if (fBitCount % CMSTATE_BITFIELD_CHUNK)
                fDynamicBuffer->fArraySize++;
            fDynamicBuffer->fBitArray = (XMLInt32**) fDynamicBuffer->fMemoryManager->allocate(fDynamicBuffer->fArraySize*sizeof(XMLInt32*));
            for(XMLSize_t index = 0; index < fDynamicBuffer->fArraySize; index++)
            {
                if(toCopy.fDynamicBuffer->fBitArray[index]!=NULL)
                {
                    fDynamicBuffer->fBitArray[index]=(XMLInt32*)fDynamicBuffer->fMemoryManager->allocate(CMSTATE_BITFIELD_INT32_SIZE * sizeof(XMLInt32));
                    memcpy((void *) fDynamicBuffer->fBitArray[index],
                           (const void *) toCopy.fDynamicBuffer->fBitArray[index],
                           CMSTATE_BITFIELD_INT32_SIZE * sizeof(XMLInt32));
                }
                else
                    fDynamicBuffer->fBitArray[index]=NULL;
            }
        }
        else
        {
            memcpy((void *) fBits,
                   (const void *) toCopy.fBits,
                   CMSTATE_CACHED_INT32_SIZE * sizeof(XMLInt32));
        }
    }

    ~CMStateSet()
    {
        if(fDynamicBuffer)
        {
            for(XMLSize_t index = 0; index < fDynamicBuffer->fArraySize; index++)
            {
                if(fDynamicBuffer->fBitArray[index]!=NULL)
                    fDynamicBuffer->fMemoryManager->deallocate(fDynamicBuffer->fBitArray[index]);
            }
            fDynamicBuffer->fMemoryManager->deallocate(fDynamicBuffer);
        }
    }


    // -----------------------------------------------------------------------
    //  Set manipulation methods
    // -----------------------------------------------------------------------
    void operator|=(const CMStateSet& setToOr)
    {
        if(fDynamicBuffer==0)
        {
            for (XMLSize_t index = 0; index < CMSTATE_CACHED_INT32_SIZE; index++)
                if(setToOr.fBits[index])
                    if(fBits[index])
                        fBits[index] |= setToOr.fBits[index];
                    else
                        fBits[index] = setToOr.fBits[index];
        }
        else
        {
            for (XMLSize_t index = 0; index < fDynamicBuffer->fArraySize; index++)
                if(setToOr.fDynamicBuffer->fBitArray[index]!=NULL)
                {
                    // if we haven't allocated the subvector yet, allocate it and copy
                    if(fDynamicBuffer->fBitArray[index]==NULL)
                    {
                        fDynamicBuffer->fBitArray[index]=(XMLInt32*)fDynamicBuffer->fMemoryManager->allocate(CMSTATE_BITFIELD_INT32_SIZE * sizeof(XMLInt32));
                        memcpy((void *) fDynamicBuffer->fBitArray[index],
                               (const void *) setToOr.fDynamicBuffer->fBitArray[index],
                               CMSTATE_BITFIELD_INT32_SIZE * sizeof(XMLInt32));
                    }
                    else
                    {
                        // otherwise, merge them
                        for(XMLSize_t subIndex = 0; subIndex < CMSTATE_BITFIELD_INT32_SIZE; subIndex++)
                            if(setToOr.fDynamicBuffer->fBitArray[index][subIndex])
                                if(fDynamicBuffer->fBitArray[index][subIndex])
                                    fDynamicBuffer->fBitArray[index][subIndex] |= setToOr.fDynamicBuffer->fBitArray[index][subIndex];
                                else
                                    fDynamicBuffer->fBitArray[index][subIndex] = setToOr.fDynamicBuffer->fBitArray[index][subIndex];
                    }
                }
        }
    }

    bool operator==(const CMStateSet& setToCompare) const
    {
        if (fBitCount != setToCompare.fBitCount)
            return false;

        if(fDynamicBuffer==0)
        {
            for (XMLSize_t index = 0; index < CMSTATE_CACHED_INT32_SIZE; index++)
            {
                if (fBits[index] != setToCompare.fBits[index])
                    return false;
            }
        }
        else
        {
            for (XMLSize_t index = 0; index < fDynamicBuffer->fArraySize; index++)
            {
                if(fDynamicBuffer->fBitArray[index]==NULL && setToCompare.fDynamicBuffer->fBitArray[index]==NULL)
                    continue;
                else if(fDynamicBuffer->fBitArray[index]==NULL || setToCompare.fDynamicBuffer->fBitArray[index]==NULL) // the other should have been empty too
                    return false;
                else
                {
                    for(XMLSize_t subIndex = 0; subIndex < CMSTATE_BITFIELD_INT32_SIZE; subIndex++)
                        if(fDynamicBuffer->fBitArray[index][subIndex]!=setToCompare.fDynamicBuffer->fBitArray[index][subIndex])
                            return false;
                }
            }
        }
        return true;
    }

    CMStateSet& operator=(const CMStateSet& srcSet)
    {
        if (this == &srcSet)
            return *this;

        // They have to be the same size
        if (fBitCount != srcSet.fBitCount)
            if(fDynamicBuffer)
                ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Bitset_NotEqualSize, fDynamicBuffer->fMemoryManager);
            else
                ThrowXML(RuntimeException, XMLExcepts::Bitset_NotEqualSize);

        if(fDynamicBuffer==0)
        {
            for (XMLSize_t index = 0; index < CMSTATE_CACHED_INT32_SIZE; index++)
                fBits[index] = srcSet.fBits[index];
        }
        else
        {
            for (XMLSize_t index = 0; index < fDynamicBuffer->fArraySize; index++)
                if(srcSet.fDynamicBuffer->fBitArray[index]==NULL)
                {
                    // delete this subentry
                    if(fDynamicBuffer->fBitArray[index]!=NULL)
                    {
                        fDynamicBuffer->fMemoryManager->deallocate(fDynamicBuffer->fBitArray[index]);
                        fDynamicBuffer->fBitArray[index]=NULL;
                    }
                }
                else
                {
                    // if we haven't allocated the subvector yet, allocate it and copy
                    if(fDynamicBuffer->fBitArray[index]==NULL)
                        fDynamicBuffer->fBitArray[index]=(XMLInt32*)fDynamicBuffer->fMemoryManager->allocate(CMSTATE_BITFIELD_INT32_SIZE * sizeof(XMLInt32));
                    memcpy((void *) fDynamicBuffer->fBitArray[index],
                           (const void *) srcSet.fDynamicBuffer->fBitArray[index],
                           CMSTATE_BITFIELD_INT32_SIZE * sizeof(XMLInt32));
                }
        }
        return *this;
    }

    XMLSize_t getBitCountInRange(XMLSize_t start, XMLSize_t end) const
    {
        XMLSize_t count = 0;
        end /= 32;
        if(fDynamicBuffer==0)
        {
            if(end > CMSTATE_CACHED_INT32_SIZE)
                end = CMSTATE_CACHED_INT32_SIZE;
            for (XMLSize_t index = start / 32; index < end; index++)
            {
                if (fBits[index] != 0)
                    for(int i=0;i<32;i++)
                    {
                        XMLInt32 mask=(1UL << i);
                        if(fBits[index] & mask)
                            count++;
                    }
            }
        }
        else
        {
            if(end > fDynamicBuffer->fArraySize)
                end = fDynamicBuffer->fArraySize;
            for (XMLSize_t index = start / 32; index < end; index++)
            {
                if(fDynamicBuffer->fBitArray[index]==NULL)
                    continue;
                for(XMLSize_t subIndex=0;subIndex < CMSTATE_BITFIELD_INT32_SIZE; subIndex++)
                {
                    if (fDynamicBuffer->fBitArray[index][subIndex] != 0)
                        for(int i=0;i<32;i++)
                        {
                            XMLInt32 mask=(1UL << i);
                            if(fDynamicBuffer->fBitArray[index][subIndex] & mask)
                                count++;
                        }
                }
            }
        }
        return count;
    }

    bool getBit(const XMLSize_t bitToGet) const
    {
        if (bitToGet >= fBitCount)
            if(fDynamicBuffer)
                ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::Bitset_BadIndex, fDynamicBuffer->fMemoryManager);
            else
                ThrowXML(ArrayIndexOutOfBoundsException, XMLExcepts::Bitset_BadIndex);

        // And access the right bit and byte
        if(fDynamicBuffer==0)
        {
            const XMLInt32 mask = (0x1UL << (bitToGet % 32));
            const XMLSize_t byteOfs = bitToGet / 32;
            return (fBits[byteOfs]!=0 && (fBits[byteOfs] & mask) != 0);
        }
        else
        {
            const XMLSize_t vectorOfs = bitToGet / CMSTATE_BITFIELD_CHUNK;
            if(fDynamicBuffer->fBitArray[vectorOfs]==NULL)
                return false;
            const XMLInt32 mask = (0x1UL << (bitToGet % 32));
            const XMLSize_t byteOfs = (bitToGet % CMSTATE_BITFIELD_CHUNK) / 32;
            return (fDynamicBuffer->fBitArray[vectorOfs][byteOfs]!=0 && (fDynamicBuffer->fBitArray[vectorOfs][byteOfs] & mask) != 0);
        }
    }

    bool isEmpty() const
    {
        if(fDynamicBuffer==0)
        {
            for (XMLSize_t index = 0; index < CMSTATE_CACHED_INT32_SIZE; index++)
            {
                if (fBits[index] != 0)
                    return false;
            }
        }
        else
        {
            for (XMLSize_t index = 0; index < fDynamicBuffer->fArraySize; index++)
            {
                if(fDynamicBuffer->fBitArray[index]==NULL)
                    continue;
                for(XMLSize_t subIndex=0;subIndex < CMSTATE_BITFIELD_INT32_SIZE; subIndex++)
                {
                    if (fDynamicBuffer->fBitArray[index][subIndex] != 0)
                        return false;
                }
            }
        }
        return true;
    }

    void setBit(const XMLSize_t bitToSet)
    {
        if (bitToSet >= fBitCount)
            if(fDynamicBuffer)
                ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::Bitset_BadIndex, fDynamicBuffer->fMemoryManager);
            else
                ThrowXML(ArrayIndexOutOfBoundsException, XMLExcepts::Bitset_BadIndex);

        const XMLInt32 mask = (0x1UL << (bitToSet % 32));

        // And access the right bit and byte
        if(fDynamicBuffer==0)
        {
            const XMLSize_t byteOfs = bitToSet / 32;
            fBits[byteOfs] &= ~mask;
            fBits[byteOfs] |= mask;
        }
        else
        {
            const XMLSize_t vectorOfs = bitToSet / CMSTATE_BITFIELD_CHUNK;
            if(fDynamicBuffer->fBitArray[vectorOfs]==NULL)
            {
                fDynamicBuffer->fBitArray[vectorOfs]=(XMLInt32*)fDynamicBuffer->fMemoryManager->allocate(CMSTATE_BITFIELD_INT32_SIZE * sizeof(XMLInt32));
                for(XMLSize_t index=0;index < CMSTATE_BITFIELD_INT32_SIZE; index++)
                    fDynamicBuffer->fBitArray[vectorOfs][index]=0;
            }
            const XMLSize_t byteOfs = (bitToSet % CMSTATE_BITFIELD_CHUNK) / 32;
            fDynamicBuffer->fBitArray[vectorOfs][byteOfs] &= ~mask;
            fDynamicBuffer->fBitArray[vectorOfs][byteOfs] |= mask;
        }
    }

    void zeroBits()
    {
        if(fDynamicBuffer==0)
        {
            for (XMLSize_t index = 0; index < CMSTATE_CACHED_INT32_SIZE; index++)
                fBits[index] = 0;
        }
        else
        {
            for (XMLSize_t index = 0; index < fDynamicBuffer->fArraySize; index++)
                // delete this subentry
                if(fDynamicBuffer->fBitArray[index]!=NULL)
                {
                    fDynamicBuffer->fMemoryManager->deallocate(fDynamicBuffer->fBitArray[index]);
                    fDynamicBuffer->fBitArray[index]=NULL;
                }
        }
    }

    XMLSize_t hashCode() const
    {
        XMLSize_t hash = 0;
        if(fDynamicBuffer==0)
        {
            for (XMLSize_t index = 0; index<CMSTATE_CACHED_INT32_SIZE; index++)
                hash = fBits[index] + hash * 31;
        }
        else
        {
            for (XMLSize_t index = 0; index<fDynamicBuffer->fArraySize; index++)
            {
                if(fDynamicBuffer->fBitArray[index]==NULL)
                    // simulates the iteration on the missing bits
                    for(XMLSize_t subIndex=0;subIndex < CMSTATE_BITFIELD_INT32_SIZE; subIndex++)
                        hash *= 31;
                else
                    for(XMLSize_t subIndex=0;subIndex < CMSTATE_BITFIELD_INT32_SIZE; subIndex++)
                        hash = fDynamicBuffer->fBitArray[index][subIndex] + hash * 31;
            }
        }
        return hash;
    }

private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    CMStateSet();


    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fBitCount
    //      The count of bits that the outside world wants to support,
    //      so its the max bit index plus one.
    //
    //  fBits
    //      When the bit count is less than a threshold (very common), these hold the bits.
    //      Otherwise, the fDynamicBuffer member holds htem.
    //
    //  fDynamicBuffer
    //      If the bit count is greater than the threshold, then we allocate this structure to
    //      store the bits, the length, and the memory manager to allocate/deallocate
    //      the memory
    //      
    // -----------------------------------------------------------------------
    XMLSize_t     fBitCount;
    XMLInt32         fBits[CMSTATE_CACHED_INT32_SIZE];
    CMDynamicBuffer* fDynamicBuffer;

    friend class CMStateSetEnumerator;
};

class CMStateSetEnumerator : public XMemory
{
public:
    CMStateSetEnumerator(const CMStateSet* toEnum) :
      fToEnum(toEnum),
      fIndexCount((XMLSize_t)-1),
      fLastValue(0)
    {
        findNext();
    }

    bool hasMoreElements()
    {
        return fLastValue!=0;
    }

    unsigned int nextElement()
    {
        for(int i=0;i<32;i++)
        {
            XMLInt32 mask=(1UL << i);
            if(fLastValue & mask)
            {
                fLastValue &= ~mask;
                unsigned int retVal=(unsigned int)fIndexCount+i;
                if(fLastValue==0)
                    findNext();
                return retVal;
            }
        }
        return 0;
    }

private:
    void findNext()
    {
        if(fToEnum->fDynamicBuffer==0)
        {
            XMLSize_t nOffset=((fIndexCount==(XMLSize_t)-1)?0:(fIndexCount/32)+1);
            for(XMLSize_t index=nOffset;index<CMSTATE_CACHED_INT32_SIZE;index++)
            {
                if(fToEnum->fBits[index]!=0)
                {
                    fIndexCount=index*32;
                    fLastValue=fToEnum->fBits[index];
                    return;
                }
            }
        }
        else
        {
            XMLSize_t nOffset=((fIndexCount==(XMLSize_t)-1)?0:(fIndexCount/CMSTATE_BITFIELD_CHUNK));
            XMLSize_t nSubOffset=((fIndexCount==(XMLSize_t)-1)?0:((fIndexCount % CMSTATE_BITFIELD_CHUNK) /32)+1);
            for (XMLSize_t index = nOffset; index<fToEnum->fDynamicBuffer->fArraySize; index++)
            {
                if(fToEnum->fDynamicBuffer->fBitArray[index]!=NULL)
                {
                    for(XMLSize_t subIndex=nSubOffset;subIndex < CMSTATE_BITFIELD_INT32_SIZE; subIndex++)
                        if(fToEnum->fDynamicBuffer->fBitArray[index][subIndex]!=0)
                        {
                            fIndexCount=index*CMSTATE_BITFIELD_CHUNK + subIndex*32;
                            fLastValue=fToEnum->fDynamicBuffer->fBitArray[index][subIndex];
                            return;
                        }
                }
                nSubOffset = 0; // next chunks will be processed from the beginning
            }
        }
    }

    const CMStateSet*   fToEnum;
    XMLSize_t        fIndexCount;
    XMLInt32            fLastValue;
};

XERCES_CPP_NAMESPACE_END

#endif
