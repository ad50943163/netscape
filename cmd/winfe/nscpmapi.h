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
//
// This is a header file for the MAPI support within 
// Communicator. 
//
// Written by: Rich Pizzarro (rhp@netscape.com)
//
#ifndef _NSCPMAPI
#define _NSCPMAPI

#ifndef MAPI_OLE	// Because MSFT doesn't do this for us :-(
#include <mapi.h>     // for MAPI specific types...        
#endif 

#ifdef WIN16
typedef unsigned char	UCHAR;
#endif 

#define           MAPI_CUSTOM_COMPOSE_MSG     "NSCP_CUSTOM_MAPI_MESSAGE"
#define           MAPI_CUSTOM_RET_CODE        1966

#define           MAX_NAME_LEN    256
#define           MAX_PW_LEN      256
#define           MAX_MSGINFO_LEN 512
#define           MAX_CON         4     // Maximum MAPI session supported
#define           MAX_POINTERS    32

//
// The MAPI class that will act as the internal mechanism for 
// Communicator to control multiple MAPI sessions.
//
class CMAPIConnection 
{
protected:
  LONG        m_ID;
  BOOL        m_defaultConnection;  
  LONG        m_sessionCount;
  LONG        m_messageIndex;
  LPVOID      m_cookie;
  UCHAR       m_messageFindInfo[MAX_MSGINFO_LEN];
  UCHAR       m_profileName[MAX_NAME_LEN];
  UCHAR       m_password[MAX_PW_LEN];

  // Methods

public:
  CMAPIConnection  ( LONG, LPSTR, LPSTR );
  ~CMAPIConnection ( );

  // ID related methods
  LONG   GetID( ) { return m_ID; } ;

  // Dealing with the default session...
  BOOL    IsDefault( ) { return m_defaultConnection; } ;
  void    SetDefault( BOOL flag ) { m_defaultConnection = flag; } ;

  // For handling multiple sessions on a profile name...
  LONG    GetSessionCount( ) { return m_sessionCount; } ;
  void    IncrementSessionCount() { ++m_sessionCount; } ;
  void    DecrementSessionCount() { --m_sessionCount; } ;

  // Information retrieval stuff...
  LPSTR   GetProfileName( ) { return (LPSTR) m_profileName; };
  LPSTR   GetPassword( ) { return (LPSTR) m_password; };

  // Dealing with message information...
  void    SetMessageIndex( LONG mIndex ) { m_messageIndex = mIndex; } ;
  LONG    GetMessageIndex( ) { return m_messageIndex; };

  void    SetMessageFindInfo( LPSTR info ) { lstrcpy((LPSTR)m_messageFindInfo, info); } ;
  LPSTR   GetMessageFindInfo( ) { return (LPSTR) m_messageFindInfo; };

  // For enumerating Messages...
  void    SetMapiListContext( LPVOID cookie) { m_cookie = cookie; } ;
  LPVOID  GetMapiListContext( ) { return m_cookie; };
};

//
// Defines needed for requests being made with the WM_COPYDATA call...
//
typedef enum {
    NSCP_MAPIStartRequestID = 100,
    NSCP_MAPILogon,
    NSCP_MAPILogoff,
    NSCP_MAPIFree,
    NSCP_MAPISendMail,
    NSCP_MAPISendDocuments, 
    NSCP_MAPIFindNext,
    NSCP_MAPIReadMail,
    NSCP_MAPISaveMail,
    NSCP_MAPIDeleteMail,
    NSCP_MAPIAddress,
    NSCP_MAPIDetails,
    NSCP_MAPIResolveName,
    NSCP_MAPI_NSCP_SynchronizeClient,
    NSCP_MAPIEndRequestID       // Note: this is a marker for MAPI IPC requests
} NSCP_IPC_REQUEST;

//
// This is to keep track of the pointers allocated in the MAPI DLL
// and deal with them correctly.
//
#define   MAPI_MESSAGE_TYPE     0
#define   MAPI_RECIPIENT_TYPE   1
 
typedef struct {
  LPVOID    lpMem;
  UCHAR     memType;
} memTrackerType;

//
// This is the generic message that WM_COPYDATA will send to the
// Communicator product to allow it to attach to shared memory.
// NOTE: On Win16, this will simply reference a pointer.
//
typedef struct {
  UCHAR   smemName[64]; // Name of shared memory
  DWORD   smemSize;     // Size of shared memory
  LPVOID  lpsmem;       // Will be really used in Win16 only
} MAPIIPCType;

//
// These are message specific structures that will be used for 
// the various MAPI operations.
//
typedef struct {
  ULONG     ulUIParam;
  FLAGS     flFlags;
  LHANDLE   lhSession;
  DWORD     ipcWorked;      // Necessary for IPC check with Communicator
// LPSTR     strSequence,  // LPSTR lpszProfileName, LPSTR lpszPassword
// This is here to document the fact there will be a string sequence at 
// this location
} MAPILogonType;

typedef struct {
  LHANDLE   lhSession;
  ULONG     ulUIParam;
  FLAGS     flFlags;
  DWORD     ipcWorked;      // Necessary for IPC check with Communicator
} MAPILogoffType;

typedef struct {
  LHANDLE   lhSession;
  ULONG     ulUIParam;
  FLAGS     flFlags;
  DWORD     ipcWorked;      // Necessary for IPC check with Communicator
  // The following is the "FLAT" representation of the (lpMapiMessage lpMessage) 
  // argument of this structure 
  FLAGS     MSG_flFlags;          // unread,return receipt                  
  ULONG     MSG_nRecipCount;      // Number of recipients                   
  ULONG     MSG_nFileCount;       // # of file attachments                  
  ULONG     MSG_ORIG_ulRecipClass; //  Recipient class - MAPI_TO, MAPI_CC, MAPI_BCC, MAPI_ORIG
  BYTE      dataBuf[1];           // For easy referencing
  //
  // This is where it gets CONFUSING...the following buffer of memory is a
  // contiguous chunk of memory for various strings that are part of this
  // multilevel structure. For any of the following structure, any numbers
  // are represented by strings that will have to be converted back to numeric
  // values with atoi() calls.

  // String 0: LPSTR lpszSubject;            // Message Subject
  // String 1: LPSTR lpszNoteText FILE NAME; // Message Text will be
  //           stored into a temp file and this will be the pointer to that file.
  // String 2: LPSTR lpszDateReceived;       // in YYYY/MM/DD HH:MM format
  // String 3: LPSTR lpszConversationID;     // conversation thread ID
  //
  // The following are for the originator of the message. Only ONE of these.
  //
  // String 4: LPSTR lpszName;             // Originator name                           
  // String 5: LPSTR lpszAddress;          // Originator address (optional)             
  //
  // The following strings are for the recipients for this message. There are
  // MSG_nRecipCount of these in a row:
  //
  // for (i=0; i<MSG_nRecipCount; i++)
  //      String x: LPSTR lpszRecipClass (ULONG) // Recipient class - MAPI_TO, MAPI_CC, MAPI_BCC, MAPI_ORIG        
  //      String x: LPSTR lpszName;     // Recipient N name                           
  //      String x: LPSTR lpszAddress;  // Recipient N address (optional)             
  //
  // Now, finally, add the attachments for this beast. There are MSG_nFileCount
  // attachments so it would look like the following:
  //
  // for (i=0; i<MSG_nFileCount; i++)
  //
  //      String x: LPSTR lpszPathName // Fully qualified path of the attached file. 
  //                                   // This path should include the disk drive letter and directory name.
  //      String x: LPSTR lpszFileName // The display name for the attached file
  //
} MAPISendMailType;

typedef struct {
    ULONG     ulUIParam;
    ULONG     nFileCount;
    DWORD     ipcWorked;      // Necessary for IPC check with Communicator
    BYTE      dataBuf[1];     // For easy referencing
    //
    // The sequence of strings to follow are groups of PathName/FileName couples.
    // The strings will be parsed in MAPI[32].DLL and then put into this format:
    //
    // for (i=0; i<nFileCount; i++)
    //
    //      String x: LPSTR lpszPathName // Fully qualified path of the attached file. 
    //                                   // This path should include the disk drive letter and directory name.
    //      String x: LPSTR lpszFileName // The display name for the attached file
} MAPISendDocumentsType;

typedef struct {
  LHANDLE     lhSession;
  ULONG       ulUIParam;
  FLAGS       flFlags;
  DWORD       ipcWorked;      // Necessary for IPC check with Communicator
  UCHAR       lpszSeedMessageID[MAX_MSGINFO_LEN];
  UCHAR       lpszMessageID[MAX_MSGINFO_LEN];
} MAPIFindNextType;

typedef struct {
  LHANDLE     lhSession;
  ULONG       ulUIParam;
  DWORD       ipcWorked;      // Necessary for IPC check with Communicator
  UCHAR       lpszMessageID[MAX_MSGINFO_LEN];
} MAPIDeleteMailType;

typedef struct {
  LHANDLE     lhSession;
  ULONG       ulUIParam;
  FLAGS       flFlags;
  DWORD       ipcWorked;      // Necessary for IPC check with Communicator
  UCHAR       lpszName[MAX_NAME_LEN];
  // These are returned by Communicator
  UCHAR       lpszABookID[MAX_NAME_LEN];         
  UCHAR       lpszABookName[MAX_NAME_LEN];            
  UCHAR       lpszABookAddress[MAX_NAME_LEN];         
} MAPIResolveNameType;

typedef struct {
  LHANDLE     lhSession;
  ULONG       ulUIParam;
  FLAGS       flFlags;
  DWORD       ipcWorked;      // Necessary for IPC check with Communicator
  UCHAR       lpszABookID[MAX_NAME_LEN];
} MAPIDetailsType;

typedef struct {
  LHANDLE     lhSession;
  ULONG       ulUIParam;
  FLAGS       flFlags;
  DWORD       ipcWorked;      // Necessary for IPC check with Communicator
  UCHAR       lpszMessageID[MAX_MSGINFO_LEN];
  //
  // The following is the "FLAT" representation of the (lpMapiMessage lpMessage) 
  // argument of this structure 
  //
  FLAGS     MSG_flFlags;          // unread, return or receipt                  
  ULONG     MSG_nRecipCount;      // Number of recipients                   
  ULONG     MSG_nFileCount;       // # of file attachments                  
  ULONG     MSG_ORIG_ulRecipClass; //  Recipient class - MAPI_TO, MAPI_CC, MAPI_BCC, MAPI_ORIG
  //
  // Output parameter for blob of information that will live on disk. 
  //
  UCHAR       lpszBlob[MAX_MSGINFO_LEN];    // file name on disk
  //
  // The format of this blob of information will be:
  //
  // String 0: LPSTR lpszSubject;            // Message Subject
  // String 1: LPSTR lpszNoteText FILE NAME; // Message Text will be
  //           stored into a temp file and this will be the pointer to that file.
  // String 2: LPSTR lpszDateReceived;       // in YYYY/MM/DD HH:MM format
  // String 3: LPSTR lpszConversationID;     // conversation thread ID
  //
  // The following are for the originator of the message. Only ONE of these.
  //
  // String 4: LPSTR lpszName;             // Originator name                           
  // String 5: LPSTR lpszAddress;          // Originator address (optional)             
  //
  // The following strings are for the recipients for this message. There are
  // MSG_nRecipCount of these in a row:
  //
  // for (i=0; i<MSG_nRecipCount; i++)
  //      String x: LPSTR lpszName;             // Recipient N name                           
  //      String x: LPSTR lpszAddress;          // Recipient N address (optional)             
  //      String x: LPSTR lpszRecipClass        // recipient class - sprintf of ULONG
  //
  // Now, finally, add the attachments for this beast. There are MSG_nFileCount
  // attachments so it would look like the following:
  //
  // for (i=0; i<MSG_nFileCount; i++)
  //
  //      String x: LPSTR lpszPathName // Fully qualified path of the attached file. 
  //                                   // This path should include the disk drive letter and directory name.
  //      String x: LPSTR lpszFileName // The display name for the attached file
  //
} MAPIReadMailType;

typedef struct {
  LHANDLE   lhSession;
  ULONG     ulUIParam;
  FLAGS     flFlags;
  UCHAR     lpszCaption[MAX_MSGINFO_LEN];
  DWORD     ipcWorked;      // Necessary for IPC check with Communicator
  // The following is the "FLAT" representation of the (lpMapiRecipDesc lpRecips) 
  // argument of this structure 
  ULONG     nRecips;              // number of recips to start with...
  ULONG     nNewRecips;           // number of recips returned...
  UCHAR     lpszBlob[MAX_MSGINFO_LEN];    // file name for blob of information 
                                          // that will live on disk. 
  BYTE      dataBuf[1];           // For easy referencing
  //
  // The following contiguous chunk of memory is the buffer that holds 
  // the recipients to load into the address picker...
  //
  // for (i=0; i<MSG_nRecipCount; i++)
  //      String x: LPSTR lpszRecipClass (ULONG) // Recipient class - MAPI_TO, MAPI_CC, MAPI_BCC, MAPI_ORIG        
  //      String x: LPSTR lpszName;     // Recipient N name                           
  //      String x: LPSTR lpszAddress;  // Recipient N address (optional)             
  //
} MAPIAddressType;

typedef struct {
  LHANDLE   lhSession;
  ULONG     ulUIParam;
  DWORD     ipcWorked;      // Necessary for IPC check with Communicator
} MAPI_NSCP_SynchronizeClientType;

#endif    // _NSCPMAPI
