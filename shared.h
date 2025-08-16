#ifndef SHARED_H
#define SHARED_H 1

#include <stdbool.h>
#ifndef __bool_true_false_are_defined
#error "non-standard stdbool.h file"
#endif

#define __nl "\r\n"
#define __str(s) #s
#define __xstr(s) __str(s)
#define __c(c, str) "\033[" __str(c) "m" str "\033[m"

#if !defined(NDEBUG)
#define pdebug(str) puts(__FILE__ ":" __xstr(__LINE__) ": " str)
#else
#define pdebug(str)
#endif

#define PROJECT_TITLE "Command-line QR Code generator"
#define PROJECT_VERSION "(v1.4.0)"
#define PROJECT_COPYRIGHT "Copyright (C) 2025 Augusto Goulart."
#define PROJECT_LICENSE \
  "Licensed under Microsoft Reciprocal License (Ms-RL)." __nl \
  "QR Code is a registered trademark of DENSO WAVE" __nl \
  "INCORPORATED in Japan and in other countries."

#define MAX_VERSION 5
#define MAX_SCALE   30
#define NUM_MASKS   8

#endif
