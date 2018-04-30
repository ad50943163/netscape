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
 * netscape/fonts/crf module C stub file
 * Generated by jmc version 1.8 -- DO NOT EDIT
 ******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "xp_mem.h"

/* Include the implementation-specific header: */
#include "Pcrf.h"

/* Include other interface headers: */

/*******************************************************************************
 * crf Methods
 ******************************************************************************/

#ifndef OVERRIDE_crf_getInterface
JMC_PUBLIC_API(void*)
_crf_getInterface(struct crf* self, jint op, const JMCInterfaceID* iid, JMCException* *exc)
{
	if (memcmp(iid, &crf_ID, sizeof(JMCInterfaceID)) == 0)
		return crfImpl2crf(crf2crfImpl(self));
	return _crf_getBackwardCompatibleInterface(self, iid, exc);
}
#endif

#ifndef OVERRIDE_crf_addRef
JMC_PUBLIC_API(void)
_crf_addRef(struct crf* self, jint op, JMCException* *exc)
{
	crfImplHeader* impl = (crfImplHeader*)crf2crfImpl(self);
	impl->refcount++;
}
#endif

#ifndef OVERRIDE_crf_release
JMC_PUBLIC_API(void)
_crf_release(struct crf* self, jint op, JMCException* *exc)
{
	crfImplHeader* impl = (crfImplHeader*)crf2crfImpl(self);
	if (--impl->refcount == 0) {
		crf_finalize(self, exc);
	}
}
#endif

#ifndef OVERRIDE_crf_hashCode
JMC_PUBLIC_API(jint)
_crf_hashCode(struct crf* self, jint op, JMCException* *exc)
{
	return (jint)self;
}
#endif

#ifndef OVERRIDE_crf_equals
JMC_PUBLIC_API(jbool)
_crf_equals(struct crf* self, jint op, void* obj, JMCException* *exc)
{
	return self == obj;
}
#endif

#ifndef OVERRIDE_crf_clone
JMC_PUBLIC_API(void*)
_crf_clone(struct crf* self, jint op, JMCException* *exc)
{
	crfImpl* impl = crf2crfImpl(self);
	crfImpl* newImpl = (crfImpl*)malloc(sizeof(crfImpl));
	if (newImpl == NULL) return NULL;
	memcpy(newImpl, impl, sizeof(crfImpl));
	((crfImplHeader*)newImpl)->refcount = 1;
	return newImpl;
}
#endif

#ifndef OVERRIDE_crf_toString
JMC_PUBLIC_API(const char*)
_crf_toString(struct crf* self, jint op, JMCException* *exc)
{
	return NULL;
}
#endif

#ifndef OVERRIDE_crf_finalize
JMC_PUBLIC_API(void)
_crf_finalize(struct crf* self, jint op, JMCException* *exc)
{
	/* Override this method and add your own finalization here. */
	XP_FREEIF(self);
}
#endif

/*******************************************************************************
 * Jump Tables
 ******************************************************************************/

const struct crfInterface crfVtable = {
	_crf_getInterface,
	_crf_addRef,
	_crf_release,
	_crf_hashCode,
	_crf_equals,
	_crf_clone,
	_crf_toString,
	_crf_finalize,
	_crf_GetMatchInfo,
	_crf_GetPointSize,
	_crf_GetMaxWidth,
	_crf_GetFontAscent,
	_crf_GetFontDescent,
	_crf_GetMaxLeftBearing,
	_crf_GetMaxRightBearing,
	_crf_SetTransformMatrix,
	_crf_GetTransformMatrix,
	_crf_MeasureText,
	_crf_MeasureBoundingBox,
	_crf_DrawText,
	_crf_PrepareDraw,
	_crf_EndDraw
};

/*******************************************************************************
 * Factory Operations
 ******************************************************************************/

JMC_PUBLIC_API(crf*)
crfFactory_Create(JMCException* *exception)
{
	crfImplHeader* impl = (crfImplHeader*)XP_NEW_ZAP(crfImpl);
	crf* self;
	if (impl == NULL) {
		JMC_EXCEPTION(exception, JMCEXCEPTION_OUT_OF_MEMORY);
		return NULL;
	}
	self = crfImpl2crf(impl);
	impl->vtablecrf = &crfVtable;
	impl->refcount = 1;
	_crf_init(self, exception);
	if (JMC_EXCEPTION_RETURNED(exception)) {
		XP_FREE(impl);
		return NULL;
	}
	return self;
}
