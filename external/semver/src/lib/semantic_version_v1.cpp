#include "semantic_version.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <sstream>
#include <tuple>
#include <vector>

using namespace std;

//------------------------------------------------------------------------------
// From http://semver.org/ - Version 2.0.0

// Pre-release versions satisfy but have a lower precedence than the associated
// normal version.

// Build versions satisfy and have a higher precedence than the associated
// normal version.

// Precedence MUST be calculated by separating the version into major, minor,
// patch, pre-release, and build identifiers in that order. Major, minor, and
// patch versions are always compared numerically. Pre-release and build version
// precedence MUST be determined by comparing each dot separated identifier as
// follows: identifiers consisting of only digits are compared numerically and
// identifiers with letters or dashes are compared lexically in ASCII sort
// order. Numeric identifiers always have lower precedence than non-numeric
// identifiers. Example: 1.0.0-alpha < 1.0.0-alpha.1 < 1.0.0-beta.2 <
// 1.0.0-beta.11 < 1.0.0-rc.1 < 1.0.0-rc.1+build.1 < 1.0.0 < 1.0.0+0.3.7 <
// 1.3.7+build < 1.3.7+build.2.b8f12d7 < 1.3.7+build.11.e0f985a.

namespace
{
  void SplitDottedString(const string& s, vector<string>& v)
  {
    istringstream ss(s);
    for (string part; getline(ss, part, '.'); v.push_back(part));
  }

  int CompareIdentifiers(const string& s1, const string& s2)
  {
    // We need to split the dotted identifier list into individual identifiers,
    // and treat purely numeric identifiers as the numbers they represent.

    vector<string> v1;
    SplitDottedString(s1, v1);

    vector<string> v2;
    SplitDottedString(s2, v2);

    for (size_t i = 0; ; ++i)
    {
      // exhausted both vectors: they must be equal
      if (i >= v1.size() && i >= v2.size())
      {
        return 0;
      }

      // exhausted one vector: it's the smaller one
      if (i >= v1.size())
      {
        return -1;
      }
      if (i >= v2.size())
      {
        return 1;
      }

      // is either of v1[i] or v2[i] a number?
      const string& id1 = v1[i];
      bool id1IsNumber = all_of(id1.cbegin(), id1.cend(),
                                [] (char c) { return isdigit(c); });
      const string& id2 = v2[i];
      bool id2IsNumber = all_of(id2.cbegin(), id2.cend(),
                                [] (char c) { return isdigit(c); });

      // if both numbers - compare them as such
      if (id1IsNumber && id2IsNumber)
      {
        long num1 = atol(id1.c_str());
        long num2 = atol(id2.c_str());
        if (num1 - num2 != 0)
        {
          return num1 - num2;
        }
        else
        {
          continue;
        }
      }
      // if one is a number - that one is lesser
      if (id1IsNumber)
      {
        return -1;
      }
      if (id2IsNumber)
      {
        return 1;
      }
      // neither are numbers: compare them
      int c = id1.compare(id2);
      if (c != 0)
      {
        return c;
      }
    }
  }

  bool IdentifierIsValid(const string& i)
  {
    vector<string> v;
    SplitDottedString(i, v);

    for (const auto& s : v)
    {
      // Identifiers must not be empty.
      if (s.empty())
      {
        return false;
      }

      // Identifiers must contain only alphanumerics and '-'.
      if (any_of(s.cbegin(), s.cend(),
                 [] (char c) { return !isalnum(c) && c != '-'; }))
      {
        return false;
      }

      // Numeric identifiers must not contain leading zeroes.
      bool numeric = all_of(s.cbegin(), s.cend(),
                            [] (char c) { return isdigit(c); });
      if (numeric && s[0] == '0')
      {
        return false;
      }
    }

    return true;
  }
}

namespace semver {
  namespace v1 {

//------------------------------------------------------------------------------
Version::Version(
    unsigned int major,
    unsigned int minor,
    unsigned int patch,
    const std::string& prerelease,
    const std::string& build)
  : m_majorVersion(major)
  , m_minorVersion(minor)
  , m_patchVersion(patch)
  , m_prereleaseVersion(prerelease)
  , m_buildVersion(build)
{
}

Version::Version(const string& s)
{
  // major.minor.patch-release+build

  istringstream ss(s);
  string part;

  if (!getline(ss, part, '.')) return;
  m_majorVersion = static_cast<unsigned int>(strtoul(part.c_str(), 0, 0));
  if (!getline(ss, part, '.')) return;
  m_minorVersion = static_cast<unsigned int>(strtoul(part.c_str(), 0, 0));
  if (!getline(ss, part, '-')) return;
  m_patchVersion = static_cast<unsigned int>(strtoul(part.c_str(), 0, 0));

  if (!getline(ss, m_prereleaseVersion, '+')) return;
  getline(ss, m_buildVersion, '\0');
}

//------------------------------------------------------------------------------
bool Version::IsWellFormed() const
{
  return IdentifierIsValid(m_prereleaseVersion)
    && IdentifierIsValid(m_buildVersion);
}

//------------------------------------------------------------------------------
// When incrementing versions, all lower version parts are reset.

Version Version::NextMajorVersion() const
{
  return Version(m_majorVersion + 1, 0, 0);
}

Version Version::NextMinorVersion() const
{
  return Version(m_majorVersion, m_minorVersion + 1, 0);
}

Version Version::NextPatchVersion() const
{
  return Version(m_majorVersion, m_minorVersion, m_patchVersion + 1);
}


//------------------------------------------------------------------------------
bool Version::Satisfies(const Version& other) const
{
  return tie(m_majorVersion, m_minorVersion, m_patchVersion) >=
    tie(other.m_majorVersion, other.m_minorVersion, other.m_patchVersion);
}

//------------------------------------------------------------------------------
bool operator==(const Version& a, const Version& b)
{
  return
    tie(a.m_majorVersion, a.m_minorVersion, a.m_patchVersion,
        a.m_prereleaseVersion, a.m_buildVersion) ==
    tie(b.m_majorVersion, b.m_minorVersion, b.m_patchVersion,
        b.m_prereleaseVersion, b.m_buildVersion);
}

//------------------------------------------------------------------------------
bool operator<(const Version& a, const Version& b)
{
  if (tie(a.m_majorVersion, a.m_minorVersion, a.m_patchVersion) <
      tie(b.m_majorVersion, b.m_minorVersion, b.m_patchVersion))
  {
    return true;
  }

  // pre-release version < normal version
  if (!a.m_prereleaseVersion.empty() && b.m_prereleaseVersion.empty())
  {
    return true;
  }
  if (a.m_prereleaseVersion.empty() && !b.m_prereleaseVersion.empty())
  {
    return false;
  }

  int prComp = CompareIdentifiers(a.m_prereleaseVersion, b.m_prereleaseVersion);
  if (prComp < 0)
  {
    return true;
  }
  else if (prComp == 0)
  {
    // build version > normal version
    if (a.m_buildVersion.empty() && !b.m_buildVersion.empty())
    {
      return true;
    }
    if (!a.m_buildVersion.empty() && b.m_buildVersion.empty())
    {
      return false;
    }

    int bComp = CompareIdentifiers(a.m_buildVersion, b.m_buildVersion);
    if (bComp < 0)
    {
      return true;
    }
  }
  return false;
}

//------------------------------------------------------------------------------
ostream& operator<<(ostream& s, const Version& v)
{
  s << v.m_majorVersion
    << '.' << v.m_minorVersion
    << '.' << v.m_patchVersion;

  if (!v.m_prereleaseVersion.empty())
  {
    s << '-' << v.m_prereleaseVersion;
  }
  if (!v.m_buildVersion.empty())
  {
    s << '+' << v.m_buildVersion;
  }

  return s;
}

  }
}
