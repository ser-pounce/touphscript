#include "field.h"

namespace ts {
namespace field {

byteVec2d toField(byteVec const& buf) {
	try {
		auto pos = buf.cbegin() + sizeof(u16); // skip null
		vec32 ptrs(get<u32>(buf, pos));
		byteVec2d out(ptrs.size());
		pos = copy(buf, pos + sizeof(u32), ptrs);

		for (u8 i = 0; i < ptrs.size(); ++i) {
			pos = buf.begin() + ptrs[i];
			out[i].resize(get<u32>(buf, pos));
			pos = copy(buf, pos + sizeof(u32), out[i]);
		}
		return out;
	} catch (runErr const& e) {
		throw runErr(string("converting to field failed, ") + e.what());
	}
}

byteVec toVec(byteVec2d const& field) {
	u32 const hdrSz = sizeof(u16)
		+ sizeof(u32) * (field.size() + 1);
	byteVec out(hdrSz + vsize(field) + sizeof(u32) * field.size());

	auto pos = copy(u16(), out.begin());
	pos = copy(static_cast<u32>(field.size()), pos);

	u32 ofst = hdrSz;
	for (byteVec const& sec : field) {
		pos = copy(ofst, pos);
		ofst += sizeof(u32) + sec.size();
	}
	for (byteVec const& sec : field) {
		pos = copy(static_cast<u32>(sec.size()), pos);
		pos = copy(sec, pos);
	}
	return out;
}

}
}
