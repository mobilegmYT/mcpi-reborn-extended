#pragma once

#include <stdio.h>
#include <stdlib.h>

// Used coloured terminal output for client only
#ifdef MCPI_SERVER_MODE

// Server Mode
#define INFO(format, ...) { fprintf(stderr, "[INFO]: " format "\n", ##__VA_ARGS__); }
#define WARN(format, ...) { fprintf(stderr, "[WARN]: " format "\n", ##__VA_ARGS__); }
#define DEBUG(format, ...) { const char *debug = getenv("MCPI_DEBUG"); if (debug != NULL && strlen(debug) > 0) { fprintf(stderr, "[DEBUG]: " format "\n", ##__VA_ARGS__); } }
#define ERR(format, ...) { fprintf(stderr, "[ERR]: (%s:%i): " format "\n", __FILE__, __LINE__, ##__VA_ARGS__); exit(EXIT_FAILURE); }
#define IMPOSSIBLE() ERR("This Should Never Be Called")
#else

// Client Mode
#define INFO(format, ...) { fprintf(stderr, "\033[96m[INFO]:\x1b[0m " format "\n", ##__VA_ARGS__); }
#define WARN(format, ...) { fprintf(stderr, "\x1b[33m[WARN]:\x1b[0m " format "\n", ##__VA_ARGS__); }
#define DEBUG(format, ...) { const char *debug = getenv("MCPI_DEBUG"); if (debug != NULL && strlen(debug) > 0) { fprintf(stderr, "\x1b[32m[DEBUG]: \x1b[0m" format "\n", ##__VA_ARGS__); } }
#define ERR(format, ...) { fprintf(stderr, "\x1b[33m[ERR]: (%s:%i):\x1b[0m " format "\n", __FILE__, __LINE__, ##__VA_ARGS__); exit(EXIT_FAILURE); }
#define IMPOSSIBLE() ERR("\x1b[41mWe've reached an unreachable state. Anything is possible. The limits were in our heads all along. Follow your dreams.\x1b[0m ");
#endif
