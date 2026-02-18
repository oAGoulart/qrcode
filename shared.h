#ifndef QR_SHARED_H
#define QR_SHARED_H 1

#if !defined(__clang__)
#   error "must be compiled with clang"
#endif

#include <stdbool.h>
#if !defined(__bool_true_false_are_defined)
#   error "non-standard stdbool.h file"
#endif

#include <errno.h>
#include <stdio.h>
#include <stdint.h>

#define _nl "\n"
#define _str(s) #s
#define _xstr(s) _str(s)
#define _c(c, str) "\033[" _str(c) "m" str "\033[m"

#if !defined(NDEBUG)
#   define pdebug(str) puts(__FILE__ ":" _xstr(__LINE__) ": " str)
#else
#   define pdebug(str)
#endif

#if defined(eprintf)
#   error "eprintf already defined"
#endif
/* NOTE: prints with new-line */
#define eprintf(format, ...) \
  fprintf(stderr, \
    __FILE_NAME__ ":" _xstr(__LINE__) ": " _c(31, "error: ") format _nl, \
    ##__VA_ARGS__)

#if defined(perrno)
#   error "perrno already defined"
#endif
#define perrno(err) \
  errno = err; \
  perror(_c(31, "  \u25CF ") "runtime error")


#if defined(pinfo)
#   error "pinfo already defined"
#endif
/* NOTE: prints with new-line; please, start sentence in capital letter! */
#define pinfo(format, ...) \
  printf(_c(36, "  INFO ") format _nl, ##__VA_ARGS__)

#if defined(__amd64__) || defined(__x86_64__)
#   define PROJECT_ARCH "amd64"
#elif defined(__aarch64__)
#   define PROJECT_ARCH "aarch64"
#elif defined(__arm__)
#   define PROJECT_ARCH "arm"
#elif defined(__ia64__)
#   define PROJECT_ARCH "ia64"
#elif defined(__i386__) || defined(_X86_)
#   define PROJECT_ARCH "i386"
#elif defined(__powerpc64__)
#   define PROJECT_ARCH "ppc64"
#elif defined(__powerpc__)
#   define PROJECT_ARCH "ppc"
#elif defined(__mips__)
#   define PROJECT_ARCH "mips"
#elif defined(__sparc__)
#   define PROJECT_ARCH "sparc"
#else
#   define PROJECT_ARCH "???"
#endif

#if defined(__MSYS__)
#   define PROJECT_TARGET "msys"
#elif defined(__CYGWIN64__)
#   define PROJECT_TARGET "cygwin64"
#elif defined(__CYGWIN__)
#   define PROJECT_TARGET "cygwin"
#elif defined(__MINGW64__)
#   define PROJECT_TARGET "mingw64"
#elif defined(__MINGW32__)
#   define PROJECT_TARGET "mingw32"
#elif defined(_WIN32)
#   define PROJECT_TARGET "windows"
#elif defined(__sun)
#   define PROJECT_TARGET "solaris"
#elif defined(__NetBSD__)
#   define PROJECT_TARGET "netbsd"
#elif defined(__FreeBSD__)
#   define PROJECT_TARGET "freebsd"
#elif defined(__serenity__)
#   define PROJECT_TARGET "serenity"
#elif defined(__MACH__)
#   define PROJECT_TARGET "macos"
#elif defined(__linux__)
#   define PROJECT_TARGET "linux"
#else
#   define PROJECT_TARGET "???"
#endif

#define PROJECT_TITLE     "Command-line QR Code generator"
#define PROJECT_VERSION   "(v1.20.0:" PROJECT_TARGET ":" PROJECT_ARCH ")"
#define PROJECT_COPYRIGHT "Copyright (C) 2025-2026 Augusto Goulart."
#define PROJECT_LICENSE \
  "Licensed under Microsoft Reciprocal License (Ms-RL)." _nl \
  "QR Code is a registered trademark of DENSO WAVE" _nl \
  "INCORPORATED in Japan and in other countries."

#define MAX_VERSION 5
#define MAX_SCALE   30
#define NUM_MASKS   8
#define EC_COUNT    4

typedef struct __attribute__((packed)) qrinfo_s
{
  uint16_t len;       /* qr data length */
  uint8_t  eccpb;     /* ec codewords per block */
  uint8_t  blocks[2]; /* number of blocks per group (2) */
  uint8_t  datapb[2]; /* data codewords per block, per group (2) */
} qrinfo_t;

__inline__ size_t __attribute__((__const__))
align_memory(const size_t size, const size_t alignment)
{
  const size_t remainder = size % alignment;
  return (remainder > 0) ? (size - remainder) + alignment : size;
}

#endif
