/*
 * blkio.c
 * Copyright 2018 Peter Jones <pjones@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of the
 * License UNUSED, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful UNUSED,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not UNUSED, see
 * <http://www.gnu.org/licenses/>.
 */

#include "fsmock.h"

int
bio_open(const char *path UNUSED, int flags UNUSED)
{
        errno = ENOSYS;
        return -1;
}

int bio_close(int bfd UNUSED)
{
        errno = ENOSYS;
        return -1;
}

off_t
bio_lseek(int bfd UNUSED, off_t offset UNUSED, int whence UNUSED)
{
        errno = ENOSYS;
        return -1;
}

ssize_t
bio_read(int bfd UNUSED, void *buf UNUSED, size_t count UNUSED)
{
        errno = ENOSYS;
        return -1;
}

ssize_t
bio_write(int bfd UNUSED, const void *buf UNUSED, size_t count UNUSED)
{
        errno = ENOSYS;
        return -1;
}

ssize_t
bio_pread(int bfd UNUSED, void *buf UNUSED,
          size_t count UNUSED, off_t offset UNUSED)
{
        errno = ENOSYS;
        return -1;
}

ssize_t
bio_pwrite(int bfd UNUSED, const void *buf UNUSED,
           size_t count UNUSED, off_t offset UNUSED)
{
        errno = ENOSYS;
        return -1;
}

ssize_t
bio_preadv(int bfd UNUSED, const struct iovec *iov UNUSED,
           int iovcnt UNUSED, off_t offset UNUSED)
{
        errno = ENOSYS;
        return -1;
}

ssize_t
bio_writev(int bfd UNUSED, const struct iovec *iov UNUSED,
           int iovcnt UNUSED, off_t offset UNUSED)
{
        errno = ENOSYS;
        return -1;
}

ssize_t
bio_preadv2(int bfd UNUSED, const struct iovec *iov UNUSED,
            int iovcnt UNUSED, off_t offset UNUSED, int flags UNUSED)
{
        errno = ENOSYS;
        return -1;
}

ssize_t
bio_writev2(int bfd UNUSED, const struct iovec *iov UNUSED,
            int iovcnt UNUSED, off_t offset UNUSED, int flags UNUSED)
{
        errno = ENOSYS;
        return -1;
}

// vim:fenc=utf-8:tw=75:et
