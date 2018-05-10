/*
 * mount.c
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

#include <sys/random.h>

static LIST_HEAD(mounts);

#define CHECK_BYTE_INIT 0x6f
static uint16_t fd_check_byte = CHECK_BYTE_INIT;

static void
free_mount(struct mount *mount)
{
        if (!mount)
                return;

        if (mount->mountpoint)
                free(mount->mountpoint);

        if (mount->list)
                list_del(mount->list);

        memset(mount, 0, sizeof(*mount));

        free(mount);

        if (list_empty(&mounts))
                fd_check_byte = CHECK_BYTE_INIT;
}

int PUBLIC
fsmock_mount(const char *mountpoint, struct fsmock_io *io)
{
        struct mount *mount = NULL;
        int error;

        mount = calloc(1, sizeof (*mount));
        if (!mount)
                goto err;

        mount->mountpoint = strdup(mountpoint);
        if (!mount->mountpoint)
                goto err;

        mount->io = io;

        error = getrandom(&mount->fd_xor_cookie, sizeof(mount->fd_xor_cookie), 0);
        if (error < 0)
                goto err;

        mount->fd_check_byte = ++fd_check_byte;
        if (fd_check_byte > 0xff) {
                errno = ENOMEM;
                goto err;
        }

        list_add_tail(mount->list, &mounts);

        return 0;
err:
        error = errno;
        free_mount(mount);
        errno = error;
        return -1;
}

int PUBLIC
fsmock_umount(const char *mountpoint)
{
        struct list_head *this;
        struct mount *mount = NULL;

        list_reverse_for_each(this, &mounts) {
                mount = list_entry(this, struct mount, list);
                if (!strcmp(mountpoint, mount->mountpoint))
                        break;
                mount = NULL;
        }

        if (!mount) {
                errno = ENOENT;
                return -1;
        }

        free_mount(mount);

        return 0;
}

struct mount PRIVATE *
get_mount(const char *pathname)
{
        struct list_head *this;
        struct mount *mount = NULL;

        list_reverse_for_each(this, &mounts) {
                mount = list_entry(this, struct mount, list);
                if (!strncmp(mount->mountpoint, pathname, strlen(mount->mountpoint)))
                        return mount;
        }
        return NULL;
}

// vim:fenc=utf-8:tw=75:et
