#include <stdio.h>
#include <string.h>

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
