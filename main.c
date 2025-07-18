#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#ifdef _WIN32
#include <stdlib.h>
#endif
#include "quickresponse.h"
#include "shared.h"

#define NUM_ARGS 3
#define NUM_MANDATORY 1

typedef enum targ_e {
  ARG_NONE = 0,
  ARG_SILENT = 1,
  ARG_DEBUG = 2,
  ARG_RAW = 4
} targ_t;

static int
print_help_(const char* cmdln)
{
  fprintf(stderr, "Usage: %s --[silent,debug,raw] <string>\r\n", cmdln);
  return EINVAL;
}

int
main(int argc, char* argv[])
{
  targ_t options = ARG_NONE;
  if (argc < NUM_MANDATORY + 1)
  {
    return print_help_(argv[0]);
  }
  int arg_count = 0;
  size_t i = 1;
  while (argv[i] != NULL)
  {
    if (argv[i][0] == '-')
    {
      const char* args[NUM_ARGS] = {"--silent", "--debug", "--raw"};
      const targ_t arge[NUM_ARGS] = {ARG_SILENT, ARG_DEBUG, ARG_RAW};
      size_t j = 0;
      for (; j < NUM_ARGS; j++)
      {
        if (!strcmp(argv[i], args[j]))
        {
          options |= arge[j];
          arg_count++;
        }
      }
    }
    i++;
  }
  if (argc - arg_count < NUM_MANDATORY + 1)
  {
    return print_help_(argv[0]);
  }

#ifdef _WIN32
  // NOTE: to allow box-drawing characters
  system("chcp 65001>nul");
#endif

  if (!(options & ARG_SILENT))
  {
    puts(PROJECT_TITLE " " PROJECT_VERSION "\r\n"
         PROJECT_COPYRIGHT "\r\n" PROJECT_LICENSE "\r\n");
  }

  qrcode_t* qr = NULL;
  int err = create_qrcode(&qr, argv[argc - 1], options & ARG_DEBUG);
  if (err)
  {
    errno = err;
    perror("\t[^] runtime error");
    return err;
  }
  qrcode_print(qr, options & ARG_RAW);
  delete_qrcode(&qr);
  return 0;
}
