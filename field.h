#ifndef TOUPHSCRIPT_FIELD
#define TOUPHSCRIPT_FIELD

#include "common.h"
#include <cstring>

namespace ts {
namespace field {

byteVec2d toField(byteVec const& buf);
byteVec toVec(byteVec2d const& field);

inline bool isField(string const& name) {
	if (!strchr(name.c_str(), '.') && strcmp(name.c_str(), "maplist"))
		return true;
	return false;
}

}
}

#endif
