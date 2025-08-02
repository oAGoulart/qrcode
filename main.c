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
  ARG_VNUM = 1 << __COUNTER__
} targ_t;
static const uint16_t numargs_ = __COUNTER__;

static int
print_help_(const char* cmdln)
{
  fprintf(stderr,
    "Usage: %s [OPTIONS] <string>\r\n"
    "OPTIONS:\r\n"
    "\t--nocopy    do not print copyright header\r\n"
    "\t--verbose   print runtime information for generated values\r\n"
    "\t--raw       print generated matrix as 1's and 0's (no Unicode)\r\n"
    "\t--mask <N>  force output of N mask, regardless of penalty; N:(0-7)\r\n"
    "\t--vnum <N>  tries to force use of N version QR Codes; N:(1-%d)\r\n",
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
  int i = 1;
  for (; i < argc; i++)
  {
    if (argv[i][0] == '-')
    {
      const char* args[] = {
        "--nocopy", "--verbose", "--raw", "--mask", "--vnum"
      };
      const targ_t arge[] = {
        ARG_NOCOPY, ARG_VERBOSE, ARG_RAW, ARG_MASK, ARG_VNUM
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
          uint8_t in8 = (uint8_t)atoi(argv[i + 1]);
          if (arge[j] == ARG_MASK)
          {
            mask = in8;
          }
          else
          {
            vnum = in8 - 1;
          }
          argcount++;
          i++;
          break;
        }
      }
    }
  }
  if (argc - argcount < NUM_MANDATORY + 1)
  {
    return print_help_(argv[0]);
  }
  if (!(options & ARG_NOCOPY))
  {
    puts(PROJECT_TITLE " " PROJECT_VERSION "\r\n"
         PROJECT_COPYRIGHT "\r\n" PROJECT_LICENSE "\r\n");
  }
  qrcode_t* qr = NULL;
  int err = create_qrcode(&qr, argv[argc - 1], options & ARG_VERBOSE, vnum);
  if (err)
  {
    errno = err;
    perror("\t[^] runtime error");
    return err;
  }
  if (options & ARG_VERBOSE && options & ARG_MASK)
  {
    printf("(INFO) Forced mask: %d\r\n", mask);
  }
  qrcode_print(qr, options & ARG_RAW, mask);
  delete_qrcode(&qr);
  return 0;
}
