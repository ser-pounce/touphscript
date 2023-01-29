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
		cloud = 0x10, barrett = 0x94, tifa = 0x118, aerith = 0x19C, 
		red13 = 0x220, yuffie = 0x2A4, cait = 0x328, sephiroth = 0x3AC, cid = 0x430, 
		sceneTbl = 0xF1C
	};
	
	static int const nSecs = 27;

	byteVec2d sections;
};

}

#endif
