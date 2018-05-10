/*
 * fsmock.h
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

#ifndef FSMOCK_H_
#define FSMOCK_H_

#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>

struct fsmock_io {
        int (*open)(const char *pathname, int flags, ...);
        int (*close)(int fd);
        int (*creat)(const char *pathname, mode_t mode);
        int (*unlink)(const char *pathname);
        int (*link)(const char *oldpath, const char *newpath);
        int (*symlink)(const char *target, const char *linkpath);
        ssize_t (*readlink)(const char *pathname, char *buf, size_t bufsiz);
        int (*rename)(const char *oldpath, const char *newpath);

        int (*access)(const char *pathname, int mode);
        int (*stat)(const char *pathname, struct stat *statbuf);
        int (*fstat)(int fd, struct stat *statbuf);
        int (*lstat)(const char *pathname, struct stat *statbuf);

        ssize_t (*read)(int fd, void *buf, size_t count);
        ssize_t (*write)(int fd, const void *buf, size_t count);
        off_t (*lseek)(int fd, off_t offset, int whence);

        int (*chmod)(const char *pathname, mode_t mode);
        int (*fchmod)(int fd, mode_t mode);

        int (*chown)(const char *pathname, uid_t owner, gid_t group);
        int (*fchown)(int fd, uid_t owner, gid_t group);
        int (*lchown)(const char *pathname, uid_t owner, gid_t group);

        int (*utime)(const char *filename, const struct utimbuf *times);
        int (*utimes)(const char *filename, const struct utimbuf times[2]);
        int (*futimes)(int fd, const struct timeval tv[2]);
        int (*lutimes)(const char *filename, const struct timeval tv[2]);
        int (*futimens)(int fd, const struct timespec times[2]);
};

extern int fsmock_mount(const char *mountpoint, struct fsmock_io *io);
extern int fsmock_umount(const char *mountpoint);

#endif /* !FSMOCK_H_ */
// vim:fenc=utf-8:tw=75:et
