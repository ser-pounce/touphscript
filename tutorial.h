#ifndef TOUPHSCRIPT_TUTORIAL
#define TOUPHSCRIPT_TUTORIAL

#include "common.h"

namespace ts {
namespace tutorial {

sVec toString(byteVec const& in);
byteVec2d split(byteVec const& in);
byteVec toTutorial(vector<sVec> const& in);

inline bool isTut(string const& name) {
	if (name.find(".tut") != name.npos)
		return true;
	return false;
}

enum ops : u8 {
	pause, up = 0x02, down, left, right, menu, cancel, confirm = 0x09,
	pgup, pgdown = 0x0C, text = 0x10, end, window
};

extern sVec const opNames;

}
}

#endif
