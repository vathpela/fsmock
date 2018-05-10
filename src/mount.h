/*
 * mount.h
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

#ifndef MOUNT_H_
#define MOUNT_H_

#include "fsmock.h"

struct mount {
        char *mountpoint;
        struct fsmock_io *io;
        int fd_xor_cookie;
        uint16_t fd_check_byte;
        struct list_head *list;
};

struct mount PRIVATE *get_mount(const char *pathname);

#endif /* !MOUNT_H_ */
// vim:fenc=utf-8:tw=75:et
