{-# LANGUAGE ForeignFunctionInterface #-}
module Semver where

#include "semantic_version_ffi.h"

import Foreign hiding (unsafePerformIO)
import Foreign.C.String
import Foreign.Storable
import System.IO.Unsafe (unsafePerformIO)

data Semver = Semver
    { vMajor :: #{type unsigned int}
    , vMinor :: #{type unsigned int}
    , vPatch :: #{type unsigned int}
    , vPrerelease :: String
    , vBuild :: String
    } deriving (Eq, Show)

-- don't bother with the strings for now

instance Storable Semver where
    sizeOf _    = #{size semver_t}
    alignment _ = alignment (undefined :: Word32)

    peek ptr = do
        r1 <- #{peek semver_t, major} ptr
        r2 <- #{peek semver_t, minor} ptr
        r3 <- #{peek semver_t, patch} ptr
        return (Semver r1 r2 r3 "" "")

    poke ptr (Semver r1 r2 r3 _ _) = do
        #{poke semver_t, major} ptr r1
        #{poke semver_t, minor} ptr r2
        #{poke semver_t, patch} ptr r3

-- c functions

foreign import ccall "semantic_version_ffi.h satisfies"
    c_satisfies :: Ptr Semver -> Ptr Semver -> IO Int
foreign import ccall "semantic_version_ffi.h lessThan"
    c_lessThan :: Ptr Semver -> Ptr Semver -> IO Int
foreign import ccall "semantic_version_ffi.h nextMajor"
    c_nextMajor :: Ptr Semver -> Ptr Semver -> IO ()
foreign import ccall "semantic_version_ffi.h nextMinor"
    c_nextMinor :: Ptr Semver -> Ptr Semver -> IO ()
foreign import ccall "semantic_version_ffi.h nextPatch"
    c_nextPatch :: Ptr Semver -> Ptr Semver -> IO ()

-- haskell wrappers

satisfies :: Semver -> Semver -> Bool
satisfies a b =
  unsafePerformIO $
  alloca $ \a_ptr ->
  alloca $ \b_ptr -> do
    poke a_ptr a
    poke b_ptr b
    r <- c_satisfies a_ptr b_ptr
    return $ if (r == 1) then True else False

lessThan :: Semver -> Semver -> Bool
lessThan a b =
  unsafePerformIO $
  alloca $ \a_ptr ->
  alloca $ \b_ptr -> do
    poke a_ptr a
    poke b_ptr b
    r <- c_lessThan a_ptr b_ptr
    return $ if (r == 1) then True else False

nextFunc :: (Ptr Semver -> Ptr Semver -> IO ()) -> Semver -> Semver
nextFunc f a =
  unsafePerformIO $
  alloca $ \a_ptr ->
  alloca $ \b_ptr -> do
    poke a_ptr a
    f a_ptr b_ptr
    peek b_ptr

nextMajor :: Semver -> Semver
nextMajor = nextFunc c_nextMajor

nextMinor :: Semver -> Semver
nextMinor = nextFunc c_nextMinor

nextPatch :: Semver -> Semver
nextPatch = nextFunc c_nextPatch
