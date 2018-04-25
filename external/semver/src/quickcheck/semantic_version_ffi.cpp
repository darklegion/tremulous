#include "semantic_version_ffi.h"
#include "semantic_version.h"

using namespace semver;

//------------------------------------------------------------------------------

int satisfies(const semver_t* a, const semver_t* b)
{
  Version va(a->major, a->minor, a->patch,
             a->prerelease, a->build);

  Version vb(b->major, b->minor, b->patch,
             b->prerelease, b->build);

  return va.Satisfies(vb) ? 1 : 0;
}

int lessThan(const semver_t* a, const semver_t* b)
{
  Version va(a->major, a->minor, a->patch,
             a->prerelease, a->build);

  Version vb(b->major, b->minor, b->patch,
             b->prerelease, b->build);

  return (va < vb) ? 1 : 0;
}

void nextMajor(const semver_t* a, semver_t* b)
{
  Version va(a->major, a->minor, a->patch,
             a->prerelease, a->build);
  Version vb = va.NextMajorVersion();
  b->major = vb.GetMajorVersion();
  b->minor = vb.GetMinorVersion();
  b->patch = vb.GetPatchVersion();
}

void nextMinor(const semver_t* a, semver_t* b)
{
  Version va(a->major, a->minor, a->patch,
             a->prerelease, a->build);
  Version vb = va.NextMinorVersion();
  b->major = vb.GetMajorVersion();
  b->minor = vb.GetMinorVersion();
  b->patch = vb.GetPatchVersion();
}

void nextPatch(const semver_t* a, semver_t* b)
{
  Version va(a->major, a->minor, a->patch,
             a->prerelease, a->build);
  Version vb = va.NextPatchVersion();
  b->major = vb.GetMajorVersion();
  b->minor = vb.GetMinorVersion();
  b->patch = vb.GetPatchVersion();
}
