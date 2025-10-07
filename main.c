#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mask.h"
#include "quickresponse.h"
#include "shared.h"

typedef enum targ_e
{
  ARG_NONE     = 0,
  ARG_NOCOPY   = 1,
  ARG_VERBOSE  = 2,
  ARG_RAW      = 4,
  ARG_NOINLINE = 8,
  ARG_INFO     = 0x10,
  ARG_OPTIMIZE = 0x20,
  ARG_RESERVED __attribute__((unavailable("bit mask limit"))) = 0x8000,
  ARG_MASK,
  ARG_VER,
  ARG_LEVEL,
  ARG_SCALE,
  ARG_BMP,
  ARG_SVG
} targ_t;
#define NUM_ARGS      12
#define NUM_MANDATORY 1

static const char* args_[NUM_ARGS] = {
  "--nocopy", "--verbose", "--raw",
  "--noinline", "--version", "--optimize",
  "-m", "-u", "-l", "-s", "-B", "-K"
};

static const targ_t arge_[NUM_ARGS] = {
  ARG_NOCOPY, ARG_VERBOSE, ARG_RAW,
  ARG_NOINLINE, ARG_INFO, ARG_OPTIMIZE,
  ARG_MASK, ARG_VER, ARG_LEVEL, ARG_SCALE,
  ARG_BMP, ARG_SVG
};

static __inline__ int
phelp_(const char* __restrict__ cmdln)
{
  fprintf(stderr, "Usage: %s [OPTIONS] <string>" _nl
    "OPTIONS:" _nl
    "  --nocopy     omit copyright header from inline printing" _nl
    "  --noinline   do not print any inline code, disregards --raw" _nl
    "  --optimize   reduce data size, encode numeric, alphanumeric, byte" _nl
    "                 segments separately (if any)" _nl
    "  --raw        print generated code with chars 1, 0 (no box-chars)" _nl
    "  --verbose    print runtime information for generated values" _nl
    "  --version    show generator's version and build information" _nl
    "  -l <char>    use a specific error correction level (l, m, q, or h)" _nl
    "  -m <uint>    force choice of mask <0-7>, regardless of penalty" _nl
    "  -s <uint>    scale image output <1-" _xstr(MAX_SCALE) "> times" _nl
    "  -u <uint>    force use of version <1-" _xstr(MAX_VERSION) "> code"
                      "(or lower, if" _nl
    "                 used with --optimize)" _nl
    "  -B <string>  create bitmap file with generated code" _nl
    "  -K <string>  create scalable vector image, disregards -s" _nl,
    cmdln);
  return EINVAL;
}

int
main(const int argc, char* argv[])
{
#if defined(_WIN32)
  // NOTE: to allow box-drawing characters
  system("chcp 65001>nul");
#endif
  if (argc < NUM_MANDATORY + 1)
  {
    eprintf("not enough arguments, provided %d", argc - 1);
    return phelp_(argv[0]);
  }

  targ_t    options = ARG_NONE;
  imgfmt_t  imgfmt  = FMT_SVG;
  eclevel_t level   = EC_LOW;
  int mask     = -1;
  int ver      = -1;
  int scale    = -1;
  int argcount =  0;
  char* imgout = NULL;

  pdebug("started parsing cmdln arguments");
  for (int i = 1; i < argc; i++)
  {
    if (argv[i][0] == '-')
    {
      for (size_t j = 0; j < NUM_ARGS; j++)
      {
        if (!__builtin_strcmp(argv[i], args_[j]))
        {
          argcount++;
          bool xarg = true;
          switch (arge_[j])
          {
          case ARG_MASK:
            mask = strtol(argv[i + 1], NULL, 0);
            break;
          case ARG_LEVEL:
          {
            if (__builtin_strlen(argv[i + 1]) == 1)
            {
              const char lvl = argv[i + 1][0];
              if (lvl == 'l' || lvl == 'L')
              {
                level = EC_LOW;
                break;
              }
              if (lvl == 'm' || lvl == 'M')
              {
                level = EC_MEDIUM;
                break;
              }
              if (lvl == 'q' || lvl == 'Q')
              {
                level = EC_QUARTILE;
                break;
              }
              if (lvl == 'h' || lvl == 'H')
              {
                level = EC_HIGH;
                break;
              }
            }
            eprintf("unsupported error correction level");
            perrno(EINVAL);
            return EINVAL;
          }
          case ARG_VER:
            ver = strtol(argv[i + 1], NULL, 0) - 1;
            break;
          case ARG_SCALE:
            scale = strtol(argv[i + 1], NULL, 0);
            break;
          case ARG_BMP:
            imgfmt = FMT_BMP;
            __attribute__((fallthrough));
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
  if (options & ARG_INFO)
  {
    puts(PROJECT_TITLE " " PROJECT_VERSION _nl
         "Built with Clang " __clang_version__ "@ " __DATE__ " " __TIME__);
    return EXIT_SUCCESS;
  }
  if (argc - argcount < NUM_MANDATORY + 1)
  {
    eprintf("must provide " _xstr(NUM_MANDATORY) " mandatory argument(s)");
    return phelp_(argv[0]);
  }
  if (!(options & ARG_NOCOPY))
  {
    puts(PROJECT_TITLE " " PROJECT_VERSION _nl
         PROJECT_COPYRIGHT _nl PROJECT_LICENSE _nl);
  }

  pdebug("creating qrcode object");
  qrcode_t* qr = NULL;
  int err = create_qrcode(&qr,
    argv[argc - 1], ver, level,
    options & ARG_OPTIMIZE,
    options & ARG_VERBOSE);
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
