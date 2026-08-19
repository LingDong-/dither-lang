#ifndef REGX_IMPL
#define REGX_IMPL

#ifdef __cplusplus
extern "C" {
#endif

int regx_impl_test(const char* str, const char* re);
int regx_impl_find(const char* str, const char* re, char**** out_groups, int** out_indices);
char* regx_impl_replace(const char* str, const char* re, const char* rep);

#ifdef __cplusplus
}
#endif

#endif

#ifdef _WIN32
#pragma comment(lib, __FILE__ ".lib")
#endif