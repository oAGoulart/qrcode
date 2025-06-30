#include <stdio.h>
#include <errno.h>
#ifdef _WIN32
#include <stdlib.h>
#endif
#include "quickresponse.h"

int
main(int argc, char* argv[])
{
  if (argc < 2) {
    fprintf(stderr, "Usage: qc <string>\r\n");
    return EINVAL;
  }

#ifdef _WIN32
  // NOTE: to allow box-drawing characters
  system("chcp 65001>nul");
#endif

  qrcode_t* qr = NULL;
  int err = create_qrcode(&qr, argv[1]);
  if (err)
  {
    fprintf(stderr, "Runtime error: %i\r\n", err);
    return err;
  }
  qrcode_print(qr);
  delete_qrcode(&qr);
  return 0;
}
