/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */
/*******************************************************************************
 * Source date: 9 Apr 1997 21:45:13 GMT
 * netscape/fonts/nfstrm public interface
 * Generated by jmc version 1.8 -- DO NOT EDIT
 ******************************************************************************/

#ifndef _Mnfstrm_H_
#define _Mnfstrm_H_

#include "jmc.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * nfstrm
 ******************************************************************************/

/* The type of the nfstrm interface. */
struct nfstrmInterface;

/* The public type of a nfstrm instance. */
typedef struct nfstrm {
	const struct nfstrmInterface*	vtable;
} nfstrm;

/* The inteface ID of the nfstrm interface. */
#ifndef JMC_INIT_nfstrm_ID
extern EXTERN_C_WITHOUT_EXTERN const JMCInterfaceID nfstrm_ID;
#else
EXTERN_C const JMCInterfaceID nfstrm_ID = { 0x3b686461, 0x5c714911, 0x4c06346e, 0x1c47186a };
#endif /* JMC_INIT_nfstrm_ID */
/*******************************************************************************
 * nfstrm Operations
 ******************************************************************************/

#define nfstrm_getInterface(self, a, exception)	\
	(((self)->vtable->getInterface)(self, nfstrm_getInterface_op, a, exception))

#define nfstrm_addRef(self, exception)	\
	(((self)->vtable->addRef)(self, nfstrm_addRef_op, exception))

#define nfstrm_release(self, exception)	\
	(((self)->vtable->release)(self, nfstrm_release_op, exception))

#define nfstrm_hashCode(self, exception)	\
	(((self)->vtable->hashCode)(self, nfstrm_hashCode_op, exception))

#define nfstrm_equals(self, a, exception)	\
	(((self)->vtable->equals)(self, nfstrm_equals_op, a, exception))

#define nfstrm_clone(self, exception)	\
	(((self)->vtable->clone)(self, nfstrm_clone_op, exception))

#define nfstrm_toString(self, exception)	\
	(((self)->vtable->toString)(self, nfstrm_toString_op, exception))

#define nfstrm_finalize(self, exception)	\
	(((self)->vtable->finalize)(self, nfstrm_finalize_op, exception))

#define nfstrm_IsWriteReady(self, exception)	\
	(((self)->vtable->IsWriteReady)(self, nfstrm_IsWriteReady_op, exception))

#define nfstrm_Write(self, a, a_length, exception)	\
	(((self)->vtable->Write)(self, nfstrm_Write_op, a, a_length, exception))

#define nfstrm_Abort(self, a, exception)	\
	(((self)->vtable->Abort)(self, nfstrm_Abort_op, a, exception))

#define nfstrm_Complete(self, exception)	\
	(((self)->vtable->Complete)(self, nfstrm_Complete_op, exception))

/*******************************************************************************
 * nfstrm Interface
 ******************************************************************************/

struct netscape_jmc_JMCInterfaceID;
struct java_lang_Object;
struct java_lang_String;

struct nfstrmInterface {
	void*	(*getInterface)(struct nfstrm* self, jint op, const JMCInterfaceID* a, JMCException* *exception);
	void	(*addRef)(struct nfstrm* self, jint op, JMCException* *exception);
	void	(*release)(struct nfstrm* self, jint op, JMCException* *exception);
	jint	(*hashCode)(struct nfstrm* self, jint op, JMCException* *exception);
	jbool	(*equals)(struct nfstrm* self, jint op, void* a, JMCException* *exception);
	void*	(*clone)(struct nfstrm* self, jint op, JMCException* *exception);
	const char*	(*toString)(struct nfstrm* self, jint op, JMCException* *exception);
	void	(*finalize)(struct nfstrm* self, jint op, JMCException* *exception);
	jint	(*IsWriteReady)(struct nfstrm* self, jint op, JMCException* *exception);
	jint	(*Write)(struct nfstrm* self, jint op, jbyte* a, jsize a_length, JMCException* *exception);
	void	(*Abort)(struct nfstrm* self, jint op, jint a, JMCException* *exception);
	void*	(*Complete)(struct nfstrm* self, jint op, JMCException* *exception);
};

/*******************************************************************************
 * nfstrm Operation IDs
 ******************************************************************************/

typedef enum nfstrmOperations {
	nfstrm_getInterface_op,
	nfstrm_addRef_op,
	nfstrm_release_op,
	nfstrm_hashCode_op,
	nfstrm_equals_op,
	nfstrm_clone_op,
	nfstrm_toString_op,
	nfstrm_finalize_op,
	nfstrm_IsWriteReady_op,
	nfstrm_Write_op,
	nfstrm_Abort_op,
	nfstrm_Complete_op
} nfstrmOperations;

/******************************************************************************/

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* _Mnfstrm_H_ */