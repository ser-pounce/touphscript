#ifndef TOUPHSCRIPT_FF7EXE
#define TOUPHSCRIPT_FF7EXE

#include "common.h"

namespace ts {

class ff7exe {
public:
	ff7exe(string const& s);
	sVec getText();
	void setText(sVec const& text);
	void updateItemOrder(sVec const& names);

	u8 maxSp;
	u8 choiceSp;
	u8 tabSp;

private:
	enum txtOfsts { max = 0x2E6F0A, choice = 0x231127, tab = 0x23108D};
	enum txtTypes { def, rgb, unicode, noffTerm, ffpadded, zeroterm };
	static vec32 const ofsts;
	static vec32 const len;
	static vec32 const type;
	std::fstream f;
};

}

#endif
