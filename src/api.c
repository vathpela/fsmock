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

int PUBLIC
close(int fd)
{
        errno = ENOSYS;
        return -1;
}

int PUBLIC
closedir(DIR *dirp)
{
        errno = ENOSYS;
        return -1;
}

int PUBLIC
dirfd(DIR *dirp)
{
        errno = ENOSYS;
        return -1;
}

int PUBLIC
fcntl(int fd, int cmd, ...)
{
        errno = ENOSYS;
        return -1;
}

FILE PUBLIC *
fdopen(int fd, const char *mode)
{
        errno = ENOSYS;
        return NULL;
}

int PUBLIC
fileno(FILE *stream)
{
        errno = ENOSYS;
        return -1;
}

FILE PUBLIC *
fopen(const char *pathname, const char *mode)
{
        errno = ENOSYS;
        return NULL;
}

FILE PUBLIC *
freopen(const char *pathname, const char *mode, FILE *stream)
{
        errno = ENOSYS;
        return NULL;
}

ssize_t
getxattr(const char *path, const char *name, void *value, size_t size)
{
        errno = ENOSYS;
        return -1;
}

int PUBLIC
ioctl(int fd, unsigned long request, ...)
{
        errno = ENOSYS;
        return -1;
}

off_t PUBLIC
lseek(int fd, off_t offset, int whence)
{
        errno = ENOSYS;
        return -1;
}

int PUBLIC
open(const char *pathname, int flags, ...)
{
        errno = ENOSYS;
        return -1;
}

int PUBLIC
openat(int dirfd, const char *pathname, int flags, ...)
{
        errno = ENOSYS;
        return -1;
}

DIR PUBLIC *
opendir(const char *name)
{
        errno = ENOSYS;
        return NULL;
}

DIR PUBLIC *
fdopendir(int fd)
{
        errno = ENOSYS;
        return NULL;
}

struct dirent PUBLIC *
readdir(DIR *dirp)
{
        errno = ENOSYS;
        return NULL;
}

ssize_t PUBLIC
readlink(const char *pathname, char *buf, size_t bufsiz)
{
        errno = ENOSYS;
        return -1;
}

ssize_t PUBLIC
readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz)
{
        errno = ENOSYS;
        return -1;
}

// vim:fenc=utf-8:tw=75
