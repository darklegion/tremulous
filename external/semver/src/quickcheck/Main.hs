module Main where

import Semver
import Test.QuickCheck

import System.Environment
import System.Console.GetOpt

-- command line options
data Options = Options { optVerbose :: Bool
                       , optNumTests :: Int
                       } deriving Show

defaultOptions :: Options
defaultOptions = Options { optVerbose = False
                         , optNumTests = 100
                         }

options :: [OptDescr (Options -> Options)]
options =
  [ Option "v" ["verbose"] (NoArg (\opts -> opts { optVerbose = True }))
    "run tests with verbose output"

  , Option "n" ["numtests"] (ReqArg
                             (\d opts -> opts { optNumTests = read d })
                          "<num>")
    "number of tests to run"
  ]

parseOpts :: [String] -> IO (Options, [String])
parseOpts argv =
  case getOpt Permute options argv of
    (o, n, []) -> return (foldl (flip id) defaultOpts o, n)
      where defaultOpts = defaultOptions
    (_, _, errs) -> ioError (userError (concat errs ++ usageInfo header options))
      where header = "Usage: test [options]"

-- generate arbitrary Senvers
instance Arbitrary Semver where
  arbitrary = do
    r1 <- arbitrary
    r2 <- arbitrary
    r3 <- arbitrary
    return (Semver r1 r2 r3 "" "")

-- properties
-- a version always satisfies itself
prop_satisfies :: Property
prop_satisfies =
  property $
  \s -> satisfies s s

-- a version is always less than its next {major, minor, patch} version
prop_lessThanNext :: (Semver -> Semver) -> Property
prop_lessThanNext f =
  property $
  \s -> let s' = f s in s `lessThan` s'

prop_lessThanNextMajor :: Property
prop_lessThanNextMajor = prop_lessThanNext nextMajor

prop_lessThanNextMinor :: Property
prop_lessThanNextMinor = prop_lessThanNext nextMinor

prop_lessThanNextPatch :: Property
prop_lessThanNextPatch = prop_lessThanNext nextPatch

-- drive quickcheck
main :: IO ()
main = do
  args <- getArgs
  (o, _) <- parseOpts args
  let numCheck = quickCheckWith stdArgs { maxSuccess = optNumTests o }
      checker = if optVerbose o
                then numCheck . verbose
                else numCheck
    in do checker prop_satisfies
          checker prop_lessThanNextMajor
          checker prop_lessThanNextMinor
          checker prop_lessThanNextPatch
