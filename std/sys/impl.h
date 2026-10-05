#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if defined(_WIN32)
#include <windows.h>

void impl_platform(char* p){
  sprintf(p, "Windows");
  typedef LONG (WINAPI *RtlGetVersion_t)(OSVERSIONINFOW *);
  RtlGetVersion_t fn = (RtlGetVersion_t)GetProcAddress(
    GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion");

  OSVERSIONINFOW vi;
  memset(&vi, 0, sizeof vi);
  vi.dwOSVersionInfoSize = sizeof vi;

  if (fn && fn(&vi) == 0) {
    DWORD major = vi.dwMajorVersion;
    // if (major == 10 && vi.dwBuildNumber >= 22000) major = 11;
    sprintf(p + strlen(p), "/%lu.%lu.%lu",
      (unsigned long)major,
      (unsigned long)vi.dwMinorVersion,
      (unsigned long)vi.dwBuildNumber);
  }

  SYSTEM_INFO si;
  GetNativeSystemInfo(&si);
  const char *arch;
  switch (si.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_AMD64: arch = " (x64)";   break;
    case PROCESSOR_ARCHITECTURE_INTEL: arch = " (x86)";   break;
    case PROCESSOR_ARCHITECTURE_ARM64: arch = " (arm64)"; break;
    case PROCESSOR_ARCHITECTURE_ARM:   arch = " (arm)";   break;
    default:                           arch = "";        break;
  }
  sprintf(p + strlen(p), "%s", arch);
}

#else
#include <sys/utsname.h>

void impl_platform(char *p){
  struct utsname u;
  if (uname(&u) != 0) {
    sprintf(p, "POSIX");
    return;
  }
  sprintf(p, "%s/%s (%s)", u.sysname, u.release, u.machine);
}
#endif


#ifdef _WIN32
#define popen  _popen
#define pclose _pclose
#endif

#define STDOUT_INHERIT 0
#define STDOUT_PIPE    1
#define STDOUT_IGNORE  2
#define STDOUT_MASK    3


char* sys_impl_command(const char *cmd, int flags, int* out_len, int* exit_code){
  *out_len = 0;
  int mode = flags & STDOUT_MASK;
  if (mode == STDOUT_INHERIT) {
    fflush(stdout);
    *exit_code = system(cmd);
    return NULL;
  }
  FILE *fp = popen(cmd, 
    "r"
#ifdef _WIN32
    "b"
#endif
  );
  if (mode == STDOUT_IGNORE) {
    char scratch[4096];
    while (fread(scratch, 1, sizeof scratch, fp) > 0){}
    *exit_code = pclose(fp);
    return NULL;
  }
  size_t cap = 4096;
  size_t len = 0;
  char *buf = malloc(cap);
  while (1) {
    if (cap - len < 1024) {
      size_t ncap = cap * 2;
      char *nbuf = realloc(buf, ncap);
      buf = nbuf;
      cap = ncap;
    }
    size_t n = fread(buf + len, 1, cap - len - 1, fp);
    if (n == 0) break;
    len += n;
  }
  buf[len] = '\0';
  *exit_code = pclose(fp);
  *out_len = len;
  char *tight = realloc(buf, len + 1);
  return tight ? tight : buf;
}
