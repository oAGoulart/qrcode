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
  ARG_NOCOPY = 1 << __COUNTER__,
  ARG_VERBOSE = 1 << __COUNTER__,
  ARG_RAW = 1 << __COUNTER__,
  ARG_MASK = 1 << __COUNTER__,
  ARG_VNUM = 1 << __COUNTER__,
  ARG_BMP = 1 << __COUNTER__
} targ_t;
static const uint16_t numargs_ = __COUNTER__;

static __inline__ int
print_help_(const char* cmdln)
{
  fprintf(stderr,
    "Usage: %s [OPTIONS] <data to encode>" __nl
    "OPTIONS:" __nl
    "\t--nocopy     do not print copyright header" __nl
    "\t--verbose    print runtime information for generated values" __nl
    "\t--raw        print generated matrix as 1's and 0's (no Unicode)" __nl
    "\t--mask <N>   force N mask output, regardless of penalty; N:(0-7)" __nl
    "\t--vnum <N>   tries to force use of N version QR Codes; N:(1-%d)" __nl
    "\t--bmp <STR>  create STR bitmap file with generated code" __nl,
    cmdln, MAX_VERSION);
  return EINVAL;
}

int
main(int argc, char* argv[])
{
#ifdef _WIN32
  // NOTE: to allow box-drawing characters
  system("chcp 65001>nul");
#endif

  if (argc < NUM_MANDATORY + 1)
  {
    return print_help_(argv[0]);
  }
  targ_t options = ARG_NONE;
  int mask = -1;
  int vnum = -1;
  int argcount = 0;
  imgfmt_t imgfmt = FMT_BMP;
  char* imgout = NULL;
  pdebug("started parsing cmdln arguments");
  int i = 1;
  for (; i < argc; i++)
  {
    if (argv[i][0] == '-')
    {
      const char* args[] = {
        "--nocopy", "--verbose", "--raw", "--mask", "--vnum", "--bmp"
      };
      const targ_t arge[] = {
        ARG_NOCOPY, ARG_VERBOSE, ARG_RAW, ARG_MASK, ARG_VNUM, ARG_BMP
      };
      size_t j = 0;
      for (; j < numargs_; j++)
      {
        if (!strcmp(argv[i], args[j]))
        {
          options |= arge[j];
          argcount++;
          if (arge[j] < ARG_MASK)
          {
            break;
          }
          if (arge[j] == ARG_MASK)
          {
            mask = (uint8_t)atoi(argv[i + 1]);
          }
          else if (arge[j] == ARG_VNUM)
          {
            vnum = (uint8_t)atoi(argv[i + 1]) - 1;
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
    return print_help_(argv[0]);
  }
  if (!(options & ARG_NOCOPY))
  {
    puts(PROJECT_TITLE " " PROJECT_VERSION __nl
         PROJECT_COPYRIGHT __nl PROJECT_LICENSE __nl);
  }
  qrcode_t* qr = NULL;
  int err = create_qrcode(&qr, argv[argc - 1], options & ARG_VERBOSE, vnum);
  fatalif(err);
  if (options & ARG_MASK)
  {
    err = qrcode_forcemask(qr, mask);
    fatalif(err);
    if (options & ARG_VERBOSE)
    {
      printf(__c(36, "INFO") " Forced mask: %d" __nl, mask);
    }
  }
  qrcode_print(qr, options & ARG_RAW);
  if (options & ARG_BMP)
  {
    err = qrcode_output(qr, imgfmt, imgout);
    fatalif(err);
    if (options & ARG_VERBOSE)
    {
      printf(__c(36, "INFO") " Image written to: %s" __nl, imgout);
    }
  }
  delete_qrcode(&qr);
  return 0;
}
