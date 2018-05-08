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

#include "bdsim.h"

#include <assert.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <stdbool.h>

int PRIVATE (*libc_access)(const char *pathname, int mode);
int PRIVATE (*libc_close)(int fd);
int PRIVATE (*libc_closedir)(DIR *dirp);
int PRIVATE (*libc_dirfd)(DIR *dirp);
int PRIVATE (*libc_faccessat)(int dirfd, const char *pathname, int mode, int flags);
int PRIVATE (*libc_fcntl)(int fd, int cmd, ...);
FILE PRIVATE *(*libc_fdopen)(int fd, const char *mode);
int PRIVATE (*libc_fileno)(FILE *stream);
FILE PRIVATE *(*libc_fopen)(const char *pathname, const char *mode);
FILE PRIVATE *(*libc_freopen)(const char *pathname, const char *mode, FILE *stream);
ssize_t PRIVATE (*libc_getxattr)(const char *path, const char *name, void *value, size_t size);
int PRIVATE (*libc_ioctl)(int fd, unsigned long request, ...);
off_t PRIVATE (*libc_lseek)(int fd, off_t offset, int whence);
int PRIVATE (*libc_open)(const char *pathname, int flags, ...);
int PRIVATE (*libc_openat)(int dirfd, const char *pathname, int flags, ...);
DIR PRIVATE *(*libc_opendir)(const char *name);
DIR PRIVATE *(*libc_fdopendir)(int fd);
struct PRIVATE dirent *(*libc_readdir)(DIR *dirp);
ssize_t PRIVATE (*libc_readlink)(const char *pathname, char *buf, size_t bufsiz);
ssize_t PRIVATE (*libc_readlinkat)(int dirfd, const char *pathname, char *buf, size_t bufsiz);

void PRIVATE *libc;
static DIR *root;
static int rootfd = -1;
static int refs;

void CONSTRUCTOR_N(101)
bdsim_init_(void)
{
        char *rootpath;
        printf("%s() %d: refs: %d\n", __func__, __LINE__, refs);
        fflush(stdout);

        if (++refs != 1)
                return;

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
        libc_fdopendir = dlvsym(libc, "fdopendir", "GLIBC_2.4");
        assert(libc_fdopendir != NULL);
        libc_openat = dlvsym(libc, "openat", "GLIBC_2.4");
        assert(libc_openat != NULL);
        libc_readlinkat = dlvsym(libc, "readlinkat", "GLIBC_2.4");
        assert(libc_readlinkat != NULL);

        rootpath = getenv("LIBBDSIM_ROOT");
        assert(rootpath != NULL);

        root = libc_opendir(rootpath);
        assert(root != NULL);

        rootfd = libc_dirfd(root);
        assert(rootfd >= 0);
}

void DESTRUCTOR_N(101)
bdsim_fini_(void)
{
        printf("%s() %d: refs: %d\n", __func__, __LINE__, refs);

        if (--refs > 0)
                return;

        if (libc) {
                dlclose(libc);
                libc = NULL;
        }
}

bool PRIVATE
is_blkdev_fd(int fd)
{
        return ((((unsigned long long)fd >> ((sizeof(fd) * 8) - 8)) & 0xffULL) == 0xbbULL);
}

static int
demangle_fd(int fd)
{
        return fd;
}

static const char *
demangle_path(const char *path)
{
        return path;
}

#define get_arg(param, type)                                    \
        ({                                                      \
                va_list ap_;                                    \
                type arg_;                                      \
                va_start(ap_, param);                           \
                arg_ = va_arg(ap_, type);                       \
                va_end(ap_);                                    \
                arg_;                                           \
        })

int PUBLIC
access(const char *pathname, int mode)
{
        const char *path = demangle_path(pathname);
        int ret;

        if (!libc) bdsim_init_();

        ret = libc_access(path, mode);
        printf("access(\"%s\", %0o) = %d", path, mode, ret);
        if (ret < 0)
                printf(" (%m)");
        putchar('\n');
        fflush(stdout);
        return ret;
}

int PUBLIC
close(int fd)
{
        int ret;

        if (!libc) bdsim_init_();

        fd = demangle_fd(fd);
        printf("close(%d)", fd);
        ret = libc_close(fd);
        printf(" = %d", ret);
        if (ret < 0)
                printf(" (%m)");
        putchar('\n');
        fflush(stdout);
        return ret;
}

int PUBLIC
closedir(DIR *dirp)
{
        int ret;

        if (!libc) bdsim_init_();

        ret = libc_closedir(dirp);
        printf("closedir(%p) = %d", dirp, ret);
        if (ret < 0)
                printf(" (%m)");
        putchar('\n');
        fflush(stdout);
        return ret;
}

int PUBLIC
dirfd(DIR *dirp)
{

        if (!libc) bdsim_init_();

        printf("dirfd(%p)\n", dirp);
        errno = ENOSYS;
        return -1;
}

int PUBLIC
faccessat(int dirfd, const char *pathname, int mode, int flags)
{

        if (!libc) bdsim_init_();

        printf("faccessat(%d,\"%s\",%d)\n", dirfd, pathname, mode);
        errno = ENOSYS;
        return -1;
}

int PUBLIC
fcntl(int fd, int cmd, ...)
{
        int d;
        int ret = -1;
        int error;

        if (!libc) bdsim_init_();


        fd = demangle_fd(fd);
        printf("fcntl(%d,", fd);
        error = errno = ENOSYS;
        switch(cmd) {
        case F_DUPFD:
                d = get_arg(cmd, int);
                printf("%s,%d)", "F_DUPFD", d);
                ret = libc_fcntl(fd, cmd, d);
                error = errno;
                break;
        case F_DUPFD_CLOEXEC:
                printf("%s)", "F_DUPFD_CLOEXEC");
                break;
        case F_GETFD:
                printf("%s)", "F_GETFD");
                ret = libc_fcntl(fd, cmd);
                error = errno;
                break;
        case F_SETFD:
                d = get_arg(cmd, int);
                printf("%s,%d)", "F_SETFD", d);
                ret = libc_fcntl(fd, cmd, d);
                error = errno;
                break;
        case F_GETFL:
                printf("%s)", "F_GETFL");
                ret = libc_fcntl(fd, cmd);
                error = errno;
                break;
        case F_SETFL:
                printf("%s)", "F_SETFL");
                break;
        case F_SETLK:
                printf("%s)", "F_SETLK");
                break;
        case F_SETLKW:
                printf("%s)", "F_SETLKW");
                break;
        case F_GETLK:
                printf("%s)", "F_GETLK");
                break;
        case F_OFD_SETLK:
                printf("%s)", "F_OFD_SETLK");
                break;
        case F_OFD_SETLKW:
                printf("%s)", "F_OFD_SETLKW");
                break;
        case F_OFD_GETLK:
                printf("%s)", "F_OFD_GETLK");
                break;
        case F_GETOWN:
                printf("%s)", "F_GETOWN");
                break;
        case F_SETOWN:
                printf("%s)", "F_SETOWN");
                break;
        case F_GETOWN_EX:
                printf("%s)", "F_GETOWN_EX");
                break;
        case F_SETOWN_EX:
                printf("%s)", "F_SETOWN_EX");
                break;
        case F_GETSIG:
                printf("%s)", "F_GETSIG");
                break;
        case F_SETSIG:
                printf("%s)", "F_SETSIG");
                break;
        case F_SETLEASE:
                printf("%s)", "F_SETLEASE");
                break;
        case F_GETLEASE:
                printf("%s)", "F_GETLEASE");
                break;
        case F_NOTIFY:
                printf("%s)", "F_NOTIFY");
                break;
        case F_SETPIPE_SZ:
                printf("%s)", "F_SETPIPE_SZ");
                break;
        case F_GETPIPE_SZ:
                printf("%s)", "F_GETPIPE_SZ");
                break;
#if 0
        case F_ADD_SEALS:
                printf("%s\n", "F_ADD_SEALS");
                break;
        case F_GET_SEALS:
                printf("%s\n", "F_GET_SEALS");
                break;
#endif
        }
        printf(" = %d", ret);
        errno = error;
        if (ret < 0)
                printf(" (%m)");
        putchar('\n');
        fflush(stdout);
        errno = error;
        return ret;
}

FILE PUBLIC *
fdopen(int fd, const char *mode)
{

        if (!libc) bdsim_init_();

        printf("fdopen(%d,\"%s\")\n", fd, mode);
        errno = ENOSYS;
        return NULL;
}

int PUBLIC
fileno(FILE *stream)
{
        int ret = libc_fileno(stream);

        if (!libc) bdsim_init_();

        printf("fileno(%p) = %d", stream, ret);
        if (ret < 0)
                printf(" (%m)");
        putchar('\n');
        fflush(stdout);
        return ret;
}

FILE PUBLIC *
fopen(const char *pathname, const char *mode)
{

        if (!libc) bdsim_init_();

        printf("fopen(\"%s\",\"%s\")\n", pathname, mode);
        errno = ENOSYS;
        return NULL;
}

FILE PUBLIC *
freopen(const char *pathname, const char *mode, FILE *stream)
{

        if (!libc) bdsim_init_();

        printf("freopen(\"%s\",\"%s\",%p)\n", pathname, mode, stream);
        errno = ENOSYS;
        return NULL;
}

ssize_t PUBLIC
getxattr(const char *path, const char *name, void *value, size_t size)
{
        if (!libc) bdsim_init_();

        printf("getxattr(\"%s\",\"%s\",%p,%zu)\n", path, name, value, size);
        errno = ENOSYS;
        return -1;
}

void PRIVATE
print_ioctl(int fd, unsigned long request)
{
        printf("ioctl(%d,0x%lx,...)\n", fd, request);
}

int PUBLIC
ioctl(int fd, unsigned long request, ...)
{
	uintptr_t arg = get_arg(request, uintptr_t);
	int ret = -1;

	printf("ioctl(%d,%lx,%p)", fd, request, (void *)arg);
	errno = ENOSYS;
	if (!is_blkdev_fd(fd))
		ret = libc_ioctl(fd, request, arg);
	printf(" = %d", ret);
	if (ret < 0)
		printf(" (%m)");
	putchar('\n');
	fflush(stdout);
        return ret;
}

off_t PUBLIC
lseek(int fd, off_t offset, int whence)
{
        int ret;

        if (!libc) bdsim_init_();

        fd = demangle_fd(fd);
        ret = libc_lseek(fd, offset, whence);
        printf("lseek(%d,%ld,%d) = %d", fd, offset, whence, ret);
        if (ret < 0)
                printf(" (%m)");
        putchar('\n');
        fflush(stdout);
        return ret;
}

int PUBLIC
open(const char *pathname, int flags, ...)
{

        if (!libc) bdsim_init_();

        int ret = -1;
        errno = ENOSYS;

        if (flags & O_CREAT) {
                mode_t mode = get_arg(flags, mode_t);

                printf("open(\"%s\", 0x%0x, 0o%0o)", pathname, flags, mode);
                if (!is_blkdev(pathname))
                        ret = libc_open(pathname, flags, mode);
        } else {
                printf("open(\"%s\", 0x%0x)", pathname, flags);
                if (!is_blkdev(pathname))
                        ret = libc_open(pathname, flags);
        }
        printf(" = %d", ret);
        if (ret < 0)
                printf(" (%m)");
        putchar('\n');
        fflush(stdout);
        return ret;
}

int PUBLIC
openat(int dirfd, const char *pathname, int flags, ...)
{

        if (!libc) bdsim_init_();

        printf("openat(%d,\"%s\",0x%0x)\n", dirfd, pathname, flags);
        errno = ENOSYS;
        return -1;
}

DIR PUBLIC *
opendir(const char *name)
{
        DIR *ret;

        if (!libc) bdsim_init_();

        ret = libc_opendir(name);
        printf("opendir(\"%s\") = %p", name, ret);
        if (ret == NULL)
                printf(" (%m)");
        putchar('\n');
        fflush(stdout);
        return ret;
}

DIR PUBLIC *
fdopendir(int fd)
{

        if (!libc) bdsim_init_();

        printf("fdopendir(%d)\n", fd);
        errno = ENOSYS;
        return NULL;
}

struct dirent PUBLIC *
readdir(DIR *dirp)
{
        struct dirent *ret;

        if (!libc) bdsim_init_();

        ret = libc_readdir(dirp);
        printf("readdir(%p) = %p", dirp, ret);
        if (ret == NULL)
                printf(" (%m)");
        putchar('\n');
        fflush(stdout);
        return ret;
}

ssize_t PUBLIC
readlink(const char *pathname, char *buf, size_t bufsiz)
{

        if (!libc) bdsim_init_();

        printf("readlink(\"%s\", %p, %zu)\n", pathname, buf, bufsiz);
        errno = ENOSYS;
        return -1;
}

ssize_t PUBLIC
readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz)
{

        if (!libc) bdsim_init_();

        printf("readlinkat(%d,\"%s\",%p,%zu)\n", dirfd, pathname, buf, bufsiz);
        errno = ENOSYS;
        return -1;
}

// vim:fenc=utf-8:tw=75
