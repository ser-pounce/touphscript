#include "kernel.h"
#include "gzip.h"

namespace ts {

kernel::kernel(string const& s): sections() {
	ifstream in(s, in.in | in.binary);
	in.exceptions(in.failbit | in.badbit | in.eofbit);

	for (u8 i = 0; i < nSecs; ++i) {
		byteVec cmp(get<u16>(in));
		u16 size = get<u16>(in);
		in.ignore(sizeof(u16));
		read(in, cmp);
		sections.push_back(gzip::decompress(cmp, size));
	}
}

byteVec2d kernel::getText() const {
	byteVec2d out;
	auto const begin = sections[3].cbegin();
	out.push_back(byteVec(begin + cloud, begin + cloud + 12));
	out.push_back(byteVec(begin + red13, begin + red13 + 12));
	out.push_back(byteVec(begin + sephiroth, begin + sephiroth + 12));
	return out;
}

void kernel::setText(byteVec2d& v) {
	for (byteVec& vec : v) {
		vec.back() = 0xFF;
		while (vec.size() < 12) vec.push_back(0xFF);
	}
	auto const begin = sections[3].begin();
	copy(v[0].cbegin(), v[0].cend(), begin + cloud);
	copy(v[1].cbegin(), v[1].cend(), begin + red13);
	copy(v[2].cbegin(), v[2].cend(), begin + sephiroth);
}

void kernel::updateBattleTbl(byteVec2d const& scene) {
	auto pos = sections[2].begin() + sceneTbl;
	auto const end = sections[2].begin() + sceneTbl + 64;
	*pos++ = 0x00;	
	for (u16 i = 0, size = 0x40; i < scene.size();) {
		if (pos >= end)
			throw runErr("Couldn't update battle lookup table");
		size += scene[i].size();
		if (size > 0x2000) {
			*pos++ = i;
			size = 0x40;
			continue;
		}
		++i;
	}
	while (pos < end) *pos++ = 0xFF;
}

byteVec kernel::save() const {
	byteVec out;
	u16 nsec = 0;
	for (byteVec const& sec : sections) {
		byteVec cmp = gzip::compress(sec);
		out.insert(out.end(), 6, 0);
		auto pos = copy(static_cast<u16>(cmp.size()), out.end() - 6);
		pos = copy(static_cast<u16>(sec.size()), pos);
		copy(nsec, pos);
		out.insert(out.end(), cmp.begin(), cmp.end());
		if (nsec < 9) ++nsec;
	}
	out.insert(out.end(), 2, 0);
	return out;
}

}
