/*
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2003 The Apache Software Foundation.  All rights
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
 * Revision 1.1  2003/09/16 14:33:36  neilg
 * PSVI/schema component model classes, with Makefile/configuration changes necessary to build them
 *
 */

#include <xercesc/framework/psvi/XSWildcard.hpp>

XERCES_CPP_NAMESPACE_BEGIN

XSWildcard::XSWildcard( MemoryManager * const manager):
            XSObject(XSConstants::WILDCARD, manager )
{
}

// XSWildcard methods


/**
 * Namespace constraint: A constraint type: any, not, list. 
 */
XSWildcard::NAMESPACE_CONSTRAINT XSWildcard::getConstraintType() const
{
    // REVISIT
    return NSCONSTRAINT_ANY;
}

/**
 * Namespace constraint. For <code>constraintType</code> 
 * <code>DERIVATION_LIST_NSCONSTRAINT</code>, the list contains allowed namespaces. 
 * For <code>constraintType</code> <code>NOT_NSCONSTRAINT</code>, the 
 * list contains disallowed namespaces. 
 */
StringList *XSWildcard::getNsConstraintList()
{
    // REVISIT
    return 0;
}

/**
 * [process contents]: one of skip, lax or strict. Valid constants values 
 * are: <code>PC_SKIP, PC_LAX, PC_STRICT</code>. 
 */
XSWildcard::PROCESS_CONTENTS XSWildcard::getProcessContents() const
{
    // REVISIT
    return PC_STRICT;
}

/**
 * Optional. An [annotation]. 
 */
XSAnnotation *XSWildcard::getAnnotation()
{
    // REVISIT
    return 0;
}

XERCES_CPP_NAMESPACE_END


