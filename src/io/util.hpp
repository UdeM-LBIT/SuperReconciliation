#ifndef IO_UTIL_HPP
#define IO_UTIL_HPP

#include <string>

/**
 * Read a whole file and return a string containing all the data.
 *
 * @param path Path of the file to read from, or '-' to read from
 * standard input.
 * @param [prompt=''] Prompt string shown to the user if input is
 * read from standard input and if standard input is a terminal.
 * @return All read data.
 */
std::string read_all_from(
    const std::string&,
    const std::string& = "");

/**
 * Overwrite a file with data.
 *
 * @param path Path of the file to write to, or '-' to write to
 * standard output.
 * @param data Data to write.
 * @param [message=''] Message shown to the user before the data
 * if it is being written to standard output and if standard output
 * is a terminal.
 */
void write_all_to(
    const std::string&,
    const std::string&,
    const std::string& = "");

#endif // IO_UTIL_HPP
