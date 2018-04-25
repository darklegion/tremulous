#pragma once

// FFI for semantic version

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct Semver
{
  unsigned int major;
  unsigned int minor;
  unsigned int patch;
  char prerelease[32];
  char build[32];
} semver_t;

int satisfies(const semver_t* a, const semver_t* b);
int lessThan(const semver_t* a, const semver_t* b);
void nextMajor(const semver_t* a, semver_t* b);
void nextMinor(const semver_t* a, semver_t* b);
void nextPatch(const semver_t* a, semver_t* b);

#ifdef __cplusplus
}
#endif
