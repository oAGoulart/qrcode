#ifndef SHARED_H
#define SHARED_H 1

#if !defined(__clang__)
#   error "must be compiled with clang"
#endif

#include <stdbool.h>
#if !defined(__bool_true_false_are_defined)
#   error "non-standard stdbool.h file"
#endif

#if defined(__STDC_VERSION__)
#   warning "use ISO C 2011 with GNU extensions"
#endif

#include <stdio.h>
#include <errno.h>

#define __nl "\r\n"
#define __str(s) #s
#define __xstr(s) __str(s)
#define __c(c, str) "\033[" __str(c) "m" str "\033[m"

#if !defined(NDEBUG)
#   define pdebug(str) puts(__FILE__ ":" __xstr(__LINE__) ": " str)
#else
#   define pdebug(str)
#endif

#if defined(eprintf)
#   error "eprintf already defined"
#endif
// NOTE: prints with new-line
#define eprintf(format, ...) \
  fprintf(stderr, \
    __FILE_NAME__ ":" __xstr(__LINE__) ": " __c(31, "error: ") format __nl, \
    ##__VA_ARGS__)

#if defined(perrno)
#   error "perrno already defined"
#endif
#define perrno(err) \
  errno = err; \
  perror(__c(31, "  \u25CF ") "runtime error")


#if defined(pinfo)
#   error "pinfo already defined"
#endif
// NOTE: prints with new-line
#define pinfo(format, ...) \
  printf(__c(36, "  INFO ") format __nl, ##__VA_ARGS__)

#if defined(__amd64__) || defined(__x86_64__)
#   define PROJECT_ARCH "AMD64"
#elif defined(__aarch64__)
#   define PROJECT_ARCH "ARM64"
#elif defined(__arm__)
#   define PROJECT_ARCH "ARM"
#elif defined(__i386__) || defined(_X86_)
#   define PROJECT_ARCH "x86"
#elif defined(__ia64__)
#   define PROJECT_ARCH "IA-64"
#elif defined(__powerpc64__)
#   define PROJECT_ARCH "PowerPC64"
#elif defined(__powerpc__)
#   define PROJECT_ARCH "PowerPC"
#elif defined(__mips__)
#   define PROJECT_ARCH "MIPS"
#elif defined(__sparc__)
#   define PROJECT_ARCH "SPARC"
#else
#   define PROJECT_ARCH "???"
#endif

#if defined(__CYGWIN__)
#   define PROJECT_TARGET "Cygwin"
#elif defined(__MINGW64__)
#   define PROJECT_TARGET "MinGW-w64"
#elif defined(__MINGW32__)
#   define PROJECT_TARGET "MinGW32"
#elif defined(_WIN32)
#   define PROJECT_TARGET "Windows"
#elif defined(__NetBSD__)
#   define PROJECT_TARGET "NetBSD"
#elif defined(__FreeBSD__)
#   define PROJECT_TARGET "FreeBSD"
#elif defined(__serenity__)
#   define PROJECT_TARGET "SerenityOS"
#elif defined(__MACH__)
#   define PROJECT_TARGET "MacOS"
#elif defined(__linux__)
#   define PROJECT_TARGET "Linux"
#else
#   define PROJECT_TARGET "???"
#endif

#define PROJECT_TITLE     "Command-line QR Code generator"
#define PROJECT_VERSION   "(v1.4.0:" PROJECT_TARGET ":" PROJECT_ARCH ")"
#define PROJECT_COPYRIGHT "Copyright (C) 2025 Augusto Goulart."
#define PROJECT_LICENSE \
  "Licensed under Microsoft Reciprocal License (Ms-RL)." __nl \
  "QR Code is a registered trademark of DENSO WAVE" __nl \
  "INCORPORATED in Japan and in other countries."

#define MAX_VERSION 5
#define MAX_SCALE   30
#define NUM_MASKS   8

#endif
