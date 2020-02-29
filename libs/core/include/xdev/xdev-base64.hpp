#pragma once

#include <string>

namespace xdev::base64 {

static const std::string _chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

static inline bool is_base64(uint8_t c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string encode(uint8_t const* bytes_to_encode, unsigned int in_len);

template <typename IteratorT>
std::string encode(IteratorT begin, IteratorT end) {
    return encode(begin, std::distance(begin, end));
}
std::string decode(std::string const& encoded_string);

}
