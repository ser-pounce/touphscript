#include "tutorial.h"
#include "ffString.h"

namespace ts {
namespace tutorial {

sVec toString(byteVec const& in) {
	byteVec2d secs = split(in);
	sVec out;

	for (byteVec const& sec : secs) {
		std::ostringstream s;
		for (auto op = sec.cbegin(); op < sec.cend() && *op != end;) {
			if (*op > window) {
				++op;
				continue;			
			}
			switch (*op) {
			case pause:
				s << ffString::toVar("PAUSE", get<u16>(sec, ++op)) << '\n';
				op += 2;
				break;
			case text: {
				op += 2;
				auto start = op++;
				while (*op++ != 0xFF);
				s << ffString::toString(byteVec(start, op)) << '\n';
				}
				break;
			case window: {
				u16 h1, h2;
				op = copy(sec, ++op, h1);
				op = copy(sec, op, h2);
				s << ffString::toVar("WINDOW", h1, h2) << '\n';
				}
				break;			
			default:
				s << ffString::toVar(opNames.at(*op++)) << '\n';
				break;
			}
			if (op > sec.cend())
				throw runErr("Unexpected end of tutorial");
		}
		out.push_back(s.str());
		out.back().erase(out.back().rfind("\n"));
	}
	return out;
}

byteVec2d split(byteVec const& in) {
	auto const pos = in.cbegin();
	vec16 ofsts(get<u16>(in, pos) / sizeof(u16));
	copy(in, pos, ofsts);
	ofsts.push_back(in.size()); // Size as end

	byteVec2d out;
	for (auto ofst = ofsts.cbegin(); ofst < ofsts.cend() - 1; ++ofst) {
		out.push_back(byteVec(ofst[1] - ofst[0]));
		copy(in, pos + *ofst, out.back());
	}
	return out;
}

byteVec toTutorial(vector<sVec> const& in) {
	byteVec2d tmp;
	for (sVec const& strVec : in) {
		tmp.push_back(byteVec());
		for (string const& s : strVec) {
			if (s[0] == '{') {
				auto it = s.cbegin() + 1;
				while (*it++ != '}')
					if (it >= s.end())
						throw runErr("Closing brace not found");
				string op(s.cbegin() + 1, it - 1);
				transform(op.begin(), op.end(), op.begin(), toupper);

				std::istringstream opss(op);
				string str;
				if (!opss.eof()) opss >> str;
				if (str == "PAUSE") {
					tmp.back().push_back(pause);
					u16 h1 = 0;
					if (opss.good()) opss >> h1;
					tmp.back().push_back(h1 & 0x00FF);
					tmp.back().push_back((h1 & 0xFF00) >> 8);
					continue;
				} else if (str == "WINDOW") {
					tmp.back().push_back(window);
					u16 h1 = 0, h2 = 0;
					if (opss.good()) opss >> h1;
					if (opss.good()) opss >> h2;
					tmp.back().push_back(h1 & 0x00FF);
					tmp.back().push_back((h1 & 0xFF00) >> 8);
					tmp.back().push_back(h2 & 0x00FF);
					tmp.back().push_back((h2 & 0xFF00) >> 8);
					continue;
				} else {
					u8 i;
					for (i = 0; i < opNames.size(); ++i)
						if (opNames[i] == str) {
							tmp.back().push_back(i);
							break;
						}
					if (i >= opNames.size())
						throw runErr("Unknown var");
				}
			} else {
				tmp.back().push_back(text);
				tmp.back().push_back(0x00);
				byteVec ffTmp = ffString::toFFString(s);
				tmp.back().insert(tmp.back().end(), ffTmp.begin(), ffTmp.end());
			}
		}
	}
	u16 size = tmp.size() * sizeof(u16);
	for (byteVec const& v : tmp)
		size += v.size();
	byteVec out(size);

	size = tmp.size() * sizeof(u16);
	auto pos = out.begin();
	for (byteVec const& v : tmp) {
		pos = copy(size, pos);
		size += v.size();
	}
	copy(tmp, pos);
	return out;
}


sVec const opNames({
	"PAUSE", "", "UP", "DOWN", "LEFT", "RIGHT", "MENU", "CANCEL",
	"", "CONFIRM", "PGUP", "", "PGDOWN"
});


}
}