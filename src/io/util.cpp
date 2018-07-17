#include "util.hpp"

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#ifdef linux
#include <unistd.h>
#include <cstdio>
#endif

namespace
{
bool is_input_interactive()
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

bool is_output_interactive()
{
#ifdef linux
    // On Linux, we consider that the process is interactive when
    // the standard input is not a terminal
    return isatty(fileno(stdout));
#else
    // FIXME (matteodelabre): Make this detection possible on other systems
    return true;
#endif
}
}

std::string read_all_from(
    const std::string& path,
    const std::string& prompt)
{
    std::ostringstream result;

    if (path == "-")
    {
        if (is_input_interactive())
        {
            std::cerr << prompt << "\n";
        }

        result << std::cin.rdbuf();
    }
    else
    {
        std::ifstream file{path};
        result << file.rdbuf();
    }

    return result.str();
}

void write_all_to(
    const std::string& path,
    const std::string& data,
    const std::string& message)
{
    if (path == "-")
    {
        if (is_output_interactive())
        {
            std::cerr << message << "\n";
        }

        std::cout << data << "\n";
    }
    else
    {
        std::ofstream file{path};
        file << data << "\n";
    }
}
