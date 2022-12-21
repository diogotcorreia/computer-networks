#ifndef STREAM_UTILS_H
#define STREAM_UTILS_H

#include <istream>
#include <ostream>

void write_uint32_t(std::ostream &ostream, uint32_t num);
void write_string(std::ostream &ostream, std::string &str);

uint32_t read_uint32_t(std::istream &istream);
std::string read_string(std::istream &istream);

#endif
