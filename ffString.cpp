#include "ffstring.h"
#include "config.h"
#include <cmath>

namespace ts {
namespace ffString {

string toString(byteVec const& v) {
	std::ostringstream s;
	for (auto c = v.cbegin(); c < v.cend() && *c != end; ++c) {
		switch (*c) {
		case func:
			s << stdFunc2String(++c);
			break;
		default:
			if (*c == tab)
				if (c[1] == tab) {
					s << charMap[choice];
					++c;
					continue;
				}
			s << text2String(*c);
			break;
		}
	}
	return s.str();
}

inline string text2String(u8 const c) {
	std::ostringstream s;
	if (c == pound || c == slash || c == obrace || c == cbrace)
		s << '\\';
	s << charMap[c];
	if (c == newW) s << '\n';
	return s.str();
}

string stdFunc2String(bvCit& c) {
switch (*c) {
	case pause: {
		u16 i = *++c | *++c << 8;
		return toVar("WAIT", i);
		}
	case mem3: {
		u16 i = *++c | *++c << 8;
		u16 j = *++c | *++c << 8;
		return toHexVar("MEM3", i, j);
		}
	default:
		return toVar(funcMap.at(*c - gray));
	}
}

byteVec toFFString(string const& s) {
	byteVec out(s.size());
	auto pos = out.begin();
	auto it = s.cbegin();
	while (it < s.cend()) {
		if (*it == '{') {
			auto start = ++it;
			while (*it++ != '}') {
				if (it >= s.end()) {
					std::ostringstream err;
					err << distance(s.begin(), start) << ": Closing brace not found"
						<< '\n';
					throw runErr(err.str());
				}
			}
			pos = copy(stdString2Func(string(start, it - 1), it, s), pos);
			continue;
		} else
			*pos++ = stdString2Text(it, s);
	}
	out.erase(pos, out.end());
	out.push_back(end);
	return out;
}

inline u8 stdString2Text(sCit& it, string const& s) {
	switch (*it) {
	case '\t':
		if (it[1] == '\t') {
			it += 2;
			return choice;
		} else {
			++it;
			return tab;
		}
	case '\\':
		if (++it >= s.end())
			throw runErr("Unexpected end of text");
		break;
	}
	if ((*it & 0xC0) == 0x80) {
		std::ostringstream err;
		err << "Unexpected continuation byte: " << distance(s.begin(), it);
		throw runErr(err.str());
	}
	char c;
	string str(1, c = *it++);
	// Read any multibyte UTF-8 chars
	if (c & 0x80) {
		c <<= 1;
		while (c & 0x80) {
			if (it >= s.end())
				throw runErr("Unexpected end of text");
			if ((*it & 0xC0) != 0x80) {
				std::ostringstream err;
				err << "Unexpected continuation byte: " << distance(s.begin(), it);
				throw runErr(err.str());
			}
			str += *it++;
			c <<= 1;
		}
	}
	auto ch = find(charMap.cbegin(), charMap.cend(), str);	
	if (ch != charMap.cend())
		return distance(charMap.cbegin(), ch);

	std::ostringstream err;
	err << distance(s.begin(), it) - str.size()
			<< ": Unknown sequence " << str;
	throw runErr(err.str());
}

byteVec stdString2Func(string op, sCit& it, string const& s) {
	byteVec ffText;
	std::transform(op.begin(), op.end(), op.begin(), toupper);
	std::istringstream opss(op);
	string str;
	if (!opss.eof()) opss >> str;
	if (str == "WAIT") {
		ffText.push_back(func);
		ffText.push_back(pause);
		u16 h1 = 0;
		if (opss.good()) opss >> h1;
		ffText.push_back(h1 & 0x00FF);
		ffText.push_back((h1 & 0xFF00) >> 8);
		return ffText;
	} else if (str == "MEM3") {
		ffText.push_back(func);
		ffText.push_back(mem3);
		u16 h1 = 0, h2 = 0;
		if (opss.good()) opss >> std::hex >> h1;
		if (opss.good()) opss >> std::hex >> h2;
		ffText.push_back(h1 & 0x00FF);
		ffText.push_back((h1 & 0xFF00) >> 8);
		ffText.push_back(h2 & 0x00FF);
		ffText.push_back((h2 & 0xFF00) >> 8);
		return ffText;
	} else if (str == "NEW") {
		ffText.push_back(newW);
		while (*it++ != '\n')
			if (it >= s.end())
				throw runErr("Unexpected end of string");
		return ffText;
	} else {		
		auto f = find(funcMap.cbegin(), funcMap.cend(), op);
		if (f != funcMap.cend()) {
			ffText.push_back(func);
			ffText.push_back(distance(funcMap.cbegin(), f) + gray);
			return ffText;
		}
		f = find(mapVars.cbegin(), mapVars.cend(), op);
		if (f != mapVars.cend()) {			
			ffText.push_back(distance(mapVars.cbegin(), f) + choice);
			return ffText;
		}

		f = find(charMap.cbegin(), charMap.cend(), "{" + op + "}");
		if (f != charMap.cend()) {
			ffText.push_back(distance(charMap.cbegin(), f));
			return ffText;
		}

		throw runErr("Unknown var");
	}
}

string toSceneString(byteVec const& v) {
	std::ostringstream s;
	for (auto c = v.cbegin(); *c != end && c < v.cend(); ++c) {
		if (*c == name) {
			c += 2;
			s << toVar(mapVars.at(10 + *c));
			continue;
		}
		s << text2String(*c);
	}
	return s.str();
}

byteVec toSceneVec(string const& s) {
	byteVec out;
	auto it = s.cbegin();
	while (it < s.cend()) {
		if (*it == '{') {
			auto start = ++it;
			while (*it++ != '}') {
				if (it >= s.end()) {
					std::ostringstream err;
					err << distance(s.begin(), start) << ": Closing brace not found"
						<< '\n';
					throw runErr(err.str());
				}
			}
			string str(start, it - 1);
			std::transform(str.begin(), str.end(), str.begin(), toupper);
			std::istringstream strs(str);
			string op;
			if (strs.good()) strs >> op;
			auto pos = std::find(mapVars.begin(), mapVars.end(), op);
			if (pos == mapVars.end())
				throw runErr("Unrecognized var");
			out.push_back(name);
			out.push_back(0x00);
			out.push_back(pos - mapVars.begin() - 10);
			continue;
		} else
			out.push_back(stdString2Text(it, s));
	}
	out.push_back(end);
	return out;
}

string toKernel2String(byteVec const& v) {
	std::ostringstream s;
	for (auto c = v.cbegin(); c < v.cend() && *c != end; ++c) {
		if (*c >= 0xEA && *c <= 0xF0) {
			s << toVar(kernel2Vars.at(*c - 0xEA));
			c += 2;
		} else if (*c == 0xF8) {
			s << toVar("RED");
			++c;
		}
		else
			s << text2String(*c);
	}
	return s.str();
}

byteVec toKernel2Vec(string const& s) {
	byteVec text;
	auto it = s.cbegin();
	while (it < s.cend()) {
		if (*it == '{') {
			auto start = ++it;
			while (*it++ != '}') {
				if (it >= s.end()) {
					std::ostringstream err;
					err << distance(s.begin(), start) << ": Closing brace not found"
						<< '\n';
					throw runErr(err.str());
				}
			}
			string str(start, it - 1);
			transform(str.begin(), str.end(), str.begin(), toupper);
			std::istringstream strs(str);
			string op;
			if (strs.good()) strs >> op;
			if (op == "RED") {
				text.push_back(0xF8);
				text.push_back(0x02);
			} else {
				u8 i;
				for (i = 0; i < kernel2Vars.size(); ++i)
					if (kernel2Vars[i] == op) {
						text.push_back(i + 0xEA);
						text.push_back(0xFF);
						text.push_back(0xFF);
						break;
					}
				if (i >= kernel2Vars.size())
					throw runErr("Unrecognized var");
			}
		}	else
			text.push_back(stdString2Text(it, s));
	}
	text.push_back(end);
	return text;
}

u16 width(byteVec const& v, byteVec const& spacing) {
	double lineWidth = spacing.back(), newWidth = spacing.back();
	bool maxW = false;
	for (auto c = v.cbegin(); *c != end && c < v.cend(); ++c) {
		switch (*c) {
		case func:
			switch (*++c) {
			case max:
				maxW = !maxW;
				continue;
			case pause:
				c += 2;
				continue;
			case mem1:
			case mem2:
				lineWidth += 5 * spacing.at(maxW ? 0x100: 0x10);
				continue;
			case mem3: {
				c += 3;
				u16 len = *c | *++c << 8;
				lineWidth += len * spacing.at(maxW ? 0x100 : 0x2F);
				continue;
				}
			}
			continue;
		case nline:
		case newW:
			if (lineWidth > newWidth) newWidth = lineWidth;
			lineWidth = spacing.back();
			continue;
		case choice:
			lineWidth += maxW ? spacing.at(choice) * spacing.at(0x100) / 2.0 :
									spacing.at(choice) * spacing.at(0x00);
			continue;
		case tab:
			lineWidth += maxW ? spacing.at(tab) * spacing.at(0x100) / 2.0 :
									spacing.at(tab) * spacing.at(0x00);
			continue;
		default:
			lineWidth += maxW ? spacing.at(0x100) / 2.0 :	spacing.at(*c);
		}
	}
	return ceil(lineWidth > newWidth ? lineWidth : newWidth);
}

u16 height(byteVec const& v) {

	if (v.size() < 2)
		return 0;
		
	u16 h = 1, nh = 1;
	for (auto c = v.cbegin(); *c != end && c < v.cend(); ++c) {
		switch (*c) {
		case func:
			switch (*++c) {
			case pause:
				c += 2;
				break;
			case mem3:
				c += 4;
				break;
			}
			break;
		case nline:
			++h;
			break;
		case newW:
			if (h > nh) nh = h;
			h = 1;
			break;
		}
	}
	if (h > nh) nh = h;
	if (nh > 13) nh = 13; // Max rows = 13
	return (nh * config::rowH1) + config::rowH2; // Row height calculation
}

sVec const funcMap({
	"GRAY", "BLUE", "RED", "PURPLE", "GREEN", "CYAN", "YELLOW", "WHITE", "FLASH",
	"RAINBOW", "OK", "", "MEM1", "", "", "MEM2", "", "", "", "", "", "", "", "MAX"
});

sVec charMap({
	"", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"{CHOICE}", "\t", ", ", ".\"", "…\"", "", "", "\n", "{NEW}", "", "{CLOUD}", "{BARRET}", "{TIFA}", "{AERIS}", "{RED XIII}", "{YUFFIE}",
	"{CAIT SITH}", "{VINCENT}", "{CID}", "{PARTY #1}", "{PARTY #2}", "{PARTY #3}", "〇", "△", "☐", "✕", "", "", "", "", "", ""
});

sVec const mapVars({
	"CHOICE", "", "", "", "", "", "", "", "", "", "CLOUD", "BARRET", "TIFA", "AERIS", "RED XIII", "YUFFIE",
	"CAIT SITH", "VINCENT", "CID", "PARTY #1", "PARTY #2", "PARTY #3"
});

sVec const kernel2Vars({
	"CHAR", "ITEM", "NUM", "TARGET", "ATTACK", "ID", "ELEMENT"
});

void loadCharmap(char const* path)
{
	std::ifstream file(path);
	removeBOM(file);

	sVec tempCharMap;
	std::string line;
	while (getline(file, line))
		tempCharMap.push_back(line);

	for (auto const& c : charMap)
		tempCharMap.push_back(c);
	
	charMap = tempCharMap;

}

}
}
