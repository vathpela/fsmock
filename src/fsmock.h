/*
 * fsmock.h
 * Copyright 2018 Peter Jones <pjones@redhat.com>
 *
 * Distributed under terms of the GPLv3 license.
 */

#ifndef FSMOCK_PRIVATE_H_
#define FSMOCK_PRIVATE_H_

#include "fix_coverity.h"

#include <fsmock.h>

#define UNUSED __attribute__((__unused__))
#define HIDDEN __attribute__((__visibility__ ("hidden")))
#define PRIVATE HIDDEN
#define PUBLIC __attribute__((__visibility__ ("default")))
#define NAKED __attribute__((naked))
#define CONSTRUCTOR __attribute__((constructor))
#define CONSTRUCTOR_N(n) __attribute__((constructor(n)))
#define DESTRUCTOR __attribute__((destructor))
#define DESTRUCTOR_N(n) __attribute__((destructor(n)))
#define VERSION(name, version) __asm__(".symver " name "," name "@@" version)
#define NORETURN __attribute__((__noreturn__))

#include "list.h"
#include "error.h"
#include "util.h"
#include "api.h"
#include "blkio.h"
#include "mount.h"

#endif /* !FSMOCK_PRIVATE_H_ */
// vim:fenc=utf-8:tw=75
