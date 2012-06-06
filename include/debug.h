#ifndef DEBUG_H
#define DEBUG_H

#ifndef DEBUG_LEVEL
 #define DEBUG_LEVEL 3
#endif

#define DEBUG_ALL 3
#define DEBUG_ERROR 1

#define DEBUG(...) _DEBUG(DEBUG_ALL, __LINE__, __FILE__, __VA_ARGS__)
#define D_ERROR(...) _DEBUG(DEBUG_ERROR, __LINE__, __FILE__, __VA_ARGS__)

void _DEBUG(int level, int line, char *file, const char *format, ...);

#endif
