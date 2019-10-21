#ifndef TOUPHSCRIPT_KERNEL
#define TOUPHSCRIPT_KERNEL

#include "common.h"

namespace ts {

class kernel {

public:
	kernel(string const& s);
	byteVec2d getText() const;
	void setText(byteVec2d& v);
	void updateBattleTbl(byteVec2d const& scene);
	byteVec save() const;

private:
	enum ofsts {
		cloud = 0x10, red13 = 0x220, sephiroth = 0x3AC, sceneTbl = 0xF1C
	};
	
	static int const nSecs = 27;

	byteVec2d sections;
};

}

#endif
