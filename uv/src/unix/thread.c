


#include "uv.h"
#include "internal.h"

#include <pthread.h>
#include <assert.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/resource.h>  /* getrlimit() */
#include <unistd.h>  /* getpagesize() */

#include <limits.h>

#undef NANOSEC
#define NANOSEC ((uint64_t) 1e9)

struct thread_ctx {
  void (*entry)(void* arg);
  void* arg;
};


static void* uv__thread_start(void *arg)
{
  struct thread_ctx *ctx_p;
  struct thread_ctx ctx;

  ctx_p = arg;
  ctx = *ctx_p;
  uv__free(ctx_p);
  ctx.entry(ctx.arg);

  return 0;
}


int uv_thread_create(uv_thread_t *tid, void (*entry)(void *arg), void *arg) {
  struct thread_ctx* ctx;
  int err;
  pthread_attr_t* attr;
#if defined(__APPLE__)
  pthread_attr_t attr_storage;
  struct rlimit lim;
#endif

  ctx = uv__malloc(sizeof(*ctx));
  if (ctx == NULL)
    return UV_ENOMEM;

  ctx->entry = entry;
  ctx->arg = arg;

  /* On OSX threads other than the main thread are created with a reduced stack
   * size by default, adjust it to RLIMIT_STACK.
   */
#if defined(__APPLE__)
  if (getrlimit(RLIMIT_STACK, &lim))
    abort();

  attr = &attr_storage;
  if (pthread_attr_init(attr))
    abort();

  if (lim.rlim_cur != RLIM_INFINITY) {
    /* pthread_attr_setstacksize() expects page-aligned values. */
    lim.rlim_cur -= lim.rlim_cur % (rlim_t) getpagesize();

    if (lim.rlim_cur >= PTHREAD_STACK_MIN)
      if (pthread_attr_setstacksize(attr, lim.rlim_cur))
        abort();
  }
#else
  attr = NULL;
#endif

  err = pthread_create(tid, attr, uv__thread_start, ctx);

  if (attr != NULL)
    pthread_attr_destroy(attr);

  if (err)
    uv__free(ctx);

  return -err;
}


uv_thread_t uv_thread_self(void) {
  return pthread_self();
}

int uv_thread_join(uv_thread_t *tid) {
  return -pthread_join(*tid, NULL);
}


int uv_thread_equal(const uv_thread_t* t1, const uv_thread_t* t2) {
  return pthread_equal(*t1, *t2);
}


int uv_mutex_init(uv_mutex_t* mutex) {
#if defined(NDEBUG) || !defined(PTHREAD_MUTEX_ERRORCHECK)
  return -pthread_mutex_init(mutex, NULL);
#else
  pthread_mutexattr_t attr;
  int err;

  if (pthread_mutexattr_init(&attr))
    abort();

  if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK))
    abort();

  err = pthread_mutex_init(mutex, &attr);

  if (pthread_mutexattr_destroy(&attr))
    abort();

  return -err;
#endif
}


void uv_mutex_destroy(uv_mutex_t* mutex) {
  if (pthread_mutex_destroy(mutex))
    abort();
}


void uv_mutex_lock(uv_mutex_t* mutex) {
  if (pthread_mutex_lock(mutex))
    abort();
}


int uv_mutex_trylock(uv_mutex_t* mutex) {
  int err;

  err = pthread_mutex_trylock(mutex);
  if (err) {
    if (err != EBUSY && err != EAGAIN)
      abort();
    return -EBUSY;
  }

  return 0;
}


void uv_mutex_unlock(uv_mutex_t* mutex) {
  if (pthread_mutex_unlock(mutex))
    abort();
}




int uv_rwlock_tryrdlock(uv_rwlock_t* rwlock) {
  int err;

  err = pthread_rwlock_tryrdlock(rwlock);
  if (err) {
    if (err != EBUSY && err != EAGAIN)
      abort();
    return -EBUSY;
  }

  return 0;
}




void uv_rwlock_wrunlock(uv_rwlock_t* rwlock) {
  if (pthread_rwlock_unlock(rwlock))
    abort();
}


void uv_once(uv_once_t* guard, void (*callback)(void)) {
  if (pthread_once(guard, callback))
    abort();
}




#if defined(__APPLE__) && defined(__MACH__)

int uv_cond_init(uv_cond_t* cond) {
  return -pthread_cond_init(cond, NULL);
}

#else /* !(defined(__APPLE__) && defined(__MACH__)) */

int uv_cond_init(uv_cond_t* cond) {
  pthread_condattr_t attr;
  int err;

  err = pthread_condattr_init(&attr);
  if (err)
    return -err;

#if !(defined(__ANDROID__) && defined(HAVE_PTHREAD_COND_TIMEDWAIT_MONOTONIC))
  err = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
  if (err)
    goto error2;
#endif

  err = pthread_cond_init(cond, &attr);
  if (err)
    goto error2;

  err = pthread_condattr_destroy(&attr);
  if (err)
    goto error;

  return 0;

error:
  pthread_cond_destroy(cond);
error2:
  pthread_condattr_destroy(&attr);
  return -err;
}

#endif /* defined(__APPLE__) && defined(__MACH__) */

void uv_cond_destroy(uv_cond_t* cond) {
#if defined(__APPLE__) && defined(__MACH__)
  /* It has been reported that destroying condition variables that have been
   * signalled but not waited on can sometimes result in application crashes.
   * See https://codereview.chromium.org/1323293005.
   */
  pthread_mutex_t mutex;
  struct timespec ts;
  int err;

  if (pthread_mutex_init(&mutex, NULL))
    abort();

  if (pthread_mutex_lock(&mutex))
    abort();

  ts.tv_sec = 0;
  ts.tv_nsec = 1;

  err = pthread_cond_timedwait_relative_np(cond, &mutex, &ts);
  if (err != 0 && err != ETIMEDOUT)
    abort();

  if (pthread_mutex_unlock(&mutex))
    abort();

  if (pthread_mutex_destroy(&mutex))
    abort();
#endif /* defined(__APPLE__) && defined(__MACH__) */

  if (pthread_cond_destroy(cond))
    abort();
}

void uv_cond_signal(uv_cond_t* cond) {
  if (pthread_cond_signal(cond))
    abort();
}

void uv_cond_broadcast(uv_cond_t* cond) {
  if (pthread_cond_broadcast(cond))
    abort();
}

void uv_cond_wait(uv_cond_t* cond, uv_mutex_t* mutex) {
  if (pthread_cond_wait(cond, mutex))
    abort();
}



