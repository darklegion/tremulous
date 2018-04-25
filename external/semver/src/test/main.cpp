#include <semantic_version.h>

using namespace semver;

#define TESTINATOR_MAIN
#include <testinator.h>

#include <algorithm>
#include <sstream>

using namespace std;

DEF_TEST(Init, SemanticVersion)
{
  Version v;
  return v.GetMajorVersion() == 0
    && v.GetMinorVersion() == 0
    && v.GetPatchVersion() == 1;
}

DEF_TEST(NextMajor, SemanticVersion)
{
  Version v1(1, 2, 3);
  Version v2(v1.NextMajorVersion());
  return v2.GetMajorVersion() == 2
    && v2.GetMinorVersion() == 0
    && v2.GetPatchVersion() == 0;
}

DEF_TEST(NextMinor, SemanticVersion)
{
  Version v1(1, 2, 3);
  Version v2(v1.NextMinorVersion());
  return v2.GetMajorVersion() == 1
    && v2.GetMinorVersion() == 3
    && v2.GetPatchVersion() == 0;
}

DEF_TEST(NextPatch, SemanticVersion)
{
  Version v1(1, 2, 3);
  Version v2(v1.NextPatchVersion());
  return v2.GetMajorVersion() == 1
    && v2.GetMinorVersion() == 2
    && v2.GetPatchVersion() == 4;
}

DEF_TEST(LessThanMajor, SemanticVersion)
{
  Version v1(1, 2, 3);
  Version v2(v1.NextMajorVersion());
  return v1 < v2;
}

DEF_TEST(LessThanMinor, SemanticVersion)
{
  Version v1(1, 2, 3);
  Version v2(v1.NextMinorVersion());
  return v1 < v2;
}

DEF_TEST(LessThanPatch, SemanticVersion)
{
  Version v1(1, 2, 3);
  Version v2(v1.NextPatchVersion());
  return v1 < v2;
}

DEF_TEST(LessThanNumericString, SemanticVersion)
{
  Version v1(1, 2, 3, "alpha.2");
  Version v2(1, 2, 3, "alpha.11");
  return v1 < v2;
}

DEF_TEST(LessThanPrerelease, SemanticVersion)
{
  Version v1(1, 2, 3, "alpha");
  Version v2(1, 2, 3, "beta");
  return v1 < v2;
}

DEF_TEST(LessThanBuild, SemanticVersion)
{
  Version v1(1, 2, 3, "", "1");
  Version v2(1, 2, 3, "", "2");
  return v1 < v2;
}

DEF_TEST(PrereleaseLessThanNormal, SemanticVersion)
{
  Version v1(1, 2, 3, "alpha");
  Version v2(1, 2, 3);
  return v1 < v2;
}

DEF_TEST(NormalLessThanBuild, SemanticVersion)
{
  Version v1(1, 2, 3);
  Version v2(1, 2, 3, "", "1");
  return v1 < v2;
}

DEF_TEST(PrereleaseLessThanBuild, SemanticVersion)
{
  Version v1(1, 2, 3, "alpha");
  Version v2(1, 2, 3, "", "1234");
  return v1 < v2;
}

DEF_TEST(Output, SemanticVersion)
{
  Version v(1, 2, 3, "alpha.2", "1234");
  ostringstream s;
  s << v;
  return s.str() == "1.2.3-alpha.2+1234";
}

DEF_TEST(SameSatisfy, SemanticVersion)
{
  Version v1(1, 2, 3);
  Version v2(1, 2, 3);
  return v1.Satisfies(v2);
}

DEF_TEST(GreaterMajorSatisfy, SemanticVersion)
{
  Version v1(2, 0, 0);
  Version v2(1, 2, 3);
  return v1.Satisfies(v2);
}

DEF_TEST(GreaterMinorSatisfy, SemanticVersion)
{
  Version v1(1, 3, 0);
  Version v2(1, 2, 3);
  return v1.Satisfies(v2);
}

DEF_TEST(GreaterPatchSatisfy, SemanticVersion)
{
  Version v1(1, 2, 4);
  Version v2(1, 2, 3);
  return v1.Satisfies(v2);
}

DEF_TEST(PrereleaseSatisfy, SemanticVersion)
{
  Version v1(1, 2, 3, "alpha");
  Version v2(1, 2, 3);
  return v1.Satisfies(v2);
}

DEF_TEST(BuildSatisfy, SemanticVersion)
{
  Version v1(1, 2, 3, "", "1234");
  Version v2(1, 2, 3);
  return v1.Satisfies(v2);
}

const vector<semver::v1::Version> s_precedences_v1 =
{
  semver::v1::Version(1, 0, 0, "alpha"),
  semver::v1::Version(1, 0, 0, "alpha.1"),
  semver::v1::Version(1, 0, 0, "alpha.beta"),
  semver::v1::Version(1, 0, 0, "beta"),
  semver::v1::Version(1, 0, 0, "beta.2"),
  semver::v1::Version(1, 0, 0, "beta.11"),
  semver::v1::Version(1, 0, 0, "rc.1"),
  semver::v1::Version(1, 0, 0, "rc.1", "build.1"),
  semver::v1::Version(1, 0, 0),
  semver::v1::Version(1, 0, 0, "", "0.3.7"),
  semver::v1::Version(1, 0, 0, "", "build"),
  semver::v1::Version(1, 0, 0, "", "build.2.b8f12d7"),
  semver::v1::Version(1, 0, 0, "", "build.11.e0f985a")
};

DEF_TEST(Precedences_v1, SemanticVersion)
{
  return adjacent_find(
      s_precedences_v1.cbegin(), s_precedences_v1.cend(),
      [] (const auto& a, const auto& b) { return a >= b; })
    == s_precedences_v1.end();
}

DEF_TEST(Satisfies_v1, SemanticVersion)
{
  return adjacent_find(
      s_precedences_v1.cbegin(), s_precedences_v1.cend(),
      [] (const auto& a, const auto& b) { return !b.Satisfies(a) ; })
    == s_precedences_v1.end();
}

const vector<semver::v2::Version> s_precedences_v2 =
{
  semver::v2::Version(1, 0, 0, "alpha"),
  semver::v2::Version(1, 0, 0, "alpha.1"),
  semver::v2::Version(1, 0, 0, "alpha.beta"),
  semver::v2::Version(1, 0, 0, "beta"),
  semver::v2::Version(1, 0, 0, "beta.2"),
  semver::v2::Version(1, 0, 0, "beta.11"),
  semver::v2::Version(1, 0, 0, "rc.1"),
  semver::v2::Version(1, 0, 0)
};

DEF_TEST(Precedences_v2, SemanticVersion)
{
  return adjacent_find(
      s_precedences_v2.cbegin(), s_precedences_v2.cend(),
      [] (const auto& a, const auto& b) { return a >= b; })
    == s_precedences_v2.end();
}

DEF_TEST(Satisfies_v2, SemanticVersion)
{
  return adjacent_find(
      s_precedences_v2.cbegin(), s_precedences_v2.cend(),
      [] (const auto& a, const auto& b) { return !b.Satisfies(a) ; })
    == s_precedences_v2.end();
}

DEF_TEST(PrecedenceIncludesBuild_v1, SemanticVersion)
{
  semver::v1::Version a(1,0,0, "", "1");
  semver::v1::Version b(1,0,0, "", "2");
  return a < b;
}

DEF_TEST(PrecedenceOmitsBuild_v2, SemanticVersion)
{
  semver::v2::Version a(1,0,0, "", "1");
  semver::v2::Version b(1,0,0, "", "2");
  return a == b;
}

DEF_TEST(ConstructFromString, SemanticVersion)
{
  Version v("1.2.3-alpha.2+build.1234");
  ostringstream s;
  s << v;
  return s.str() == "1.2.3-alpha.2+build.1234";
}

DEF_TEST(WellFormed, SemanticVersion)
{
  Version v("1.2.3-alpha.2+build.1234");
  return v.IsWellFormed();
}

DEF_TEST(ParseIllFormed, SemanticVersion)
{
  Version v("1.2.3-alpha-2+build+1234");
  return !v.IsWellFormed();
}

namespace testinator
{
  template <>
  struct Arbitrary<semver::v2::Version>
  {
    using Version = semver::v2::Version;

    static Version generate(std::size_t g, unsigned long int s)
    {
      // prerelease part shouldn't contain a +
      string pre = Arbitrary<string>::generate(g, s);
      replace(pre.begin(), pre.end(), '+', '_');

      return Version {
          Arbitrary<unsigned int>::generate(g, s),
          Arbitrary<unsigned int>::generate(g, s>>1),
          Arbitrary<unsigned int>::generate(g, s>>2),
          pre,
          Arbitrary<string>::generate(g, s>>1)
          };
    }
    static vector<Version> shrink(const Version&)
    {
      return vector<Version>{};
    }
  };
}

DEF_PROPERTY(Roundtrip, SemanticVersion, const semver::v2::Version& before)
{
  ostringstream s;
  s << before;
  Version after(s.str());
  return after.Equals(before);
}

DEF_TEST(LessThan_Issue2, SemanticVersion)
{
  Version a("4.5.6");
  Version b("1.2.3");
  return !(a < b);
}
