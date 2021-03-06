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

#ifndef nspr_solaris_defs_h___
#define nspr_solaris_defs_h___

/*
 * Internal configuration macros
 */

#define PR_LINKER_ARCH	"solaris"
#define _PR_SI_SYSNAME	"SOLARIS"
#define _PR_SI_ARCHITECTURE	"sparc"
#define PR_DLL_SUFFIX		".so"

#define _PR_VMBASE		0x30000000
#define _PR_STACK_VMBASE	0x50000000
#define _MD_DEFAULT_STACK_SIZE	(2*65536L)
#define _MD_MMAP_FLAGS          MAP_SHARED

#undef  HAVE_STACK_GROWING_UP

#ifndef HAVE_WEAK_IO_SYMBOLS
#define	HAVE_WEAK_IO_SYMBOLS
#endif

#undef	HAVE_WEAK_MALLOC_SYMBOLS
#define	HAVE_DLL
#define	USE_DLFCN
#define NEED_STRFTIME_LOCK

#ifdef _PR_LOCAL_THREADS_ONLY
#undef _PR_HAVE_ATOMIC_OPS
#else
#define _PR_HAVE_ATOMIC_OPS
#endif

PR_EXTERN(PRIntervalTime) _MD_Solaris_GetInterval(void);
#define _MD_GET_INTERVAL                  _MD_Solaris_GetInterval
PR_EXTERN(PRIntervalTime) _MD_Solaris_TicksPerSecond(void);
#define _MD_INTERVAL_PER_SEC              _MD_Solaris_TicksPerSecond

#if defined(_PR_HAVE_ATOMIC_OPS)
/*
** Atomic Operations
*/
#define _MD_INIT_ATOMIC()

PR_EXTERN(PRInt32) _MD_AtomicIncrement(PRInt32 *val);
#define _MD_ATOMIC_INCREMENT _MD_AtomicIncrement

PR_EXTERN(PRInt32) _MD_AtomicDecrement(PRInt32 *val);
#define _MD_ATOMIC_DECREMENT _MD_AtomicDecrement

PR_EXTERN(PRInt32) _MD_AtomicSet(PRInt32 *val, PRInt32 newval);
#define _MD_ATOMIC_SET _MD_AtomicSet
#endif /* _PR_HAVE_ATOMIC_OPS */

#if defined(_PR_PTHREADS)

PR_EXTERN(void)		_MD_EarlyInit(void);

#define _MD_EARLY_INIT		_MD_EarlyInit
#define _MD_FINAL_INIT		_PR_UnixInit

#elif defined(_PR_GLOBAL_THREADS_ONLY)

#include "prthread.h"

#include <ucontext.h>

/*
** Iinitialization Related definitions
*/

PR_EXTERN(void)		_MD_EarlyInit(void);

#define _MD_EARLY_INIT		_MD_EarlyInit
#define _MD_FINAL_INIT		_PR_UnixInit

#define _MD_GET_SP(threadp)	threadp->md.sp

/*
** Clean-up the thread machine dependent data structure
*/
#define	_MD_INIT_THREAD				_MD_InitializeThread
#define	_MD_INIT_ATTACHED_THREAD	_MD_InitializeThread

PR_EXTERN(PRStatus) _MD_CreateThread(PRThread *thread, 
					void (*start)(void *), 
					PRThreadPriority priority,
					PRThreadScope scope, 
					PRThreadState state, 
					PRUint32 stackSize);
#define _MD_CREATE_THREAD _MD_CreateThread

#define	_PR_CONTEXT_TYPE	ucontext_t

#define CONTEXT(_thread) (&(_thread)->md.context)

#include <thread.h>
#include <sys/lwp.h>
#include <synch.h>

extern struct PRLock *_pr_schedLock;

/*
** Thread Local Storage 
*/

#define THREAD_KEY_T thread_key_t

extern struct PRThread *_pr_current_thread_tls();
extern struct _PRCPU *_pr_current_cpu_tls();
extern struct PRThread *_pr_last_thread_tls();

extern THREAD_KEY_T threadid_key;
extern THREAD_KEY_T cpuid_key;
extern THREAD_KEY_T last_thread_key;

#define _MD_CURRENT_THREAD() _pr_current_thread_tls()
#define _MD_CURRENT_CPU() _pr_current_cpu_tls()
#define _MD_LAST_THREAD() _pr_last_thread_tls()
	
#define _MD_SET_CURRENT_THREAD(newval) 			\
	PR_BEGIN_MACRO					\
	thr_setspecific(threadid_key, (void *)newval);	\
	PR_END_MACRO

#define _MD_SET_CURRENT_CPU(newval) 			\
	PR_BEGIN_MACRO					\
	thr_setspecific(cpuid_key, (void *)newval);	\
	PR_END_MACRO

#define _MD_SET_LAST_THREAD(newval)	 			\
	PR_BEGIN_MACRO						\
	thr_setspecific(last_thread_key, (void *)newval);	\
	PR_END_MACRO
	
#define	_MD_CLEAN_THREAD(_thread)	_MD_cleanup_thread(_thread)
extern void _MD_exit_thread(PRThread *thread);
#define _MD_EXIT_THREAD(thread)		_MD_exit_thread(thread)

#define	_MD_SUSPEND_THREAD(thread)	_MD_Suspend(thread)
#define	_MD_RESUME_THREAD(thread)	thr_continue((thread)->md.handle)

/* XXXX Needs to be defined - Prashant */
#define _MD_SUSPEND_CPU(cpu)
#define _MD_RESUME_CPU(cpu)

extern void _MD_Begin_SuspendAll(void);
extern void _MD_End_SuspendAll(void);
extern void _MD_End_ResumeAll(void);
#define _MD_BEGIN_SUSPEND_ALL()		_MD_Begin_SuspendAll()
#define _MD_BEGIN_RESUME_ALL()		
#define	_MD_END_SUSPEND_ALL()		_MD_End_SuspendAll()
#define	_MD_END_RESUME_ALL()		_MD_End_ResumeAll()

#define _MD_INIT_LOCKS()
#define _MD_NEW_LOCK(md_lockp) (mutex_init(&((md_lockp)->lock),USYNC_THREAD,NULL) ? PR_FAILURE : PR_SUCCESS)
#define _MD_FREE_LOCK(md_lockp) mutex_destroy(&((md_lockp)->lock))
#define _MD_UNLOCK(md_lockp) mutex_unlock(&((md_lockp)->lock))
#define _MD_TEST_AND_LOCK(md_lockp) mutex_trylock(&((md_lockp)->lock))
struct _MDLock;
PR_EXTERN(void) _MD_lock(struct _MDLock *md_lock);
#undef PROFILE_LOCKS
#ifndef PROFILE_LOCKS
#define _MD_LOCK(md_lockp) _MD_lock(md_lockp)
#else
#define _MD_LOCK(md_lockp)                 \
    PR_BEGIN_MACRO \
    int rv = _MD_TEST_AND_LOCK(md_lockp); \
    if (rv == 0) { \
        (md_lockp)->hitcount++; \
    } else { \
        (md_lockp)->misscount++; \
        _MD_lock(md_lockp); \
    } \
    PR_END_MACRO
#endif

#define _PR_LOCK_HEAP() if (_pr_heapLock) _MD_LOCK(&_pr_heapLock->md)
#define _PR_UNLOCK_HEAP() if (_pr_heapLock) _MD_UNLOCK(&_pr_heapLock->md)

#define _MD_ATTACH_THREAD(threadp)


#define THR_KEYCREATE thr_keycreate
#define THR_SELF thr_self
#define _MD_NEW_CV(condp) cond_init(&((condp)->cv), USYNC_THREAD, 0)
#define COND_WAIT(condp, mutexp) cond_wait(condp, mutexp)
#define COND_TIMEDWAIT(condp, mutexp, tspec) \
                                     cond_timedwait(condp, mutexp, tspec)
#define _MD_NOTIFY_CV(condp, lockp) cond_signal(&((condp)->cv))
#define _MD_NOTIFYALL_CV(condp,unused) cond_broadcast(&((condp)->cv))	
#define _MD_FREE_CV(condp) cond_destroy(&((condp)->cv))
#define _MD_YIELD() thr_yield()
#include <time.h>
/* 
 * Because clock_gettime() on Solaris/x86 2.4 always generates a
 * segmentation fault, we use an emulated version _pr_solx86_clock_gettime(),
 * which is implemented using gettimeofday().
 */
#if defined(i386) && defined(SOLARIS2_4)
extern int _pr_solx86_clock_gettime(clockid_t clock_id, struct timespec *tp);
#define GETTIME(tt) _pr_solx86_clock_gettime(CLOCK_REALTIME, (tt))
#else
#define GETTIME(tt) clock_gettime(CLOCK_REALTIME, (tt))
#endif  /* i386 && SOLARIS2_4 */

#define MUTEX_T mutex_t
#define COND_T cond_t

#define _MD_NEW_SEM(md_semp,_val)  sema_init(&((md_semp)->sem),_val,USYNC_THREAD,NULL)
#define _MD_DESTROY_SEM(md_semp) sema_destroy(&((md_semp)->sem))
#define _MD_WAIT_SEM(md_semp) sema_wait(&((md_semp)->sem))
#define _MD_POST_SEM(md_semp) sema_post(&((md_semp)->sem))

#define _MD_SAVE_ERRNO(_thread)
#define _MD_RESTORE_ERRNO(_thread)
#define _MD_INIT_RUNNING_CPU(cpu) _MD_unix_init_running_cpu(cpu)

extern struct _MDLock _pr_ioq_lock;
#define _MD_IOQ_LOCK()		_MD_LOCK(&_pr_ioq_lock)
#define _MD_IOQ_UNLOCK()	_MD_UNLOCK(&_pr_ioq_lock)

extern PRStatus _MD_wait(struct PRThread *, PRIntervalTime timeout);
#define _MD_WAIT _MD_wait

extern PRStatus _MD_WakeupWaiter(struct PRThread *);
#define _MD_WAKEUP_WAITER _MD_WakeupWaiter

PR_EXTERN(void) _MD_InitIO(void);
#define _MD_INIT_IO _MD_InitIO

#define _MD_INIT_CONTEXT(_thread, _sp, _main, status)
#define _MD_SWITCH_CONTEXT(_thread)
#define _MD_RESTORE_CONTEXT(_newThread)

struct _MDLock {
    MUTEX_T lock;
#ifdef PROFILE_LOCKS
    PRInt32 hitcount;
    PRInt32 misscount;
#endif
};

struct _MDCVar {
    COND_T cv;
};

struct _MDSemaphore {
    sema_t sem;
};

struct _MDThread {
    _PR_CONTEXT_TYPE context;
    thread_t handle;
    lwpid_t lwpid;
    uint_t sp;		/* stack pointer */
    uint_t threadID;	/* ptr to solaris-internal thread id structures */
    struct _MDSemaphore waiter_sem;
};

struct _MDThreadStack {
    PRInt8 notused;
};

struct _MDSegment {
    PRInt8 notused;
};

struct _MDCPU {
	struct _MDCPU_Unix md_unix;
};

/* The following defines the unwrapped versions of select() and poll(). */
extern int _select(int nfds, fd_set *readfds, fd_set *writefds,
	fd_set *exceptfds, struct timeval *timeout);
#define _MD_SELECT	_select

#include <stropts.h>
#include <poll.h>
#define _MD_POLL _poll
extern int _poll(struct pollfd *fds, unsigned long nfds, int timeout);

PR_BEGIN_EXTERN_C

/*
** Missing function prototypes
*/
extern int gethostname (char *name, int namelen);

PR_END_EXTERN_C

#else /* _PR_GLOBAL_THREADS_ONLY */

/*
 * LOCAL_THREADS_ONLY implementation on Solaris
 */

#include "prthread.h"

#include <errno.h>
#include <ucontext.h>
#include <sys/stack.h>
#include <synch.h>

/*
** Iinitialization Related definitions
*/

PR_EXTERN(void)				_MD_EarlyInit(void);
PR_EXTERN(void)				_MD_SolarisInit();
#define _MD_EARLY_INIT		_MD_EarlyInit
#define _MD_FINAL_INIT		_MD_SolarisInit
#define	_MD_INIT_THREAD		_MD_InitializeThread

#ifdef USE_SETJMP

#include <setjmp.h>

#define _PR_CONTEXT_TYPE	jmp_buf

#ifdef sparc
#define _MD_GET_SP(_t)		(_t)->md.context[2]
#else
#define _MD_GET_SP(_t)		(_t)->md.context[4]
#endif

#define PR_NUM_GCREGS		_JBLEN
#define CONTEXT(_thread)	(_thread)->md.context

#else  /* ! USE_SETJMP */

#ifdef sparc
#define	_PR_CONTEXT_TYPE	ucontext_t
#define _MD_GET_SP(_t)		(_t)->md.context.uc_mcontext.gregs[REG_SP]
/*
** Sparc's use register windows. the _MD_GetRegisters for the sparc's
** doesn't actually store anything into the argument buffer; instead the
** register windows are homed to the stack. I assume that the stack
** always has room for the registers to spill to...
*/
#define PR_NUM_GCREGS		0
#else
#define _PR_CONTEXT_TYPE	unsigned int edi; sigset_t oldMask, blockMask; ucontext_t
#define _MD_GET_SP(_t)		(_t)->md.context.uc_mcontext.gregs[USP]
#define PR_NUM_GCREGS		_JBLEN
#endif

#define CONTEXT(_thread)	(&(_thread)->md.context)

#endif /* ! USE_SETJMP */

#include <time.h>
/* 
 * Because clock_gettime() on Solaris/x86 always generates a
 * segmentation fault, we use an emulated version _pr_solx86_clock_gettime(),
 * which is implemented using gettimeofday().
 */
#ifdef i386
#define GETTIME(tt) _pr_solx86_clock_gettime(CLOCK_REALTIME, (tt))
#else
#define GETTIME(tt) clock_gettime(CLOCK_REALTIME, (tt))
#endif  /* i386 */

#define _MD_SAVE_ERRNO(_thread)			(_thread)->md.errcode = errno;
#define _MD_RESTORE_ERRNO(_thread)		errno = (_thread)->md.errcode;

#ifdef sparc

#ifdef USE_SETJMP
#define _MD_INIT_CONTEXT(_thread, _sp, _main, status)	      \
    PR_BEGIN_MACRO				      \
	int *context = (_thread)->md.context;	      \
    *status = PR_TRUE;              \
	(void) setjmp(context);			      \
	(_thread)->md.context[1] = (int) ((_sp) - 64); \
	(_thread)->md.context[2] = (int) _main;	      \
	(_thread)->md.context[3] = (int) _main + 4; \
    _thread->no_sched = 0; \
    PR_END_MACRO

#define _MD_SWITCH_CONTEXT(_thread)    \
    if (!setjmp(CONTEXT(_thread))) { \
	_MD_SAVE_ERRNO(_thread)    \
	_MD_SET_LAST_THREAD(_thread);	 \
    _MD_SET_CURRENT_THREAD(_thread);	 \
	_PR_Schedule();		     \
    }

#define _MD_RESTORE_CONTEXT(_newThread)	    \
{				     \
	_MD_RESTORE_ERRNO(_newThread)	    \
	_MD_SET_CURRENT_THREAD(_newThread); \
    longjmp(CONTEXT(_newThread), 1);    \
}

#else
/*
** Initialize the thread context preparing it to execute _main.
*/
#define _MD_INIT_CONTEXT(_thread, _sp, _main, status)					\
    PR_BEGIN_MACRO				      									\
    ucontext_t *uc = CONTEXT(_thread);									\
    *status = PR_TRUE;													\
    getcontext(uc);														\
    uc->uc_stack.ss_sp = (char *) ((unsigned long)(_sp - WINDOWSIZE - SA(MINFRAME)) & 0xfffffff8);	\
    uc->uc_stack.ss_size = _thread->stack->stackSize; 					\
    uc->uc_stack.ss_flags = 0; 				/* ? */		        		\
    uc->uc_mcontext.gregs[REG_SP] = (unsigned int) uc->uc_stack.ss_sp;	\
    uc->uc_mcontext.gregs[REG_PC] = (unsigned int) _main;				\
    uc->uc_mcontext.gregs[REG_nPC] = (unsigned int) ((char*)_main)+4;	\
    uc->uc_flags = UC_ALL;												\
    _thread->no_sched = 0;												\
    PR_END_MACRO

/*
** Switch away from the current thread context by saving its state and
** calling the thread scheduler. Reload cpu when we come back from the
** context switch because it might have changed.
*/
#define _MD_SWITCH_CONTEXT(_thread)    				\
    PR_BEGIN_MACRO                     				\
		if (!getcontext(CONTEXT(_thread))) { 		\
			_MD_SAVE_ERRNO(_thread);    			\
			_MD_SET_LAST_THREAD(_thread);	 		\
			_PR_Schedule();			 				\
		}					 						\
    PR_END_MACRO

/*
** Restore a thread context that was saved by _MD_SWITCH_CONTEXT or
** initialized by _MD_INIT_CONTEXT.
*/
#define _MD_RESTORE_CONTEXT(_newThread)	    				\
    PR_BEGIN_MACRO			    							\
    	ucontext_t *uc = CONTEXT(_newThread); 				\
    	uc->uc_mcontext.gregs[11] = 1;     					\
		_MD_RESTORE_ERRNO(_newThread);	    				\
		_MD_SET_CURRENT_THREAD(_newThread); 				\
    	setcontext(uc);		       							\
    PR_END_MACRO
#endif

#else  /* x86 solaris */

#ifdef USE_SETJMP

#define _MD_INIT_CONTEXT(_thread, _sp, _main, status) \
    PR_BEGIN_MACRO \
    *status = PR_TRUE; \
    if (setjmp(CONTEXT(_thread))) _main(); \
    _MD_GET_SP(_thread) = (int) ((_sp) - 64); \
    PR_END_MACRO

#define _MD_SWITCH_CONTEXT(_thread) \
    if (!setjmp(CONTEXT(_thread))) { \
        _MD_SAVE_ERRNO(_thread) \
        _PR_Schedule();	\
    }

#define _MD_RESTORE_CONTEXT(_newThread) \
{ \
    _MD_RESTORE_ERRNO(_newThread) \
    _MD_SET_CURRENT_THREAD(_newThread); \
    longjmp(CONTEXT(_newThread), 1); \
}

#else /* USE_SETJMP */

#define WINDOWSIZE		0
 
int getedi(void);
void setedi(int);
 
#define _MD_INIT_CONTEXT(_thread, _sp, _main, status)	      \
	PR_BEGIN_MACRO					\
	ucontext_t *uc = CONTEXT(_thread);		\
        *status = PR_TRUE;              \
	getcontext(uc);					\
	/* Force sp to be double aligned! */		\
    	uc->uc_mcontext.gregs[USP] = (int) ((unsigned long)(_sp - WINDOWSIZE - SA(MINFRAME)) & 0xfffffff8);	\
	uc->uc_mcontext.gregs[PC] = (int) _main;	\
	(_thread)->no_sched = 0; \
	PR_END_MACRO

/* getcontext() may return 1, contrary to what the man page says */
#define _MD_SWITCH_CONTEXT(_thread)			\
	PR_BEGIN_MACRO					\
	ucontext_t *uc = CONTEXT(_thread);		\
	PR_ASSERT(_thread->no_sched);			\
	sigfillset(&((_thread)->md.blockMask));		\
	sigprocmask(SIG_BLOCK, &((_thread)->md.blockMask),	\
		&((_thread)->md.oldMask));		\
	(_thread)->md.edi = getedi();			\
	if (! getcontext(uc)) {				\
		sigprocmask(SIG_SETMASK, &((_thread)->md.oldMask), NULL); \
		uc->uc_mcontext.gregs[EDI] = (_thread)->md.edi;	\
		_MD_SAVE_ERRNO(_thread)    		\
	        _MD_SET_LAST_THREAD(_thread);	        \
		_PR_Schedule();				\
	} else {					\
		sigprocmask(SIG_SETMASK, &((_thread)->md.oldMask), NULL); \
		setedi((_thread)->md.edi);		\
		PR_ASSERT(_MD_LAST_THREAD() !=_MD_CURRENT_THREAD()); \
		_MD_LAST_THREAD()->no_sched = 0;	\
	}						\
	PR_END_MACRO

/*
** Restore a thread context, saved by _PR_SWITCH_CONTEXT
*/
#define _MD_RESTORE_CONTEXT(_newthread)			\
	PR_BEGIN_MACRO					\
	ucontext_t *uc = CONTEXT(_newthread);		\
	uc->uc_mcontext.gregs[EAX] = 1;			\
	_MD_RESTORE_ERRNO(_newthread)  			\
	_MD_SET_CURRENT_THREAD(_newthread);		\
	(_newthread)->no_sched = 1;			\
	setcontext(uc);					\
	PR_END_MACRO
#endif /* USE_SETJMP */

#endif /* sparc */

struct _MDLock {
	PRInt8 notused;
};

struct _MDCVar {
	PRInt8 notused;
};

struct _MDSemaphore {
	PRInt8 notused;
};

struct _MDThread {
    _PR_CONTEXT_TYPE context;
    int errcode;
    int id;
};

struct _MDThreadStack {
    PRInt8 notused;
};

struct _MDSegment {
    PRInt8 notused;
};

struct _MDCPU {
	struct _MDCPU_Unix md_unix;
};

#ifndef _PR_PTHREADS
#define _MD_INIT_LOCKS()
#endif
#define _MD_NEW_LOCK(lock)				PR_SUCCESS
#define _MD_FREE_LOCK(lock)
#define _MD_LOCK(lock)
#define _MD_UNLOCK(lock)
#define _MD_INIT_IO()
#define _MD_IOQ_LOCK()
#define _MD_IOQ_UNLOCK()

#define _MD_INIT_RUNNING_CPU(cpu)		_MD_unix_init_running_cpu(cpu)
#define _MD_INIT_THREAD					_MD_InitializeThread
#define _MD_EXIT_THREAD(thread)
#define _MD_SUSPEND_THREAD(thread)
#define _MD_RESUME_THREAD(thread)
#define _MD_CLEAN_THREAD(_thread)

extern PRStatus _MD_WAIT(struct PRThread *, PRIntervalTime timeout);
extern PRStatus _MD_WAKEUP_WAITER(struct PRThread *);
extern void     _MD_YIELD(void);
extern PRStatus _MD_InitializeThread(PRThread *thread);
extern void     _MD_SET_PRIORITY(struct _MDThread *thread,
	PRThreadPriority newPri);
extern PRStatus _MD_CREATE_THREAD(PRThread *thread, void (*start) (void *),
	PRThreadPriority priority, PRThreadScope scope, PRThreadState state,
        PRUint32 stackSize);

PR_EXTERN(PRIntervalTime)				_MD_Solaris_GetInterval(void);
#define _MD_GET_INTERVAL				_MD_Solaris_GetInterval
PR_EXTERN(PRIntervalTime)				_MD_Solaris_TicksPerSecond(void);
#define _MD_INTERVAL_PER_SEC			_MD_Solaris_TicksPerSecond

/* The following defines the unwrapped versions of select() and poll(). */
extern int _select(int nfds, fd_set *readfds, fd_set *writefds,
	fd_set *exceptfds, struct timeval *timeout);
#define _MD_SELECT	_select

#include <stropts.h>
#include <poll.h>
#define _MD_POLL _poll
extern int _poll(struct pollfd *fds, unsigned long nfds, int timeout);

PR_BEGIN_EXTERN_C

/*
** Missing function prototypes
*/
extern int gethostname (char *name, int namelen);

PR_END_EXTERN_C

#endif /* _PR_GLOBAL_THREADS_ONLY */

#endif /* nspr_solaris_defs_h___ */

