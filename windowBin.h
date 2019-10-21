#ifndef TOUPHSCRIPT_WINDOWBIN
#define TOUPHSCRIPT_WINDOWBIN

#include "gzip.h"

namespace ts {

class windowBin {
public:
	windowBin(string const& s);
	byteVec getSpacingTable() const;

private:
	byteVec2d secs;
};

inline windowBin::windowBin(string const& s): secs(3) {
	ifstream in(s, in.in | in.binary);
	in.exceptions(in.failbit | in.badbit | in.eofbit);

	for (u8 i = 0; i < secs.size(); ++i) {
		byteVec cmp(get<u16>(in));
		secs[i] = byteVec(get<u16>(in));
		in.ignore(sizeof(u16));
		read(in, cmp);
		secs[i] = gzip::decompress(cmp, secs[i].size());
	}
}

inline byteVec windowBin::getSpacingTable() const {
	byteVec table(0x100);
	for (u16 i = 0x00; i < table.size(); ++i)
		// Up to 0x1F px for each char, each 0x20 adds + 1 px
		table[i] = (secs[2].at(i) & 0x1F) + secs[2][i] / 0x20;
	return table;
}

}

#endif
