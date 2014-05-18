/*
    except.c

    This file implements the exception raising scheme.
	It is thread safe, although Alt nodes may not behave
	as expected.

    Copyright (c) 1988-1996 NeXT Software, Inc. as an unpublished work.
    All rights reserved.
*/

#if defined(WIN32)
    #include <winnt-pdo.h>
    #include <windows.h>
    #include <objc/zone.h> 		// for pdo_malloc and pdo_free
#endif 

#if defined(KERNEL)
    #import <mach/mach_types.h>
    #import <kernserv/prototypes.h>
    #import <kern/lock.h>
#else
    #import <stdlib.h>
#endif

#import <stdio.h>
#include <unistd.h>
#if defined(_POSIX_THREADS)
#include <pthread.h>
#else
#import <mach/cthreads.h>
#endif
#import <objc/objc.h>

#if defined(NeXT_PDO)
    #import <pdo.h>
#endif

#if defined(__MACH__)
    #import <string.h>
#endif

#import <objc/error.h>
typedef void AltProc(void *context, int code, const void *data1, const void *data2);
OBJC_EXPORT NXHandler *_NXAddAltHandler (AltProc *proc, void *context);
OBJC_EXPORT void _NXRemoveAltHandler (NXHandler *handler);

OBJC_EXPORT void _NXLogError(const char *format, ...);

#if defined(KERNEL)
    #import <mach/machine/simple_lock.h>

    #if defined(hppa)
        #define SIMPLE_LOCK_INITIALIZER { 1, 1, 1, 1 }
    #else
        #define SIMPLE_LOCK_INITIALIZER { 1 }
    #endif

    #define LOCK_T		simple_lock_data_t
    #define LOCK_INITIALIZER	SIMPLE_LOCK_INITIALIZER
    #define LOCK(x)		simple_lock(&(x))
    #define UNLOCK(x)		simple_unlock(&(x))
    #define LOCK_ALLOC		simple_lock_alloc
    #define cthread_t		mach_port_t
    #define cthread_self()	current_thread()

    int _setjmp(jmp_buf env) { return setjmp(env); }
    void _longjmp(jmp_buf env, int val) { longjmp(env, val); }
    NXUncaughtExceptionHandler *_NXUncaughtExceptionHandler;

#elif defined(__MACH__)
    #if defined(_POSIX_THREADS)
        #define LOCK_T			pthread_mutex_t
        #define LOCK_INITIALIZER	PTHREAD_MUTEX_INITIALIZER
        #define LOCK(x)			pthread_mutex_lock(&x)
        #define UNLOCK(x) 		pthread_mutex_unlock(&x)
        #define cthread_t		pthread_t
	#define cthread_self()		pthread_self()
    #else
        #define LOCK_T		struct mutex
        #define LOCK_INITIALIZER	MUTEX_INITIALIZER
        #define LOCK(x)		mutex_lock(&x)
        #define UNLOCK(x) 		mutex_unlock(&x)
        #define cthread_t		mach_port_t
        extern void _set_cthread_free_callout (void (*) (void));
        static void _NXClearExceptionStack (void);
    #endif

#elif defined(WIN32)
    #define LOCK_T		long
    #define LOCK_INITIALIZER	0
    #define LOCK(x)		mutex_lock(&x)
    #define UNLOCK(x) 		mutex_unlock(&x)
    #define cthread_t		int
    #define _setjmp 		setjmp
    #define _longjmp 		longjmp

#elif defined(__svr4__) || defined(hpux)
    #define LOCK_T		mutex_t
    #define LOCK_INITIALIZER	NULL
    #define LOCK(x)		mutex_lock(x)
    #define UNLOCK(x) 		mutex_unlock(x)
    #define LOCK_ALLOC		mutex_alloc
#endif // KERNEL

// this maps the address of the exception handler into data space
// we want everyone to find this handler
#if defined(NeXT_PDO)
    extern _setjmp(jmp_buf env);
    extern void _longjmp(jmp_buf env, int val);
    NXUncaughtExceptionHandler *_NXUncaughtExceptionHandler;
#endif 

/*  These nodes represent handlers that are called as normal procedures
    instead of longjmp'ed to.  When these procs return, the next handler
    in the chain is processed.
 */
typedef struct {		/* an alternative node in the handler chain */
    struct _NXHandler *next;		/* ptr to next handler */
    AltProc *proc;			/* proc to call */
    void *context;			/* blind data for client */
} AltHandler;

/* Error buffers.  Linked to together when created as needed.
 */

typedef struct _ErrorBuffer {
    struct _ErrorBuffer *next;		/* linked list */
    char data[0];			/* variable sized user data  */
} ErrorBuffer;

static ErrorBuffer *ErrorBufferChain = NULL;
static LOCK_T ErrorDataLock = LOCK_INITIALIZER;

 /* Multiple thread support.
	Basic strategy is to collect globals into a struct; singly link them
	(probably not worth it put them in a hashtable) using the cthread id
	as a key.  Upon use, look up proper stack of handlers...
 */

typedef struct xxx {
	NXHandler *handlerStack;	/* start of handler chain */
	AltHandler *altHandlers;
	int altHandlersAlloced;
	int altHandlersUsed;
	cthread_t	thread;			/* key */
	struct xxx	*next;			/* link */
} ExceptionHandlerStack;

/* Allocate the handler stack for the first thread statically so that
   we don't keep an extra page in the heap hot (the libsys data is
   already hot).  Also statically allocate a small number of altHandlers.
   This should be the maximum typical depth of lockFocus'es. */

static AltHandler BaseAltHandlers[16] = { {0} };

static ExceptionHandlerStack Base =
{
  NULL,
  BaseAltHandlers,
  sizeof (BaseAltHandlers) / sizeof (BaseAltHandlers[0]),
  0,
  0,
  NULL
};

static LOCK_T Lock = LOCK_INITIALIZER;

static ExceptionHandlerStack *addme (cthread_t self)
{
  ExceptionHandlerStack *stack;
  
#if defined(KERNEL) || defined(__svr4__) || defined(hpux)
  if (!Lock)
    Lock = LOCK_ALLOC ();	// not really thread safe XXX
#endif

  LOCK (Lock);			// lookup is thread safe; addition isn't
  
#if defined(KERNEL)
  for (stack = &Base; stack; stack = stack->next)
    if (stack->thread == 0) {
      stack->thread = self;
      UNLOCK (Lock);
      return stack;
    }
#else	// not KERNEL
  if (Base.thread == 0)		// try statically allocated stack
    {
      Base.thread = self;
      UNLOCK (Lock);
      return &Base;
    }
  
  /* Pass exception handler cleanup routine to cthread package.  */
    #if defined(__MACH__) && !defined(_POSIX_THREADS)
        _set_cthread_free_callout (&_NXClearExceptionStack);
    #endif
#endif	// KERNEL
  
  stack = calloc (sizeof (ExceptionHandlerStack), 1);
  stack->thread = self;
  stack->next = Base.next;
  Base.next = stack;		// insert atomically
  
  UNLOCK (Lock);
  
  return stack;
}


/*
	Get callers thread & find | allocate a stack.  Allocation is only
	proper for some usages, but we don't check for them...
	Also, these structs are not recovered upon thread death. XXX
*/
#if !defined(BUG67896)
static inline 
#endif
ExceptionHandlerStack *findme (void)
{
  ExceptionHandlerStack *stack;
  cthread_t self = (cthread_t)cthread_self ();
  
  for (stack = &Base; stack; stack = stack->next)
    if (stack->thread == self)
      return stack;
  
  return addme (self);
}

#ifdef	KERNEL
/*
 * Call back from thread_deallocate()
 * to indicate that a thread is being
 * destroyed.  Kernel thread ids aren't
 * reused the way that cthread ids are,
 * so we have to nuke the value in the
 * structure.
 *
 * XXX Should free data structures here.
 */
void
_threadFreeExceptionStack(
    thread_t		thread
)
{
  ExceptionHandlerStack *stack;

  // Don't allocate a stack for this thread if it doesn't already have one.
  for (stack = &Base; stack; stack = stack->next)
    if (stack->thread == thread)
      break;

  if (stack)
    {
      stack->handlerStack = 0;    	 // reset the handler chain
      stack->altHandlersUsed = 0; 	 // reset the alt handler count
      stack->thread = 0;
    }
}
#elif defined(__MACH__) && !defined(_POSIX_THREADS)
/* cthreads will reuse a stack and a cthread_id.
 * We provide this call-in to clean up matters
 */
static void _NXClearExceptionStack (void)
{
  ExceptionHandlerStack *stack;
  cthread_t self = (cthread_t)cthread_self ();
  
  // Don't allocate a stack for this thread if it doesn't already have one.
  for (stack = &Base; stack; stack = stack->next)
    if (stack->thread == self)
      break;

  if (stack)
    {
      stack->handlerStack = 0;    	 // reset the handler chain
      stack->altHandlersUsed = 0; 	 // reset the alt handler count
    }
}
#endif	KERNEL

#define IS_ALT(ptr)			((arith_t)(ptr) % 2)
#define NEXT_HANDLER(ptr)		\
	(IS_ALT(ptr) ? ALT_CODE_TO_PTR(ptr)->next : ((NXHandler *)ptr)->next)
#define ALT_PTR_TO_CODE(ptr)		(((ptr) - me->altHandlers) * 2 + 1)
#define ALT_CODE_TO_PTR(code)	(me->altHandlers + ((arith_t)(code) - 1) / 2)

/* if the node passed in isnt on top of the stack, something's fishy */

static void trickyRemoveHandler (NXHandler *handler, int removingAlt)
{
  AltHandler *altNode;
  NXHandler *node;
  NXHandler **nodePtr;
  ExceptionHandlerStack *me = findme ();
  
  /* try to find the node anywhere on the stack */
  node = me->handlerStack;
  while (node != handler && node)
    {
      /* Watch for attempts to remove handlers which are outside the
	 active portion of the stack.  This happens when you return
	 from an NX_DURING context without removing the handler.
	 This code assumes the stack grows downward. */
#if hppa
	/* stack grows upward */
      if (IS_ALT (node) || (void *) node < (void *) &altNode)
	node = NEXT_HANDLER (node);
      else
#else
      if (IS_ALT (node) || (void *) node > (void *) &altNode)
	node = NEXT_HANDLER (node);
      else
#endif
	{
	  _NXLogError ("Exception handlers were not properly removed.");
#if defined(WIN32)
	  (void)RaiseException(0xdead, EXCEPTION_NONCONTINUABLE, 0, NULL);
#else
	  (void)abort ();
#endif
	}
    }
  
  if (node)
    {
      /* 
      * Clean off the stack up to the out of place node.  If we are trying
      * to remove an non-alt handler, we pop eveything off the stack
      * including that handler, calling any alt procs along the way.  If
      * we are removing an alt handler, we leave all the non-alt handlers
      * alone as we clean off the stack, but pop off all the alt handlers
      * we find, including the node we were asked to remove.
      */
      if (!removingAlt)
	_NXLogError ("Exception handlers were not properly removed.");
      
      nodePtr = &me->handlerStack;
      do
	{
	  node = *nodePtr;
	  if (IS_ALT(node))
	    {
	      altNode = ALT_CODE_TO_PTR(node);
	      if (removingAlt)
	        {
		  if (node == handler)
		    *nodePtr = altNode->next;	/* del matching node */
		  else
		    nodePtr = &altNode->next;	/* skip node */
	        }
	      else
	        {
		  if (node != handler)
		    (*altNode->proc)(altNode->context, 1, 0, 0);
		  me->altHandlersUsed = altNode - me->altHandlers;
		  *nodePtr = altNode->next;	/* del any alt node */
	        }
	    }
	  else
	    {
	      if (removingAlt)
		nodePtr = &node->next;		/* skip node */
	      else
		*nodePtr = node->next;		/* nuke non-alt node */
	    }
	}
      while (node != handler);
    }
  else
    {
#ifdef KERNEL
      ;
#else /* KERNEL */
      _NXLogError ("Attempt to remove unrecognized exception handler.");
#endif /* KERNEL */
    }
}


NXHandler *_NXAddAltHandler (AltProc *proc, void *context)
{
  AltHandler *new;
  ExceptionHandlerStack *me = findme ();

  if (me->altHandlersUsed == me->altHandlersAlloced) {
      if (me->altHandlers == BaseAltHandlers) {
	  me->altHandlers = malloc (++me->altHandlersAlloced *
				    sizeof(AltHandler));
	  bcopy (BaseAltHandlers, me->altHandlers, sizeof (BaseAltHandlers));
	} else {
	  volatile AltHandler *tempAlt = realloc (me->altHandlers, ++me->altHandlersAlloced *
	    sizeof(AltHandler));
	  me->altHandlers = (AltHandler*)tempAlt;
	}
    }
  
  new = me->altHandlers + me->altHandlersUsed++;
  new->next = me->handlerStack;
  me->handlerStack = (NXHandler *) ALT_PTR_TO_CODE (new);
  new->proc = proc;
  new->context = context;
  return me->handlerStack;
}


void _NXRemoveAltHandler (NXHandler *handler)
{
  ExceptionHandlerStack *me;
  
  for (me = &Base; me; me = me->next)
    if (me->handlerStack == handler)
      {
	AltHandler *altNode = ALT_CODE_TO_PTR (handler);
	
	me->altHandlersUsed = altNode - me->altHandlers;
	me->handlerStack = altNode->next;
	return;
      }
  
  trickyRemoveHandler (handler, TRUE);
}


void _NXAddHandler (NXHandler *handler)
{
  ExceptionHandlerStack *me = findme ();
  
  handler->next = me->handlerStack;
  me->handlerStack = handler;
  handler->code = 0;
}


void _NXRemoveHandler (NXHandler *handler)
{
  ExceptionHandlerStack *me;
  
  for (me = &Base; me; me = me->next)
    if (me->handlerStack == handler)
      {
        me->handlerStack = me->handlerStack->next;
	return;
      }
  
  trickyRemoveHandler (handler, FALSE);
}

/* forwards the error to the next handler */
volatile void NXDefaultExceptionRaiser(int code, const void *data1, const void *data2)
{
    NXHandler *destination;
    AltHandler *altDest;
	ExceptionHandlerStack *me = findme ();

    while (1) {
		destination = me->handlerStack;
		if (!destination) {
			if (_NXUncaughtExceptionHandler)
				(*_NXUncaughtExceptionHandler)(code, data1, data2);
			else {
#ifndef KERNEL
				_NXLogError("Uncaught exception #%d\n", code);
#endif /* not KERNEL */
			}
#ifdef KERNEL
				panic("Uncaught exception");
#else /* KERNEL */
#if defined(WIN32)
				(void)RaiseException(0xdead, EXCEPTION_NONCONTINUABLE, 0, NULL);
#else
				exit(-1);
#endif
#endif /* KERNEL */
		} else if (IS_ALT(destination)) {
			altDest = ALT_CODE_TO_PTR(destination);
			me->handlerStack = altDest->next;
			me->altHandlersUsed = altDest - me->altHandlers;
			(*altDest->proc)(altDest->context, code, data1, data2);
		} else {
			destination->code = code;
			destination->data1 = data1;
			destination->data2 = data2;
			me->handlerStack = destination->next;
			_longjmp(destination->jumpState, 1);
		}
    }
}

static NXExceptionRaiser *ExceptionRaiser = &NXDefaultExceptionRaiser;

void NXSetExceptionRaiser(NXExceptionRaiser *proc)
{
    ExceptionRaiser = proc;
}


NXExceptionRaiser *NXGetExceptionRaiser (void)
{
    return ExceptionRaiser;
}

#if defined(__GNUC__)
    #if !defined(__STRICT_ANSI__)
        #if !defined(NeXT_PDO)
            __volatile  /* never returns */
        #endif 
    #endif
#endif 
void _NXRaiseError(int code, const void *data1, const void *data2)
{
    (void)(*ExceptionRaiser)(code, data1, data2);
#if defined(WIN32)
    (void)RaiseException(0xdead, EXCEPTION_NONCONTINUABLE, 0, NULL); // we should never get here 
#else
    abort(); // we should never get here
#endif
}



/* stack allocates some space from the error buffer */
void NXAllocErrorData(int size, void **data)
{
    ErrorBuffer *buffer;

#if defined(KERNEL) || defined(__svr4__) || defined(hpux)
    if (!ErrorDataLock) {
        ErrorDataLock = LOCK_ALLOC();	// not really thread safe XXX
    }
#endif /* KERNEL || __svr4__ || hpux */

    buffer = malloc (sizeof (ErrorBuffer) + size);
    *data  = &buffer -> data;
	
    LOCK (ErrorDataLock);
    buffer->next = ErrorBufferChain;
    ErrorBufferChain = buffer;
    UNLOCK (ErrorDataLock);
}

void NXResetErrorData(void)
{
   ErrorBuffer *chain, *next;

#if defined(KERNEL) || defined(__svr4__) || defined(hpux)
   /* No lock means nothing's been allocated either. */
   if (!ErrorDataLock)
      return;
#endif /* KERNEL || __svr4__ || hpux */

   LOCK (ErrorDataLock);
   chain = ErrorBufferChain;
   ErrorBufferChain = NULL;
   UNLOCK (ErrorDataLock);
 
   while (chain)
     {
       next = chain->next;
       free (chain);
       chain = next;
     }
}
