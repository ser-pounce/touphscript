#ifndef TOUPHSCRIPT_KERNEL2
#define TOUPHSCRIPT_KERNEL2

#include "common.h"

namespace ts {
namespace kernel2 {

typedef vector<byteVec2d> textVec;

u8 const nSecs = 18;

textVec toText(string const& path);
byteVec toKernel2(textVec const& text);

extern sVec const secNames;

}
}

#endif
