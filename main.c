#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#ifdef _WIN32
#include <stdlib.h>
#endif
#include "quickresponse.h"
#include "shared.h"

typedef enum targ_e {
  ARG_NONE = 0,
  ARG_SILENT = 1
} targ_t;

int
main(int argc, char* argv[])
{
  targ_t options = ARG_NONE;
  if (argc < 2)
  {
    fputs("Usage: qrcode [--silent] <string>", stderr);
    return EINVAL;
  }
  size_t i = 1;
  while (argv[i] != NULL)
  {
    if (argv[i][0] == '-')
    {
      if (!strcmp(argv[i], "--silent"))
      {
        options |= ARG_SILENT;
      }
    }
    i++;
  }

#ifdef _WIN32
  // NOTE: to allow box-drawing characters
  system("chcp 65001>nul");
#endif

  if (!(options & ARG_SILENT))
  {
    puts(PROJECT_TITLE " " PROJECT_VERSION "\r\n"
         PROJECT_COPYRIGHT "\r\n" PROJECT_LICENSE);
  }

  char* str = argv[argc - 1];
  qrcode_t* qr = NULL;
  int err = create_qrcode(&qr, str);
  if (err)
  {
    fprintf(stderr, "Runtime error: %i\r\n", err);
    return err;
  }
  qrcode_print(qr);
  delete_qrcode(&qr);
  return 0;
}
