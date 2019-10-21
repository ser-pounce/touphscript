#include "world.h"
#include "ffString.h"

namespace ts {
namespace world {

byteVec2d toText(byteVec const& in) {
	auto const begin = in.cbegin();
	u8 ofstSz = sizeof(u16);
	// (first pointer - ofstSz) / ofstSz = real count
	vec16 txtOfsts((get<u16>(in, begin + ofstSz) - ofstSz) / ofstSz);
	copy(in, begin + ofstSz, txtOfsts);
	txtOfsts.push_back(in.size()); // End relative to table

	byteVec2d text;
	for (auto ofst = txtOfsts.cbegin(); ofst < txtOfsts.cend() - 1; ++ofst) {
		text.push_back(byteVec(ofst[1] - ofst[0]));
		copy(in, begin + *ofst, text.back());
	}
	return text;
}

byteVec toMes(byteVec2d const& text) {
	u16 txtOfst = (text.size() + 1) * sizeof(u16);
	byteVec out(txtOfst + vsize(text));	
	auto pos = copy(static_cast<u16>(text.size()), out.begin());
	for (byteVec const& txtEnt : text) {
		pos = copy(txtOfst, pos);
		txtOfst += txtEnt.size();
	}
	copy(text, pos);
	return out;
}

vector<event::window> event::getWindows() const {
	vector<window> windows;
	for (auto it = buf.cbegin(); it < buf.cend(); ++it)
		if (*it == win) {
			windows.push_back({null, it[-7], it[-5], it[-3], ffString::lineHeight(it[-1]), null, null});
			window& win = windows.back();
			while (*++it != message && *it != ask)
				if (it >= buf.cend()) return windows;
			if (*it == message)
				win.id = it[-1];
			else {
				win.id = it[-6];
				win.first = it[-4];
				win.last = it[-2];
			}
		}
	return windows;
}

void event::setWindows(vector<window> const& windows) {
	for (auto it = buf.begin(); it < buf.end(); ++it)
		if (*it == win) {
			auto const w = it;
			while (*++it != message && *it != ask)
				if (it >= buf.cend()) return;
			u16 const id = *it == message ? it[-1] : it[-6];
			auto win = find_if(windows.cbegin(), windows.cend(), [id](window const& w) {
				return w.id == id;
			});
			if (win < windows.cend()) {
				if (win->x != null) w[-7] = win->x;
				if (win->y != null) w[-5] = win->y;
				if (win->w != null) w[-3] = win->w;
				if (win->h != null) w[-1] = ffString::pxHeight(win->h);
				if (win->center) {
					w[-7] = 160 - w[-3] / 2;
				}
				if (*it == ask) {
					if (win->first != null) it[-4] = win->first;
					if (win->last != null) it[-2] = win->last;
				}
			}
		}
}

void event::autosize(byteVec2d const& text, byteVec const& spacing) {
	for (auto it = buf.begin(); it < buf.end(); ++it)
		if (*it == win) {
			auto const w = it;
			while (*++it != message && *it != ask)
				if (it >= buf.cend()) return;
			u16 const id = *it == message ? it[-1] : it[-6];
			w[-3] = ffString::width(text[id], spacing);
			w[-1] = ffString::height(text[id]);
		}
}

}
}
