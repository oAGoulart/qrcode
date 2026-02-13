#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mask.h"
#include "code.h"
#include "shared.h"

/* NOTE: not to be confused with MAX_VERSION */
#define INLINE_VERSION_LIMIT 5

typedef enum cli_argument_e
{
  ARG_NONE     = 0,
  ARG_RAW      = 1,
  ARG_INFO     = 2,
  ARG_OPTIMIZE = 4,
  ARG_HELP     = 8,
  ARG_NOLIMIT  = 0x10,
  ARG_RESERVED __attribute__((unavailable("bit mask limit"))) = 0x8000,
  ARG_VERBOSE,
  ARG_MASK,
  ARG_VERSION,
  ARG_LEVEL,
  ARG_SCALE,
  ARG_BMP,
  ARG_SVG
} cli_argument_t;

typedef struct cli_option_s
{
  const char*    flag;
  cli_argument_t type;
} cli_option_t;

static const cli_option_t cli_options_[] = {
  { "--raw", ARG_RAW },
  { "--version", ARG_INFO },
  { "-v", ARG_INFO },
  { "--optimize", ARG_OPTIMIZE },
  { "--help", ARG_HELP },
  { "-h", ARG_HELP },
  { "--nolimit", ARG_NOLIMIT },
  { "-g", ARG_VERBOSE },
  { "-m", ARG_MASK },
  { "-l", ARG_LEVEL },
  { "-s", ARG_SCALE },
  { "-V", ARG_VERSION },
  { "-B", ARG_BMP },
  { "-K", ARG_SVG }
};
#define NUM_CLI_ARGS (sizeof(cli_options_) / sizeof(cli_options_[0]))
#define NUM_MANDATORY_ARGS 1

static __inline__ int
print_help_(const char* __restrict__ cmdln)
{
  fprintf(stderr,
    "Usage: %s [OPTIONS] <data to encode>" _nl
    "OPTIONS:" _nl
    "  -h, --help     show this help message" _nl
    "  --nolimit      ignore inline Version limit (for larger terminals)" _nl
    "  --optimize     reduce data size, encode numeric, alphanumeric, byte" _nl
    "                   segments separately (if any)" _nl
    "  --raw          print generated code with chars 1, 0 (no box-chars)" _nl
    "  -v, --version  show generator's version and build information" _nl
    "  -g <uint>      level of on-screen information <0-3>" _nl
    "  -l <char>      use a specific error correction level (l, m, q, or h)" _nl
    "  -m <uint>      force use of mask <0-7>, regardless of penalty" _nl
    "  -s <uint>      scale image output <1-" _xstr(MAX_SCALE) "> times" _nl
    "  -V <uint>      force use of version <1-" _xstr(MAX_VERSION) "> code"
                      " (or lower, if" _nl
    "                   used with --optimize)" _nl
    "  -B <string>    create bitmap file with generated code" _nl
    "  -K <string>    create scalable vector image, disregards -s" _nl,
    cmdln
  );
  return EINVAL;
}

int
main(const int argc, char* argv[])
{
#if defined(_WIN32) || defined(__CYGWIN__)
  /* NOTE: to allow box-drawing characters */
  system("chcp 65001>nul");
#endif

  cli_argument_t options = ARG_NONE;
  imgfmt_t       imgfmt  = FMT_SVG;
  eclevel_t      level   = EC_LOW;
  int verbose    = 1;
  int mask       = -1;
  int version    = -1;
  int scale      = -1;
  int arg_count  =  0;
  char* filename = NULL;

  pdebug("started parsing cmdln arguments");
  for (int arg = 1; arg < argc; arg++)
  {
    if (argv[arg][0] == '-')
    {
      for (size_t j = 0; j < NUM_CLI_ARGS; j++)
      {
        if (!strcmp(argv[arg], cli_options_[j].flag))
        {
          arg_count++;
          bool exclusive_arg = true;
          switch (cli_options_[j].type)
          {
          case ARG_VERBOSE:
            verbose = strtol(argv[arg + 1], NULL, 0);
            break;
          case ARG_MASK:
            mask = strtol(argv[arg + 1], NULL, 0);
            break;
          case ARG_LEVEL:
          {
            if (strlen(argv[arg + 1]) == 1)
            {
              const char lvl = (char)tolower(argv[arg + 1][0]);
              if (strchr("lmqh", lvl) != NULL)
              {
                level = lvl - 'h';
                break;
              }
            }
            eprintf("unsupported error correction level");
            perrno(EINVAL);
            return EINVAL;
          }
          case ARG_VERSION:
            version = strtol(argv[arg + 1], NULL, 0) - 1;
            break;
          case ARG_SCALE:
            scale = strtol(argv[arg + 1], NULL, 0);
            break;
          case ARG_BMP:
            imgfmt = FMT_BMP;
            __attribute__((fallthrough));
          case ARG_SVG:
            filename = argv[arg + 1];
            break;
          default:
            options |= cli_options_[j].type;
            exclusive_arg = false;
            break;
          } /* switch */
          if (exclusive_arg)
          {
            arg_count++;
            arg++;
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
  if (options & ARG_HELP)
  {
    return print_help_(argv[0]);
  }
  if (argc - arg_count < NUM_MANDATORY_ARGS + 1)
  {
    eprintf("must provide " _xstr(NUM_MANDATORY_ARGS) " mandatory argument(s)");
    return EXIT_FAILURE;
  }
  if (verbose > 0)
  {
    puts(PROJECT_TITLE " " PROJECT_VERSION _nl
         PROJECT_COPYRIGHT _nl PROJECT_LICENSE _nl);
  }

  pdebug("creating qrcode object");
  qrcode_t* qr = NULL;
  int err = create_qrcode(&qr,
    argv[argc - 1], version, level,
    options & ARG_OPTIMIZE,
    verbose > 1
  );
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
      else if (verbose > 1)
      {
        pinfo("Forced mask: %d", mask);
      }
    }
    if (verbose > 0)
    {
      const uint8_t vers = qrcode_version(qr);
      if (vers > INLINE_VERSION_LIMIT && !(options & ARG_NOLIMIT))
      {
        eprintf("could not print inline qrcode, version %uhh too high,"
                "use --nolimit and try again", vers);
      }
      else
      {
        pdebug("printing inline qrcode");
        qrcode_print(qr, options & ARG_RAW);
      }
    }
    if (filename != NULL)
    {
      err = qrcode_output(qr, imgfmt, scale, filename);
      if (err != 0)
      {
        eprintf("could not output qrcode image");
      }
      else if (verbose > 1)
      {
        pinfo("Image saved to: %s", filename);
      }
    }
  }
  pdebug("resources clean-up");
  delete_qrcode(&qr);
  return err;
}
