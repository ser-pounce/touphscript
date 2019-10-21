#include "kernel2.h"
#include "lzs.h"

namespace ts {
namespace kernel2 {

textVec toText(string const& s) {
	ifstream in(s, in.in | in.binary | in.ate);
	in.exceptions(in.failbit | in.badbit | in.eofbit);
	byteVec buf(in.tellg());
	in.seekg(0);
	read(in, buf);
	in.close();

	buf = lzs::decompress(buf);
	textVec out(nSecs);
	auto pos = buf.cbegin();
	for (u8 i = 0; i < nSecs; ++i) {
		byteVec tmp(get<u32>(buf, pos));
		pos += sizeof(u32);
		pos = copy(buf, pos, tmp);

		// (first pointer - ofstSz) / sizeof(u16) = count
		vec16 txtOfsts((get<u16>(tmp, tmp.cbegin())) / sizeof(u16));
		copy(tmp, tmp.cbegin(), txtOfsts);
		txtOfsts.push_back(tmp.size()); // End relative to table

		for (auto ofst = txtOfsts.cbegin(); ofst < txtOfsts.cend() - 1 && *ofst < tmp.size(); ++ofst) {
			auto end = tmp.cbegin() + *ofst;
			while (*end != 0xFF) {
				if (*end >= 0xEA && *end <= 0xF0)
					end += 3;
				else ++end;
			}
			++end;
			out[i].push_back(byteVec(end - (tmp.cbegin() + *ofst)));
			copy(tmp, tmp.cbegin() + *ofst, out[i].back());
		}
	}
	return out;
}

byteVec toKernel2(textVec const& text) {
	u32 size = 0;
	for (byteVec2d const& v : text)
		size 	+= sizeof(u32)
					+ v.size() * sizeof(u16)
					+ vsize(v);
					
	byteVec out(size);
	
	auto pos = out.begin();
	for (byteVec2d const& v : text) {
		size	= v.size() * sizeof(u16)
					+ vsize(v);
					
		pos = copy(size, pos);		
		u16 ofst = v.size() * sizeof(u16);
		
		for (byteVec const& t : v) {
			pos = copy(ofst, pos);
			ofst += t.size();
		}
		pos = copy(v, pos);
	}
	return lzs::compress(out);
}

sVec const secNames({
	"# Command window descriptions",
	"# Magic, summon, enemy skill, limit break descriptions",
	"# Item descriptions",
	"# Weapon descriptions",
	"# Armour descriptions",
	"# Accessory descriptions",
	"# Materia descriptions",
	"# Key Item descriptions",
	"# Command names",
	"# Magic, enemy skill, summon, limit break names",
	"# Item names",
	"# Weapon names",
	"# Armour names",
	"# Accessory names",
	"# Materia names",
	"# Key Item names",
	"# Misc battle text",
	"# Summon attack names"
});

}
}
