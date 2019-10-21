#include "scene.h"
#include "gzip.h"

namespace ts {
namespace scene {

byteVec2d toScene(string const& s) {
	try {
		ifstream in(s, in.in | in.binary);
		in.exceptions(in.failbit | in.badbit | in.eofbit);
		byteVec2d out;

		for (u32 pos = 0; out.size() < count; pos += blocksz) {
			vec32 ofsts(nofsts);
			in.seekg(pos);
			read(in, ofsts);
			auto nend = ofsts.end();
			if ((nend = find(ofsts.begin(), ofsts.end(), nullPtr)) < ofsts.end())
				ofsts.erase(nend, ofsts.end());
			ofsts.push_back(blocksz / sizeof(u32));
			for(auto ofst = ofsts.cbegin(); ofst < ofsts.cend() - 1; ++ofst) {
				byteVec buf((ofst[1] - ofst[0]) * sizeof(u32));
				in.seekg(pos + *ofst * sizeof(u32));
				read(in, buf);
				out.push_back(gzip::decompress(buf, uncSz));
			}
		}
		return out;
	} catch (runErr const& e) {
		throw runErr(string("converting to scene failed, ") + e.what());
	}
}

byteVec toVec(byteVec2d const& scene) {
	byteVec out;
	for (u16 i = 0; i < scene.size();) {
		byteVec otmp(blocksz, 0xFF);
		u32 ofst = 0x40;
		u16 j = i;
		for (auto pos = otmp.begin(); ofst + scene[j].size() <= blocksz && j < count; ++j) {
			pos = copy(ofst / 4, pos);
			ofst += scene[j].size();
		}
		for (auto pos = otmp.begin() + nofsts * sizeof(u32); i < j; ++i)
			pos = copy(scene[i], pos);
		out.insert(out.end(), otmp.cbegin(), otmp.cend());
	}
	return out;
}

sceneFile::sceneFile(byteVec const& in): buf(in.cbegin(), in.cbegin() + fScript),
fScripts(4, byteVec2d(nofsts)), eScripts(3, byteVec2d(nofsts)) {
	setScript(fScripts, byteVec(in.cbegin() + fScript, in.cbegin() + eScript));
	setScript(eScripts, byteVec(in.cbegin() + eScript, in.cbegin() + in.size()));
}

void sceneFile::setScript(scripts& s, byteVec const& in) {
	vec16 ofsts(s.size());
	copy(in, in.cbegin(), ofsts);
	ofsts.push_back(in.size());

	for (u8 i = 0; i < ofsts.size() - 1; ++i) {
		if (ofsts[i] == nullPtr) continue;
		u8 j = i;
		while (ofsts[++j] == nullPtr);
		byteVec tmp(ofsts[j] - ofsts[i]);
		copy(in, in.cbegin() + ofsts[i], tmp);

		vec16 sofsts(nofsts);
		copy(tmp, tmp.cbegin(), sofsts);
		sofsts.push_back(tmp.size());

		for (u8 k = 0; k < sofsts.size() - 1; ++k) {
			if (sofsts[k] == nullPtr) continue;
			u8 l = k;
			while (sofsts[++l] == nullPtr);
			s[i][k] = byteVec(sofsts[l] - sofsts[k]);
			copy(tmp, tmp.cbegin() + sofsts[k], s[i][k]);
			while (s[i][k].back() == 0xFF) s[i][k].pop_back();
		}
	}
}

byteVec2d sceneFile::getText() const {
	byteVec2d text;
	text.push_back(byteVec(buf.begin() + enemy1, buf.begin() + enemy1 + 32));
	text.push_back(byteVec(buf.begin() + enemy2, buf.begin() + enemy2 + 32));
	text.push_back(byteVec(buf.begin() + enemy3, buf.begin() + enemy3 + 32));

	for (u16 i = 0; i < 32 * 32; i += 32)
		text.push_back(byteVec(buf.begin() + moves + i, buf.begin() + moves + i + 32));

	getScriptText(fScripts, text);
	getScriptText(eScripts, text);
	return text;
}

void sceneFile::setText(byteVec2d& text) {
	auto txtIt = text.begin();
	for (u8 i = 0; i < 35; ++i) {
		txtIt[i].back() = 0xFF;
		while (txtIt[i].size() < 32) txtIt[i].push_back(0xFF);
		txtIt[i].resize(32);
	}
	std::copy(txtIt->data(), txtIt->data() + txtIt->size(), buf.begin() + enemy1);
	++txtIt;
	std::copy(txtIt->data(), txtIt->data() + txtIt->size(), buf.begin() + enemy2);
	++txtIt;
	std::copy(txtIt->data(), txtIt->data() + txtIt->size(), buf.begin() + enemy3);
	++txtIt;

	for (u16 i = 0; i < 32 * 32; i += 32) {
		std::copy(txtIt->data(), txtIt->data() + txtIt->size(),
			buf.begin() + moves + i);
		++txtIt;
	}
	setScriptText(fScripts, txtIt);
	setScriptText(eScripts, txtIt);
}

void sceneFile::getScriptText(scripts const& s, byteVec2d& text) const {
	for (byteVec2d const& scrs : s)
		for (byteVec const& script : scrs)
			for (auto op = script.cbegin(); op < script.cend() && *op != end;)
				switch (*op) {
				case fftext: {
					auto tstart = ++op;
					while (*op++ != 0xFF);
					text.push_back(byteVec(tstart, op));
					}
					continue;
				case debug:
					op += 2;
					while (*op++ != 0x00);
					continue;
				default:
					if (!oplen[*op]) throw runErr("Unknown opcode");
					op += oplen[*op];
					continue;
				}
}

void sceneFile::setScriptText(scripts& s, bv2dIt txtIt) {
	for (auto& scrs : s)
		for (auto& script : scrs) {
		vec16 labels(getLabels(script));
			for (auto op = script.begin(); op < script.end() && *op != end;)
				switch (*op) {
				case fftext: {
					u16 pos = distance(script.begin(), ++op);
					auto tstart = op;
					while (*op++ != 0xFF);
					u16 oldSize = op - tstart;
					u16 oldEnd = distance(script.begin(), op);
					script.erase(tstart, op);
					script.insert(script.begin() + pos, txtIt->cbegin(), txtIt->cend());
					for (u16& label : labels)
						if (label >= oldEnd)
							label += txtIt->size() - oldSize;
					op = script.begin() + pos + txtIt++->size();
					}
					continue;
				case debug:
					op += 2;
					while (*op++ != 0x00);
					continue;
				default:
					if (!oplen[*op]) throw runErr("Unknown opcode");
					op += oplen[*op];
					continue;
				}
			setLabels(script, labels);
		}
}

vec16 sceneFile::getLabels(byteVec const& scr) const {
	vec16 labels;
	for (auto op = scr.cbegin(); op < scr.cend();) {
		switch (*op) {
		case jmp0:
		case jmpne:
		case jmp:
			labels.push_back(u16());
			op = copy(scr, ++op, labels.back());
			continue;
		case fftext:
			while (*op++ != 0xFF);
			continue;
		case debug:
			op += 2;
			while (*op++ != 0x00);
			continue;
		default: op += oplen[*op];
		}
	}
	return labels;
}

void sceneFile::setLabels(byteVec& scr, vec16 const& labels) {
	auto label(labels.cbegin());
	for (auto op = scr.begin(); op < scr.end();) {
		switch (*op) {
		case jmp0:
		case jmpne:
		case jmp:
			op = copy(*label++, ++op);
			continue;
		case fftext:
			while (*op++ != 0xFF);
			continue;
		case debug:
			op += 2;
			while (*op++ != 0x00);
			continue;
		default: op += oplen[*op];
		}
	}
}

byteVec sceneFile::save() const {
	byteVec out(uncSz, 0xFF);
	copy(buf, out.begin());

	byteVec s(saveScript(fScripts));
	copy(s, out.begin() + fScript);
	s = saveScript(eScripts);
	copy(s, out.begin() + eScript);

	byteVec cmp(gzip::compress(out));
	while (cmp.size() % 4) cmp.push_back(0xFF);
	return cmp;
}

byteVec sceneFile::saveScript(scripts const& s) const {
	u16 size = s.size() * sizeof(u16);
	for (byteVec2d const& vec : s) {
		if (!vsize(vec)) continue;
		size += 16 * sizeof(u16);
		size += vsize(vec);
		if (vsize(vec) % 2) ++size;
	}
	byteVec out(size, 0xFF);
	auto pos = out.begin();
	u16 ofst = s.size() * sizeof(u16);
	for (byteVec2d const& vec : s) {
		pos = copy(vsize(vec) ? ofst : nullPtr, pos);
		if (!vsize(vec)) continue;
		ofst += vsize(vec) + 16 * sizeof(u16);
		if (ofst % 2) ++ofst;
	}
	for (byteVec2d const& vec : s) {
		if (!vsize(vec)) continue;
		u16 ofst = 16 * sizeof(u16);
		for (byteVec const& script : vec) {
			pos = copy(script.size() ? ofst : nullPtr, pos);
			ofst += script.size();
		}
		pos = copy(vec, pos);
		if (distance(out.begin(), pos) % 2)
			++pos;
	}
	return out;
}

byteVec const sceneFile::oplen({
	3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	2, 3, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	3, 3, 3, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
});

}
}
