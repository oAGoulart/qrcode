#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#ifdef _WIN32
#include <stdlib.h>
#endif
#include "quickresponse.h"
#include "shared.h"

typedef enum targ_e {
  ARG_NONE = 0,
  ARG_NOCOPY = 1,
  ARG_VERBOSE = 2,
  ARG_RAW = 4,
  ARG_NOINLINE = 8,
  ARG_VERSION = 0x10,
  ARG_OPTIMIZE = 0x20,
  ARG_RESERVED = 0x8000, // NOTE: exclusive options below
  ARG_MASK,
  ARG_VNUM,
  ARG_SCALE,
  ARG_BMP,
  ARG_SVG
} targ_t;
#define NUM_ARGS 10
#define NUM_MANDATORY 1

static const char* args_[NUM_ARGS] = {
  "--nocopy", "--verbose", "--raw", "--noinline", "--version", "--optimize",
  "-m", "-u", "-s", "-B", "-G"
};

static const targ_t arge_[NUM_ARGS] = {
  ARG_NOCOPY, ARG_VERBOSE, ARG_RAW, ARG_NOINLINE, ARG_VERSION, ARG_OPTIMIZE,
  ARG_MASK, ARG_VNUM, ARG_SCALE, ARG_BMP, ARG_SVG
};

static __inline__ int
phelp_(const char* __restrict__ cmdln)
{
  fprintf(stderr,
    "Usage: %s [OPTIONS] <data to encode>" __nl
    "OPTIONS:" __nl
    "\t--nocopy    omit copyright header from inline printing" __nl
    "\t--noinline  do not print any inline code, disregards --raw" __nl
    "\t--optimize  reduce data size, encode numeric, alphanumeric, byte" __nl
    "\t            segments separately (if any)" __nl
    "\t--raw       print generated code with chars 1, 0 (no box-chars)" __nl
    "\t--verbose   print runtime information for generated values" __nl
    "\t--version   show generator's version and build information" __nl
    "\t-m <N>      force choice of N mask, regardless of penalty; N:(0-7)" __nl
    "\t-s <N>      scale image output by N times; N:(1-"
                   __xstr(MAX_SCALE) ")" __nl
    "\t-u <N>      tries to force use of N version code; N:(1-"
                   __xstr(MAX_VERSION) ")" __nl
    "\t-B <STR>    create STR bitmap file with generated code" __nl
    "\t-G <STR>    create STR scalable vector image, disregards -s" __nl,
    cmdln);
  return EINVAL;
}

int
main(int argc, char* argv[])
{
#ifdef _WIN32
  // NOTE: to allow box-drawing characters
  system("chcp 65001");
#endif
  if (argc < NUM_MANDATORY + 1)
  {
    pdebug(__c(31, "error:") " not enough arguments");
    return phelp_(argv[0]);
  }

  targ_t options = ARG_NONE;
  int mask = -1;
  int vnum = -1;
  int scale = -1;
  int argcount = 0;
  imgfmt_t imgfmt = FMT_SVG;
  char* imgout = NULL;

  pdebug("started parsing cmdln arguments");
  int i = 1;
  for (; i < argc; i++)
  {
    if (argv[i][0] == '-')
    {
      size_t j = 0;
      for (; j < NUM_ARGS; j++)
      {
        if (!strcmp(argv[i], args_[j]))
        {
          argcount++;
          bool xarg = true;
          switch (arge_[j])
          {
          case ARG_MASK:
            mask = (uint8_t)atoi(argv[i + 1]);
            break;
          case ARG_VNUM:
            vnum = (uint8_t)atoi(argv[i + 1]) - 1;
            break;
          case ARG_SCALE:
            scale = (uint8_t)atoi(argv[i + 1]);
            break;
          case ARG_BMP:
            imgfmt = FMT_BMP;
            __attribute__ ((fallthrough));
          case ARG_SVG:
            imgout = argv[i + 1];
            break;
          default:
            options |= arge_[j];
            xarg = false;
            break;
          }
          if (xarg)
          {
            argcount++;
            i++;
          }
          break;
        }
      }
    }
  }
  pdebug("finished parsing cmdln arguments");
  if (options & ARG_VERSION)
  {
    puts(PROJECT_TITLE " " PROJECT_VERSION __nl
      "Built with GCC " __xstr(__GNUC__) "." __xstr(__GNUC_MINOR__) "."
      __xstr(__GNUC_PATCHLEVEL__) " @ " __DATE__ " " __TIME__);
    return EXIT_SUCCESS;
  }
  if (argc - argcount < NUM_MANDATORY + 1)
  {
    pdebug(__c(31, "error:") " did not provide mandatory arguments");
    return phelp_(argv[0]);
  }
  if (!(options & ARG_NOCOPY))
  {
    puts(PROJECT_TITLE " " PROJECT_VERSION __nl
         PROJECT_COPYRIGHT __nl PROJECT_LICENSE __nl);
  }

  pdebug("creating qrcode object");
  qrcode_t* qr = NULL;
  int err = create_qrcode(&qr, argv[argc - 1], vnum,
                          options & ARG_OPTIMIZE,
                          options & ARG_VERBOSE);
  if (err != 0)
  {
    errno = err;
    perror(__c(31, "\t\u25CF") " create_qrcode error");
  }
  else
  {
    if (mask != -1)
    {
      err = qrcode_forcemask(qr, mask);
      if (err != 0)
      {
        errno = err;
        perror(__c(31, "\t\u25CF") " qrcode_forcemask error");
      }
      else if (options & ARG_VERBOSE)
      {
        printf(__c(36, "INFO") " Forced mask: %d" __nl, mask);
      }
    }
    if (!(options & ARG_NOINLINE))
    {
      pdebug("printing inline qrcode");
      qrcode_print(qr, options & ARG_RAW);
    }
    if (imgout != NULL)
    {
      err = qrcode_output(qr, imgfmt, scale, imgout);
      if (err != 0)
      {
        errno = err;
        perror(__c(31, "\t\u25CF") " qrcode_output error");
      }
      else if (options & ARG_VERBOSE)
      {
        printf(__c(36, "INFO") " Image written to: %s" __nl, imgout);
      }
    }
  }
  pdebug("resources clean-up");
  delete_qrcode(&qr);
  return err;
}
