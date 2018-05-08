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

extern int access(const char *pathname, int mode) PUBLIC;
extern int close(int fd) PUBLIC;
extern int closedir(DIR *dirp) PUBLIC;
extern int dirfd(DIR *dirp) PUBLIC;
extern int faccessat(int dirfd, const char *pathname, int mode, int flags) PUBLIC;
extern int fcntl(int fd, int cmd, ...) PUBLIC;
extern FILE *fdopen(int fd, const char *mode) PUBLIC;
extern int fileno(FILE *stream) PUBLIC;
extern FILE *fopen(const char *pathname, const char *mode) PUBLIC;
extern FILE *freopen(const char *pathname, const char *mode, FILE *stream) PUBLIC;
extern ssize_t getxattr(const char *path, const char *name, void *value, size_t size) PUBLIC;
extern int ioctl(int fd, unsigned long request, ...) PUBLIC;
extern off_t lseek(int fd, off_t offset, int whence) PUBLIC;
extern int open(const char *pathname, int flags, ...) PUBLIC;
extern int openat(int dirfd, const char *pathname, int flags, ...) PUBLIC;
extern DIR *opendir(const char *name) PUBLIC;
extern DIR *fdopendir(int fd) PUBLIC;
extern struct dirent *readdir(DIR *dirp) PUBLIC;
extern ssize_t readlink(const char *pathname, char *buf, size_t bufsiz) PUBLIC;
extern ssize_t readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz) PUBLIC;

#endif /* !BDSIM_API_H_ */
// vim:fenc=utf-8:tw=75
