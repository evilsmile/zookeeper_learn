#ifndef __ZK_UTIL_H___
#define __ZK_UTIL_H__
#include <string>

namespace util {

std::string sha1sum(const std::string& data);
std::string base64_encode(const std::string& strData);
std::string base64_decode(const std::string& strData);

}

#endif
