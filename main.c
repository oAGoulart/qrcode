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

#define NUM_MANDATORY 1

typedef enum targ_e {
  ARG_NONE = 0,
  ARG_NOCOPY = 1,
  ARG_VERBOSE = 2,
  ARG_RAW = 4,
  ARG_NOINLINE = 8,
  ARG_RESERVED = 0x8000, // NOTE: exclusive options below
  ARG_MASK,
  ARG_VNUM,
  ARG_SCALE,
  ARG_BMP
} targ_t;
#define NUM_ARGS 8
static const char* args_[NUM_ARGS] = {
  "--nocopy", "--verbose", "--raw", "--noinline",
  "-m", "-v", "-s", "-B"
};
static const targ_t arge_[NUM_ARGS] = {
  ARG_NOCOPY, ARG_VERBOSE, ARG_RAW, ARG_NOINLINE,
  ARG_MASK, ARG_VNUM, ARG_SCALE, ARG_BMP
};

static __inline__ int
phelp_(const char* restrict cmdln)
{
  fprintf(stderr,
    "Usage: %s [OPTIONS] <data to encode>" __nl
    "OPTIONS:" __nl
    "\t--nocopy    do not print copyright header" __nl
    "\t--verbose   print runtime information for generated values" __nl
    "\t--raw       print generated matrix as 1's and 0's (no Unicode)" __nl
    "\t--noinline  do not print any inline code, disregards --raw" __nl
    "\t-m <N>      force N mask output, regardless of penalty; N:(0-7)" __nl
    "\t-v <N>      tries to force use of N version QR Codes; N:(1-"
    __xstr(MAX_VERSION) ")" __nl
    "\t-s <N>      scale image output by N times; N:(1-"
    __xstr(MAX_SCALE) ")" __nl
    "\t-B <STR>    create STR bitmap file with generated code" __nl,
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
    return phelp_(argv[0]);
  }
  targ_t options = ARG_NONE;
  int mask = -1;
  int vnum = -1;
  int scale = -1;
  int argcount = 0;
  imgfmt_t imgfmt = FMT_BMP;
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
          if (arge_[j] < ARG_RESERVED)
          {
            options |= arge_[j];
            break;
          }
          if (arge_[j] == ARG_MASK)
          {
            mask = (uint8_t)atoi(argv[i + 1]);
          }
          else if (arge_[j] == ARG_VNUM)
          {
            vnum = (uint8_t)atoi(argv[i + 1]) - 1;
          }
          else if (arge_[j] == ARG_SCALE)
          {
            scale = (uint8_t)atoi(argv[i + 1]);
          }
          else // NOTE: ARG_BMP
          {
            imgout = argv[i + 1];
            // imgfmt = FMT_BMP;
          }
          argcount++;
          i++;
          break;
        }
      }
    }
  }
  pdebug("finished parsing cmdln arguments");
  if (argc - argcount < NUM_MANDATORY + 1)
  {
    return phelp_(argv[0]);
  }
  if (!(options & ARG_NOCOPY))
  {
    puts(PROJECT_TITLE " " PROJECT_VERSION __nl
         PROJECT_COPYRIGHT __nl PROJECT_LICENSE __nl);
  }
  qrcode_t* qr = NULL;
  int err = create_qrcode(&qr, argv[argc - 1], options & ARG_VERBOSE, vnum);
  fatalif(err);
  if (mask != -1)
  {
    err = qrcode_forcemask(qr, mask);
    fatalif(err);
    if (options & ARG_VERBOSE)
    {
      printf(__c(36, "INFO") " Forced mask: %d" __nl, mask);
    }
  }
  if (!(options & ARG_NOINLINE))
  {
    qrcode_print(qr, options & ARG_RAW);
  }
  if (imgout != NULL)
  {
    err = qrcode_output(qr, imgfmt, scale, imgout);
    fatalif(err);
    if (options & ARG_VERBOSE)
    {
      printf(__c(36, "INFO") " Image written to: %s" __nl, imgout);
    }
  }
  delete_qrcode(&qr);
  return 0;
}
