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

/* Debug */
#include "logging.h"
#define LOGD(message, ...) HYBRIS_DEBUG_LOG(HOOKS, message, ##__VA_ARGS__)

/* we have a value p:
 *  - if p <= ANDROID_TOP_ADDR_VALUE_MUTEX then it is an android mutex, not one we processed
 *  - if p > VMALLOC_END, then the pointer is not a result of malloc ==> it is an shm offset
 */

uintptr_t _hybris_stack_chk_guard = 0;

static void __attribute__((constructor)) __init_stack_check_guard() {
    _hybris_stack_chk_guard = *((uintptr_t*) getauxval(AT_RANDOM));
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

static int my_set_errno(int oi_errno)
{
    errno = oi_errno;
    return -1;
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

static struct _hook main_hooks[] = {
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
    {"strchr",strchr},
    {"strrchr",strrchr},
    {"strlen",my_strlen},
    {"__strlen_chk",my_strlen_chk},
    {"strcmp",strcmp},
    {"strcpy",strcpy},
    {"strcat",strcat},
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
    /* errno.h */
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
    {"sysconf", my_sysconf},
    {"dlopen", android_dlopen},
    {"dlerror", android_dlerror},
    {"dlsym", my_android_dlsym},
    {"dladdr", android_dladdr},
    {"dlclose", android_dlclose},
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
    {"wcrtomb", wcrtomb},
    {"mbrtowc", mbrtowc},
    {"wcscoll", wcscoll},
    {"wcsxfrm", wcsxfrm},
    {"wcsftime", wcsftime},
    {NULL, NULL},
};
REGISTER_HOOKS(main_hooks)
static struct _hook* user_hooks = NULL;
static int user_hooks_size = 0;
static int user_hooks_arr_size = 0;

void user_hooks_resize() {
    if (user_hooks_arr_size == 0) {
        user_hooks_arr_size = 512;
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

    for (int i = 0; i < user_hooks_size; i++) {
        if (strcmp(user_hooks[i].name, h.name) == 0) {
            printf("warn: duplicate hook: %s\n", h.name);
        }
    }
    user_hooks[user_hooks_size++] = h;
}

void hybris_register_hooks(struct _hook *hooks) {
    struct _hook *ptr = &hooks[0];
    while (ptr->name != NULL)
    {
        add_user_hook(*ptr);
        ptr++;
    }
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

    static int counter = -1;

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
