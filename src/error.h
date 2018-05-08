/*
 * Copyright 2018 Red Hat, Inc.
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
 */
#ifndef BDSIM_ERROR_H_
#define BDSIM_ERROR_H_ 1

#include <endian.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <byteswap.h>

extern int PUBLIC bdsim_error_get(unsigned int n, char ** const filename,
                                  char ** const function, int *line,
                                  char ** const message, int *error)
        __attribute__((__nonnull__ (2, 3, 4, 5, 6)));
extern int PUBLIC
        __attribute__((__nonnull__ (1, 2, 5)))
        __attribute__((__format__ (printf, 5, 6)))
bdsim_error_set(const char *filename, const char *function,
                                  int line, int error, const char *fmt, ...)
        ;
extern void PUBLIC bdsim_error_clear(void);

#define bdsim_error_real__(errval, file, function, line, fmt, args...) \
        bdsim_error_set(file, function, line, errval, (fmt), ## args)

#define bdsim_error(fmt, args...) \
        bdsim_error_real__(errno, __FILE__, __func__, __LINE__, (fmt), ## args)
#define bdsim_error_val(errval, msg, args...) \
        bdsim_error_real__(errval, __FILE__, __func__, __LINE__, (fmt), ## args)

#endif /* BDSIM_ERROR_H_ */
