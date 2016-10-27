// __SSE2__ and __SSE4_2__ are recognized by gcc, clang, and the Intel compiler.
// We use -march=native with gmake to enable -msse2 and -msse4.2, if supported.
#if defined(__SSE4_2__)
#  define RAPIDJSON_SSE42
#elif defined(__SSE2__)
#  define RAPIDJSON_SSE2
#endif

#include "rapidjson/allocators.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/reader.h"
#include "rapidjson/document.h"
#include "rapidjson/memorybuffer.h"
#include "rapidjson/schema.h"
#include "rapidjson/encodedstream.h"
#include "rapidjson/memorystream.h"
#include "rapidjson/stream.h"
#include "rapidjson/encodings.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/pointer.h"
#include "rapidjson/writer.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/fwd.h"
#include "rapidjson/rapidjson.h"

#include "rapidjson/error/error.h"
#include "rapidjson/error/en.h"

