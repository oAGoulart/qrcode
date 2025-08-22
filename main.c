#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quickresponse.h"
#include "shared.h"

typedef enum targ_e
{
  ARG_NONE     = 0,
  ARG_NOCOPY   = 1,
  ARG_VERBOSE  = 2,
  ARG_RAW      = 4,
  ARG_NOINLINE = 8,
  ARG_VERSION  = 0x10,
  ARG_OPTIMIZE = 0x20,
  ARG_RESERVED __attribute__((unavailable("bit mask limit"))) = 0x8000,
  ARG_MASK,
  ARG_VNUM,
  ARG_SCALE,
  ARG_BMP,
  ARG_SVG
} targ_t;
#define NUM_ARGS      11
#define NUM_MANDATORY 1

static const char* args_[NUM_ARGS] = {
  "--nocopy", "--verbose", "--raw", "--noinline", "--version", "--optimize",
  "-m", "-u", "-s", "-B", "-K"
};

static const targ_t arge_[NUM_ARGS] = {
  ARG_NOCOPY, ARG_VERBOSE, ARG_RAW, ARG_NOINLINE, ARG_VERSION, ARG_OPTIMIZE,
  ARG_MASK, ARG_VNUM, ARG_SCALE, ARG_BMP, ARG_SVG
};

static __inline__ int
phelp_(const char* __restrict__ cmdln)
{
  fprintf(stderr,
    "Usage: %s [OPTIONS] <string>" __nl
    "OPTIONS:" __nl
    "  --nocopy     omit copyright header from inline printing" __nl
    "  --noinline   do not print any inline code, disregards --raw" __nl
    "  --optimize   reduce data size, encode numeric, alphanumeric, byte" __nl
    "                 segments separately (if any)" __nl
    "  --raw        print generated code with chars 1, 0 (no box-chars)" __nl
    "  --verbose    print runtime information for generated values" __nl
    "  --version    show generator's version and build information" __nl
    "  -m <uint>    force choice of mask <0-7>, regardless of penalty" __nl
    "  -s <uint>    scale image output <1-" __xstr(MAX_SCALE) "> times" __nl
    "  -u <uint>    force use of version <1-" __xstr(MAX_VERSION) "> code"
                      "(or lower, if" __nl
    "                 used with --optimize)" __nl
    "  -B <string>  create bitmap file with generated code" __nl
    "  -K <string>  create scalable vector image, disregards -s" __nl,
    cmdln);
  return EINVAL;
}

int
main(int argc, char* argv[])
{
#if defined(_WIN32)
  // NOTE: to allow box-drawing characters
  system("chcp 65001>nul");
#endif
  if (argc < NUM_MANDATORY + 1)
  {
    eprintf("not enough arguments, provided %d", argc);
    return phelp_(argv[0]);
  }

  targ_t   options = ARG_NONE;
  imgfmt_t imgfmt  = FMT_SVG;
  int mask     = -1;
  int vnum     = -1;
  int scale    = -1;
  int argcount =  0;
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
      "Built with clang " __clang_version__ " @ " __DATE__ " " __TIME__);
    return EXIT_SUCCESS;
  }
  if (argc - argcount < NUM_MANDATORY + 1)
  {
    eprintf("must provide " __xstr(NUM_MANDATORY) " mandatory argument(s)");
    return phelp_(argv[0]);
  }
  if (!(options & ARG_NOCOPY))
  {
    puts(PROJECT_TITLE " " PROJECT_VERSION __nl
         PROJECT_COPYRIGHT __nl PROJECT_LICENSE __nl);
  }

  pdebug("creating qrcode object");
  qrcode_t* qr = NULL;
  int err = create_qrcode(&qr,
    argv[argc - 1], vnum, options & ARG_OPTIMIZE, options & ARG_VERBOSE);
  if (err != 0)
  {
    eprintf("could not create qrcode");
    perrno(err);
  }
  else
  {
    if (mask != -1)
    {
      err = qrcode_forcemask(qr, mask);
      if (err != 0)
      {
        eprintf("could not force qrcode mask choice");
      }
      else if (options & ARG_VERBOSE)
      {
        pinfo("Forced mask: %d", mask);
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
        eprintf("could not output qrcode image");
      }
      else if (options & ARG_VERBOSE)
      {
        pinfo("Image saved to: %s", imgout);
      }
    }
  }
  pdebug("resources clean-up");
  delete_qrcode(&qr);
  return err;
}
