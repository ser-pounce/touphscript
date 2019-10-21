#ifndef TOUPHSCRIPT_WORLD
#define TOUPHSCRIPT_WORLD

#include "common.h"

namespace ts {
namespace world {

byteVec2d toText(byteVec const& in);
byteVec toMes(byteVec2d const& in);

class event {
public:
	struct window {
		u16 id;
		u16 x, y, w, h;
		u16 first, last;
    bool center;
		window(): id(null), x(null), y(null), w(null), h(null),
			first(null), last(null), center() { }
		window(u16 iid, u16 ix, u16 iy, u16 iw, u16 ih, u16 ifi, u16 il):
			id(iid), x(ix), y(iy), w(iw), h(ih), first(ifi), last(il), center() { }
	};
	event(byteVec const& in): buf(in.size() / sizeof(u16)) {
		copy(in, in.cbegin(), buf);
	}
	vector<window> getWindows() const;
	void setWindows(vector<window> const& windows);
	void autosize(byteVec2d const& text, byteVec const& spacing);
	byteVec save() const {
		byteVec out(buf.size() * sizeof(u16));
		copy(buf, out.begin());
		return out;
	}

	static u16 const null = 0xFFFF;

private:
	enum { win = 0x0324, message, ask };

	vec16 buf;
};

}
}

# endif
