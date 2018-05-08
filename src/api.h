/*
 * api.h
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
 *
 * Author(s): Peter Jones <pjones@redhat.com>
 */
#ifndef BDSIM_API_H_
#define BDSIM_API_H_

#include "bdsim.h"

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <unistd.h>

extern int PUBLIC close(int fd);
extern int PUBLIC closedir(DIR *dirp);
extern int PUBLIC dirfd(DIR *dirp);
extern int PUBLIC fcntl(int fd, int cmd, ...);
extern FILE * PUBLIC fdopen(int fd, const char *mode);
extern int PUBLIC fileno(FILE *stream);
extern FILE * PUBLIC fopen(const char *pathname, const char *mode);
extern FILE * PUBLIC freopen(const char *pathname, const char *mode, FILE *stream);
extern ssize_t getxattr(const char *path, const char *name, void *value, size_t size);
extern int PUBLIC ioctl(int fd, unsigned long request, ...);
extern off_t PUBLIC lseek(int fd, off_t offset, int whence);
extern int PUBLIC open(const char *pathname, int flags, ...);
extern int PUBLIC openat(int dirfd, const char *pathname, int flags, ...);
extern DIR * PUBLIC opendir(const char *name);
extern DIR * PUBLIC fdopendir(int fd);
extern struct dirent * PUBLIC readdir(DIR *dirp);
extern ssize_t PUBLIC readlink(const char *pathname, char *buf, size_t bufsiz);
extern ssize_t PUBLIC readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz);

#endif /* !BDSIM_API_H_ */
// vim:fenc=utf-8:tw=75
