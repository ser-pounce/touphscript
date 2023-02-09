#ifndef TOUPHSCRIPT_FFSTRING
#define TOUPHSCRIPT_FFSTRING

#include "common.h"
#include "config.h"

namespace ts {
namespace ffString {

string toString(byteVec const& v);
byteVec toFFString(string const& s);

string toSceneString(byteVec const& v);
byteVec toSceneVec(string const& s);

string toKernel2String(byteVec const& v);
byteVec toKernel2Vec(string const& s);

string stdFunc2String(bvCit& it);
byteVec stdString2Func(string op, sCit& it, string const& s);

string text2String(u8 const c);
u8 stdString2Text(sCit& it, string const& s);

u16 width(byteVec const& v, byteVec const& spacing);
u16 height(byteVec const& v);
inline u8 lineHeight(u16 const height) {
	u8 nh = (height - config::rowH2) / config::rowH1;
	return nh > 13 ? 13 : nh;
}
inline u16 pxHeight(u16 const height) {
	u16 nh = height * config::rowH1 + config::rowH2;
	return nh > 217 ? 217 : nh;
}

inline string toVar(string const& s) {
	std::ostringstream ss;
	ss << '{' << s << '}';
	return ss.str();
}
inline string toVar(string const s, int const i) {
	std::ostringstream ss;
	ss << '{' << s << ' ' << i << '}';
	return ss.str();
}
inline string toVar(string const s, int const i, int const j) {
	std::ostringstream ss;
	ss << '{' << s << ' ' << i << ' ' << j << '}';
	return ss.str();
}
inline string toHexVar(string const& s, int const i, int const j) {
	std::ostringstream ss;
	ss << '{' << s << ' ' <<
		std::hex << std::uppercase << i << ' ' << j << '}';
	return ss.str();
}

enum funcCodes : u8 {
	gray = 0xD2, pause = 0xDD, mem1, mem2 = 0xE1, mem3, max = 0xE9
};
enum charCodes : u8 {
	pound = 0x03, comma = 0x0C, slash = 0x3C, obrace = 0x5B, cbrace = 0x5D, choice = 0xE0, tab,
	nline = 0xE7, newW, func = 0xFE, end
};
enum otherOps : u8 {
	name = 0xEA
};

extern sVec const funcMap;
extern sVec charMap;
extern sVec const mapVars;
extern sVec const kernel2Vars;

void loadCharmap(char const* path);
}
}

#endif
