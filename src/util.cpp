#include "util.hpp"

#ifdef linux
#include <unistd.h>
#include <cstdio>
#endif

bool is_interactive()
{
#ifdef linux
    // On Linux, we consider that the process is interactive when
    // the standard input is not a terminal
    return isatty(fileno(stdin));
#else
    // FIXME (matteodelabre): Make this detection possible on other systems
    return true;
#endif
}
