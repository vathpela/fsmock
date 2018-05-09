/*
 * api.c
 * Copyright 2018 Peter Jones <pjones@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "fsmock.h"

#include <assert.h>
#include <dlfcn.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>

/*
 * Our constructor will set these up as the calls to libc's functions.
 */
int PRIVATE (*libc_access)(const char *pathname, int mode);
int PRIVATE (*libc_close)(int fd);
int PRIVATE (*libc_closedir)(DIR *dirp);
int PRIVATE (*libc_dirfd)(DIR *dirp);
int PRIVATE (*libc_faccessat)(int dirfd, const char *pathname, int mode, int flags);
int PRIVATE (*libc_fcntl)(int fd, int cmd, ...);
FILE PRIVATE *(*libc_fdopen)(int fd, const char *mode);
DIR PRIVATE *(*libc_fdopendir)(int fd);
int PRIVATE (*libc_fileno)(FILE *stream);
FILE PRIVATE *(*libc_fopen)(const char *pathname, const char *mode);
FILE PRIVATE *(*libc_freopen)(const char *pathname, const char *mode, FILE *stream);
ssize_t PRIVATE (*libc_getxattr)(const char *path, const char *name, void *value, size_t size);
int PRIVATE (*libc_ioctl)(int fd, unsigned long request, ...);
off_t PRIVATE (*libc_lseek)(int fd, off_t offset, int whence);
int PRIVATE (*libc_open)(const char *pathname, int flags, ...);
int PRIVATE (*libc_openat)(int dirfd, const char *pathname, int flags, ...);
DIR PRIVATE *(*libc_opendir)(const char *name);
struct PRIVATE dirent *(*libc_readdir)(DIR *dirp);
ssize_t PRIVATE (*libc_readlink)(const char *pathname, char *buf, size_t bufsiz);
ssize_t PRIVATE (*libc_readlinkat)(int dirfd, const char *pathname, char *buf, size_t bufsiz);
int PRIVATE (*libc_stat)(const char *pathname, struct stat *statbuf);

/*
 * If we need other libc symbols, this is a dlhandle for our private copy of
 * libc
 */
void PRIVATE *libc;

static void log_call(const char *name, ...);

/*
 * a DIR * and a fd for our root filesystem path
 */
DIR PRIVATE *rootdir = NULL;
int PRIVATE rootfd = -1;

static void
fsmock_init(void)
{
        char *rootpath;

        if (libc)
                return;

        rootpath = getenv("LIBFSMOCK_ROOT");
        assert(rootpath != NULL);

        libc = dlmopen(LM_ID_NEWLM, "libc.so.6",
                       RTLD_NOW|RTLD_LOCAL|RTLD_NODELETE|RTLD_DEEPBIND);
        assert_perror(libc == NULL ? errno : 0);

        libc_access = dlvsym(libc, "access", "GLIBC_2.2.5");
        assert(libc_access != NULL);
        libc_close = dlvsym(libc, "close", "GLIBC_2.2.5");
        assert(libc_close != NULL);
        libc_closedir = dlvsym(libc, "closedir", "GLIBC_2.2.5");
        assert(libc_closedir != NULL);
        libc_dirfd = dlvsym(libc, "dirfd", "GLIBC_2.2.5");
        assert(libc_dirfd != NULL);
        libc_fcntl = dlvsym(libc, "fcntl", "GLIBC_2.2.5");
        assert(libc_fcntl != NULL);
        libc_fdopen = dlvsym(libc, "fdopen", "GLIBC_2.2.5");
        assert(libc_fdopen != NULL);
        libc_fdopendir = dlvsym(libc, "fdopendir", "GLIBC_2.4");
        assert(libc_fdopendir != NULL);
        libc_fileno = dlvsym(libc, "fileno", "GLIBC_2.2.5");
        assert(libc_fileno != NULL);
        libc_fopen = dlvsym(libc, "fopen", "GLIBC_2.2.5");
        assert(libc_fopen != NULL);
        libc_freopen = dlvsym(libc, "freopen", "GLIBC_2.2.5");
        assert(libc_freopen != NULL);
        libc_ioctl = dlvsym(libc, "ioctl", "GLIBC_2.2.5");
        assert(libc_ioctl != NULL);
        libc_lseek = dlvsym(libc, "lseek", "GLIBC_2.2.5");
        assert(libc_lseek != NULL);
        libc_open = dlvsym(libc, "open", "GLIBC_2.2.5");
        assert(libc_open != NULL);
        libc_opendir = dlvsym(libc, "opendir", "GLIBC_2.2.5");
        assert(libc_opendir != NULL);
        libc_readdir = dlvsym(libc, "readdir", "GLIBC_2.2.5");
        assert(libc_readdir != NULL);
        libc_readlink = dlvsym(libc, "readlink", "GLIBC_2.2.5");
        assert(libc_readlink != NULL);
        libc_getxattr = dlvsym(libc, "getxattr", "GLIBC_2.3");
        assert(libc_getxattr != NULL);
        libc_faccessat = dlvsym(libc, "faccessat", "GLIBC_2.4");
        assert(libc_faccessat != NULL);
        libc_openat = dlvsym(libc, "openat", "GLIBC_2.4");
        assert(libc_openat != NULL);
        libc_readlinkat = dlvsym(libc, "readlinkat", "GLIBC_2.4");
        assert(libc_readlinkat != NULL);
        libc_stat = dlvsym(libc, "__xstat", "GLIBC_2.2.5");

        rootdir = libc_opendir(rootpath);
        assert(rootdir != NULL);

        rootfd = libc_dirfd(rootdir);
        assert(rootfd >= 0);
}

static void DESTRUCTOR
fsmock_fini(void)
{
        if (libc) {
                dlclose(libc);
                libc = NULL;
        }
}

static inline UNUSED const char *
demangle_path(const char *path)
{
        return path;
}

#define do_call(rtype, name, ...)                                       \
        ({                                                              \
                rtype ret_;                                             \
                                                                        \
                ret_ = libc_ ## name (__VA_ARGS__);                     \
                log_call(#name, ret_, __VA_ARGS__);                     \
                ret_;                                                   \
        })

int PUBLIC
access(const char *pathname, int mode)
{
        int ret;

        fsmock_init();

        if (is_our_path(pathname))
                ret = libc_faccessat(rootfd, pathname, mode, 0);
        else
                ret = libc_access(pathname, mode);
        log_call("access", ret, pathname, mode);

        return ret;
}

int PUBLIC
close(int fd)
{
        fsmock_init();

        fd = demangle_fd(fd);
        return do_call(int, close, fd);
}

int PUBLIC
closedir(DIR *dirp)
{
        fsmock_init();

        return do_call(int, closedir, dirp);
}

int PUBLIC
dirfd(DIR *dirp)
{
        fsmock_init();

        return do_call(int, dirfd, dirp);
}

int PUBLIC
faccessat(int dirfd, const char *pathname, int mode, int flags UNUSED)
{
        fsmock_init();

        errno = ENOSYS;
        log_call("faccessat", -1, dirfd, pathname, mode, flags);
        return -1;
}

int PUBLIC
fcntl(int fd, int cmd, ...)
{
        int d;
        int ret = -1;
        const char *cmdstr = "";
        void *val = NULL;

        fsmock_init();

        fd = demangle_fd(fd);
        errno = ENOSYS;
        switch(cmd) {
        case F_DUPFD:
                d = get_arg(cmd, int);
                val = &d;
                cmdstr = "F_DUPFD";
                ret = libc_fcntl(fd, cmd, d);
                break;
        case F_DUPFD_CLOEXEC:
                cmdstr = "F_DUPFD_CLOEXEC";
                break;
        case F_GETFD:
                cmdstr = "F_GETFD";
                ret = libc_fcntl(fd, cmd);
                break;
        case F_SETFD:
                cmdstr = "F_SETFD";
                d = get_arg(cmd, int);
                val = &d;
                ret = libc_fcntl(fd, cmd, d);
                break;
        case F_GETFL:
                cmdstr = "F_GETFL";
                ret = libc_fcntl(fd, cmd);
                break;
        case F_SETFL:
                cmdstr = "F_SETFL";
                break;
        case F_SETLK:
                cmdstr = "F_SETLK";
                break;
        case F_SETLKW:
                cmdstr = "F_SETLKW";
                break;
        case F_GETLK:
                cmdstr = "F_GETLK";
                break;
        case F_OFD_SETLK:
                cmdstr = "F_OFD_SETLK";
                break;
        case F_OFD_SETLKW:
                cmdstr = "F_OFD_SETLKW";
                break;
        case F_OFD_GETLK:
                cmdstr = "F_OFD_GETLK";
                break;
        case F_GETOWN:
                cmdstr = "F_GETOWN";
                break;
        case F_SETOWN:
                cmdstr = "F_SETOWN";
                break;
        case F_GETOWN_EX:
                cmdstr = "F_GETOWN_EX";
                break;
        case F_SETOWN_EX:
                cmdstr = "F_SETOWN_EX";
                break;
        case F_GETSIG:
                cmdstr = "F_GETSIG";
                break;
        case F_SETSIG:
                cmdstr = "F_SETSIG";
                break;
        case F_SETLEASE:
                cmdstr = "F_SETLEASE";
                break;
        case F_GETLEASE:
                cmdstr = "F_GETLEASE";
                break;
        case F_NOTIFY:
                cmdstr = "F_NOTIFY";
                break;
        case F_SETPIPE_SZ:
                cmdstr = "F_SETPIPE_SZ";
                break;
        case F_GETPIPE_SZ:
                cmdstr = "F_GETPIPE_SZ";
                break;
#if 0
        case F_ADD_SEALS:
                cmdstr = "F_ADD_SEALS";
                break;
        case F_GET_SEALS:
                cmdstr = "F_GET_SEALS";
                break;
#endif
        default:
                errno = ENOSYS;
                ret = -1;
                break;
        }
        if (val == &d)
                log_call("fcntl", ret, fd, cmdstr, d);
        else
                log_call("fcntl", ret, fd, cmdstr, NULL);
        return ret;
}

FILE PUBLIC *
fdopen(int fd, const char *mode)
{
        fsmock_init();

        return do_call(FILE *, fdopen, fd, mode);
}

DIR PUBLIC *
fdopendir(int fd)
{
        fsmock_init();

        errno = ENOSYS;
        log_call("fdopendir", NULL, fd);
        return NULL;
}

int PUBLIC
fileno(FILE *stream)
{
        fsmock_init();

        return do_call(int, fileno, stream);
}

FILE PUBLIC *
fopen(const char *pathname, const char *mode)
{
        fsmock_init();

        return do_call(FILE *, fopen, pathname, mode);
}

FILE PUBLIC *
freopen(const char *pathname, const char *mode, FILE *stream)
{
        fsmock_init();

        return do_call(FILE *, freopen, pathname, mode, stream);
}

ssize_t PUBLIC
getxattr(const char *path, const char *name, void *value, size_t size)
{
        fsmock_init();

        return do_call(ssize_t, getxattr, path, name, value, size);
}

int PUBLIC
ioctl(int fd, unsigned long request, ...)
{
	uintptr_t arg = get_arg(request, uintptr_t);
	int ret = -1;

	errno = ENOSYS;
	if (!is_blkdev_fd(fd))
		ret = libc_ioctl(fd, request, arg);
        log_call("ioctl", ret, fd, request, arg);
        return ret;
}

off_t PUBLIC
lseek(int fd, off_t offset, int whence)
{
        fsmock_init();

        fd = demangle_fd(fd);
        return do_call(off_t, lseek, fd, offset, whence);
}

int PUBLIC
open(const char *pathname, int flags, ...)
{
        fsmock_init();

        mode_t mode = 0;

        if (flags & O_CREAT)
                mode = get_arg(flags, mode_t);

        if (is_our_path(pathname)) {
                int ret;
                if (mode)
                        ret = do_call(int, openat, rootfd, pathname, flags, mode);
                else
                        ret = do_call(int, openat, rootfd, pathname, flags);
                ret = mangle_fd(ret);
                return ret;
        } else {
                if (mode)
                        return do_call(int, open, pathname, flags, mode);
                else
                        return do_call(int, open, pathname, flags);
        }

        errno = ENOSYS;
        return -1;
}

int PUBLIC
openat(int dirfd, const char *pathname, int flags, ...)
{
        fsmock_init();

        errno = ENOSYS;
        if (flags & O_CREAT) {
                mode_t mode = get_arg(flags, mode_t);
                if (dirfd == AT_FDCWD)
                        return do_call(int, openat, dirfd, pathname, flags, mode);
                log_call("openat", -1, dirfd, pathname, flags, mode);
        } else {
                if (dirfd == AT_FDCWD)
                        return do_call(int, openat, dirfd, pathname, flags);
                log_call("openat", -1, dirfd, pathname, flags);
        }
        return -1;
}

DIR PUBLIC *
opendir(const char *name)
{
        fsmock_init();

        return do_call(DIR *, opendir, name);
}

struct dirent PUBLIC *
readdir(DIR *dirp)
{
        fsmock_init();

        return do_call(struct dirent *, readdir, dirp);
}

ssize_t PUBLIC
readlink(const char *pathname, char *buf, size_t bufsiz)
{
        fsmock_init();

        return do_call(ssize_t, readlink, pathname, buf, bufsiz);
}

ssize_t PUBLIC
readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz)
{
        fsmock_init();

        errno = ENOSYS;
        log_call("readlinkat", -1, dirfd, pathname, buf, bufsiz);
        return -1;
}

int PUBLIC stat(const char *pathname, struct stat *statbuf)
{
        fsmock_init();

        return do_call(int, stat, pathname, statbuf);
}
#pragma weak __xstat = stat

static const char *
fmt_open(va_list ap0)
{
        va_list ap1;
        const char *creat = "\"%s\", 0x%0x, 0o%0o";
        const char *no_creat = "\"%s\", 0x%0x";

        int flags;

        va_copy(ap1, ap0);
        va_arg(ap1, const char *);
        flags = va_arg(ap1, int);
        va_end(ap1);

        if (flags & O_CREAT)
                return creat;
        return no_creat;
}

static const char *
fmt_openat(va_list ap0)
{
        va_list ap1;
        const char *creat = "%d, \"%s\", 0x%0x, 0o%0o";
        const char *no_creat = "%d, \"%s\", 0x%0x";

        int flags;

        va_copy(ap1, ap0);
        va_arg(ap1, int);
        va_arg(ap1, const char *);
        flags = va_arg(ap1, int);
        va_end(ap1);

        if (flags & O_CREAT)
                return creat;
        return no_creat;
}


enum ret_type { VOID, INT, SSIZE_T, OFF_T, FILEP, DIRP, DIRENTP };
typedef const char *(*format_maker)(va_list ap);

static void
log_call(const char *name, ...)
{
        FILE *f = stdout;
        struct call {
                const char *name;
                enum ret_type ret_type;
                int nargs;
                const char *fmt;
                format_maker mkfmt;
        } syscalls[] = {
                {"access", INT, 2, "\"%s\", %d", },
                {"close", INT, 1, "%d", },
                {"closedir", INT, 1, "%p", },
                {"dirfd", INT, 1, "%p", },
                {"faccessat", INT, 4, "%d, \"%s\", 0o%0o, 0x%0x", },
                {"fcntl", INT, 3, "%d, %s, 0x%" PRIxPTR, },
                {"fdopen", FILEP, 2, "%d, \"%s\"", },
                {"fileno", INT, 1, "%p", },
                {"fopen", FILEP, 2, "\"%s\", \"%s\"", },
                {"freopen", FILEP, 3, "\"%s\", \"%s\", %p", },
                {"getxattr", SSIZE_T, 4, "\"%s\", \"%s\", %p, %zu", },
                {"ioctl", INT, 3, "%d, %lu, 0x%" PRIxPTR, },
                {"lseek", OFF_T, 3, "%d, %zd, 0x%0x", },
                {"open", INT, 3, NULL, (format_maker)fmt_open, },
                {"openat", INT, 4, NULL, (format_maker)fmt_openat, },
                {"opendir", DIRP, 1, "\"%s\"", },
                {"fdopendir", DIRP, 1, "%d", },
                {"readdir", DIRENTP, 1, "%p", },
                {"readlink", SSIZE_T, 3, "\"%s\", %p, %zu", },
                {"readlinkat", SSIZE_T, 4, "%d, \"%s\", %p, %zu", },

                {NULL, }
        };
        va_list ap;
        int error = errno;

        va_start(ap, name);

        for (unsigned int i = 0; syscalls[i].name; i++) {
                struct call *call = &syscalls[i];
                va_list ap0;

                bool is_error = false;
                int d = 0;
                ssize_t zd = 0;
                void *p = NULL;

                if (strcmp(name, call->name))
                        continue;

                fprintf(f, "%s(", name);
                switch (call->ret_type) {
                case VOID:
                        break;
                case INT:
                        d = get_ap_arg(ap, int);
                        break;
                case SSIZE_T:
                        zd = get_ap_arg(ap, ssize_t);
                        break;
                case OFF_T:
                        zd = get_ap_arg(ap, off_t);
                        break;
                case FILEP:
                case DIRP:
                case DIRENTP:
                        p = get_ap_arg(ap, void *);
                        break;
                default:
                        assert(call->ret_type != call->ret_type);
                        break;
                }
                va_copy(ap0, ap);
                if (call->fmt) {
                        vfprintf(f, call->fmt, ap0);
                } else if (call->mkfmt) {
                        const char *fmt = call->mkfmt(ap0);
                        vfprintf(f, fmt, ap0);
                }
                va_end(ap0);

                switch (call->ret_type) {
                case VOID:
                        fprintf(f, ")");
                        break;
                case INT:
                        fprintf(f, ") = %d", d);
                        is_error = d < 0;
                        break;
                case SSIZE_T:
                case OFF_T:
                        fprintf(f, ") = %zd", zd);
                        is_error = zd < 0;
                        break;
                case FILEP:
                case DIRP:
                case DIRENTP:
                        fprintf(f, ") = %p", p);
                        is_error = p == NULL;
                        break;
                default:
                        assert(call->ret_type != call->ret_type);
                        break;
                }

                if (is_error)
                        fprintf(f, " (%m)");
                fprintf(f, "\n");
                fflush(f);
        }
        va_end(ap);
        errno = error;
}


// vim:fenc=utf-8:tw=75
