/*
 * Copyright (c) 2012 Carsten Munk <carsten.munk@gmail.com>
 * Copyright (c) 2012 Canonical Ltd
 * Copyright (c) 2013 Christophe Chapuis <chris.chapuis@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "../include/hybris/binding.h"

#include "hooks_shm.h"

#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <stdio_ext.h>
#include <stddef.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <strings.h>
#include <dlfcn.h>
#include <pthread.h>
#include <sys/xattr.h>
#include <grp.h>
#include <signal.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <stdarg.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>

#include <netdb.h>
#include <unistd.h>
#include <syslog.h>
#include <locale.h>
#include <sys/syscall.h>
#include <sys/auxv.h>

#include <arpa/inet.h>
#include <assert.h>
#include <sys/mman.h>
#include <wchar.h>
#include <sys/utsname.h>
#include <math.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <net/if.h>
#include <utime.h>
#include <wctype.h>
#include <sys/poll.h>
#include <ctype.h>

#include "../include/hybris/hook.h"
#include "../include/hybris/properties.h"
#include "ctype.h"

static locale_t hybris_locale;
static int locale_inited = 0;
/* TODO:
*  - Check if the int arguments at attr_set/get match the ones at Android
*  - Check how to deal with memory leaks (specially with static initializers)
*  - Check for shared rwlock
*/

/* Base address to check for Android specifics */
#define ANDROID_TOP_ADDR_VALUE_MUTEX  0xFFFF
#define ANDROID_TOP_ADDR_VALUE_COND   0xFFFF
#define ANDROID_TOP_ADDR_VALUE_RWLOCK 0xFFFF

#define ANDROID_MUTEX_SHARED_MASK      0x2000
#define ANDROID_COND_SHARED_MASK       0x0001
#define ANDROID_RWLOCKATTR_SHARED_MASK 0x0010

/* For the static initializer types */
#define ANDROID_PTHREAD_MUTEX_INITIALIZER            0
#define ANDROID_PTHREAD_RECURSIVE_MUTEX_INITIALIZER  0x4000
#define ANDROID_PTHREAD_ERRORCHECK_MUTEX_INITIALIZER 0x8000
#define ANDROID_PTHREAD_COND_INITIALIZER             0
#define ANDROID_PTHREAD_RWLOCK_INITIALIZER           0

/* Debug */
#include "logging.h"
#define LOGD(message, ...) HYBRIS_DEBUG_LOG(HOOKS, message, ##__VA_ARGS__)

/* we have a value p:
 *  - if p <= ANDROID_TOP_ADDR_VALUE_MUTEX then it is an android mutex, not one we processed
 *  - if p > VMALLOC_END, then the pointer is not a result of malloc ==> it is an shm offset
 */

struct _hook {
    const char *name;
    void *func;
};

uintptr_t _hybris_stack_chk_guard = 0;

static void __attribute__((constructor)) __init_stack_check_guard() {
    _hybris_stack_chk_guard = *((uintptr_t*) getauxval(AT_RANDOM));
}

/* Helpers */
static int hybris_check_android_shared_mutex(unsigned int mutex_addr)
{
    /* If not initialized or initialized by Android, it should contain a low
     * address, which is basically just the int values for Android's own
     * pthread_mutex_t */
    if ((mutex_addr <= ANDROID_TOP_ADDR_VALUE_MUTEX) &&
                    (mutex_addr & ANDROID_MUTEX_SHARED_MASK))
        return 1;

    return 0;
}

static int hybris_check_android_shared_cond(unsigned int cond_addr)
{
    /* If not initialized or initialized by Android, it should contain a low
     * address, which is basically just the int values for Android's own
     * pthread_cond_t */
    if ((cond_addr <= ANDROID_TOP_ADDR_VALUE_COND) &&
                    (cond_addr & ANDROID_COND_SHARED_MASK))
        return 1;

    return 0;
}

static void hybris_set_mutex_attr(unsigned int android_value, pthread_mutexattr_t *attr)
{
    /* Init already sets as PTHREAD_MUTEX_NORMAL */
    pthread_mutexattr_init(attr);

    if (android_value & ANDROID_PTHREAD_RECURSIVE_MUTEX_INITIALIZER) {
        pthread_mutexattr_settype(attr, PTHREAD_MUTEX_RECURSIVE);
    } else if (android_value & ANDROID_PTHREAD_ERRORCHECK_MUTEX_INITIALIZER) {
        pthread_mutexattr_settype(attr, PTHREAD_MUTEX_ERRORCHECK);
    }
}

static pthread_mutex_t* hybris_alloc_init_mutex(unsigned int android_mutex)
{
    pthread_mutex_t *realmutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutexattr_t attr;
    hybris_set_mutex_attr(android_mutex, &attr);
    pthread_mutex_init(realmutex, &attr);
    return realmutex;
}

static pthread_cond_t* hybris_alloc_init_cond(void)
{
    pthread_cond_t *realcond = malloc(sizeof(pthread_cond_t));
    pthread_condattr_t attr;
    pthread_condattr_init(&attr);
    pthread_cond_init(realcond, &attr);
    return realcond;
}

static pthread_rwlock_t* hybris_alloc_init_rwlock(void)
{
    pthread_rwlock_t *realrwlock = malloc(sizeof(pthread_rwlock_t));
    pthread_rwlockattr_t attr;
    pthread_rwlockattr_init(&attr);
    pthread_rwlock_init(realrwlock, &attr);
    return realrwlock;
}

/*
 * utils, such as malloc, memcpy
 *
 * Useful to handle hacks such as the one applied for Nvidia, and to
 * avoid crashes.
 *
 * */

static void *my_malloc(size_t size)
{
    return malloc(size);
}

static void *my_memcpy(void *dst, const void *src, size_t len)
{
    if (src == NULL || dst == NULL)
        return NULL;

    return memcpy(dst, src, len);
}

static size_t my_strlen(const char *s)
{

    if (s == NULL)
        return -1;

    return strlen(s);
}

size_t my_strlen_chk(const char *s, size_t s_len) {
    size_t ret = strlen(s);
    if (ret >= s_len) {
        LOGD("__strlen_chk: ret >= s_len");
        abort();
    }
    return ret;
}

extern size_t strlcpy(char *dst, const char *src, size_t siz);

static pid_t my_gettid( void )
{
        return syscall( __NR_gettid );
}

/*
 * Main pthread functions
 *
 * Custom implementations to workaround difference between Bionic and Glibc.
 * Our own pthread_create helps avoiding direct handling of TLS.
 *
 * */

static int my_pthread_create(pthread_t *thread, const pthread_attr_t *__attr,
                             void *(*start_routine)(void*), void *arg)
{
    pthread_attr_t *realattr = NULL;

    if (__attr != NULL)
        realattr = (pthread_attr_t *) *(unsigned int *) __attr;

    return pthread_create(thread, realattr, start_routine, arg);
}

/*
 * pthread_attr_* functions
 *
 * Specific implementations to workaround the differences between at the
 * pthread_attr_t struct differences between Bionic and Glibc.
 *
 * */

static int my_pthread_attr_init(pthread_attr_t *__attr)
{
    pthread_attr_t *realattr;

    realattr = malloc(sizeof(pthread_attr_t));
    *((unsigned int *)__attr) = (unsigned int) realattr;

    return pthread_attr_init(realattr);
}

static int my_pthread_attr_destroy(pthread_attr_t *__attr)
{
    int ret;
    pthread_attr_t *realattr = (pthread_attr_t *) *(unsigned int *) __attr;

    ret = pthread_attr_destroy(realattr);
    /* We need to release the memory allocated at my_pthread_attr_init
     * Possible side effects if destroy is called without our init */
    free(realattr);

    return ret;
}

static int my_pthread_attr_setdetachstate(pthread_attr_t *__attr, int state)
{
    pthread_attr_t *realattr = (pthread_attr_t *) *(unsigned int *) __attr;
    return pthread_attr_setdetachstate(realattr, state);
}

static int my_pthread_attr_getdetachstate(pthread_attr_t const *__attr, int *state)
{
    pthread_attr_t *realattr = (pthread_attr_t *) *(unsigned int *) __attr;
    return pthread_attr_getdetachstate(realattr, state);
}

static int my_pthread_attr_setschedpolicy(pthread_attr_t *__attr, int policy)
{
    pthread_attr_t *realattr = (pthread_attr_t *) *(unsigned int *) __attr;
    return pthread_attr_setschedpolicy(realattr, policy);
}

static int my_pthread_attr_getschedpolicy(pthread_attr_t const *__attr, int *policy)
{
    pthread_attr_t *realattr = (pthread_attr_t *) *(unsigned int *) __attr;
    return pthread_attr_getschedpolicy(realattr, policy);
}

static int my_pthread_attr_setschedparam(pthread_attr_t *__attr, struct sched_param const *param)
{
    pthread_attr_t *realattr = (pthread_attr_t *) *(unsigned int *) __attr;
    return pthread_attr_setschedparam(realattr, param);
}

static int my_pthread_attr_getschedparam(pthread_attr_t const *__attr, struct sched_param *param)
{
    pthread_attr_t *realattr = (pthread_attr_t *) *(unsigned int *) __attr;
    return pthread_attr_getschedparam(realattr, param);
}

static int my_pthread_attr_setstacksize(pthread_attr_t *__attr, size_t stack_size)
{
    pthread_attr_t *realattr = (pthread_attr_t *) *(unsigned int *) __attr;
    return pthread_attr_setstacksize(realattr, stack_size);
}

static int my_pthread_attr_getstacksize(pthread_attr_t const *__attr, size_t *stack_size)
{
    pthread_attr_t *realattr = (pthread_attr_t *) *(unsigned int *) __attr;
    return pthread_attr_getstacksize(realattr, stack_size);
}

static int my_pthread_attr_setstackaddr(pthread_attr_t *__attr, void *stack_addr)
{
    pthread_attr_t *realattr = (pthread_attr_t *) *(unsigned int *) __attr;
    return pthread_attr_setstackaddr(realattr, stack_addr);
}

static int my_pthread_attr_getstackaddr(pthread_attr_t const *__attr, void **stack_addr)
{
    pthread_attr_t *realattr = (pthread_attr_t *) *(unsigned int *) __attr;
    return pthread_attr_getstackaddr(realattr, stack_addr);
}

static int my_pthread_attr_setstack(pthread_attr_t *__attr, void *stack_base, size_t stack_size)
{
    pthread_attr_t *realattr = (pthread_attr_t *) *(unsigned int *) __attr;
    return pthread_attr_setstack(realattr, stack_base, stack_size);
}

static int my_pthread_attr_getstack(pthread_attr_t const *__attr, void **stack_base, size_t *stack_size)
{
    pthread_attr_t *realattr = (pthread_attr_t *) *(unsigned int *) __attr;
    return pthread_attr_getstack(realattr, stack_base, stack_size);
}

static int my_pthread_attr_setguardsize(pthread_attr_t *__attr, size_t guard_size)
{
    pthread_attr_t *realattr = (pthread_attr_t *) *(unsigned int *) __attr;
    return pthread_attr_setguardsize(realattr, guard_size);
}

static int my_pthread_attr_getguardsize(pthread_attr_t const *__attr, size_t *guard_size)
{
    pthread_attr_t *realattr = (pthread_attr_t *) *(unsigned int *) __attr;
    return pthread_attr_getguardsize(realattr, guard_size);
}

static int my_pthread_attr_setscope(pthread_attr_t *__attr, int scope)
{
    pthread_attr_t *realattr = (pthread_attr_t *) *(unsigned int *) __attr;
    return pthread_attr_setscope(realattr, scope);
}

static int my_pthread_attr_getscope(pthread_attr_t const *__attr)
{
    int scope;
    pthread_attr_t *realattr = (pthread_attr_t *) *(unsigned int *) __attr;

    /* Android doesn't have the scope attribute because it always
     * returns PTHREAD_SCOPE_SYSTEM */
    pthread_attr_getscope(realattr, &scope);

    return scope;
}

static int my_pthread_getattr_np(pthread_t thid, pthread_attr_t *__attr)
{
    pthread_attr_t *realattr;

    realattr = malloc(sizeof(pthread_attr_t));
    *((unsigned int *)__attr) = (unsigned int) realattr;

    return pthread_getattr_np(thid, realattr);
}

/*
 * pthread_mutex* functions
 *
 * Specific implementations to workaround the differences between at the
 * pthread_mutex_t struct differences between Bionic and Glibc.
 *
 * */

static int my_pthread_mutex_init(pthread_mutex_t *__mutex,
                          __const pthread_mutexattr_t *__mutexattr)
{
    pthread_mutex_t *realmutex = NULL;

    int pshared = 0;
    if (__mutexattr)
        pthread_mutexattr_getpshared(__mutexattr, &pshared);

    if (!pshared) {
        /* non shared, standard mutex: use malloc */
        realmutex = malloc(sizeof(pthread_mutex_t));

        *((unsigned int *)__mutex) = (unsigned int) realmutex;
    }
    else {
        /* process-shared mutex: use the shared memory segment */
        hybris_shm_pointer_t handle = hybris_shm_alloc(sizeof(pthread_mutex_t));

        *((hybris_shm_pointer_t *)__mutex) = handle;

        if (handle)
            realmutex = (pthread_mutex_t *)hybris_get_shmpointer(handle);
    }

    return pthread_mutex_init(realmutex, __mutexattr);
}

static int my_pthread_mutex_destroy(pthread_mutex_t *__mutex)
{
    int ret;

    if (!__mutex)
        return EINVAL;

    pthread_mutex_t *realmutex = (pthread_mutex_t *) *(unsigned int *) __mutex;

    if (!realmutex)
        return EINVAL;

    if (!hybris_is_pointer_in_shm((void*)realmutex)) {
        ret = pthread_mutex_destroy(realmutex);
        free(realmutex);
    }
    else {
        realmutex = (pthread_mutex_t *)hybris_get_shmpointer((hybris_shm_pointer_t)realmutex);
        ret = pthread_mutex_destroy(realmutex);
    }

    *((unsigned int *)__mutex) = 0;

    return ret;
}

static int my_pthread_mutex_lock(pthread_mutex_t *__mutex)
{
    if (!__mutex) {
        LOGD("Null mutex lock, not locking.");
        return 0;
    }

    unsigned int value = (*(unsigned int *) __mutex);
    if (hybris_check_android_shared_mutex(value)) {
        LOGD("Shared mutex with Android, not locking.");
        return 0;
    }

    pthread_mutex_t *realmutex = (pthread_mutex_t *) value;
    if (hybris_is_pointer_in_shm((void*)value))
        realmutex = (pthread_mutex_t *)hybris_get_shmpointer((hybris_shm_pointer_t)value);

    if (value <= ANDROID_TOP_ADDR_VALUE_MUTEX) {
        realmutex = hybris_alloc_init_mutex(value);
        *((unsigned int *)__mutex) = (unsigned int) realmutex;
    }

    return pthread_mutex_lock(realmutex);
}

static int my_pthread_mutex_trylock(pthread_mutex_t *__mutex)
{
    unsigned int value = (*(unsigned int *) __mutex);

    if (hybris_check_android_shared_mutex(value)) {
        LOGD("Shared mutex with Android, not try locking.");
        return 0;
    }

    pthread_mutex_t *realmutex = (pthread_mutex_t *) value;
    if (hybris_is_pointer_in_shm((void*)value))
        realmutex = (pthread_mutex_t *)hybris_get_shmpointer((hybris_shm_pointer_t)value);

    if (value <= ANDROID_TOP_ADDR_VALUE_MUTEX) {
        realmutex = hybris_alloc_init_mutex(value);
        *((unsigned int *)__mutex) = (unsigned int) realmutex;
    }

    return pthread_mutex_trylock(realmutex);
}

static int my_pthread_mutex_unlock(pthread_mutex_t *__mutex)
{
    if (!__mutex) {
        LOGD("Null mutex lock, not unlocking.");
        return 0;
    }

    unsigned int value = (*(unsigned int *) __mutex);
    if (hybris_check_android_shared_mutex(value)) {
        LOGD("Shared mutex with Android, not unlocking.");
        return 0;
    }

    if (value <= ANDROID_TOP_ADDR_VALUE_MUTEX) {
        LOGD("Trying to unlock a lock that's not locked/initialized"
               " by Hybris, not unlocking.");
        return 0;
    }

    pthread_mutex_t *realmutex = (pthread_mutex_t *) value;
    if (hybris_is_pointer_in_shm((void*)value))
        realmutex = (pthread_mutex_t *)hybris_get_shmpointer((hybris_shm_pointer_t)value);

    return pthread_mutex_unlock(realmutex);
}

static int my_pthread_mutex_lock_timeout_np(pthread_mutex_t *__mutex, unsigned __msecs)
{
    struct timespec tv;
    pthread_mutex_t *realmutex;
    unsigned int value = (*(unsigned int *) __mutex);

    if (hybris_check_android_shared_mutex(value)) {
        LOGD("Shared mutex with Android, not lock timeout np.");
        return 0;
    }

    realmutex = (pthread_mutex_t *) value;

    if (value <= ANDROID_TOP_ADDR_VALUE_MUTEX) {
        realmutex = hybris_alloc_init_mutex(value);
        *((int *)__mutex) = (int) realmutex;
    }

    clock_gettime(CLOCK_REALTIME, &tv);
    tv.tv_sec += __msecs/1000;
    tv.tv_nsec += (__msecs % 1000) * 1000000;
    if (tv.tv_nsec >= 1000000000) {
      tv.tv_sec++;
      tv.tv_nsec -= 1000000000;
    }

    return pthread_mutex_timedlock(realmutex, &tv);
}

static int my_pthread_mutexattr_setpshared(pthread_mutexattr_t *__attr,
                                           int pshared)
{
    return pthread_mutexattr_setpshared(__attr, pshared);
}

/*
 * pthread_cond* functions
 *
 * Specific implementations to workaround the differences between at the
 * pthread_cond_t struct differences between Bionic and Glibc.
 *
 * */

static int my_pthread_cond_init(pthread_cond_t *cond,
                                const pthread_condattr_t *attr)
{
    pthread_cond_t *realcond = NULL;

    int pshared = 0;

    if (attr)
        pthread_condattr_getpshared(attr, &pshared);

    if (!pshared) {
        /* non shared, standard cond: use malloc */
        realcond = malloc(sizeof(pthread_cond_t));

        *((unsigned int *) cond) = (unsigned int) realcond;
    }
    else {
        /* process-shared condition: use the shared memory segment */
        hybris_shm_pointer_t handle = hybris_shm_alloc(sizeof(pthread_cond_t));

        *((unsigned int *)cond) = (unsigned int) handle;

        if (handle)
            realcond = (pthread_cond_t *)hybris_get_shmpointer(handle);
    }

    return pthread_cond_init(realcond, attr);
}

static int my_pthread_cond_destroy(pthread_cond_t *cond)
{
    int ret;
    pthread_cond_t *realcond = (pthread_cond_t *) *(unsigned int *) cond;

    if (!realcond) {
      return EINVAL;
    }

    if (!hybris_is_pointer_in_shm((void*)realcond)) {
        ret = pthread_cond_destroy(realcond);
        free(realcond);
    }
    else {
        realcond = (pthread_cond_t *)hybris_get_shmpointer((hybris_shm_pointer_t)realcond);
        ret = pthread_cond_destroy(realcond);
    }

    *((unsigned int *)cond) = 0;

    return ret;
}

static int my_pthread_cond_broadcast(pthread_cond_t *cond)
{
    unsigned int value = (*(unsigned int *) cond);
    if (hybris_check_android_shared_cond(value)) {
        LOGD("shared condition with Android, not broadcasting.");
        return 0;
    }

    pthread_cond_t *realcond = (pthread_cond_t *) value;
    if (hybris_is_pointer_in_shm((void*)value))
        realcond = (pthread_cond_t *)hybris_get_shmpointer((hybris_shm_pointer_t)value);

    if (value <= ANDROID_TOP_ADDR_VALUE_COND) {
        realcond = hybris_alloc_init_cond();
        *((unsigned int *) cond) = (unsigned int) realcond;
    }

    return pthread_cond_broadcast(realcond);
}

static int my_pthread_cond_signal(pthread_cond_t *cond)
{
    unsigned int value = (*(unsigned int *) cond);

    if (hybris_check_android_shared_cond(value)) {
        LOGD("Shared condition with Android, not signaling.");
        return 0;
    }

    pthread_cond_t *realcond = (pthread_cond_t *) value;
    if (hybris_is_pointer_in_shm((void*)value))
        realcond = (pthread_cond_t *)hybris_get_shmpointer((hybris_shm_pointer_t)value);

    if (value <= ANDROID_TOP_ADDR_VALUE_COND) {
        realcond = hybris_alloc_init_cond();
        *((unsigned int *) cond) = (unsigned int) realcond;
    }

    return pthread_cond_signal(realcond);
}

static int my_pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    /* Both cond and mutex can be statically initialized, check for both */
    unsigned int cvalue = (*(unsigned int *) cond);
    unsigned int mvalue = (*(unsigned int *) mutex);

    if (hybris_check_android_shared_cond(cvalue) ||
        hybris_check_android_shared_mutex(mvalue)) {
        LOGD("Shared condition/mutex with Android, not waiting.");
        return 0;
    }

    pthread_cond_t *realcond = (pthread_cond_t *) cvalue;
    if (hybris_is_pointer_in_shm((void*)cvalue))
        realcond = (pthread_cond_t *)hybris_get_shmpointer((hybris_shm_pointer_t)cvalue);

    if (cvalue <= ANDROID_TOP_ADDR_VALUE_COND) {
        realcond = hybris_alloc_init_cond();
        *((unsigned int *) cond) = (unsigned int) realcond;
    }

    pthread_mutex_t *realmutex = (pthread_mutex_t *) mvalue;
    if (hybris_is_pointer_in_shm((void*)mvalue))
        realmutex = (pthread_mutex_t *)hybris_get_shmpointer((hybris_shm_pointer_t)mvalue);

    if (mvalue <= ANDROID_TOP_ADDR_VALUE_MUTEX) {
        realmutex = hybris_alloc_init_mutex(mvalue);
        *((unsigned int *) mutex) = (unsigned int) realmutex;
    }

    return pthread_cond_wait(realcond, realmutex);
}

static int my_pthread_cond_timedwait(pthread_cond_t *cond,
                pthread_mutex_t *mutex, const struct timespec *abstime)
{
    /* Both cond and mutex can be statically initialized, check for both */
    unsigned int cvalue = (*(unsigned int *) cond);
    unsigned int mvalue = (*(unsigned int *) mutex);

    if (hybris_check_android_shared_cond(cvalue) ||
         hybris_check_android_shared_mutex(mvalue)) {
        LOGD("Shared condition/mutex with Android, not waiting.");
        return 0;
    }

    pthread_cond_t *realcond = (pthread_cond_t *) cvalue;
    if (hybris_is_pointer_in_shm((void*)cvalue))
        realcond = (pthread_cond_t *)hybris_get_shmpointer((hybris_shm_pointer_t)cvalue);

    if (cvalue <= ANDROID_TOP_ADDR_VALUE_COND) {
        realcond = hybris_alloc_init_cond();
        *((unsigned int *) cond) = (unsigned int) realcond;
    }

    pthread_mutex_t *realmutex = (pthread_mutex_t *) mvalue;
    if (hybris_is_pointer_in_shm((void*)mvalue))
        realmutex = (pthread_mutex_t *)hybris_get_shmpointer((hybris_shm_pointer_t)mvalue);

    if (mvalue <= ANDROID_TOP_ADDR_VALUE_MUTEX) {
        realmutex = hybris_alloc_init_mutex(mvalue);
        *((unsigned int *) mutex) = (unsigned int) realmutex;
    }

    return pthread_cond_timedwait(realcond, realmutex, abstime);
}

static int my_pthread_cond_timedwait_relative_np(pthread_cond_t *cond,
                pthread_mutex_t *mutex, const struct timespec *reltime)
{
    /* Both cond and mutex can be statically initialized, check for both */
    unsigned int cvalue = (*(unsigned int *) cond);
    unsigned int mvalue = (*(unsigned int *) mutex);

    if (hybris_check_android_shared_cond(cvalue) ||
         hybris_check_android_shared_mutex(mvalue)) {
        LOGD("Shared condition/mutex with Android, not waiting.");
        return 0;
    }

    pthread_cond_t *realcond = (pthread_cond_t *) cvalue;
    if( hybris_is_pointer_in_shm((void*)cvalue) )
        realcond = (pthread_cond_t *)hybris_get_shmpointer((hybris_shm_pointer_t)cvalue);

    if (cvalue <= ANDROID_TOP_ADDR_VALUE_COND) {
        realcond = hybris_alloc_init_cond();
        *((unsigned int *) cond) = (unsigned int) realcond;
    }

    pthread_mutex_t *realmutex = (pthread_mutex_t *) mvalue;
    if (hybris_is_pointer_in_shm((void*)mvalue))
        realmutex = (pthread_mutex_t *)hybris_get_shmpointer((hybris_shm_pointer_t)mvalue);

    if (mvalue <= ANDROID_TOP_ADDR_VALUE_MUTEX) {
        realmutex = hybris_alloc_init_mutex(mvalue);
        *((unsigned int *) mutex) = (unsigned int) realmutex;
    }

    struct timespec tv;
    clock_gettime(CLOCK_REALTIME, &tv);
    tv.tv_sec += reltime->tv_sec;
    tv.tv_nsec += reltime->tv_nsec;
    if (tv.tv_nsec >= 1000000000) {
      tv.tv_sec++;
      tv.tv_nsec -= 1000000000;
    }
    return pthread_cond_timedwait(realcond, realmutex, &tv);
}

/*
 * pthread_rwlockattr_* functions
 *
 * Specific implementations to workaround the differences between at the
 * pthread_rwlockattr_t struct differences between Bionic and Glibc.
 *
 * */

static int my_pthread_rwlockattr_init(pthread_rwlockattr_t *__attr)
{
    pthread_rwlockattr_t *realattr;

    realattr = malloc(sizeof(pthread_rwlockattr_t));
    *((unsigned int *)__attr) = (unsigned int) realattr;

    return pthread_rwlockattr_init(realattr);
}

static int my_pthread_rwlockattr_destroy(pthread_rwlockattr_t *__attr)
{
    int ret;
    pthread_rwlockattr_t *realattr = (pthread_rwlockattr_t *) *(unsigned int *) __attr;

    ret = pthread_rwlockattr_destroy(realattr);
    free(realattr);

    return ret;
}

static int my_pthread_rwlockattr_setpshared(pthread_rwlockattr_t *__attr,
                                            int pshared)
{
    pthread_rwlockattr_t *realattr = (pthread_rwlockattr_t *) *(unsigned int *) __attr;
    return pthread_rwlockattr_setpshared(realattr, pshared);
}

static int my_pthread_rwlockattr_getpshared(pthread_rwlockattr_t *__attr,
                                            int *pshared)
{
    pthread_rwlockattr_t *realattr = (pthread_rwlockattr_t *) *(unsigned int *) __attr;
    return pthread_rwlockattr_getpshared(realattr, pshared);
}

/*
 * pthread_rwlock_* functions
 *
 * Specific implementations to workaround the differences between at the
 * pthread_rwlock_t struct differences between Bionic and Glibc.
 *
 * */

static int my_pthread_rwlock_init(pthread_rwlock_t *__rwlock,
                                  __const pthread_rwlockattr_t *__attr)
{
    pthread_rwlock_t *realrwlock = NULL;
    pthread_rwlockattr_t *realattr = NULL;
    int pshared = 0;

    if (__attr != NULL)
        realattr = (pthread_rwlockattr_t *) *(unsigned int *) __attr;

    if (realattr)
        pthread_rwlockattr_getpshared(realattr, &pshared);

    if (!pshared) {
        /* non shared, standard rwlock: use malloc */
        realrwlock = malloc(sizeof(pthread_rwlock_t));

        *((unsigned int *) __rwlock) = (unsigned int) realrwlock;
    }
    else {
        /* process-shared condition: use the shared memory segment */
        hybris_shm_pointer_t handle = hybris_shm_alloc(sizeof(pthread_rwlock_t));

        *((unsigned int *)__rwlock) = (unsigned int) handle;

        if (handle)
            realrwlock = (pthread_rwlock_t *)hybris_get_shmpointer(handle);
    }

    return pthread_rwlock_init(realrwlock, realattr);
}

static int my_pthread_rwlock_destroy(pthread_rwlock_t *__rwlock)
{
    int ret;
    pthread_rwlock_t *realrwlock = (pthread_rwlock_t *) *(unsigned int *) __rwlock;

    if (!hybris_is_pointer_in_shm((void*)realrwlock)) {
        ret = pthread_rwlock_destroy(realrwlock);
        free(realrwlock);
    }
    else {
        ret = pthread_rwlock_destroy(realrwlock);
        realrwlock = (pthread_rwlock_t *)hybris_get_shmpointer((hybris_shm_pointer_t)realrwlock);
    }

    return ret;
}

static pthread_rwlock_t* hybris_set_realrwlock(pthread_rwlock_t *rwlock)
{
    unsigned int value = (*(unsigned int *) rwlock);
    pthread_rwlock_t *realrwlock = (pthread_rwlock_t *) value;
    if (hybris_is_pointer_in_shm((void*)value))
        realrwlock = (pthread_rwlock_t *)hybris_get_shmpointer((hybris_shm_pointer_t)value);

    if (realrwlock <= ANDROID_TOP_ADDR_VALUE_RWLOCK) {
        realrwlock = hybris_alloc_init_rwlock();
        *((unsigned int *)rwlock) = (unsigned int) realrwlock;
    }
    return realrwlock;
}

static int my_pthread_rwlock_rdlock(pthread_rwlock_t *__rwlock)
{
    pthread_rwlock_t *realrwlock = hybris_set_realrwlock(__rwlock);
    return pthread_rwlock_rdlock(realrwlock);
}

static int my_pthread_rwlock_tryrdlock(pthread_rwlock_t *__rwlock)
{
    pthread_rwlock_t *realrwlock = hybris_set_realrwlock(__rwlock);
    return pthread_rwlock_tryrdlock(realrwlock);
}

static int my_pthread_rwlock_timedrdlock(pthread_rwlock_t *__rwlock,
                                         __const struct timespec *abs_timeout)
{
    pthread_rwlock_t *realrwlock = hybris_set_realrwlock(__rwlock);
    return pthread_rwlock_timedrdlock(realrwlock, abs_timeout);
}

static int my_pthread_rwlock_wrlock(pthread_rwlock_t *__rwlock)
{
    pthread_rwlock_t *realrwlock = hybris_set_realrwlock(__rwlock);
    return pthread_rwlock_wrlock(realrwlock);
}

static int my_pthread_rwlock_trywrlock(pthread_rwlock_t *__rwlock)
{
    pthread_rwlock_t *realrwlock = hybris_set_realrwlock(__rwlock);
    return pthread_rwlock_trywrlock(realrwlock);
}

static int my_pthread_rwlock_timedwrlock(pthread_rwlock_t *__rwlock,
                                         __const struct timespec *abs_timeout)
{
    pthread_rwlock_t *realrwlock = hybris_set_realrwlock(__rwlock);
    return pthread_rwlock_timedwrlock(realrwlock, abs_timeout);
}

static int my_pthread_rwlock_unlock(pthread_rwlock_t *__rwlock)
{
    unsigned int value = (*(unsigned int *) __rwlock);
    if (value <= ANDROID_TOP_ADDR_VALUE_RWLOCK) {
        LOGD("Trying to unlock a rwlock that's not locked/initialized"
               " by Hybris, not unlocking.");
        return 0;
    }
    pthread_rwlock_t *realrwlock = (pthread_rwlock_t *) value;
    if (hybris_is_pointer_in_shm((void*)value))
        realrwlock = (pthread_rwlock_t *)hybris_get_shmpointer((hybris_shm_pointer_t)value);

    return pthread_rwlock_unlock(realrwlock);
}

static void my_pthread_cleanup_push(void (*routine)(void *),
                                   void *arg)
{
    LOGD("pthread_cleanup_push: not implemented");
}

static void my_pthread_cleanup_pop(int execute)
{
    LOGD("pthread_cleanup_pop: not implemented");
}


static int my_set_errno(int oi_errno)
{
    errno = oi_errno;
    return -1;
}

/*
 * __isthreaded is used in bionic's stdio.h to choose between a fast internal implementation
 * and a more classic stdio function call.
 * For example:
 * #define  __sfeof(p)  (((p)->_flags & __SEOF) != 0)
 * #define  feof(p)     (!__isthreaded ? __sfeof(p) : (feof)(p))
 *
 * We see here that if __isthreaded is false, then it will use directly the bionic's FILE structure
 * instead of calling one of the hooked methods.
 * Therefore we need to set __isthreaded to true, even if we are not in a multi-threaded context.
 */
static int __my_isthreaded = 1;

struct aFILE {
    unsigned char* _p;
    int	_r, _w;
#if defined(__LP64__)
    int	_flags, _file;
#else
    short _flags, _file;
#endif
    FILE* actual;
};

/*
 * redirection for bionic's __sF, which is defined as:
 *   FILE __sF[3];
 *   #define stdin  &__sF[0];
 *   #define stdout &__sF[1];
 *   #define stderr &__sF[2];
 *   So the goal here is to catch the call to file methods where the FILE* pointer
 *   is either stdin, stdout or stderr, and translate that pointer to a valid glibc
 *   pointer.
 *   Currently, only fputs is managed.
 */
#define BIONIC_SIZEOF_FILE 84
static char my_sF[3*BIONIC_SIZEOF_FILE] = {0};
static FILE *_get_actual_fp(struct aFILE *fp)
{
    char *c_fp = (char*)fp;
    if (c_fp == &my_sF[0])
        return stdin;
    else if (c_fp == &my_sF[BIONIC_SIZEOF_FILE])
        return stdout;
    else if (c_fp == &my_sF[BIONIC_SIZEOF_FILE*2])
        return stderr;

    return fp->actual;
}

static void my_clearerr(FILE *fp)
{
    clearerr(_get_actual_fp(fp));
}

static struct aFILE* my_fopen(const char *filename, const char *mode)
{
    FILE* file = fopen(filename, mode);
    if (file == NULL)
        return NULL;
    struct aFILE* afile = (struct aFILE*) malloc(sizeof(struct aFILE));
    afile->_file = fileno(file);
    afile->actual = file;
    afile->_flags = 0;
    return afile;
}

static struct aFILE* my_fdopen(const char *filename, const char *mode)
{
    FILE* file = fdopen(filename, mode);
    if (file == NULL)
        return NULL;
    struct aFILE* afile = (struct aFILE*) malloc(sizeof(struct aFILE));
    afile->_file = fileno(file);
    afile->actual = file;
    return afile;
}

static int my_fclose(struct aFILE* fp)
{
    return fclose(_get_actual_fp(fp));
}

static int my_feof(struct aFILE* fp)
{
    return feof(_get_actual_fp(fp));
}

static int my_ferror(struct aFILE* fp)
{
    return ferror(_get_actual_fp(fp));
}

static int my_fflush(struct aFILE* fp)
{
    return fflush(_get_actual_fp(fp));
}

static int my_fgetc(struct aFILE* fp)
{
    return fgetc(_get_actual_fp(fp));
}

static int my_fgetpos(struct aFILE* fp, fpos_t *pos)
{
    return fgetpos(_get_actual_fp(fp), pos);
}

static char* my_fgets(char *s, int n, struct aFILE* fp)
{
    return fgets(s, n, _get_actual_fp(fp));
}

FP_ATTRIB static int my_fprintf(struct aFILE* fp, const char *fmt, ...)
{
    int ret = 0;

    va_list args;
    va_start(args,fmt);
    ret = vfprintf(_get_actual_fp(fp), fmt, args);
    va_end(args);

    return ret;
}

static int my_fputc(int c, struct aFILE* fp)
{
    return fputc(c, _get_actual_fp(fp));
}

static int my_fputs(const char *s, struct aFILE* fp)
{
    return fputs(s, _get_actual_fp(fp));
}

static size_t my_fread(void *ptr, size_t size, size_t nmemb, struct aFILE* fp)
{
    size_t ret = fread(ptr, size, nmemb, _get_actual_fp(fp));
    fp->_flags = (short) (feof_unlocked(_get_actual_fp(fp)) ? 0x0020 : 0);
    return ret;
}

static FILE* my_freopen(const char *filename, const char *mode, struct aFILE* fp)
{
    return freopen(filename, mode, _get_actual_fp(fp));
}

FP_ATTRIB static int my_fscanf(struct aFILE* fp, const char *fmt, ...)
{
    int ret = 0;

    va_list args;
    va_start(args,fmt);
    ret = vfscanf(_get_actual_fp(fp), fmt, args);
    va_end(args);

    return ret;
}

static int my_fseek(struct aFILE* fp, long offset, int whence)
{
    return fseek(_get_actual_fp(fp), offset, whence);
}

static int my_fseeko(struct aFILE* fp, off_t offset, int whence)
{
    return fseeko(_get_actual_fp(fp), offset, whence);
}

static int my_fsetpos(struct aFILE* fp, const fpos_t *pos)
{
    return fsetpos(_get_actual_fp(fp), pos);
}

static long my_ftell(struct aFILE* fp)
{
    return ftell(_get_actual_fp(fp));
}

static off_t my_ftello(struct aFILE* fp)
{
    return ftello(_get_actual_fp(fp));
}

static size_t my_fwrite(const void *ptr, size_t size, size_t nmemb, struct aFILE* fp)
{
    return fwrite(ptr, size, nmemb, _get_actual_fp(fp));
}

static int my_getc(struct aFILE* fp)
{
    return getc(_get_actual_fp(fp));
}

static ssize_t my_getdelim(char ** lineptr, size_t *n, int delimiter, struct aFILE* fp)
{
    return getdelim(lineptr, n, delimiter, _get_actual_fp(fp));
}

static ssize_t my_getline(char **lineptr, size_t *n, struct aFILE* fp)
{
    return getline(lineptr, n, _get_actual_fp(fp));
}


static int my_putc(int c, struct aFILE* fp)
{
    return putc(c, _get_actual_fp(fp));
}

static void my_rewind(struct aFILE* fp)
{
    rewind(_get_actual_fp(fp));
}

static void my_setbuf(struct aFILE* fp, char *buf)
{
    setbuf(_get_actual_fp(fp), buf);
}

static int my_setvbuf(struct aFILE* fp, char *buf, int mode, size_t size)
{
    return setvbuf(_get_actual_fp(fp), buf, mode, size);
}

static int my_ungetc(int c, struct aFILE* fp)
{
    return ungetc(c, _get_actual_fp(fp));
}

static int my_vfprintf(struct aFILE* fp, const char *fmt, va_list arg)
{
    return vfprintf(_get_actual_fp(fp), fmt, arg);
}


static int my_vfscanf(struct aFILE* fp, const char *fmt, va_list arg)
{
    return vfscanf(_get_actual_fp(fp), fmt, arg);
}

static int my_fileno(struct aFILE* fp)
{
    return fileno(_get_actual_fp(fp));
}


static int my_pclose(struct aFILE* fp)
{
    return pclose(_get_actual_fp(fp));
}

static void my_flockfile(struct aFILE* fp)
{
    return flockfile(_get_actual_fp(fp));
}

static int my_ftrylockfile(struct aFILE* fp)
{
    return ftrylockfile(_get_actual_fp(fp));
}

static void my_funlockfile(struct aFILE* fp)
{
    return funlockfile(_get_actual_fp(fp));
}


static int my_getc_unlocked(struct aFILE* fp)
{
    return getc_unlocked(_get_actual_fp(fp));
}

static int my_putc_unlocked(int c, struct aFILE* fp)
{
    return putc_unlocked(c, _get_actual_fp(fp));
}

/* exists only on the BSD platform
static char* my_fgetln(FILE *fp, size_t *len)
{
    return fgetln(_get_actual_fp(fp), len);
}
*/
static int my_fpurge(struct aFILE* fp)
{
    __fpurge(_get_actual_fp(fp));

    return 0;
}

static int my_getw(struct aFILE* fp)
{
    return getw(_get_actual_fp(fp));
}

static int my_putw(int w, struct aFILE* fp)
{
    return putw(w, _get_actual_fp(fp));
}

static void my_setbuffer(struct aFILE* fp, char *buf, int size)
{
    setbuffer(_get_actual_fp(fp), buf, size);
}

static int my_setlinebuf(struct aFILE* fp)
{
    setlinebuf(_get_actual_fp(fp));

    return 0;
}

static wint_t my_getwc(struct aFILE* fp)
{
    return getwc(_get_actual_fp(fp));
}

static wint_t my_ungetwc(wint_t c, struct aFILE* fp)
{
    return ungetwc(c, _get_actual_fp(fp));
}

static wint_t my_putwc(wchar_t c, struct aFILE* fp)
{
    return putwc(c, _get_actual_fp(fp));
}

/* "struct dirent" from bionic/libc/include/dirent.h */
struct bionic_dirent {
    uint64_t         d_ino;
    int64_t          d_off;
    unsigned short   d_reclen;
    unsigned char    d_type;
    char             d_name[256];
};

static struct bionic_dirent *my_readdir(DIR *dirp)
{
    /**
     * readdir(3) manpage says:
     *  The data returned by readdir() may be overwritten by subsequent calls
     *  to readdir() for the same directory stream.
     *
     * XXX: At the moment, for us, the data will be overwritten even by
     * subsequent calls to /different/ directory streams. Eventually fix that
     * (e.g. by storing per-DIR * bionic_dirent structs, and removing them on
     * closedir, requires hooking of all funcs returning/taking DIR *) and
     * handling the additional data attachment there)
     **/

    static struct bionic_dirent result;

    struct dirent *real_result = readdir(dirp);
    if (!real_result) {
        return NULL;
    }

    result.d_ino = real_result->d_ino;
    result.d_off = real_result->d_off;
    result.d_reclen = real_result->d_reclen;
    result.d_type = real_result->d_type;
    memcpy(result.d_name, real_result->d_name, sizeof(result.d_name));

    // Make sure the string is zero-terminated, even if cut off (which
    // shouldn't happen, as both bionic and glibc have d_name defined
    // as fixed array of 256 chars)
    result.d_name[sizeof(result.d_name)-1] = '\0';
    return &result;
}

static int my_readdir_r(DIR *dir, struct bionic_dirent *entry,
        struct bionic_dirent **result)
{
    struct dirent entry_r;
    struct dirent *result_r;

    int res = readdir_r(dir, &entry_r, &result_r);

    if (res == 0) {
        if (result_r != NULL) {
            *result = entry;

            entry->d_ino = entry_r.d_ino;
            entry->d_off = entry_r.d_off;
            entry->d_reclen = entry_r.d_reclen;
            entry->d_type = entry_r.d_type;
            memcpy(entry->d_name, entry_r.d_name, sizeof(entry->d_name));

            // Make sure the string is zero-terminated, even if cut off (which
            // shouldn't happen, as both bionic and glibc have d_name defined
            // as fixed array of 256 chars)
            entry->d_name[sizeof(entry->d_name) - 1] = '\0';
        } else {
            *result = NULL;
        }
    }

    return res;
}

static int my_alphasort(struct bionic_dirent **a,
                        struct bionic_dirent **b)
{
    return strcoll((*a)->d_name, (*b)->d_name);
}

static int my_versionsort(struct bionic_dirent **a,
                          struct bionic_dirent **b)
{
    return strverscmp((*a)->d_name, (*b)->d_name);
}

static int my_scandirat(int fd, const char *dir,
                      struct bionic_dirent ***namelist,
                      int (*filter) (const struct bionic_dirent *),
                      int (*compar) (const struct bionic_dirent **,
                                     const struct bionic_dirent **))
{
    struct dirent **namelist_r;
    struct bionic_dirent **result;
    struct bionic_dirent *filter_r;

    int i = 0;
    size_t nItems = 0;

    int res = scandirat(fd, dir, &namelist_r, NULL, NULL);

    if (res != 0 && namelist_r != NULL) {

        result = malloc(res * sizeof(struct bionic_dirent));
        if (!result)
            return -1;

        for (i = 0; i < res; i++) {
            filter_r = malloc(sizeof(struct bionic_dirent));
            if (!filter_r) {
                while (i-- > 0)
                        free(result[i]);
                    free(result);
                    return -1;
            }
            filter_r->d_ino = namelist_r[i]->d_ino;
            filter_r->d_off = namelist_r[i]->d_off;
            filter_r->d_reclen = namelist_r[i]->d_reclen;
            filter_r->d_type = namelist_r[i]->d_type;

            strcpy(filter_r->d_name, namelist_r[i]->d_name);
            filter_r->d_name[sizeof(namelist_r[i]->d_name) - 1] = '\0';

            if (filter != NULL && !(*filter)(filter_r)) {//apply filter
                free(filter_r);
                continue;
            }

            result[nItems++] = filter_r;
        }
        if (nItems && compar != NULL) // sort
            qsort(result, nItems, sizeof(struct bionic_dirent *), compar);

        *namelist = result;
    }

    return res;
}

static int my_scandir(const char *dir,
                      struct bionic_dirent ***namelist,
                      int (*filter) (const struct bionic_dirent *),
                      int (*compar) (const struct bionic_dirent **,
                                     const struct bionic_dirent **))
{
    return my_scandirat(AT_FDCWD, dir, namelist, filter, compar);
}

extern long my_sysconf(int name);

FP_ATTRIB static double my_strtod(const char *nptr, char **endptr)
{
	if (locale_inited == 0)
	{
		hybris_locale = newlocale(LC_ALL_MASK, "C", 0);
		locale_inited = 1;
	}
	return strtod_l(nptr, endptr, hybris_locale);
}

extern int __cxa_atexit(void (*)(void*), void*, void*);
extern void __cxa_finalize(void * d);

struct open_redirect {
	const char *from;
	const char *to;
};

struct open_redirect open_redirects[] = {
	{ "/dev/log/main", "/dev/log_main" },
	{ "/dev/log/radio", "/dev/log_radio" },
	{ "/dev/log/system", "/dev/log_system" },
	{ "/dev/log/events", "/dev/log_events" },
	{ NULL, NULL }
};

int my_open(const char *pathname, int flags, ...)
{
	va_list ap;
	mode_t mode = 0;
	const char *target_path = pathname;

	if (pathname != NULL) {
		struct open_redirect *entry = &open_redirects[0];
		while (entry->from != NULL) {
			if (strcmp(pathname, entry->from) == 0) {
				target_path = entry->to;
				break;
			}
			entry++;
		}
	}

	if (flags & O_CREAT) {
		va_start(ap, flags);
		mode = va_arg(ap, mode_t);
		va_end(ap);
	}

	return open(target_path, flags, mode);
}

struct bionic_stat64 {
    unsigned long long st_dev;
    unsigned char __pad0[4];
    unsigned long __st_ino;
    unsigned int st_mode;
    nlink_t st_nlink;
    uid_t st_uid;
    gid_t st_gid;
    unsigned long long st_rdev;
    unsigned char __pad3[4];
    long long st_size;
    unsigned long st_blksize;
    unsigned long long st_blocks;
    struct timespec st_atim;
    struct timespec st_mtim;
    struct timespec st_ctim;
    unsigned long long st_ino;
};

void stat_to_bionic_stat(struct stat *s, struct bionic_stat64 *b) {
    b->st_dev = s->st_dev;
    b->__st_ino = s->st_ino;
    b->st_mode = s->st_mode;
    b->st_nlink = s->st_nlink;
    b->st_uid = s->st_uid;
    b->st_gid = s->st_gid;
    b->st_rdev = s->st_rdev;
    b->st_size = s->st_size;
    b->st_blksize = (unsigned long) s->st_blksize;
    b->st_blocks = (unsigned long long) s->st_blocks;
    b->st_atim = s->st_atim;
    b->st_mtim = s->st_mtim;
    b->st_ctim = s->st_ctim;
    b->st_ino = s->st_ino;
}

void stat64_to_bionic_stat(struct stat64 *s, struct bionic_stat64 *b) {
    b->st_dev = s->st_dev;
    b->__st_ino = s->__st_ino;
    b->st_mode = s->st_mode;
    b->st_nlink = s->st_nlink;
    b->st_uid = s->st_uid;
    b->st_gid = s->st_gid;
    b->st_rdev = s->st_rdev;
    b->st_size = s->st_size;
    b->st_blksize = (unsigned long) s->st_blksize;
    b->st_blocks = (unsigned long long) s->st_blocks;
    b->st_atim = s->st_atim;
    b->st_mtim = s->st_mtim;
    b->st_ctim = s->st_ctim;
    b->st_ino = s->st_ino;
}

int my_stat(const char* path, struct bionic_stat64 *s)
{
    struct stat tmp;
    int ret = stat(path, &tmp);
    stat_to_bionic_stat(&tmp, s);
    return ret;
}

int my_fstat(int fd, struct bionic_stat64 *s)
{
    struct stat tmp;
    int ret = fstat(fd, &tmp);
    stat_to_bionic_stat(&tmp, s);
    return ret;
}

int my_stat64(const char* path, struct bionic_stat64 *s)
{
    struct stat64 tmp;
    int ret = stat64(path, &tmp);
    stat64_to_bionic_stat(&tmp, s);
    return ret;
}

int my_fstat64(int fd, struct bionic_stat64 *s)
{
    struct stat64 tmp;
    int ret = fstat64(fd, &tmp);
    stat64_to_bionic_stat(&tmp, s);
    return ret;
}

/**
 * NOTE: Normally we don't have to wrap __system_property_get (libc.so) as it is only used
 * through the property_get (libcutils.so) function. However when property_get is used
 * internally in libcutils.so we don't have any chance to hook our replacement in.
 * Therefore we have to hook __system_property_get too and just replace it with the
 * implementation of our internal property handling
 */

int my_system_property_get(const char *name, const char *value)
{
	return property_get(name, value, NULL);
}

static __thread void *tls_hooks[16];

void *__get_tls_hooks()
{
  return tls_hooks;
}

struct android_addrinfo {
    int	ai_flags;	/* AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST */
    int	ai_family;	/* PF_xxx */
    int	ai_socktype;	/* SOCK_xxx */
    int	ai_protocol;	/* 0 or IPPROTO_xxx for IPv4 and IPv6 */
    socklen_t ai_addrlen;	/* length of ai_addr */
    char	*ai_canonname;	/* canonical name for hostname */
    struct	sockaddr *ai_addr;	/* binary address */
    struct	android_addrinfo *ai_next;	/* next structure in linked list */
};
struct android_addrinfo* convert_addrinfo(struct addrinfo* res) {
    struct android_addrinfo* ares = (struct android_addrinfo*) malloc(sizeof(struct android_addrinfo));
    ares->ai_flags = res->ai_flags;
    ares->ai_family = res->ai_family;
    ares->ai_socktype = res->ai_socktype;
    ares->ai_protocol = res->ai_protocol;
    ares->ai_addrlen = res->ai_addrlen;
    ares->ai_canonname = res->ai_canonname;
    ares->ai_addr = res->ai_addr;
    ares->ai_next = NULL;
    if (res->ai_next != NULL) {
        ares->ai_next = convert_addrinfo(res->ai_next);
    }
    return ares;
}

int my_getaddrinfo(const char *node, const char *service,
                const struct android_addrinfo *ahints,
                struct android_addrinfo **ares) {
    struct addrinfo hints;
    if (ahints != NULL) {
        hints.ai_flags = ahints->ai_flags;
        hints.ai_family = ahints->ai_family;
        hints.ai_socktype = ahints->ai_socktype;
        hints.ai_protocol = ahints->ai_protocol;
        hints.ai_addrlen = ahints->ai_addrlen;
        hints.ai_canonname = ahints->ai_canonname;
        hints.ai_addr = ahints->ai_addr;
    }
    struct addrinfo* res;
    int ret = getaddrinfo(node, service, (ahints == NULL ? NULL : &hints), &res);
    if (ret != 0) {
        return ret;
    }
    if (res != NULL) {
        *ares = convert_addrinfo(res);
        freeaddrinfo(res);
    } else {
        *ares = NULL;
    }
    return ret;
}

void my_freeaddrinfo(struct android_addrinfo *ai) {
    struct android_addrinfo *ai_next;
    while (ai) {
        if (ai->ai_canonname)
            free(ai->ai_canonname);
        ai_next = ai->ai_next;
        free(ai);
        ai = ai_next;
    }
}

#define	A_NI_NOFQDN	0x00000001
#define	A_NI_NUMERICHOST	0x00000002
#define	A_NI_NAMEREQD	0x00000004
#define	A_NI_NUMERICSERV	0x00000008
#define	A_NI_DGRAM	0x00000010

int my_getnameinfo (const struct sockaddr *__restrict sa,
                    socklen_t salen, char *__restrict host,
                    socklen_t hostlen, char *__restrict serv,
                    socklen_t servlen, int flags) {
    int glibc_flags = 0;
    if (flags & A_NI_NOFQDN)
        glibc_flags |= A_NI_NOFQDN;
    if (flags & A_NI_NUMERICHOST)
        glibc_flags |= NI_NUMERICHOST;
    if (flags & A_NI_NAMEREQD)
        glibc_flags |= NI_NAMEREQD;
    if (flags & A_NI_NUMERICSERV)
        glibc_flags |= NI_NUMERICSERV;
    if (flags & A_NI_DGRAM)
        glibc_flags |= NI_DGRAM;
    return getnameinfo(sa, salen, host, hostlen, serv, servlen, glibc_flags);
}

extern off_t __umoddi3(off_t a, off_t b);
extern off_t __udivdi3(off_t a, off_t b);
extern off_t __divdi3(off_t a, off_t b);

void _hybris_stack_stack_chk_fail() {
    printf("__stack_chk_fail\n");
    abort();
}


void *get_hooked_symbol(const char *sym);
void *my_android_dlsym(void *handle, const char *symbol)
{
    void *retval = get_hooked_symbol(symbol);
    if (retval != NULL) {
        return retval;
    }
    return android_dlsym(handle, symbol);
}

static struct _hook hooks[] = {
    {"property_get", property_get },
    {"property_set", property_set },
    {"__system_property_get", my_system_property_get },
    {"__stack_chk_fail", _hybris_stack_stack_chk_fail},
    {"__stack_chk_guard", &_hybris_stack_chk_guard},
    {"printf", printf },
    {"malloc", my_malloc },
    {"memalign", memalign },
    {"pvalloc", pvalloc },
    {"getxattr", getxattr},
    {"__assert", __assert },
    {"__assert2", __assert },
    {"uname", uname },
    {"sched_yield", sched_yield},
    {"ldexp", ldexp},
    {"getrlimit", getrlimit},
    {"gettimeofday", gettimeofday},
    {"ioctl", ioctl},
    {"select", select},
    {"utime", utime},
    {"poll", poll},
    {"setlocale", setlocale},
    {"__umoddi3", __umoddi3},
    {"__udivdi3", __udivdi3},
    {"__divdi3", __divdi3},
    /* stdlib.h */
    {"__ctype_get_mb_cur_max", __ctype_get_mb_cur_max},
    {"atof", atof},
    {"atoi", atoi},
    {"atol", atol},
    {"atoll", atoll},
    {"strtod", strtod},
    {"strtof", strtof},
    {"strtold", strtold},
    {"strtol", strtol},
    {"strtoul", strtoul},
    {"strtoq", strtoq},
    {"strtouq", strtouq},
    {"strtoll", strtoll},
    {"strtoull", strtoull},
    {"strtol_l", strtol_l},
    {"strtoul_l", strtoul_l},
    {"strtoll_l", strtoll_l},
    {"strtoull_l", strtoull_l},
    {"strtod_l", strtod_l},
    {"strtof_l", strtof_l},
    {"strtold_l", strtold_l},
    {"l64a", l64a},
    {"a64l", a64l},
    {"random", random},
    {"srandom", srandom},
    {"initstate", initstate},
    {"setstate", setstate},
    {"random_r", random_r},
    {"srandom_r", srandom_r},
    {"initstate_r", initstate_r},
    {"setstate_r", setstate_r},
    {"rand", rand},
    {"srand", srand},
    {"rand_r", rand_r},
    {"drand48", drand48},
    {"erand48", erand48},
    {"lrand48", lrand48},
    {"nrand48", nrand48},
    {"mrand48", mrand48},
    {"jrand48", jrand48},
    {"srand48", srand48},
    {"seed48", seed48},
    {"lcong48", lcong48},
    {"drand48_r", drand48_r},
    {"erand48_r", erand48_r},
    {"lrand48_r", lrand48_r},
    {"nrand48_r", nrand48_r},
    {"mrand48_r", mrand48_r},
    {"jrand48_r", jrand48_r},
    {"srand48_r", srand48_r},
    {"seed48_r", seed48_r},
    {"lcong48_r", lcong48_r},
    {"malloc", malloc},
    {"calloc", calloc},
    {"realloc", realloc},
    {"free", free},
    {"valloc", valloc},
    {"posix_memalign", posix_memalign},
    {"aligned_alloc", aligned_alloc},
    {"abort", abort},
    {"atexit", atexit},
    {"on_exit", on_exit},
    {"exit", exit},
    {"quick_exit", quick_exit},
    {"_Exit", _Exit},
    {"getenv", getenv},
    {"secure_getenv", secure_getenv},
    {"putenv", putenv},
    {"setenv", setenv},
    {"unsetenv", unsetenv},
    {"clearenv", clearenv},
    {"mkstemp", mkstemp},
    {"mkstemp64", mkstemp64},
    {"mkstemps", mkstemps},
    {"mkstemps64", mkstemps64},
    {"mkdtemp", mkdtemp},
    {"mkostemp", mkostemp},
    {"mkostemp64", mkostemp64},
    {"mkostemps", mkostemps},
    {"mkostemps64", mkostemps64},
    {"system", system},
    {"canonicalize_file_name", canonicalize_file_name},
    {"realpath", realpath},
    {"bsearch", bsearch},
    {"qsort", qsort},
    {"qsort_r", qsort_r},
    {"abs", abs},
    {"labs", labs},
    {"llabs", llabs},
    {"div", div},
    {"ldiv", ldiv},
    {"lldiv", lldiv},
    {"ecvt", ecvt},
    {"fcvt", fcvt},
    {"gcvt", gcvt},
    {"qecvt", qecvt},
    {"qfcvt", qfcvt},
    {"qgcvt", qgcvt},
    {"ecvt_r", ecvt_r},
    {"fcvt_r", fcvt_r},
    {"qecvt_r", qecvt_r},
    {"qfcvt_r", qfcvt_r},
    {"mblen", mblen},
    {"mbtowc", mbtowc},
    {"wctomb", wctomb},
    {"mbstowcs", mbstowcs},
    {"wcstombs", wcstombs},
    {"rpmatch", rpmatch},
    {"getsubopt", getsubopt},
    {"posix_openpt", posix_openpt},
    {"grantpt", grantpt},
    {"unlockpt", unlockpt},
    {"ptsname", ptsname},
    {"ptsname_r", ptsname_r},
    {"getpt", getpt},
    {"getloadavg", getloadavg},
    /* string.h */
    {"memccpy",memccpy},
    {"memchr",memchr},
    {"memrchr",memrchr},
    {"memcmp",memcmp},
    {"memcpy",my_memcpy},
    {"memmove",memmove},
    {"memset",memset},
    {"memmem",memmem},
    //  {"memswap",memswap},
    {"index",index},
    {"rindex",rindex},
    {"strchr",strchr},
    {"strrchr",strrchr},
    {"strlen",my_strlen},
    {"__strlen_chk",my_strlen_chk},
    {"strcmp",strcmp},
    {"strcpy",strcpy},
    {"strcat",strcat},
    {"strcasecmp",strcasecmp},
    {"strncasecmp",strncasecmp},
    {"strdup",strdup},
    {"strstr",strstr},
    {"strtok",strtok},
    {"strtok_r",strtok_r},
    {"strerror",strerror},
    {"strerror_r",strerror_r},
    {"strnlen",strnlen},
    {"strncat",strncat},
    {"strndup",strndup},
    {"strncmp",strncmp},
    {"strncpy",strncpy},
    {"strtod", my_strtod},
    //{"strlcat",strlcat},
    {"strlcpy",strlcpy},
    {"strcspn",strcspn},
    {"strpbrk",strpbrk},
    {"strsep",strsep},
    {"strspn",strspn},
    {"strsignal",strsignal},
    {"getgrnam", getgrnam},
    {"strcoll",strcoll},
    {"strxfrm",strxfrm},
    /* wchar.h */
    {"wcslen",wcslen},
    /* strings.h */
    {"bcmp",bcmp},
    {"bcopy",bcopy},
    {"bzero",bzero},
    {"ffs",ffs},
    {"index",index},
    {"rindex",rindex},
    {"strcasecmp",strcasecmp},
    {"strncasecmp",strncasecmp},
    /* dirent.h */
    {"opendir", opendir},
    {"closedir", closedir},
    /* pthread.h */
    {"getauxval", getauxval},
    {"gettid", my_gettid},
    {"getpid", getpid},
    {"pthread_atfork", pthread_atfork},
    {"pthread_create", my_pthread_create},
    {"pthread_kill", pthread_kill},
    {"pthread_exit", pthread_exit},
    {"pthread_join", pthread_join},
    {"pthread_detach", pthread_detach},
    {"pthread_self", pthread_self},
    {"pthread_equal", pthread_equal},
    {"pthread_getschedparam", pthread_getschedparam},
    {"pthread_setschedparam", pthread_setschedparam},
    {"pthread_mutex_init", my_pthread_mutex_init},
    {"pthread_mutex_destroy", my_pthread_mutex_destroy},
    {"pthread_mutex_lock", my_pthread_mutex_lock},
    {"pthread_mutex_unlock", my_pthread_mutex_unlock},
    {"pthread_mutex_trylock", my_pthread_mutex_trylock},
    {"pthread_mutex_lock_timeout_np", my_pthread_mutex_lock_timeout_np},
    {"pthread_mutexattr_init", pthread_mutexattr_init},
    {"pthread_mutexattr_destroy", pthread_mutexattr_destroy},
    {"pthread_mutexattr_gettype", pthread_mutexattr_gettype},
    {"pthread_mutexattr_settype", pthread_mutexattr_settype},
    {"pthread_mutexattr_getpshared", pthread_mutexattr_getpshared},
    {"pthread_mutexattr_setpshared", my_pthread_mutexattr_setpshared},
    {"pthread_condattr_init", pthread_condattr_init},
    {"pthread_condattr_getpshared", pthread_condattr_getpshared},
    {"pthread_condattr_setpshared", pthread_condattr_setpshared},
    {"pthread_condattr_destroy", pthread_condattr_destroy},
    {"pthread_cond_init", my_pthread_cond_init},
    {"pthread_cond_destroy", my_pthread_cond_destroy},
    {"pthread_cond_broadcast", my_pthread_cond_broadcast},
    {"pthread_cond_signal", my_pthread_cond_signal},
    {"pthread_cond_wait", my_pthread_cond_wait},
    {"pthread_cond_timedwait", my_pthread_cond_timedwait},
    {"pthread_cond_timedwait_monotonic", my_pthread_cond_timedwait},
    {"pthread_cond_timedwait_monotonic_np", my_pthread_cond_timedwait},
    {"pthread_cond_timedwait_relative_np", my_pthread_cond_timedwait_relative_np},
    {"pthread_key_delete", pthread_key_delete},
    {"pthread_setname_np", pthread_setname_np},
    {"pthread_once", pthread_once},
    {"pthread_key_create", pthread_key_create},
    {"pthread_setspecific", pthread_setspecific},
    {"pthread_getspecific", pthread_getspecific},
    {"pthread_attr_init", my_pthread_attr_init},
    {"pthread_attr_destroy", my_pthread_attr_destroy},
    {"pthread_attr_setdetachstate", my_pthread_attr_setdetachstate},
    {"pthread_attr_getdetachstate", my_pthread_attr_getdetachstate},
    {"pthread_attr_setschedpolicy", my_pthread_attr_setschedpolicy},
    {"pthread_attr_getschedpolicy", my_pthread_attr_getschedpolicy},
    {"pthread_attr_setschedparam", my_pthread_attr_setschedparam},
    {"pthread_attr_getschedparam", my_pthread_attr_getschedparam},
    {"pthread_attr_setstacksize", my_pthread_attr_setstacksize},
    {"pthread_attr_getstacksize", my_pthread_attr_getstacksize},
    {"pthread_attr_setstackaddr", my_pthread_attr_setstackaddr},
    {"pthread_attr_getstackaddr", my_pthread_attr_getstackaddr},
    {"pthread_attr_setstack", my_pthread_attr_setstack},
    {"pthread_attr_getstack", my_pthread_attr_getstack},
    {"pthread_attr_setguardsize", my_pthread_attr_setguardsize},
    {"pthread_attr_getguardsize", my_pthread_attr_getguardsize},
    {"pthread_attr_setscope", my_pthread_attr_setscope},
    {"pthread_attr_setscope", my_pthread_attr_getscope},
    {"pthread_getattr_np", my_pthread_getattr_np},
    {"pthread_rwlockattr_init", my_pthread_rwlockattr_init},
    {"pthread_rwlockattr_destroy", my_pthread_rwlockattr_destroy},
    {"pthread_rwlockattr_setpshared", my_pthread_rwlockattr_setpshared},
    {"pthread_rwlockattr_getpshared", my_pthread_rwlockattr_getpshared},
    {"pthread_rwlock_init", my_pthread_rwlock_init},
    {"pthread_rwlock_destroy", my_pthread_rwlock_destroy},
    {"pthread_rwlock_unlock", my_pthread_rwlock_unlock},
    {"pthread_rwlock_wrlock", my_pthread_rwlock_wrlock},
    {"pthread_rwlock_rdlock", my_pthread_rwlock_rdlock},
    {"pthread_rwlock_tryrdlock", my_pthread_rwlock_tryrdlock},
    {"pthread_rwlock_trywrlock", my_pthread_rwlock_trywrlock},
    {"pthread_rwlock_timedrdlock", my_pthread_rwlock_timedrdlock},
    {"pthread_rwlock_timedwrlock", my_pthread_rwlock_timedwrlock},
    {"__pthread_cleanup_push", my_pthread_cleanup_push},
    {"__pthread_cleanup_pop", my_pthread_cleanup_pop},
    /* stdio.h */
    {"__isthreaded", &__my_isthreaded},
    {"__sF", &my_sF},
    {"fopen", my_fopen},
    {"fdopen", my_fdopen},
    {"popen", popen},
    {"puts", puts},
    {"sprintf", sprintf},
    {"asprintf", asprintf},
    {"vasprintf", vasprintf},
    {"snprintf", snprintf},
    {"vsprintf", vsprintf},
    {"vsnprintf", vsnprintf},
    {"clearerr", my_clearerr},
    {"fclose", my_fclose},
    {"feof", my_feof},
    {"ferror", my_ferror},
    {"fflush", my_fflush},
    {"fgetc", my_fgetc},
    {"fgetpos", my_fgetpos},
    {"fgets", my_fgets},
    {"fprintf", my_fprintf},
    {"fputc", my_fputc},
    {"fputs", my_fputs},
    {"fread", my_fread},
    {"freopen", my_freopen},
    {"fscanf", my_fscanf},
    {"fseek", my_fseek},
    {"fseeko", my_fseeko},
    {"fsetpos", my_fsetpos},
    {"ftell", my_ftell},
    {"ftello", my_ftello},
    {"fwrite", my_fwrite},
    {"getc", my_getc},
    {"getdelim", my_getdelim},
    {"getline", my_getline},
    {"putc", my_putc},
    {"rewind", my_rewind},
    {"setbuf", my_setbuf},
    {"setvbuf", my_setvbuf},
    {"ungetc", my_ungetc},
    {"vasprintf", vasprintf},
    {"vfprintf", my_vfprintf},
    {"vfscanf", my_vfscanf},
    {"fileno", my_fileno},
    {"pclose", my_pclose},
    {"perror", perror},
    {"flockfile", my_flockfile},
    {"ftrylockfile", my_ftrylockfile},
    {"funlockfile", my_funlockfile},
    {"getc_unlocked", my_getc_unlocked},
    {"putc_unlocked", my_putc_unlocked},
    //{"fgetln", my_fgetln},
    {"fpurge", my_fpurge},
    {"getw", my_getw},
    {"putw", my_putw},
    {"setbuffer", my_setbuffer},
    {"setlinebuf", my_setlinebuf},
    {"remove", remove},
    {"rename", rename},
    {"__errno", __errno_location},
    {"__set_errno", my_set_errno},
    /* socket.h */
    {"socket", socket},
    {"socketpair", socketpair},
    {"bind", bind},
    {"getsockname", getsockname},
    {"connect", connect},
    {"getpeername", getpeername},
    {"send", send},
    {"recv", recv},
    {"sendto", sendto},
    {"recvfrom", recvfrom},
    {"sendmsg", sendmsg},
    {"sendmmsg", sendmmsg},
    {"recvmsg", recvmsg},
    {"recvmmsg", recvmmsg},
    {"getsockopt", getsockopt},
    {"setsockopt", setsockopt},
    {"listen", listen},
    {"accept", accept},
    {"accept4", accept4},
    {"shutdown", shutdown},
    /* net specifics, to avoid __res_get_state */
    {"getaddrinfo", my_getaddrinfo},
    {"freeaddrinfo", my_freeaddrinfo},
    {"gethostbyaddr", gethostbyaddr},
    {"gethostbyname", gethostbyname},
    {"gethostbyname2", gethostbyname2},
    {"gethostent", gethostent},
    {"getnameinfo", my_getnameinfo},
    {"gai_strerror", gai_strerror},
    {"strftime", strftime},
    {"sysconf", my_sysconf},
    {"dlopen", android_dlopen},
    {"dlerror", android_dlerror},
    {"dlsym", my_android_dlsym},
    {"dladdr", android_dladdr},
    {"dlclose", android_dlclose},
    /* dirent.h */
    {"opendir", opendir},
    {"fdopendir", fdopendir},
    {"closedir", closedir},
    {"readdir", my_readdir},
    {"readdir_r", my_readdir_r},
    {"rewinddir", rewinddir},
    {"seekdir", seekdir},
    {"telldir", telldir},
    {"dirfd", dirfd},
    {"scandir", my_scandir},
    {"scandirat", my_scandirat},
    {"alphasort", my_alphasort},
    {"versionsort", my_versionsort},
    /* fcntl.h */
    {"open", my_open},
    {"__get_tls_hooks", __get_tls_hooks},
    {"sscanf", sscanf},
    {"scanf", scanf},
    {"vscanf", vscanf},
    {"vsscanf", vsscanf},
    {"openlog", openlog},
    {"syslog", syslog},
    {"closelog", closelog},
    {"vsyslog", vsyslog},
    {"timer_create", timer_create},
    {"timer_settime", timer_settime},
    {"timer_gettime", timer_gettime},
    {"timer_delete", timer_delete},
    {"timer_getoverrun", timer_getoverrun},
    {"localtime", localtime},
    {"localtime_r", localtime_r},
    {"gmtime", gmtime},
    {"abort", abort},
    {"writev", writev},
    {"fcntl", fcntl},
    /* unistd.h */
    {"access", access},
    {"lseek", lseek},
    {"lseek64", lseek64},
    {"close", close},
    {"read", read},
    {"write", write},
    {"pread", pread},
    {"pwrite", pwrite},
    {"pread64", pread64},
    {"pwrite64", pwrite64},
    {"pipe", pipe},
    {"pipe2", pipe2},
    {"alarm", alarm},
    {"sleep", sleep},
    {"usleep", usleep},
    {"pause", pause},
    {"chown", chown},
    {"fchown", fchown},
    {"lchown", lchown},
    {"chdir", chdir},
    {"fchdir", fchdir},
    {"getcwd", getcwd},
    {"get_current_dir_name", get_current_dir_name},
    {"dup", dup},
    {"dup2", dup2},
    {"dup3", dup3},
    {"execve", execve},
    {"execv", execv},
    {"execle", execle},
    {"execl", execl},
    {"execvp", execvp},
    {"execlp", execlp},
    {"execvpe", execvpe},
    {"nice", nice},
    {"_exit", _exit},
    {"pathconf", pathconf},
    {"fpathconf", fpathconf},
    {"sysconf", sysconf},
    {"confstr", confstr},
    {"getpid", getpid},
    {"getppid", getppid},
    {"getpgrp", getpgrp},
    {"__getpgid", __getpgid},
    {"getpgid", getpgid},
    {"setpgid", setpgid},
    {"setpgrp", setpgrp},
    {"setsid", setsid},
    {"getsid", getsid},
    {"getuid", getuid},
    {"geteuid", geteuid},
    {"getgid", getgid},
    {"getegid", getegid},
    {"getgroups", getgroups},
    {"group_member", group_member},
    {"setuid", setuid},
    {"setreuid", setreuid},
    {"seteuid", seteuid},
    {"setgid", setgid},
    {"setregid", setregid},
    {"setegid", setegid},
    {"getresuid", getresuid},
    {"getresgid", getresgid},
    {"setresuid", setresuid},
    {"setresgid", setresgid},
    {"fork", fork},
    {"vfork", vfork},
    {"ttyname", ttyname},
    {"ttyname_r", ttyname_r},
    {"isatty", isatty},
    {"ttyslot", ttyslot},
    {"link", link},
    {"symlink", symlink},
    {"readlink", readlink},
    {"unlink", unlink},
    {"rmdir", rmdir},
    {"rmdir", rmdir},
    {"tcgetpgrp", tcgetpgrp},
    {"getlogin", getlogin},
    {"getlogin_r", getlogin_r},
    {"gethostname", gethostname},
    {"sethostname", sethostname},
    {"sethostid", sethostid},
    {"getdomainname", getdomainname},
    {"setdomainname", setdomainname},
    {"vhangup", vhangup},
    {"profil", profil},
    {"acct", acct},
    {"getusershell", getusershell},
    {"endusershell", endusershell},
    {"setusershell", setusershell},
    {"daemon", daemon},
    {"chroot", chroot},
    {"getpass", getpass},
    {"fsync", fsync},
    {"syncfs", syncfs},
    {"gethostid", gethostid},
    {"sync", sync},
    {"getpagesize", getpagesize},
    {"getdtablesize", getdtablesize},
    {"truncate", truncate},
    {"truncate64", truncate64},
    {"ftruncate", ftruncate},
    {"ftruncate64", ftruncate64},
    {"brk", brk},
    {"sbrk", sbrk},
    {"syscall", syscall},
    {"lockf", lockf},
    {"lockf64", lockf64},
    {"fdatasync", fdatasync},
    {"swab", swab},
    /* time.h */
    {"clock", clock},
    {"time", time},
    {"difftime", difftime},
    {"mktime", mktime},
    {"strftime", strftime},
    {"strptime", strptime},
    {"strftime_l", strftime_l},
    {"strptime_l", strptime_l},
    {"gmtime", gmtime},
    {"localtime", localtime},
    {"gmtime_r", gmtime_r},
    {"localtime_r", localtime_r},
    {"asctime", asctime},
    {"ctime", ctime},
    {"asctime_r", asctime_r},
    {"ctime_r", ctime_r},
    {"__tzname", __tzname},
    {"__daylight", &__daylight},
    {"__timezone", &__timezone},
    {"tzname", tzname},
    {"tzset", tzset},
    {"daylight", &daylight},
    {"timezone", &timezone},
    {"stime", stime},
    {"timegm", timegm},
    {"timelocal", timelocal},
    {"dysize", dysize},
    {"nanosleep", nanosleep},
    {"clock_getres", clock_getres},
    {"clock_gettime", clock_gettime},
    {"clock_settime", clock_settime},
    {"clock_nanosleep", clock_nanosleep},
    {"clock_getcpuclockid", clock_getcpuclockid},
    /* mman.h */
    {"mmap", mmap},
    {"munmap", munmap},
    {"mprotect", mprotect},
    {"msync", msync},
    {"mlock", mlock},
    {"munlock", munlock},
    {"mlockall", mlockall},
    {"munlockall", munlockall},
    /* signal.h */
    {"__sysv_signal", __sysv_signal},
    {"sysv_signal", sysv_signal},
    {"signal", signal},
    {"bsd_signal", signal},
    {"kill", kill},
    {"killpg", killpg},
    {"raise", raise},
    {"sigaction", sigaction},
    {"sigprocmask", sigprocmask},
    /* sys/stat.h */
    {"stat", my_stat},
    {"fstat", my_fstat},
    {"stat64", my_stat64},
    {"fstat64", my_fstat64},
    {"chmod", chmod},
    {"fchmod", fchmod},
    {"umask", umask},
    {"mkdir", mkdir},
    /* sys/epoll.h */
    {"epoll_create", epoll_create},
    {"epoll_create1", epoll_create1},
    {"epoll_ctl", epoll_ctl},
    {"epoll_wait", epoll_wait},
    /* grp.h */
    {"getgrgid", getgrgid},
    {"__cxa_atexit", __cxa_atexit},
    {"__cxa_finalize", __cxa_finalize},
    /* arpa/inet.h */
    {"inet_addr", inet_addr},
    {"inet_lnaof", inet_lnaof},
    {"inet_makeaddr", inet_makeaddr},
    {"inet_netof", inet_netof},
    {"inet_network", inet_network},
    {"inet_ntoa", inet_ntoa},
    {"inet_pton", inet_pton},
    {"inet_ntop", inet_ntop},
    /* net/if.h */
    {"if_nametoindex", if_nametoindex},
    {"if_indextoname", if_indextoname},
    {"if_nameindex", if_nameindex},
    {"if_freenameindex", if_freenameindex},
    /* ctype.h */
    {"isalnum", hybris_isalnum},
    {"isalpha", hybris_isalpha},
    {"isblank", hybris_isblank},
    {"iscntrl", hybris_iscntrl},
    {"isdigit", hybris_isdigit},
    {"isgraph", hybris_isgraph},
    {"islower", hybris_islower},
    {"isprint", hybris_isprint},
    {"ispunct", hybris_ispunct},
    {"isspace", hybris_isspace},
    {"isupper", hybris_isupper},
    {"isxdigit", hybris_isxdigit},
    {"tolower", tolower},
    {"toupper", toupper},
    {"_tolower_tab_", &_hybris_tolower_tab_},
    {"_toupper_tab_", &_hybris_toupper_tab_},
    {"_ctype_", &_hybris_ctype_},
    /* wctype.h */
    {"wctype", wctype},
    {"iswspace", iswspace},
    {"iswctype", iswctype},
    {"towlower", towlower},
    {"towupper", towupper},
     /* wchar.h */
    {"wctob", wctob},
    {"btowc", btowc},
    {"wmemchr", wmemchr},
    {"wmemcmp", wmemcmp},
    {"wmemcpy", wmemcpy},
    {"wmemset", wmemset},
    {"wmemmove", wmemmove},
    {"getwc", my_getwc},
    {"ungetwc", my_ungetwc},
    {"putwc", my_putwc},
    {"wcrtomb", wcrtomb},
    {"mbrtowc", mbrtowc},
    {"wcscoll", wcscoll},
    {"wcsxfrm", wcsxfrm},
    {"wcsftime", wcsftime},
    {NULL, NULL},
};
static struct _hook* user_hooks = NULL;
static int user_hooks_size = 0;
static int user_hooks_arr_size = 0;

void user_hooks_resize() {
    if (user_hooks_arr_size == 0) {
        user_hooks_arr_size = 256;
        user_hooks = (struct _hook*) malloc(user_hooks_arr_size * sizeof(struct _hook));
    } else {
        user_hooks_arr_size *= 2;
        struct _hook* new_array = (struct _hook*) malloc(user_hooks_arr_size * sizeof(struct _hook));
        memcpy(&new_array[0], &user_hooks[0], user_hooks_size * sizeof(struct _hook));
        free(user_hooks);
        user_hooks = new_array;
    }
}

void add_user_hook(struct _hook h) {
    if (user_hooks_size + 1 >= user_hooks_arr_size)
        user_hooks_resize();

    user_hooks[user_hooks_size++] = h;
}

void hybris_hook(const char *name, void* func) {
    struct _hook h;
    h.name = name;
    h.func = func;
    add_user_hook(h);
}

void *get_hooked_symbol(const char *sym)
{
    int i;
    struct _hook *ptr = &hooks[0];
    static int counter = -1;

    while (ptr->name != NULL)
    {
        if (strcmp(sym, ptr->name) == 0){
            return ptr->func;
        }
        ptr++;
    }
    for (i = 0; i < user_hooks_size; i++) {
        struct _hook* h = &user_hooks[i];
        if (strcmp(sym, h->name) == 0) {
            //printf("redirect %s --> %s\n", sym, h->name);
            return h->func;
        }
    }

    if (strstr(sym, "pthread") != NULL)
    {
        /* safe */
        if (strcmp(sym, "pthread_sigmask") == 0)
           return NULL;
        /* not safe */
        counter--;
        LOGD("%s %i\n", sym, counter);
        return (void *) counter;
    }
    return NULL;
}

void android_linker_init()
{
}
