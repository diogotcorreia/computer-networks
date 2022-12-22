#include "stream_utils.hpp"

void write_uint32_t(std::ostream &ostream, uint32_t num) {
  // stored as big-endian
  ostream.put((char)((num >> 24) & 0xff));
  ostream.put((char)((num >> 16) & 0xff));
  ostream.put((char)((num >> 8) & 0xff));
  ostream.put((char)(num & 0xff));
}

void write_string(std::ostream &ostream, std::string &str) {
  write_uint32_t(ostream, (uint32_t)str.size());
  for (char c : str) {
    ostream.put(c);
  }
}

void write_bool(std::ostream &ostream, bool b) {
  ostream.put(b);
}

uint32_t read_uint32_t(std::istream &istream) {
  // stored as big-endian
  uint32_t result = istream.get() & 0xff;
  result = (result << 8) | (istream.get() & 0xff);
  result = (result << 8) | (istream.get() & 0xff);
  result = (result << 8) | (istream.get() & 0xff);
  return result;
}

std::string read_string(std::istream &istream) {
  uint32_t size = read_uint32_t(istream);
  std::string result;

  for (uint32_t i = 0; i < size; ++i) {
    result += (char)istream.get();
  }

  return result;
}

bool read_bool(std::istream &istream) {
  return istream.get() != 0;
}
