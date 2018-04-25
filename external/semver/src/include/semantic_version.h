#pragma once

// Semantic version - see http://semver.org/

// The major difference between v1::Version and v2::Version is that v1 includes
// the build identifier when calculating precedence of versions (see
// http://semver.org/spec/v2.0.0-rc.1.html points 10,11) while v2 omits the
// build identifier when calculating precedence of versions (see
// http://semver.org/spec/v2.0.0.html points 10,11).

#include <string>
#include <ostream>

namespace semver
{
  namespace v1
  {

    class Version
    {
    public:
      // By-parts (and default) version constructor: no validation.
      Version(
          unsigned int major = 0,
          unsigned int minor = 0,
          unsigned int patch = 1,
          const std::string& prerelease = std::string{},
          const std::string& build = std::string{});

      // Parse from a version string: no validation.
      explicit Version(const std::string& s);

      // Is a version well-formed according to the spec?
      // The semver spec stipulates a few properties of a well-formed version:
      // that both the dot-separated prerelease version and the build version
      // MUST comprise only [0-9A-Za-z-]; that numerical dot-separated
      // identifiers MUST NOT include leading zeroes; and that such identifiers
      // MUST NOT be empty.
      bool IsWellFormed() const;

      // Parse from a version string, with validation according to the spec.
      static Version Parse(const std::string& s, bool& wellformed);

      unsigned int GetMajorVersion() const { return m_majorVersion; }
      unsigned int GetMinorVersion() const { return m_minorVersion; }
      unsigned int GetPatchVersion() const { return m_patchVersion; }

      // Create a new version by incrementing a part of a version. Other parts
      // will be reset to 0, per the spec.
      Version NextMajorVersion() const;
      Version NextMinorVersion() const;
      Version NextPatchVersion() const;

      // A version satisfies another version if it is greater than or equal to
      // it in precedence. Additionally, pre-release versions and build versions
      // both satisfy their corresponding normal versions, and all pre-release
      // or build versions thereof.
      bool Satisfies(const Version& other) const;

      // Precedence is calculated by comparing the major, minor, patch,
      // pre-release and build parts in that order. Pre-release and build
      // strings are considered as lists of dot-separated identifiers, whereby
      // if an identifier is numeric, it is compared as such; otherwise
      // comparison is based on ASCII sort order.
      friend bool operator<(const Version& a, const Version& b);
      friend bool operator==(const Version& a, const Version& b);

      // A version is output as X.Y.Z (Major.Minor.Patch)
      // A pre-release string (eg alpha.1) may follow immediately, joined by a -
      // A build string may follow (eg 123.f83eaa931), joined by a +
      friend std::ostream& operator<<(std::ostream& s, const Version& v);

    private:
      unsigned int m_majorVersion = 0;
      unsigned int m_minorVersion = 0;
      unsigned int m_patchVersion = 1;
      std::string m_prereleaseVersion;
      std::string m_buildVersion;
    };

    inline bool operator!=(const Version& a, const Version& b)
    {
      return !(a == b);
    }

    inline bool operator>=(const Version& a, const Version& b)
    {
      return !(a < b);
    }

    inline bool operator<=(const Version& a, const Version& b)
    {
      return (a == b) || (a < b);
    }

    inline bool operator>(const Version& a, const Version& b)
    {
      return (a != b) && (a >= b);
    }
  }

  inline namespace v2
  {

    class Version
    {
    public:
      // By-parts (and default) version constructor: no validation.
      Version(
          unsigned int major = 0,
          unsigned int minor = 0,
          unsigned int patch = 1,
          const std::string& prerelease = std::string{},
          const std::string& build = std::string{});

      // Parse from a version string: no validation.
      explicit Version(const std::string& s);

      // Is a version well-formed according to the spec?
      // The semver spec stipulates a few properties of a well-formed version:
      // that both the dot-separated prerelease version and the build version
      // MUST comprise only [0-9A-Za-z-]; that numerical dot-separated
      // identifiers MUST NOT include leading zeroes; and that such identifiers
      // MUST NOT be empty.
      bool IsWellFormed() const;

      unsigned int GetMajorVersion() const { return m_majorVersion; }
      unsigned int GetMinorVersion() const { return m_minorVersion; }
      unsigned int GetPatchVersion() const { return m_patchVersion; }

      // Create a new version by incrementing a part of a version. Other parts
      // will be reset to 0, per the spec.
      Version NextMajorVersion() const;
      Version NextMinorVersion() const;
      Version NextPatchVersion() const;

      // A version satisfies another version if it is greater than or equal to
      // it in precedence. Additionally, pre-release versions and build versions
      // both satisfy their corresponding normal versions, and all pre-release
      // or build versions thereof.
      bool Satisfies(const Version& other) const;

      // Precedence is calculated by comparing the major, minor, patch, and
      // pre-release parts in that order. Pre-release strings are considered as
      // lists of dot-separated identifiers, whereby if an identifier is
      // numeric, it is compared as such; otherwise comparison is based on ASCII
      // sort order. Note: according to semver v2.0.0, build version is NOT
      // considered when determining precedence.
      friend bool operator<(const Version& a, const Version& b);
      friend bool operator==(const Version& a, const Version& b);

      // For exact equality (as opposed to precedence equality)
      bool Equals(const Version& other) const;

      // A version is output as X.Y.Z (Major.Minor.Patch)
      // A pre-release string (eg alpha.1) may follow immediately, joined by a -
      // A build string may follow (eg 123.f83eaa931), joined by a +
      friend std::ostream& operator<<(std::ostream& s, const Version& v);

    private:
      unsigned int m_majorVersion = 0;
      unsigned int m_minorVersion = 0;
      unsigned int m_patchVersion = 1;
      std::string m_prereleaseVersion;
      std::string m_buildVersion;
    };

    inline bool operator!=(const Version& a, const Version& b)
    {
      return !(a == b);
    }

    inline bool operator>=(const Version& a, const Version& b)
    {
      return !(a < b);
    }

    inline bool operator<=(const Version& a, const Version& b)
    {
      return (a == b) || (a < b);
    }

    inline bool operator>(const Version& a, const Version& b)
    {
      return (a != b) && (a >= b);
    }
  }
}

