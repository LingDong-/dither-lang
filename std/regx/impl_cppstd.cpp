#include "impl_cppstd.h"
#include <regex>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>

namespace {
char* dup_cstr(const std::string& s) {
  char* p = static_cast<char*>(std::malloc(s.size() + 1));
  if (p) std::memcpy(p, s.c_str(), s.size() + 1);
  return p;
}
}

extern "C" {
int regx_impl_test(const char* str, const char* re) {
  if (!str || !re) return -1;
  try {
    std::regex rx(re, std::regex::ECMAScript);
    return std::regex_match(std::string(str), rx) ? 1 : 0;
  } catch (const std::regex_error&) {
    return -1;
  }
}
int regx_impl_find(const char* str, const char* re, char**** out_groups, int** out_indices) {
  if (!str || !re || !out_groups || !out_indices) return -1;
  *out_groups = nullptr;
  *out_indices = nullptr;
  try {
    std::regex rx(re, std::regex::ECMAScript);
    std::string s(str);
    std::vector<char**> matches;
    std::vector<int>    indices;
    auto begin = std::sregex_iterator(s.begin(), s.end(), rx);
    auto end   = std::sregex_iterator();

    for (auto it = begin; it != end; ++it) {
      const std::smatch& m = *it;
      char** groups = static_cast<char**>(
      std::malloc((m.size() + 1) * sizeof(char*)));
      if (!groups) throw std::bad_alloc();
      for (size_t g = 0; g < m.size(); ++g) {
        groups[g] = m[g].matched ? dup_cstr(m[g].str())
                                   : dup_cstr(std::string());
      }
      groups[m.size()] = nullptr;
      matches.push_back(groups);
      indices.push_back(static_cast<int>(m.position(0)));
    }
    int count = static_cast<int>(matches.size());
    if (count == 0) return 0;
    char*** grp_arr = static_cast<char***>(std::malloc(count * sizeof(char**)));
    int*    idx_arr = static_cast<int*>(std::malloc(count * sizeof(int)));
    if (!grp_arr || !idx_arr) throw std::bad_alloc();
    for (int i = 0; i < count; ++i) {
      grp_arr[i] = matches[i];
      idx_arr[i] = indices[i];
    }
    *out_groups  = grp_arr;
    *out_indices = idx_arr;
    return count;
  } catch (const std::regex_error&) {
    return -1;
  } catch (const std::bad_alloc&) {
    return -1;
  }
}

char* regx_impl_replace(const char* str, const char* re, const char* rep) {
  if (!str || !re || !rep) return nullptr;
  try {
    std::regex rx(re, std::regex::ECMAScript);
    std::string result = std::regex_replace(std::string(str), rx,
                                              std::string(rep));
    return dup_cstr(result);
  } catch (const std::regex_error&) {
    return nullptr;
  }
}

}