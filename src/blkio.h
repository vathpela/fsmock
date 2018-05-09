/*
 * blkio.h
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

#ifndef BDSIM_BLKIO_H_
#define BDSIM_BLKIO_H_

extern int bio_open(const char *path, int flags);
extern int bio_close(int bfd);
extern off_t bio_lseek(int bfd, off_t offset, int whence);
extern ssize_t bio_read(int bfd, void *buf, size_t count);
extern ssize_t bio_write(int bfd, const void *buf, size_t count);
extern ssize_t bio_pread(int bfd, void *buf, size_t count, off_t offset);
extern ssize_t bio_pwrite(int bfd, const void *buf, size_t count, off_t offset);
extern ssize_t bio_preadv(int bfd, const struct iovec *iov, int iovcnt, off_t offset);
extern ssize_t bio_writev(int bfd, const struct iovec *iov, int iovcnt, off_t offset);
extern ssize_t bio_preadv2(int bfd, const struct iovec *iov, int iovcnt, off_t offset, int flags);
extern ssize_t bio_writev2(int bfd, const struct iovec *iov, int iovcnt, off_t offset, int flags);

#endif /* !BDSIM_BLKIO_H_ */
// vim:fenc=utf-8:tw=75:et
