/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.  Portions created by Netscape are Copyright (C) 1998
 * Netscape Communications Corporation.  All Rights Reserved.
 */


#ifndef __ANIMECHO_H
//	Avoid include redundancy
//
#define __ANIMECHO_H

class CDDEAnimationEcho
{
protected:
	static CPtrList m_Registry;
	POSITION m_rIndex;
	CString m_csServiceName;
	
	
	CDDEAnimationEcho(CString& csServiceName) 
	{
		m_rIndex = m_Registry.AddTail(this);
		m_csServiceName = csServiceName;
	}

	~CDDEAnimationEcho()	
	{
		m_Registry.RemoveAt(m_rIndex);
	}
	
	//	Must override.
	void EchoAnimation(DWORD dwWindowID, DWORD dwAnimationState);
	
	
public:
	CString GetServiceName()	{
		return(m_csServiceName);
	}

	//	Consider these the constructor, destructor.
	static void DDERegister(CString &csServiceName);
	static BOOL DDEUnRegister(CString &csServiceName);
	
	static void Echo(DWORD dwWindowID, DWORD dwAnimationState);

};

#endif // __URLECHO_H
