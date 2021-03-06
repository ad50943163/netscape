/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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

/*
 * The purpose of this file is to help phase out XP_ library
 * from the image library. In general, XP_ data structures and
 * functions will be replaced with the PR_ or PL_ equivalents.
 * In cases where the PR_ or PL_ equivalents don't yet exist,
 * this file (and its header equivalent) will play the role 
 * of the XP_ library.
 */
#include "xpcompat.h"
#include <stdlib.h>
/* BSDI did not have this header and we do not need it here.  -slamm */
/* #include <search.h> */
#include "prlog.h"
#include "prmem.h"
#include "plstr.h"
#include "ilISystemServices.h"

extern ilISystemServices *il_ss;

#if 0
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <limits.h>
#endif /* XP_PC */

PR_BEGIN_EXTERN_C
int MK_UNABLE_TO_LOCATE_FILE = -1;
int MK_OUT_OF_MEMORY = -2;

int XP_MSG_IMAGE_PIXELS = -7;
int XP_MSG_IMAGE_NOT_FOUND = -8;
int XP_MSG_XBIT_COLOR = -9;	
int XP_MSG_1BIT_MONO = -10;
int XP_MSG_XBIT_GREYSCALE = -11;
int XP_MSG_XBIT_RGB = -12;
int XP_MSG_DECODED_SIZE = -13;	
int XP_MSG_WIDTH_HEIGHT = -14;	
int XP_MSG_SCALED_FROM = -15;	
int XP_MSG_IMAGE_DIM = -16;	
int XP_MSG_COLOR = -17;	
int XP_MSG_NB_COLORS = -18;	
int XP_MSG_NONE = -19;	
int XP_MSG_COLORMAP = -20;	
int XP_MSG_BCKDRP_VISIBLE = -21;	
int XP_MSG_SOLID_BKGND = -22;	
int XP_MSG_JUST_NO = -23;	
int XP_MSG_TRANSPARENCY = -24;	
int XP_MSG_COMMENT = -25;	
int XP_MSG_UNKNOWN = -26;	
int XP_MSG_COMPRESS_REMOVE = -27;	
PR_END_EXTERN_C

char *XP_GetString(int i)
{
  return ("XP_GetString replacement needed");
}

/* We need this because Solaris' version of qsort is broken and
 * causes array bounds reads.
 */
#if defined(SOLARIS) || defined(XP_MAC)


/* prototypes for local routines */
static void  shortsort(char *lo, char *hi, unsigned width,
                int ( *comp)(const void *, const void *));
static void  swap(char *p, char *q, unsigned int width);

/* this parameter defines the cutoff between using quick sort and
   insertion sort for arrays; arrays with lengths shorter or equal to the
   below value use insertion sort */

#define CUTOFF 8            /* testing shows that this is good value */


/***
*XP_QSORT(base, num, wid, comp) - quicksort function for sorting arrays
*
*Purpose:
*       quicksort the array of elements
*       side effects:  sorts in place
*
*Entry:
*       char *base = pointer to base of array
*       unsigned num  = number of elements in the array
*       unsigned width = width in bytes of each array element
*       int (*comp)() = pointer to function returning analog of strcmp for
*               strings, but supplied by user for comparing the array elements.
*               it accepts 2 pointers to elements and returns neg if 1<2, 0 if
*               1=2, pos if 1>2.
*
*Exit:
*       returns void
*
*Exceptions:
*
*******************************************************************************/

/* sort the array between lo and hi (inclusive) */

void XP_QSORT (
    void *base,
    unsigned num,
    unsigned width,
    int ( *comp)(const void *, const void *)
    )
{
    char *lo, *hi;              /* ends of sub-array currently sorting */
    char *mid;                  /* points to middle of subarray */
    char *loguy, *higuy;        /* traveling pointers for partition step */
    unsigned size;              /* size of the sub-array */
    char *lostk[30], *histk[30];
    int stkptr;                 /* stack for saving sub-array to be processed */

    /* Note: the number of stack entries required is no more than
       1 + log2(size), so 30 is sufficient for any array */

    if (num < 2 || width == 0)
        return;                 /* nothing to do */

    stkptr = 0;                 /* initialize stack */

    lo = (char*)base;
    hi = (char *)base + width * (num-1);        /* initialize limits */

    /* this entry point is for pseudo-recursion calling: setting
       lo and hi and jumping to here is like recursion, but stkptr is
       prserved, locals aren't, so we preserve stuff on the stack */
recurse:

    size = (hi - lo) / width + 1;        /* number of el's to sort */

    /* below a certain size, it is faster to use a O(n^2) sorting method */
    if (size <= CUTOFF) {
         shortsort(lo, hi, width, comp);
    }
    else {
        /* First we pick a partititioning element.  The efficiency of the
           algorithm demands that we find one that is approximately the
           median of the values, but also that we select one fast.  Using
           the first one produces bad performace if the array is already
           sorted, so we use the middle one, which would require a very
           wierdly arranged array for worst case performance.  Testing shows
           that a median-of-three algorithm does not, in general, increase
           performance. */

        mid = lo + (size / 2) * width;      /* find middle element */
        swap(mid, lo, width);               /* swap it to beginning of array */

        /* We now wish to partition the array into three pieces, one
           consisiting of elements <= partition element, one of elements
           equal to the parition element, and one of element >= to it.  This
           is done below; comments indicate conditions established at every
           step. */

        loguy = lo;
        higuy = hi + width;

        /* Note that higuy decreases and loguy increases on every iteration,
           so loop must terminate. */
        for (;;) {
            /* lo <= loguy < hi, lo < higuy <= hi + 1,
               A[i] <= A[lo] for lo <= i <= loguy,
               A[i] >= A[lo] for higuy <= i <= hi */

            do  {
                loguy += width;
            } while (loguy <= hi && comp(loguy, lo) <= 0);

            /* lo < loguy <= hi+1, A[i] <= A[lo] for lo <= i < loguy,
               either loguy > hi or A[loguy] > A[lo] */

            do  {
                higuy -= width;
            } while (higuy > lo && comp(higuy, lo) >= 0);

            /* lo-1 <= higuy <= hi, A[i] >= A[lo] for higuy < i <= hi,
               either higuy <= lo or A[higuy] < A[lo] */

            if (higuy < loguy)
                break;

            /* if loguy > hi or higuy <= lo, then we would have exited, so
               A[loguy] > A[lo], A[higuy] < A[lo],
               loguy < hi, highy > lo */

            swap(loguy, higuy, width);

            /* A[loguy] < A[lo], A[higuy] > A[lo]; so condition at top
               of loop is re-established */
        }

        /*     A[i] >= A[lo] for higuy < i <= hi,
               A[i] <= A[lo] for lo <= i < loguy,
               higuy < loguy, lo <= higuy <= hi
           implying:
               A[i] >= A[lo] for loguy <= i <= hi,
               A[i] <= A[lo] for lo <= i <= higuy,
               A[i] = A[lo] for higuy < i < loguy */

        swap(lo, higuy, width);     /* put partition element in place */

        /* OK, now we have the following:
              A[i] >= A[higuy] for loguy <= i <= hi,
              A[i] <= A[higuy] for lo <= i < higuy
              A[i] = A[lo] for higuy <= i < loguy    */

        /* We've finished the partition, now we want to sort the subarrays
           [lo, higuy-1] and [loguy, hi].
           We do the smaller one first to minimize stack usage.
           We only sort arrays of length 2 or more.*/

        if ( higuy - 1 - lo >= hi - loguy ) {
            if (lo + width < higuy) {
                lostk[stkptr] = lo;
                histk[stkptr] = higuy - width;
                ++stkptr;
            }                           /* save big recursion for later */

            if (loguy < hi) {
                lo = loguy;
                goto recurse;           /* do small recursion */
            }
        }
        else {
            if (loguy < hi) {
                lostk[stkptr] = loguy;
                histk[stkptr] = hi;
                ++stkptr;               /* save big recursion for later */
            }

            if (lo + width < higuy) {
                hi = higuy - width;
                goto recurse;           /* do small recursion */
            }
        }
    }

    /* We have sorted the array, except for any pending sorts on the stack.
       Check if there are any, and do them. */

    --stkptr;
    if (stkptr >= 0) {
        lo = lostk[stkptr];
        hi = histk[stkptr];
        goto recurse;           /* pop subarray from stack */
    }
    else
        return;                 /* all subarrays done */
}


/***
*shortsort(hi, lo, width, comp) - insertion sort for sorting short arrays
*
*Purpose:
*       sorts the sub-array of elements between lo and hi (inclusive)
*       side effects:  sorts in place
*       assumes that lo < hi
*
*Entry:
*       char *lo = pointer to low element to sort
*       char *hi = pointer to high element to sort
*       unsigned width = width in bytes of each array element
*       int (*comp)() = pointer to function returning analog of strcmp for
*               strings, but supplied by user for comparing the array elements.
*               it accepts 2 pointers to elements and returns neg if 1<2, 0 if
*               1=2, pos if 1>2.
*
*Exit:
*       returns void
*
*Exceptions:
*
*******************************************************************************/

static void  shortsort (
    char *lo,
    char *hi,
    unsigned width,
    int ( *comp)(const void *, const void *)
    )
{
    char *p, *max;

    /* Note: in assertions below, i and j are alway inside original bound of
       array to sort. */

    while (hi > lo) {
        /* A[i] <= A[j] for i <= j, j > hi */
        max = lo;
        for (p = lo+width; p <= hi; p += width) {
            /* A[i] <= A[max] for lo <= i < p */
            if (comp(p, max) > 0) {
                max = p;
            }
            /* A[i] <= A[max] for lo <= i <= p */
        }

        /* A[i] <= A[max] for lo <= i <= hi */

        swap(max, hi, width);

        /* A[i] <= A[hi] for i <= hi, so A[i] <= A[j] for i <= j, j >= hi */

        hi -= width;

        /* A[i] <= A[j] for i <= j, j > hi, loop top condition established */
    }
    /* A[i] <= A[j] for i <= j, j > lo, which implies A[i] <= A[j] for i < j,
       so array is sorted */
}


/***
*swap(a, b, width) - swap two elements
*
*Purpose:
*       swaps the two array elements of size width
*
*Entry:
*       char *a, *b = pointer to two elements to swap
*       unsigned width = width in bytes of each array element
*
*Exit:
*       returns void
*
*Exceptions:
*
*******************************************************************************/

static void  swap (
    char *a,
    char *b,
    unsigned width
    )
{
    char tmp;

    if ( a != b )
        /* Do the swap one character at a time to avoid potential alignment
           problems. */
        while ( width-- ) {
            tmp = *a;
            *a++ = *b;
            *b++ = tmp;
        }
}

#endif /* SOLARIS or XP_MAC */


/*	Allocate a new copy of a block of binary data, and returns it
 */
char * 
NET_BACopy (char **destination, const char *source, size_t length)
{
	if(*destination)
	  {
	    PR_FREEIF(*destination);
		*destination = 0;
	  }

    if (! source)
	  {
        *destination = NULL;
	  }
    else 
	  {
        *destination = (char *) PR_MALLOC (length);
        if (*destination == NULL) 
	        return(NULL);
        XP_MEMCPY(*destination, source, length);
      }
    return *destination;
}

/*	binary block Allocate and Concatenate
 *
 *   destination_length  is the length of the existing block
 *   source_length   is the length of the block being added to the 
 *   destination block
 */
char * 
NET_BACat (char **destination, 
		   size_t destination_length, 
		   const char *source, 
		   size_t source_length)
{
    if (source) 
	  {
        if (*destination) 
	      {
      	    *destination = (char *) PR_REALLOC (*destination, destination_length + source_length);
            if (*destination == NULL) 
	          return(NULL);

            XP_MEMMOVE (*destination + destination_length, source, source_length);

          } 
		else 
		  {
            *destination = (char *) PR_MALLOC (source_length);
            if (*destination == NULL) 
	          return(NULL);

            XP_MEMCPY(*destination, source, source_length);
          }
    }

  return *destination;
}

/*	Very similar to strdup except it free's too
 */
char * 
NET_SACopy (char **destination, const char *source)
{
	if(*destination)
	  {
	    PR_FREEIF(*destination);
		*destination = 0;
	  }
    if (! source)
	  {
        *destination = NULL;
	  }
    else 
	  {
        *destination = (char *) PR_MALLOC (PL_strlen(source) + 1);
        if (*destination == NULL) 
 	        return(NULL);

        PL_strcpy (*destination, source);
      }
    return *destination;
}

/*  Again like strdup but it concatinates and free's and uses Realloc
*/
char *
NET_SACat (char **destination, const char *source)
{
    if (source && *source)
      {
        if (*destination)
          {
            int length = PL_strlen (*destination);
            *destination = (char *) PR_REALLOC (*destination, length + PL_strlen(source) + 1);
            if (*destination == NULL)
            return(NULL);

            PL_strcpy (*destination + length, source);
          }
        else
          {
            *destination = (char *) PR_MALLOC (PL_strlen(source) + 1);
            if (*destination == NULL)
                return(NULL);

             PL_strcpy (*destination, source);
          }
      }
    return *destination;
}

#if 0
#include <windows.h>
#include <limits.h>

static void wfe_ProcessTimeouts(DWORD dwNow);

// structure to keep track of FE_SetTimeOut objects
typedef struct WinTimeStruct {
    TimeoutCallbackFunction   fn;
    void                    * closure;
    DWORD                     dwFireTime;
    struct WinTimeStruct    * pNext;
} WinTime;

// the one and only list of objects waiting for FE_SetTimeOut
WinTime *gTimeOutList = NULL;

//  Process timeouts now.
//  Called once per round of fire.
UINT uTimeoutTimer = 0;
DWORD dwNextFire = (DWORD)-1;
DWORD dwSyncHack = 0;

void CALLBACK FireTimeout(HWND hWnd, UINT uMessage, UINT uTimerID, DWORD dwTime)
{
    static BOOL bCanEnter = TRUE;

    //  Don't allow old timer messages in here.
    if(uMessage != WM_TIMER)    {
        PR_ASSERT(0);
        return;
    }
    if(uTimerID != uTimeoutTimer)   {
        return;
    }

    //  Block only one entry into this function, or else.
    if(bCanEnter)   {
        bCanEnter = FALSE;
        // see if we need to fork off any timeout functions
        if(gTimeOutList)    {
            wfe_ProcessTimeouts(dwTime);
        }
        bCanEnter = TRUE;
    }
}

//  Function to correctly have the timer be set.
void SyncTimeoutPeriod(DWORD dwTickCount)
{
    //  May want us to set tick count ourselves.
    if(dwTickCount == 0)    {
        if(dwSyncHack == 0) {
            dwTickCount = GetTickCount();
        }
        else    {
            dwTickCount = dwSyncHack;
        }
    }

    //  If there's no list, we should clear the timer.
    if(!gTimeOutList)    {
        if(uTimeoutTimer)   {
            ::KillTimer(NULL, uTimeoutTimer);
            uTimeoutTimer = 0;
            dwNextFire = (DWORD)-1;
        }
    }
    else {
        //  See if we need to clear the current timer.
        //  Curcumstances are that if the timer will not
        //      fire on time for the next timeout.
        BOOL bSetTimer = FALSE;
        WinTime *pTimeout = gTimeOutList;
        if(uTimeoutTimer)   {
            if(pTimeout->dwFireTime != dwNextFire)   {
                //  There is no need to kill the timer if we are just going to set it again.
                //  Windows will simply respect the new time interval passed in.
                ::KillTimer(NULL, uTimeoutTimer);
                uTimeoutTimer = 0;
                dwNextFire = (DWORD)-1;

                //  Set the timer.
                bSetTimer = TRUE;
            }
        }
        else    {
            //  No timer set, attempt.
            bSetTimer = TRUE;
        }

        if(bSetTimer)   {
            DWORD dwFireWhen = pTimeout->dwFireTime > dwTickCount ?
                pTimeout->dwFireTime - dwTickCount : 0;
            UINT uFireWhen = (UINT)dwFireWhen;

            PR_ASSERT(uTimeoutTimer == 0);
            uTimeoutTimer = ::SetTimer(
                NULL, 
                0, 
                uFireWhen, 
                (TIMERPROC)FireTimeout);

            if(uTimeoutTimer)   {
                //  Set the fire time.
                dwNextFire = pTimeout->dwFireTime;
            }
        }
    }
}

/* this function should register a function that will
 * be called after the specified interval of time has
 * elapsed.  This function should return an id 
 * that can be passed to FE_ClearTimeout to cancel 
 * the Timeout request.
 *
 * A) Timeouts never fail to trigger, and
 * B) Timeouts don't trigger *before* their nominal timestamp expires, and
 * C) Timeouts trigger in the same ordering as their timestamps
 *
 * After the function has been called it is unregistered
 * and will not be called again unless re-registered.
 *
 * func:    The function to be invoked upon expiration of
 *          the Timeout interval 
 * closure: Data to be passed as the only argument to "func"
 * msecs:   The number of milli-seconds in the interval
 */
void * 
FE_SetTimeout(TimeoutCallbackFunction func, void * closure, uint32 msecs)
{
    WinTime * pTime = new WinTime;
    if(!pTime)
        return(NULL);

    // fill it out
    DWORD dwNow = GetTickCount();
    pTime->fn = func;
    pTime->closure = closure;
    pTime->dwFireTime = (DWORD) msecs + dwNow;
    //CLM: Always clear this else the last timer added will have garbage!
    //     (Win16 revealed this bug!)
    pTime->pNext = NULL;
    
    // add it to the list
    if(!gTimeOutList) {        
        // no list add it
        gTimeOutList = pTime;
    } else {

        // is it before everything else on the list?
        if(pTime->dwFireTime < gTimeOutList->dwFireTime) {

            pTime->pNext = gTimeOutList;
            gTimeOutList = pTime;

        } else {

            WinTime * pPrev = gTimeOutList;
            WinTime * pCurrent = gTimeOutList;

            while(pCurrent && (pCurrent->dwFireTime <= pTime->dwFireTime)) {
                pPrev = pCurrent;
                pCurrent = pCurrent->pNext;
            }

            PR_ASSERT(pPrev);

            // insert it after pPrev (this could be at the end of the list)
            pTime->pNext = pPrev->pNext;
            pPrev->pNext = pTime;

        }

    }

    //  Sync the timer fire period.
    SyncTimeoutPeriod(dwNow);

    return(pTime);
}


/* This function cancels a Timeout that has previously been
 * set.  
 * Callers should not pass in NULL or a timer_id that
 * has already expired.
 */
void 
FE_ClearTimeout(void * pStuff)
{
    WinTime * pTimer = (WinTime *) pStuff;

    if(!pTimer) {
        return;
    }

    if(gTimeOutList == pTimer) {

        // first element in the list lossage
        gTimeOutList = pTimer->pNext;

    } else {

        // walk until no next pointer
        for(WinTime * p = gTimeOutList; p && p->pNext && (p->pNext != pTimer); p = p->pNext)
            ;

        // if we found something valid pull it out of the list
        if(p && p->pNext && p->pNext == pTimer) {
            p->pNext = pTimer->pNext;

        } else {
            // get out before we delete something that looks bogus
            return;
        }

    }

    // if we got here it must have been a valid element so trash it
    delete pTimer;

    //  If there's now no be sure to clear the timer.
    SyncTimeoutPeriod(0);
}

//
// Walk down the timeout list and launch anyone appropriate
// Cleaned up logic 04-30-96 GAB
//
static void wfe_ProcessTimeouts(DWORD dwNow)
{
    WinTime *p = gTimeOutList;
    if(dwNow == 0)   {
        dwNow = GetTickCount();
    }

    BOOL bCalledSync = FALSE;

    //  Set the hack, such that when FE_ClearTimeout
    //      calls SyncTimeoutPeriod, that GetTickCount()
    //      overhead is not incurred.
    dwSyncHack = dwNow;
    
    // loop over all entries
    while(p) {
        // send it
        if(p->dwFireTime < dwNow) {
            //  Fire it.
            (*p->fn) (p->closure);

            //  Clear the timer.
            //  Period synced.
            FE_ClearTimeout(p);
            bCalledSync = TRUE;

            //  Reset the loop (can't look at p->pNext now, and called
            //      code may have added/cleared timers).
            //  (could do this by going recursive and returning).
            p = gTimeOutList;
        } else {
			//	Make sure we fire an timer.
            //  Also, we need to check to see if things are backing up (they
            //      may be asking to be fired long before we ever get to them,
            //      and we don't want to pass in negative values to the real
            //      timer code, or it takes days to fire....
            if(bCalledSync == FALSE)    {
                SyncTimeoutPeriod(dwNow);
                bCalledSync = TRUE;
            }
            //  Get next timer.
            p = p->pNext;
        }
    }
    dwSyncHack = 0;
}
#else
void * 
FE_SetTimeout(TimeoutCallbackFunction func, void * closure, uint32 msecs)
{
    return il_ss->SetTimeout((ilTimeoutCallbackFunction)func,
                             closure, msecs);
}

void 
FE_ClearTimeout(void *timer_id)
{
    il_ss->ClearTimeout(timer_id);
}
#endif /* XP_PC */

