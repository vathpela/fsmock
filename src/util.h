/*
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
#ifndef BDSIM_UTIL_H_
#define BDSIM_UTIL_H_ 1

#include <alloca.h>
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <sched.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <tgmath.h>
#include <unistd.h>

/*
 * I'm not actually sure when these appear, but they're present in the
 * version in front of me.
 */
#if defined(__GNUC__) && defined(__GNUC_MINOR__)
#if __GNUC__ >= 5 && __GNUC_MINOR__ >= 1
#define int_add(a, b, c) __builtin_add_overflow(a, b, c)
#define long_add(a, b, c) __builtin_add_overflow(a, b, c)
#define long_mult(a, b, c) __builtin_mul_overflow(a, b, c)
#define ulong_add(a, b, c) __builtin_add_overflow(a, b, c)
#define ulong_mult(a, b, c) __builtin_mul_overflow(a, b, c)
#endif
#endif
#ifndef int_add
#define int_add(a, b, c) ({                                             \
                const int _limit = INT_MAX;                             \
                int _ret;                                               \
                _ret = _limit - ((unsigned long long)a) >               \
                          ((unsigned long long)b);                      \
                if (!_ret)                                              \
                        *(c) = ((a) + (b));                             \
                _ret;                                                   \
        })
#endif
#ifndef long_add
#define long_add(a, b, c) ({                                            \
                const long _limit = LONG_MAX;                           \
                int _ret;                                               \
                _ret = _limit - ((unsigned long long)a) >               \
                           ((unsigned long long)b);                     \
                if (!_ret)                                              \
                        *(c) = ((a) + (b));                             \
                _ret;                                                   \
        })
#endif
#ifndef long_mult
#define long_mult(a, b, c) ({                                           \
                const long _limit = LONG_MAX;                           \
                int _ret = 1;                                           \
                if ((a) == 0 || (b) == 0)                               \
                        _ret = 0;                                       \
                else                                                    \
                        _ret = _limit / (a) < (b);                      \
                if (!_ret)                                              \
                        *(c) = ((a) * (b));                             \
                _ret;                                                   \
        })
#endif
#ifndef ulong_add
#define ulong_add(a, b, c) ({                                           \
                const unsigned long _limit = ULONG_MAX;                 \
                int _ret;                                               \
                _ret = _limit - ((unsigned long long)a) >               \
                            ((unsigned long long)b);                    \
                if (!_ret)                                              \
                        *(c) = ((a) + (b));                             \
                _ret;                                                   \
        })
#endif
#ifndef ulong_mult
#define ulong_mult(a, b, c) ({                                          \
                const unsigned long _limit = ULONG_MAX;                 \
                int _ret = 1;                                           \
                if ((a) == 0 || (b) == 0)                               \
                        _ret = 0;                                       \
                else                                                    \
                        _ret = _limit / (a) < (b);                      \
                if (!_ret)                                              \
                        *(c) = ((a) * (b));                             \
                _ret;                                                   \
        })
#endif

#if defined(__GNUC__) && defined(__GNUC_MINOR__)
#if __GNUC__ >= 5 && __GNUC_MINOR__ >= 1
#define add(a, b, c) _Generic((c),                                      \
                              int *: int_add(a,b,c),                    \
                              long *: long_add(a,b,c),                  \
                              unsigned long *: ulong_add(a,b,c))
#define mult(a, b, c) _Generic((c),                                     \
                              long *: long_mult(a,b,c),                 \
                              unsigned long *: ulong_mult(a,b,c))
#endif
#endif

#ifndef add
#define add(a, b, c) ({                                                 \
                (*(c)) = ((a) + (b));                                   \
                })
#endif
#ifndef mult
#define mult(a, b, c) ({                                                \
                (*(c)) = ((a) * (b));                                   \
                })
#endif

#define startswith(str, prefix) ({!strncmp(str, prefix, strlen(prefix));})

static inline int UNUSED
read_file(int fd, uint8_t **buf, size_t *bufsize)
{
        uint8_t *p;
        size_t size = 4096;
        size_t filesize = 0;
        ssize_t s = 0;

        uint8_t *newbuf;
        if (!(newbuf = calloc(size, sizeof (uint8_t)))) {
                bdsim_error("could not allocate memory");
                return -1;
        }
        *buf = newbuf;

        do {
                p = *buf + filesize;
                /* size - filesize shouldn't exceed SSIZE_MAX because we're
                 * only allocating 4096 bytes at a time and we're checking that
                 * before doing so. */
                s = read(fd, p, size - filesize);
                if (s < 0 && errno == EAGAIN) {
                        /*
                         * if we got EAGAIN, there's a good chance we've hit
                         * the kernel rate limiter.  Doing more reads is just
                         * going to make it worse, so instead, give it a rest.
                         */
                        sched_yield();
                        continue;
                } else if (s < 0) {
                        int saved_errno = errno;
                        free(*buf);
                        *buf = NULL;
                        *bufsize = 0;
                        errno = saved_errno;
                        bdsim_error("could not read from file");
                        return -1;
                }
                filesize += s;
                /* only exit for empty reads */
                if (s == 0)
                        break;
                if (filesize >= size) {
                        /* See if we're going to overrun and return an error
                         * instead. */
                        if (size > (size_t)-1 - 4096) {
                                free(*buf);
                                *buf = NULL;
                                *bufsize = 0;
                                errno = ENOMEM;
                                bdsim_error("could not read from file");
                                return -1;
                        }
                        newbuf = realloc(*buf, size + 4096);
                        if (newbuf == NULL) {
                                int saved_errno = errno;
                                free(*buf);
                                *buf = NULL;
                                *bufsize = 0;
                                errno = saved_errno;
                                bdsim_error("could not allocate memory");
                                return -1;
                        }
                        *buf = newbuf;
                        memset(*buf + size, '\0', 4096);
                        size += 4096;
                }
        } while (1);

        newbuf = realloc(*buf, filesize+1);
        if (!newbuf) {
                free(*buf);
                *buf = NULL;
                bdsim_error("could not allocate memory");
                return -1;
        }
        newbuf[filesize] = '\0';
        *buf = newbuf;
        *bufsize = filesize+1;
        return 0;
}

static inline uint64_t UNUSED
lcm(uint64_t x, uint64_t y)
{
        uint64_t m = x, n = y, o;
        while ((o = m % n)) {
                m = n;
                n = o;
        }
        return (x / n) * y;
}

/************************************************************
 * get_sector_size
 * Requires:
 *  - filedes is an open file descriptor, suitable for reading
 * Modifies: nothing
 * Returns:
 *  sector size, or 512.
 ************************************************************/
static inline int UNUSED
get_sector_size(int filedes)
{
        int rc, sector_size = 512;

        rc = ioctl(filedes, BLKSSZGET, &sector_size);
        if (rc)
                sector_size = 512;
        return sector_size;
}

#ifndef strndupa
#define strndupa(s, l)                                                  \
       (__extension__ ({                                                \
                const char *__in = (s);                                 \
                size_t __len = strnlen (__in, (l));                     \
                char *__out = (char *) alloca (__len + 1);              \
                strncpy(__out, __in, __len);                            \
                __out[__len] = '\0';                                    \
                __out;                                                  \
        }))
#endif

#define asprintfa(str, fmt, args...)                                    \
        ({                                                              \
                char *_tmp = NULL;                                      \
                int _rc;                                                \
                *(str) = NULL;                                          \
                _rc = asprintf((str), (fmt), ## args);                  \
                if (_rc > 0) {                                          \
                        _tmp = strdupa(*(str));                         \
                        if (!_tmp) {                                    \
                                _rc = -1;                               \
                        } else {                                        \
                                free(*(str));                           \
                                *(str) = _tmp;                          \
                        }                                               \
                } else {                                                \
                        _rc = -1;                                       \
                }                                                       \
                _rc;                                                    \
        })

#define vasprintfa(str, fmt, ap)                                        \
        ({                                                              \
                char *_tmp = NULL;                                      \
                int _rc;                                                \
                *(str) = NULL;                                          \
                _rc = vasprintf((str), (fmt), (ap));                    \
                if (_rc > 0) {                                          \
                        _tmp = strdupa(*(str));                         \
                        if (!_tmp) {                                    \
                                _rc = -1;                               \
                        } else {                                        \
                                free(*(str));                           \
                                *(str) = _tmp;                          \
                        }                                               \
                } else {                                                \
                        _rc = -1;                                       \
                }                                                       \
                _rc;                                                    \
        })

/*
 * This returns a static buffer containing either "" or whatever is set in the
 * environment variable LIBBDSIM_SYSFS
 */
static inline char *
sysfs_root(void)
{
        static char *sysfs_root;
        char *default_sysfs_root = "";

        if (sysfs_root)
                return sysfs_root;
        sysfs_root = getenv("LIBBDSIM_SYSFS");
        if (!sysfs_root)
                sysfs_root = default_sysfs_root;
        return sysfs_root;
}

#define sysfs_opendir(path)                                             \
        ({                                                              \
                int rc_ = -1;                                           \
                char *dirname_ = NULL;                                  \
                DIR *d_ = NULL;                                         \
                                                                        \
                rc_ = asprintfa(&dirname_, "%s/%s", sysfs_root(),       \
                                path);                                  \
                if (rc_ < 0) {                                          \
                        bdsim_error("asprintfa() failed");              \
                } else {                                                \
                        d_ = opendir(dirname_);                         \
                        if (!d_)                                        \
                                bdsim_error("opendir failed on \"%s\"", \
                                          path);                        \
                }                                                       \
                d_;                                                     \
        })

static inline ssize_t
get_file(uint8_t **result, const char * const fmt, ...)
{
        char *path;
        uint8_t *buf = NULL;
        size_t bufsize = 0;
        ssize_t rc;
        va_list ap;
        int error;
        int fd;

        if (result == NULL) {
                bdsim_error("invalid parameter 'result'");
                return -1;
        }

        va_start(ap, fmt);
        rc = vasprintfa(&path, fmt, ap);
        va_end(ap);
        if (rc < 0) {
                bdsim_error("could not allocate memory");
                return -1;
        }

        fd = open(path, O_RDONLY);
        if (fd < 0) {
                bdsim_error("could not open file \"%s\" for reading",
                            path);
                return -1;
        }

        rc = read_file(fd, &buf, &bufsize);
        error = errno;
        close(fd);
        errno = error;

        if (rc < 0) {
                bdsim_error("could not read file \"%s\"", path);
                return -1;
        }

        *result = buf;
        return bufsize;
}

#define read_sysfs_file(buf, fmt, args...)                              \
        ({                                                              \
                uint8_t *buf_ = NULL;                                   \
                ssize_t bufsize_ = -1;                                  \
                int error_;                                             \
                                                                        \
                bufsize_ = get_file(&buf_, "%s" fmt,                    \
                                    sysfs_root(), ## args);             \
                if (bufsize_ > 0) {                                     \
                        uint8_t *buf2_ = alloca(bufsize_);              \
                        error_ = errno;                                 \
                        if (buf2_)                                      \
                                memcpy(buf2_, buf_, bufsize_);          \
                        free(buf_);                                     \
                        *(buf) = (__typeof__(*(buf)))buf2_;             \
                        errno = error_;                                 \
                }                                                       \
                bufsize_;                                               \
        })

#define sysfs_readlink(linkbuf, fmt, args...)                           \
        ({                                                              \
                char *_lb = alloca(PATH_MAX+1);                         \
                char *_pn;                                              \
                int _rc;                                                \
                                                                        \
                *(linkbuf) = NULL;                                      \
                _rc = asprintfa(&_pn, "%s" fmt, sysfs_root(), ## args); \
                if (_rc >= 0) {                                         \
                        ssize_t _linksz;                                \
                        _rc = _linksz = readlink(_pn, _lb, PATH_MAX);   \
                        if (_linksz >= 0)                               \
                                _lb[_linksz] = '\0';                    \
                        else                                            \
                                bdsim_error("readlink of %s failed", _pn);\
                        *(linkbuf) = _lb;                               \
                } else {                                                \
                        bdsim_error("could not allocate memory");       \
                }                                                       \
                _rc;                                                    \
        })

#define sysfs_stat(statbuf, fmt, args...)                               \
        ({                                                              \
                int rc_;                                                \
                char *pn_;                                              \
                                                                        \
                rc_ = asprintfa(&pn_, "%s" fmt, sysfs_root(), ## args); \
                if (rc_ >= 0) {                                         \
                        rc_ = stat(pn_, statbuf);                       \
                        if (rc_ < 0)                                    \
                                bdsim_error("could not stat %s", pn_);  \
                } else {                                                \
                        bdsim_error("could not allocate memory");       \
                }                                                       \
                rc_;                                                    \
        })

static inline bool UNUSED
cinpat(const char c, const char *pat)
{
        for (unsigned int i = 0; pat[i]; i++)
                if (pat[i] == c)
                        return true;
        return false;
}

static inline unsigned int UNUSED
strxcspn(const char *s, const char *pattern)
{
        unsigned int i;
        for (i = 0; s[i]; i++) {
                if (!cinpat(s[i], pattern))
                        break;
        }
        return i;
}

struct span {
        const char *pos;
        size_t len;
};

/*
 * count how many parts of a path there are, with some caveats:
 * a leading / is one because it's a directory, but all other slashes are
 * treated as separators, so i.e.:
 * 1: /
 * 2: /foo foo/bar foo/bar/
 * 3: /foo/bar /foo/bar/ foo/bar/baz
 *
 * the usage model here is 1 pass to count, one allocation, one pass to
 * separate.
 */
static inline unsigned int UNUSED
count_spans(const char *str, const char *reject, unsigned int *chars)
{
        unsigned int s = 0, c = 0, pos = 0;

        if (str[0] == '/') {
                s += 1;
                c += 2;
                pos += 1;
        }

        while (str[pos]) {
                unsigned int n;

                n = strcspn(str + pos, reject);
                if (n) {
                        s += 1;
                        c += n + 1;
                        pos += n;
                }

                pos += strxcspn(str + pos, reject);
        }

        if (chars)
                *chars = c;
        return s;
}

static inline void UNUSED
fill_spans(const char *str, const char *reject, void *spanbuf)
{
        struct span *spans = (struct span *)spanbuf;
        struct span *span = spans;
        unsigned int pos = 0;

        if (str[0] == '/') {
                span->pos = str;
                span->len = 1;
                span++;
                pos += 1;
        }

        while (str[pos]) {
                unsigned int n;

                n = strcspn(str + pos, reject);
                if (n) {
                        span->pos = str + pos;
                        span->len = n;
                        span++;
                        pos += n;
                }

                pos += strxcspn(str + pos, reject);
        }
        span->pos = NULL;
        span->len = 0;
}

#define split_spans(str, reject)                                        \
        ({                                                              \
                struct span *ret_ = NULL;                               \
                unsigned int s_, c_;                                    \
                                                                        \
                s_ = count_spans(str, "/", &c_);                        \
                if (s_) {                                               \
                        ret_ = alloca(sizeof(struct span[s_+1]));       \
                        if (ret_)                                       \
                                fill_spans(str, reject, ret_);          \
                } else {                                                \
                        errno = 0;                                      \
                }                                                       \
                ret_;                                                   \
        })

static inline int UNUSED
find_path_segment(const char *path, int segment, const char **pos, size_t *len)
{
        struct span *span, *last;
        int nspans = 0;

        if (!pos || !len) {
                errno = EINVAL;
                return -1;
        }

        span = split_spans(path, "/");
        if (!span) {
                if (errno)
                        return -1;
                *pos = NULL;
                *len = 0;
                return 0;
        }

        for (last = span; last->pos; last++)
                nspans += 1;

        //printf("nspans: %d\n", nspans);
        if (segment < 0) {
                //printf("segment: %d -> ", segment);
                segment = nspans + segment;
                //printf("%d\n", segment);
        }

        if (nspans < 1 || segment < 0 || segment >= nspans) {
                errno = ENOENT;
                return -1;
        }

        for (int i = 0; i < segment; i++)
                span++;

        *pos = span->pos;
        *len = span->len;
        return 0;
}

#define pathseg(path, seg)                                              \
        ({                                                              \
                const char *pos_ = NULL;                                \
                char *ret_ = NULL;                                      \
                size_t len_ = 0;                                        \
                int rc_;                                                \
                                                                        \
                rc_ = find_path_segment(path, seg, &pos_, &len_);       \
                if (rc_ >= 0) {                                         \
                        ret_ = alloca(len_ + 1);                        \
                        if (ret_) {                                     \
                                memcpy(ret_, pos_, len_);               \
                                ret_[len_] = '\0';                      \
                        }                                               \
                }                                                       \
                ret_;                                                   \
        })

#define get_arg(param, type)                                    \
        ({                                                      \
                va_list ap_;                                    \
                type arg_;                                      \
                va_start(ap_, param);                           \
                arg_ = va_arg(ap_, type);                       \
                va_end(ap_);                                    \
                arg_;                                           \
        })


#define get_ap_arg(param, type) va_arg(param, type)

extern DIR PRIVATE *rootdir;
extern int PRIVATE rootfd;

static inline bool
is_blkdev_fd(int fd)
{
        return ((((unsigned long long)fd >> ((sizeof(fd) * 8) - 8)) & 0xffULL) == 0xbbULL);
}

static inline int
demangle_fd(int fd)
{
        if (is_blkdev_fd(fd)) {
                unsigned int mask = ~(0x0ffU << ((sizeof(fd) * 8U) - 8U));

                fd &= mask;
        }
        return fd;
}

static inline int
mangle_fd(int fd)
{
        unsigned int mask = ~(0x0ffU << ((sizeof(fd) * 8U) - 8U));
        unsigned int marker = 0x0bbU << ((sizeof(fd) * 8U) - 8U);

        return ((fd & mask) | marker);
}

static inline bool UNUSED
is_blkdev_at(int dirfd, const char *pathname)
{
        struct stat sb;
        int rc;

        memset(&sb, 0, sizeof (sb));
        rc = fstatat(dirfd, pathname, &sb, AT_EMPTY_PATH|AT_NO_AUTOMOUNT);
        if (rc < 0)
                return false;
        return S_ISBLK(sb.st_mode);
}

static inline bool UNUSED
is_blkdev(const char *pathname)
{
        struct stat sb;
        int rc;

        memset(&sb, 0, sizeof (sb));
        rc = stat(pathname, &sb);
        if (rc < 0)
                return false;
        return S_ISBLK(sb.st_mode);
}

static inline bool UNUSED
is_our_path(const char *pathname)
{
        char real[PATH_MAX];
        char *ret;

        ret = realpath(pathname, real);
        if (ret == NULL)
                return false;
        return (startswith(real, "/dev/")
                || startswith(real, "/sys/")
                || startswith(real, "/proc/")
                || is_blkdev(real));
}

#define debug_(file, line, func, level, fmt, args...)                   \
        ({                                                              \
                if (efi_get_verbose() >= level) {                       \
                        FILE *logfile_ = efi_get_logfile();             \
                        int len_ = strlen(fmt);                         \
                        fprintf(logfile_, "%s:%d %s(): ",               \
                                file, line, func);                      \
                        fprintf(logfile_, fmt, ## args);                \
                        if (!len_ || fmt[len_ - 1] != '\n')             \
                                fprintf(logfile_, "\n");                \
                }                                                       \
        })

#define debug(level, fmt, args...) debug_(__FILE__, __LINE__, __func__, level, fmt, ## args)

#define DEBUG 1

extern void PUBLIC bdsim_set_verbose(int verbosity, FILE *errlog);
extern int PUBLIC bdsim_get_verbose(void);
extern FILE PUBLIC *bdsim_get_logfile(void);

#endif /* BDSIM_UTIL_H_ */
