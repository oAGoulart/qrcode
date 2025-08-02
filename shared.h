#ifndef SHARED_H
#define SHARED_H 1

#define PROJECT_TITLE "Command-line QR Code generator"
#define PROJECT_VERSION "(v1.2.2)"
#define PROJECT_COPYRIGHT "Copyright (C) 2025 Augusto Goulart."
#define PROJECT_LICENSE \
  "Licensed under Microsoft Reciprocal License (Ms-RL).\r\n" \
  "QR Code is a registered trademark of DENSO WAVE\r\n" \
  "INCORPORATED in Japan and in other countries."

#ifdef TRUE
#undef TRUE
#endif
#define TRUE 1
#ifdef FALSE
#undef FALSE
#endif
#define FALSE 0

#define MAX_VERSION 5

#endif
