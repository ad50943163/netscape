/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
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

#include "nspr.h"
#include "prpriv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#endif

static int _debug_on = 0;

#ifdef XP_MAC
#include "prlog.h"
#include "primpl.h"
#define printf PR_LogPrint
#define setbuf(x,y)
extern void SetupMacPrintfLog(char *logFile);
#endif

#ifdef XP_PC
#define mode_t int
#endif

#define DPRINTF(arg) if (_debug_on) printf arg

PRLock *lock;
PRMonitor *mon;
PRMonitor *mon2;
PRInt32 count;
int thread_count;

#ifdef WIN16
#define	BUF_DATA_SIZE	256 * 120
#else
#define	BUF_DATA_SIZE	256 * 1024
#endif

#define NUM_RDWR_THREADS 10
#define CHUNK_SIZE 512

typedef struct buffer {
	char	data[BUF_DATA_SIZE];
} buffer;

typedef struct File_Rdwr_Param {
	char	*pathname;
	char	*buf;
	int	offset;
	int	len;
} File_Rdwr_Param;

#ifdef XP_PC
char *TEST_DIR = "C:\\temp\\prdir";
char *FILE_NAME = "pr_testfile";
char *HIDDEN_FILE_NAME = "hidden_pr_testfile";
#else
char *TEST_DIR = "/tmp/testfile_dir";
char *FILE_NAME = "pr_testfile";
char *HIDDEN_FILE_NAME = ".hidden_pr_testfile";
#endif
buffer *in_buf, *out_buf;
char pathname[256], renamename[256];

static void PR_CALLBACK File_Write(void *arg)
{
PRFileDesc *fd_file;
File_Rdwr_Param *fp = (File_Rdwr_Param *) arg;
char *name, *buf;
int offset, len;

	setbuf(stdout, NULL);
	name = fp->pathname;
	buf = fp->buf;
	offset = fp->offset;
	len = fp->len;
	
	fd_file = PR_Open(name, PR_RDWR | PR_CREATE_FILE, 0777);
	if (fd_file == NULL) {
		printf("testfile failed to create/open file %s\n",name);
		return;
	}
	if (PR_Seek(fd_file, offset, PR_SEEK_SET) < 0) {
		printf("testfile failed to seek in file %s\n",name);
		return;
	}	
	if ((PR_Write(fd_file, buf, len)) < 0) {
		printf("testfile failed to write to file %s\n",name);
		return;
	}	
	DPRINTF(("Write out_buf[0] = 0x%x\n",(*((int *) buf))));
	PR_Close(fd_file);

	PR_EnterMonitor(mon2);
	--thread_count;
	PR_Notify(mon2);
	PR_ExitMonitor(mon2);
}

static void PR_CALLBACK File_Read(void *arg)
{
PRFileDesc *fd_file;
File_Rdwr_Param *fp = (File_Rdwr_Param *) arg;
char *name, *buf;
int offset, len;

	setbuf(stdout, NULL);
	name = fp->pathname;
	buf = fp->buf;
	offset = fp->offset;
	len = fp->len;
	
	fd_file = PR_Open(name, PR_RDONLY, 0);
	if (fd_file == NULL) {
		printf("testfile failed to open file %s\n",name);
		return;
	}
	if (PR_Seek(fd_file, offset, PR_SEEK_SET) < 0) {
		printf("testfile failed to seek in file %s\n",name);
		return;
	}	
	if ((PR_Read(fd_file, buf, len)) < 0) {
		printf("testfile failed to read to file %s\n",name);
		return;
	}	
	DPRINTF(("Read in_buf[0] = 0x%x\n",(*((int *) buf))));
	PR_Close(fd_file);

	PR_EnterMonitor(mon2);
	--thread_count;
	PR_Notify(mon2);
	PR_ExitMonitor(mon2);
}


static void Misc_File_Tests(char *pathname)
{
PRFileDesc *fd_file;
int len;
PRFileInfo file_info, file_info1;
char tmpname[1024];

	setbuf(stdout, NULL);
	/*
	 * Test PR_Available, PR_Seek, PR_GetFileInfo, PR_Rename, PR_Access
	 */

	fd_file = PR_Open(pathname, PR_RDWR | PR_CREATE_FILE, 0777);

	if (fd_file == NULL) {
		printf("testfile failed to create/open file %s\n",pathname);
		return;
	}
	if (PR_GetOpenFileInfo(fd_file, &file_info) < 0) {
		printf("testfile PR_GetFileInfo failed on file %s\n",pathname);
		goto cleanup;
	}
	if (PR_Access(pathname, PR_ACCESS_EXISTS) != 0) {
		printf("testfile PR_Access failed on file %s\n",pathname);
		goto cleanup;
	}
	if (PR_Access(pathname, PR_ACCESS_WRITE_OK) != 0) {
		printf("testfile PR_Access failed on file %s\n",pathname);
		goto cleanup;
	}
	if (PR_Access(pathname, PR_ACCESS_READ_OK) != 0) {
		printf("testfile PR_Access failed on file %s\n",pathname);
		goto cleanup;
	}


	if (PR_GetFileInfo(pathname, &file_info) < 0) {
		printf("testfile PR_GetFileInfo failed on file %s\n",pathname);
		return;
	}
	if (file_info.type != PR_FILE_FILE) {
		printf(
		"testfile PR_GetFileInfo returned incorrect type for file %s\n",
		pathname);
		goto cleanup;
	}
	if (file_info.size != 0) {
		printf(
		"testfile PR_GetFileInfo returned incorrect size (%d should be 0) for file %s\n",
		file_info.size, pathname);
		goto cleanup;
	}
	file_info1 = file_info;

	len = PR_Available(fd_file);
	if (len < 0) {
		printf("testfile PR_Available failed on file %s\n",pathname);
		goto cleanup;
	} else if (len != 0) {
		printf(
		"testfile PR_Available failed: expected/returned = %d/%d bytes\n",
			0, len);
		goto cleanup;
	}
	len = PR_Write(fd_file, out_buf->data, CHUNK_SIZE);
	if (len < 0) {
		printf("testfile failed to write to file %s\n",pathname);
		goto cleanup;
	}
	if (PR_GetOpenFileInfo(fd_file, &file_info) < 0) {
		printf("testfile PR_GetFileInfo failed on file %s\n",pathname);
		goto cleanup;
	}
	if (file_info.size != CHUNK_SIZE) {
		printf(
		"testfile PR_GetFileInfo returned incorrect size (%d should be %d) for file %s\n",
		file_info.size, CHUNK_SIZE, pathname);
		goto cleanup;
	}
	if (LL_NE(file_info.creationTime , file_info1.creationTime)) {
		printf(
		"testfile PR_GetFileInfo returned incorrect creation time: %s\n",
		pathname);
		goto cleanup;
	}
	if (LL_CMP(file_info.modifyTime, > , file_info1.modifyTime)) {
		printf(
		"testfile PR_GetFileInfo returned incorrect modify time: %s\n",
		pathname);
		goto cleanup;
	}

	len = PR_Available(fd_file);
	if (len < 0) {
		printf("testfile PR_Available failed on file %s\n",pathname);
		goto cleanup;
	} else if (len != 0) {
		printf(
		"testfile PR_Available failed: expected/returned = %d/%d bytes\n",
			0, len);
		goto cleanup;
	}
	
	PR_Seek(fd_file, 0, PR_SEEK_SET);
	len = PR_Available(fd_file);
	if (len < 0) {
		printf("testfile PR_Available failed on file %s\n",pathname);
		return;
	} else if (len != CHUNK_SIZE) {
		printf(
		"testfile PR_Available failed: expected/returned = %d/%d bytes\n",
			CHUNK_SIZE, len);
		goto cleanup;
	}
    PR_Close(fd_file);

	strcpy(tmpname,pathname);
	strcat(tmpname,".RENAMED");
	if (PR_FAILURE == PR_Rename(pathname, tmpname)) {
		printf("testfile failed to rename file %s\n",pathname);
		goto cleanup;
	}

	fd_file = PR_Open(pathname, PR_RDWR | PR_CREATE_FILE, 0777);
	len = PR_Write(fd_file, out_buf->data, CHUNK_SIZE);
    PR_Close(fd_file);
	if (PR_SUCCESS == PR_Rename(pathname, tmpname)) {
		printf("testfile renamed to existing file %s\n",pathname);
	}

	if ((PR_Delete(tmpname)) < 0)
		printf("testfile failed to unlink file %s\n",tmpname);

cleanup:
	if ((PR_Delete(pathname)) < 0)
		printf("testfile failed to unlink file %s\n",pathname);
}


static PRInt32 PR_CALLBACK FileTest(void)
{
PRDir *fd_dir;
int i, offset, len;
PRThread *t;
File_Rdwr_Param *fparamp;

	/*
	 * Create Test dir
	 */
	if ((PR_MkDir(TEST_DIR, 0777)) < 0) {
		printf("testfile failed to create dir %s\n",TEST_DIR);
		return -1;
	}
	fd_dir = PR_OpenDir(TEST_DIR);
	if (fd_dir == NULL) {
		printf("testfile failed to open dir %s\n",TEST_DIR);
		return -1;
	}

    PR_CloseDir(fd_dir);

	strcat(pathname, TEST_DIR);
	strcat(pathname, "/");
	strcat(pathname, FILE_NAME);

	in_buf = PR_NEW(buffer);
	if (in_buf == NULL) {
		printf(
		"testfile failed to alloc buffer struct\n");
		return -1;
	}
	out_buf = PR_NEW(buffer);
	if (out_buf == NULL) {
		printf(
		"testfile failed to alloc buffer struct\n");
		return -1;
	}

	/*
	 * Start a bunch of writer threads
	 */
	offset = 0;
	len = CHUNK_SIZE;
	mon2 = PR_NewMonitor();
	if (mon2 == NULL) {
		printf("testfile: PR_NewMonitor failed\n");
		return -1;
	}
	PR_EnterMonitor(mon2);
	for (i = 0; i < NUM_RDWR_THREADS; i++) {
		fparamp = PR_NEW(File_Rdwr_Param);
		if (fparamp == NULL) {
			printf(
			"testfile failed to alloc File_Rdwr_Param struct\n");
			return -1;
		}
		fparamp->pathname = pathname;
		fparamp->buf = out_buf->data + offset;
		fparamp->offset = offset;
		fparamp->len = len;
		memset(fparamp->buf, i, len);
		/*
		 * Create LOCAL and GLOBAL Threads, alternately
		 */
		if (i % 1)
			t = PR_CreateThread(PR_USER_THREAD,
				      File_Write, (void *)fparamp, 
				      PR_PRIORITY_NORMAL,
				      PR_GLOBAL_THREAD,
    				  PR_UNJOINABLE_THREAD,
				      0);
		else
			t = PR_CreateThread(PR_USER_THREAD,
				      File_Write, (void *)fparamp, 
				      PR_PRIORITY_NORMAL,
				      PR_LOCAL_THREAD,
    				  PR_UNJOINABLE_THREAD,
				      0);
		offset += len;
	}
	thread_count = i;
	/* Wait for writer threads to exit */
	while (thread_count) {
		PR_Wait(mon2, PR_INTERVAL_NO_TIMEOUT);
	}
	PR_ExitMonitor(mon2);


	/*
	 * Start a bunch of reader threads
	 */
	offset = 0;
	len = CHUNK_SIZE;
	PR_EnterMonitor(mon2);
	for (i = 0; i < NUM_RDWR_THREADS; i++) {
		fparamp = PR_NEW(File_Rdwr_Param);
		if (fparamp == NULL) {
			printf(
			"testfile failed to alloc File_Rdwr_Param struct\n");
			return -1;
		}
		fparamp->pathname = pathname;
		fparamp->buf = in_buf->data + offset;
		fparamp->offset = offset;
		fparamp->len = len;
		/*
		 * Create LOCAL and GLOBAL Threads, alternately
		 */
		if (i % 1)
			t = PR_CreateThread(PR_USER_THREAD,
				      File_Read, (void *)fparamp, 
				      PR_PRIORITY_NORMAL,
				      PR_LOCAL_THREAD,
    				  PR_UNJOINABLE_THREAD,
				      0);
		else
			t = PR_CreateThread(PR_USER_THREAD,
				      File_Read, (void *)fparamp, 
				      PR_PRIORITY_NORMAL,
				      PR_GLOBAL_THREAD,
    				  PR_UNJOINABLE_THREAD,
				      0);
		offset += len;
		if ((offset + len) > BUF_DATA_SIZE)
			break;
	}
	thread_count = i;

	/* Wait for reader threads to exit */
	while (thread_count) {
		PR_Wait(mon2, PR_INTERVAL_NO_TIMEOUT);
	}
	PR_ExitMonitor(mon2);

	if (memcmp(in_buf->data, out_buf->data, offset) != 0) {
		printf("File Test failed: file data corrupted\n");
	}

	if ((PR_Delete(pathname)) < 0) {
		printf("testfile failed to unlink file %s\n",pathname);
		return -1;
	}

	/*
	 * Test PR_Available, PR_Seek, PR_GetFileInfo, PR_Rename, PR_Access
	 */
	Misc_File_Tests(pathname);

	if ((PR_RmDir(TEST_DIR)) < 0) {
		printf("testfile failed to rmdir %s\n", TEST_DIR);
		return -1;
	}
	return 0;
}

static PRInt32 PR_CALLBACK DirTest(void)
{
PRFileDesc *fd_file;
PRDir *fd_dir;
int i;
int path_len;
PRDirEntry *dirEntry;
PRFileInfo info;
PRInt32 num_files = 0;
#if defined(XP_PC) && defined(WIN32)
HANDLE hfile;
#endif

#define  FILES_IN_DIR 20

	/*
	 * Create Test dir
	 */
	DPRINTF(("Creating test dir %s\n",TEST_DIR));
	if ((PR_MkDir(TEST_DIR, 0777)) < 0) {
		printf(
			"testfile failed to create dir %s [%d, %d]\n",
			TEST_DIR, PR_GetError(), PR_GetOSError());
		return -1;
	}
	fd_dir = PR_OpenDir(TEST_DIR);
	if (fd_dir == NULL) {
		printf(
			"testfile failed to open dirctory %s [%d, %d]\n",
			TEST_DIR, PR_GetError(), PR_GetOSError());
		return -1;
	}

	strcpy(pathname, TEST_DIR);
	strcat(pathname, "/");
	strcat(pathname, FILE_NAME);
	path_len = strlen(pathname);
	
	for (i = 0; i < FILES_IN_DIR; i++) {

		sprintf(pathname + path_len,"%d%s",i,"");

		DPRINTF(("Creating test file %s\n",pathname));

		fd_file = PR_Open(pathname, PR_RDWR | PR_CREATE_FILE, 0777);

		if (fd_file == NULL) {
			printf(
					"testfile failed to create/open file %s [%d, %d]\n",
					pathname, PR_GetError(), PR_GetOSError());
			return -1;
		}
        PR_Close(fd_file);
	}
#if defined(XP_UNIX) || defined(XP_MAC) || (defined(XP_PC) && defined(WIN32))
	/*
	 * Create a hidden file - a platform-dependent operation
	 */
	strcpy(pathname, TEST_DIR);
	strcat(pathname, "/");
	strcat(pathname, HIDDEN_FILE_NAME);
#if defined(XP_UNIX) || defined(XP_MAC)
	DPRINTF(("Creating hidden test file %s\n",pathname));
	fd_file = PR_Open(pathname, PR_RDWR | PR_CREATE_FILE, 0777);

	if (fd_file == NULL) {
		printf(
				"testfile failed to create/open hidden file %s [%d, %d]\n",
				pathname, PR_GetError(), PR_GetOSError());
		return -1;
	}

#if defined(XP_MAC)
	{
#include <files.h>

	OSErr			err;
	FCBPBRec		fcbpb;
	CInfoPBRec		pb;
	Str255			pascalMacPath;

	fcbpb.ioNamePtr = pascalMacPath;
	fcbpb.ioVRefNum = 0;
	fcbpb.ioRefNum = fd_file->secret->md.osfd;
	fcbpb.ioFCBIndx = 0;
	
	err = PBGetFCBInfoSync(&fcbpb);
	if (err != noErr) {
    	PR_Close(fd_file);
    	return -1;
	}
	
	pb.hFileInfo.ioNamePtr = pascalMacPath;
	pb.hFileInfo.ioVRefNum = fcbpb.ioFCBVRefNum;
	pb.hFileInfo.ioDirID = fcbpb.ioFCBParID;
	pb.hFileInfo.ioFDirIndex = 0;
	
	err = PBGetCatInfoSync(&pb);
	if (err != noErr) {
    	PR_Close(fd_file);
    	return -1;
	}

	pb.hFileInfo.ioNamePtr = pascalMacPath;
	pb.hFileInfo.ioVRefNum = fcbpb.ioFCBVRefNum;
	pb.hFileInfo.ioDirID = fcbpb.ioFCBParID;
	pb.hFileInfo.ioFDirIndex = 0;
	
	pb.hFileInfo.ioFlFndrInfo.fdFlags |= fInvisible;

	err = PBSetCatInfoSync(&pb);
	if (err != noErr) {
    	PR_Close(fd_file);
    	return -1;
	}

	}
#endif

    PR_Close(fd_file);

	
#elif defined(XP_PC) && defined(WIN32)
	DPRINTF(("Creating hidden test file %s\n",pathname));
	hfile = CreateFile(pathname, GENERIC_READ,
						FILE_SHARE_READ|FILE_SHARE_WRITE,
						NULL,
						CREATE_NEW,
						FILE_ATTRIBUTE_HIDDEN,
						NULL);
	if (hfile == INVALID_HANDLE_VALUE) {
		printf("testfile failed to create/open hidden file %s [0, %d]\n",
				pathname, GetLastError());
		return -1;
	}
	CloseHandle(hfile);
						
#endif	/* XP _UNIX || XP_MAC*/

#endif	/* XP_UNIX || XP_MAC ||(XP_PC && WIN32) */


	if (PR_FAILURE == PR_CloseDir(fd_dir))
	{
		printf(
			"testfile failed to close dirctory %s [%d, %d]\n",
			TEST_DIR, PR_GetError(), PR_GetOSError());
		return -1;
	}
	fd_dir = PR_OpenDir(TEST_DIR);
	if (fd_dir == NULL) {
		printf(
			"testfile failed to reopen dirctory %s [%d, %d]\n",
			TEST_DIR, PR_GetError(), PR_GetOSError());
		return -1;
	}
  
	/*
	 * List all files, including hidden files
	 */
	DPRINTF(("Listing all files in directory %s\n",TEST_DIR));
#if defined(XP_UNIX) || defined(XP_MAC) || (defined(XP_PC) && defined(WIN32))
	num_files = FILES_IN_DIR + 1;
#else
	num_files = FILES_IN_DIR;
#endif
	while ((dirEntry = PR_ReadDir(fd_dir, PR_SKIP_BOTH)) != NULL) {
		num_files--;
		strcpy(pathname, TEST_DIR);
		strcat(pathname, "/");
		strcat(pathname, dirEntry->name);
		DPRINTF(("\t%s\n",dirEntry->name));

		if ((PR_GetFileInfo(pathname, &info)) < 0) {
			printf(
				"testfile failed to GetFileInfo file %s [%d, %d]\n",
				pathname, PR_GetError(), PR_GetOSError());
			return -1;
		}
		
		if (info.type != PR_FILE_FILE) {
			printf(
				"testfile incorrect fileinfo for file %s [%d, %d]\n",
				pathname, PR_GetError(), PR_GetOSError());
			return -1;
		}
	}
	if (num_files != 0)
	{
		printf(
			"testfile failed to find all files in directory %s [%d, %d]\n",
			TEST_DIR, PR_GetError(), PR_GetOSError());
		return -1;
	}

    PR_CloseDir(fd_dir);

#if defined(XP_UNIX) || defined(XP_MAC) || (defined(XP_PC) && defined(WIN32))

	/*
	 * List all files, except hidden files
	 */

	fd_dir = PR_OpenDir(TEST_DIR);
	if (fd_dir == NULL) {
		printf(
			"testfile failed to reopen dirctory %s [%d, %d]\n",
			TEST_DIR, PR_GetError(), PR_GetOSError());
		return -1;
	}
  
	DPRINTF(("Listing non-hidden files in directory %s\n",TEST_DIR));
	while ((dirEntry = PR_ReadDir(fd_dir, PR_SKIP_HIDDEN)) != NULL) {
		DPRINTF(("\t%s\n",dirEntry->name));
		if (!strcmp(HIDDEN_FILE_NAME, dirEntry->name)) {
			printf("testfile found hidden file %s\n", pathname);
			return -1;
		}

	}
	/*
	 * Delete hidden file
	 */
	strcpy(pathname, TEST_DIR);
	strcat(pathname, "/");
	strcat(pathname, HIDDEN_FILE_NAME);
	if (PR_FAILURE == PR_Delete(pathname)) {
		printf(
			"testfile failed to delete hidden file %s [%d, %d]\n",
			pathname, PR_GetError(), PR_GetOSError());
		return -1;
	}

    PR_CloseDir(fd_dir);
#endif	/* XP_UNIX || XP_MAC || (XP_PC && WIN32) */

	strcpy(renamename, TEST_DIR);
	strcat(renamename, ".RENAMED");
	if (PR_FAILURE == PR_Rename(TEST_DIR, renamename)) {
		printf(
			"testfile failed to rename directory %s [%d, %d]\n",
			TEST_DIR, PR_GetError(), PR_GetOSError());
		return -1;
	}
    
	if (PR_FAILURE == PR_MkDir(TEST_DIR, 0777)) {
		printf(
			"testfile failed to recreate dir %s [%d, %d]\n",
			TEST_DIR, PR_GetError(), PR_GetOSError());
		return -1;
	}
	if (PR_SUCCESS == PR_Rename(renamename, TEST_DIR)) {
		printf(
			"testfile renamed directory to existing name %s\n",
			renamename);
		return -1;
	}

	if (PR_FAILURE == PR_RmDir(TEST_DIR)) {
		printf(
			"testfile failed to rmdir %s [%d, %d]\n",
			TEST_DIR, PR_GetError(), PR_GetOSError());
		return -1;
	}

	if (PR_FAILURE == PR_Rename(renamename, TEST_DIR)) {
		printf(
			"testfile failed to rename directory %s [%d, %d]\n",
			renamename, PR_GetError(), PR_GetOSError());
		return -1;
	}
	fd_dir = PR_OpenDir(TEST_DIR);
	if (fd_dir == NULL) {
		printf(
			"testfile failed to reopen directory %s [%d, %d]\n",
			TEST_DIR, PR_GetError(), PR_GetOSError());
		return -1;
	}

	strcpy(pathname, TEST_DIR);
	strcat(pathname, "/");
	strcat(pathname, FILE_NAME);
	path_len = strlen(pathname);
	
	for (i = 0; i < FILES_IN_DIR; i++) {

		sprintf(pathname + path_len,"%d%s",i,"");

		if (PR_FAILURE == PR_Delete(pathname)) {
			printf(
				"testfile failed to delete file %s [%d, %d]\n",
				pathname, PR_GetError(), PR_GetOSError());
			return -1;
		}
	}

    PR_CloseDir(fd_dir);

	if (PR_FAILURE == PR_RmDir(TEST_DIR)) {
		printf(
			"testfile failed to rmdir %s [%d, %d]\n",
			TEST_DIR, PR_GetError(), PR_GetOSError());
		return -1;
	}

	return 0;
}
/************************************************************************/

/*
 * Test file and directory NSPR APIs
 */

void main(int argc, char **argv)
{
#ifdef XP_UNIX
        int opt;
        extern char *optarg;
	extern int optind;
#endif
#ifdef XP_UNIX
        while ((opt = getopt(argc, argv, "d")) != EOF) {
                switch(opt) {
                        case 'd':
                                _debug_on = 1;
                                break;
                        default:
                                break;
                }
        }
#endif
	PR_Init(PR_USER_THREAD, PR_PRIORITY_NORMAL, 0);
    PR_STDIO_INIT();

#ifdef XP_MAC
	SetupMacPrintfLog("testfile.log");
#endif

	mon2 = PR_NewMonitor();

	if (FileTest() < 0) {
		printf("File Test failed\n");
		exit(2);
	}
	printf("File Test passed\n");

	if (DirTest() < 0) {
		printf("Dir Test failed\n");
		exit(2);
	}
	printf("Dir Test passed\n");

	PR_Cleanup();
}
