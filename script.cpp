#include "script.h"
#include "ffString.h"

namespace ts {
namespace script {

section0::section0(byteVec const& buf):
hdr(), entNms(), akaoBlks(), scripts(), text(), pcs(), mods(), locs() {
	try {
		auto const begin = buf.cbegin();
		auto pos = copy(buf, begin, hdr);
		entNms.assign(hdr.nEnts, byteVec(nmSz));
		vec32 akOfsts(hdr.akOfsts);
		vec16 scrOfsts(hdr.nEnts * nScr);
		scripts.assign(hdr.nEnts, byteVec2d(nScr));

		pos = copy(buf, pos, entNms);
		pos = copy(buf, pos, akOfsts);
		copy(buf, pos, scrOfsts);
		akOfsts.push_back(buf.size()); // size as akao end
		scrOfsts.push_back(hdr.strOfst); // strOfst as script end

		u8 ent = 0, script = 0;
		for (auto ofst = scrOfsts.cbegin(); ofst < scrOfsts.cend() - 1;) {
			byteVec& curScr = scripts[ent][script];
			auto nxOfst = ofst;
			u8 nxScript = script;
			while (*nxOfst == *ofst) {
				++nxOfst;
				++nxScript;
			}
			if (*ofst < hdr.strOfst) { // skip incorrect pointers (snw_w)
				curScr.assign(*nxOfst - *ofst, 0);
				copy(buf, begin + *ofst, curScr);
			}
			ofst = nxOfst;
			script = nxScript % nScr;
			ent += nxScript / nScr;
		}
		for (auto ofst = akOfsts.cbegin(); ofst < akOfsts.cend() - 1; ++ofst) {
			akaoBlks.push_back(byteVec(ofst[1] - ofst[0]));
			copy(buf, begin + *ofst, akaoBlks.back());
		}
		init();

		// No text (life1 & 2)
		if (!(static_cast<u32>(hdr.strOfst + minTblSz) <= akOfsts[0])) return;

		/* Ignore count, use first pointer
		(0 count means 256 in smkin_1, pillar_1 - 3, blin66_1 - 5,
			blin67_1 - 4, blin671b, blin673b and blin68_1 - 2;
		(first pointer - ofstSz) / ofstSz = real count */

		u8 ofstSz = sizeof(u16);
		auto const txtBegin = begin + hdr.strOfst;
		vec16 txtOfsts((get<u16>(buf, txtBegin + ofstSz) - ofstSz) / ofstSz);
		copy(buf, txtBegin + ofstSz, txtOfsts);
		txtOfsts.push_back(akOfsts[0] - hdr.strOfst); // End relative to table

		for (auto ofst = txtOfsts.cbegin(); ofst < txtOfsts.cend() - 1; ++ofst) {
			// Fix for dupe text ptrs (smkin_1, ...?)
			if (ofst[1] == ofst[0]) {
				auto nx = ofst;
				while (*++nx == *ofst)
					// Fix for dodgy pointers (loslake2, ...?)
					if (nx >= txtOfsts.cend() - 1) return;
				text.push_back(byteVec(*nx - *ofst));
			} else if (ofst[1] < ofst[0]) {
				// Fix for swapped offsets in Italian fields (blin67_1, blin671b, ...?)
				text.push_back(byteVec(ofst[2] - ofst[0]));
			} else
				text.push_back(byteVec(ofst[1] - ofst[0]));
			copy(buf, txtBegin + *ofst, text.back());
		}
	} catch (runErr const& e) {
		throw runErr(string("converting field section 1 to script failed, ") + e.what());
	}
}

void section0::init() {
	for (u8 ent = 0; ent < scripts.size(); ++ent) {
		byteVec const& s = scripts[ent][0];
		for (auto op = s.cbegin(); op < s.cend() && *op != ret; op += opSize(op)) {
			switch (*op) {
			case pc:
				pcs.insert(ent);
				break;
			case fchar:
				mods.insert(ent);
				break;
			case line:
				locs.insert(ent);
				break;
			}
		}
	}
}

byteVec section0::getScript() const {
	byteVec out;
	for (byteVec2d const& ent : scripts)
		for (byteVec const& script : ent)
			out.insert(out.end(), script.cbegin(), script.cend());
	return out;
}

inline u16 section0::scriptOfst() const {
	return (
		sizeof hdr
		+ nmSz * entNms.size()
		+ sizeof(u32) * akaoBlks.size()
		+ sizeof(u16) * nScr * scripts.size()
	);
}

inline u16 section0::textOfst() const {
	return scriptOfst() + vsize(scripts);
}

inline u32 section0::akaoOfst() const {
	u32 size = textOfst();
	size += (text.size() + 1) * sizeof(u16);
	return size + vsize(text);
}

u32 section0::intSize() const {
	return akaoOfst() + vsize(akaoBlks);
}

byteVec section0::save() const {
	byteVec out(intSize());

	header nh = hdr;
	nh.strOfst = textOfst();
	auto pos = copy(nh, out.begin());
	pos = copy(entNms, pos);

	u32 akOfst = akaoOfst();
	for (byteVec const& blk : akaoBlks) {
		pos = copy(akOfst, pos);
		akOfst += blk.size();
	}
	u16 scrOfst = scriptOfst();
	u16 pscrOfst = scrOfst;
	for (byteVec2d const& ent : scripts)
		for (byteVec const& script : ent) {
			if (!script.size())
				// Null entries point to previous script
				pos = copy(pscrOfst, pos);
			else {
				pos = copy(scrOfst, pos);
				pscrOfst = scrOfst;
				scrOfst += script.size();
			}
		}
	pos = copy(scripts, pos);

	pos = copy(static_cast<u16>(text.size()), pos);
	u16 txtOfst = (text.size() + 1) * sizeof(u16);
	for (byteVec const& txtEnt : text) {
		pos = copy(txtOfst, pos);
		txtOfst += txtEnt.size();
	}
	pos = copy(text, pos);
	copy(akaoBlks, pos);
	return out;
}

void removeUnused::action() {
	u8 ent = stack.back().first;
	u8 script = stack.back().second;
	called[ent].insert(script);
	byteVec const& scr = s[ent][script];
	for (auto op = scr.cbegin(); op < scr.cend(); op += opSize(op))
		scriptCall(op);
}

void removeUnused::removeOrphans() {
	for (u8 ent = 0; ent < s.size(); ++ent)
		for (u8 script = 0; script < 32; ++script) {
			if (!called[ent].count(script))
				s[ent][script].clear();
			if (script == 0) {
				byteVec& s0 = s[ent][script];
				if (!s0.size())
					s0 = byteVec(2, ret);
				else if (s0.size() == 1) {
					if (s0.back() != ret)
						s0.push_back(ret);
					s0.push_back(ret);
				} else if (s0.back() != ret)
					s0.push_back(ret);
			}
		}
}

void getTextIds::action() {
	byteVec const& scr = s[stack.back().first][stack.back().second];
	for (auto op = scr.cbegin(); op < scr.cend(); op += opSize(op)) {
		switch (*op) {
		case mpnam:
			ids.insert(op[1]);
			break;
		case message:
			ids.insert(op[2]);
			break;
		case ask:
			ids.insert(op[3]);
			break;
		case special:
			if (op[1] == spcnm)
				ids.insert(op[3]);
			break;
		}
	}
}

void textStrip::action() {
	auto& scr = s[stack.back().first][stack.back().second];
	for (auto op = scr.begin(); op < scr.end(); op += opSize(op)) {
		u8 id = 0;
		switch (*op) {
		case mpnam:
			id = 1;
			break;
		case message:
			id = 2;
			break;
		case ask:
			id = 3;
			break;
		case special:
			if (op[1] == spcnm) {
				id = 3;
				break;
			}
			continue;
		default: continue;
		}
		if (!get.ids.count(op[id])) {
			std::ostringstream err;
			err << "Text id " << static_cast<int>(op[id]) << " not found";
			throw runErr(err.str());
		}
		op[id] = distance(get.ids.cbegin(), get.ids.find(op[id]));
	}
}

void textStrip::strip() {
	byteVec2d text;
	for (u16 i = 0; i < s.getText().size(); ++i)
		if (get.ids.count(i))
			text.push_back(s.getText()[i]);
	s.setText(text);
}

void getWin::action() {
	u8 ent = stack.back().first;
	u8 script = stack.back().second;
	byteVec const& scr = s[ent][script];
	for (auto op = scr.cbegin(); op < scr.cend(); op += opSize(op)) {
		scriptCall(op);
		if (!called[ent].count(script))
			switch (*op) {
			case wspcl: {
				if (!tmpWindows[op[1]].size()) {
					if (!op[2]) continue;
					tmpWindows[op[1]].push_back(params());
				}
				params& win = tmpWindows[op[1]].back();
				if (op[2]) {
					win.sx = op[3];
					win.sh = op[4];
				} else {
					win.sx = null;
					win.sh = null;
				}
				continue;
			}
			case wsizw:
			case fwindow: {
				vector<params>& pvec = tmpWindows.at(op[1]);
				if (!pvec.size() || pvec.back().x != null)
					pvec.push_back(params());
				params& win = pvec.back();
				copy(scr, op + 2, sizeof(u16) * 4, &win);
				win.h = ffString::lineHeight(win.h);
				continue;
			}
			case mpnam:
				windows[op[1]];
				continue;
			case special:
				if (op[1] == spcnm)
					windows[op[3]];
				continue;
			}
		switch (*op) {
		case message: case ask:
			break;
		default: continue;
		}
		u8 const wId = *op == ask ? op[2] : op[1];
		u8 const tId = *op == ask ? op[3] : op[2];
		auto const& prms = tmpWindows[wId];
		auto& wout = windows[tId];
		wout.p.insert(wout.p.end(), prms.cbegin(), prms.cend());
		if (*op == ask) {
			windows[tId].first = op[4];
			windows[tId].last = op[5];
		}
		tmpWindows[wId].clear();
	}
	called[ent].insert(script);
}

void setWin::action() {
	u8 ent = stack.back().first;
	u8 script = stack.back().second;
	byteVec& scr = s[ent][script];
	for (auto op = scr.begin(); op < scr.end(); op += opSize(op)) {
		scriptCall(op);
		if (!called[ent].count(script))
			switch (*op) {
			case wspcl:
				if (op[2]) { // numeric or clock
					if (!tmpSpecial[op[1]].size()) // params override each other
						tmpSpecial[op[1]].push_back(op);
					else
						tmpWindows[op[1]].back() = op;
				} else // reset special window
					tmpWindows[op[1]].clear();
				continue;
			case wsizw:
			case fwindow:
				tmpWindows[op[1]].push_back(op);
				continue;
			}
		switch (*op) {
		case ifub: case ifubl: case setbyte: {
			auto const& var =
				find_if(qvars.cbegin(), qvars.cend(), [op](lineVar const& l) {
					return (l.bank == (op[1]&0xF0) >> 4 && l.address == op[2]);
				});
			/* Update question var cmps and sets,
			 only if var != 0, or 0 is an actual line no. */
			if (var != qvars.cend()/*  && (op[3] || var->first == 0) */)
				op[3] = var->last - (var->plast - op[3]);
			continue;
		}
		case message: case ask:
			break;
		default: continue;
		}
		u8 const wId = *op == ask ? op[2] : op[1];
		u8 const tId = *op == ask ? op[3] : op[2];
		if (windows[tId].p.size()) {
			for (auto& win : tmpWindows[wId]) {
				params& p = windows[tId].p.front();	
				if (p.w != null) copy(p.w, win + 6);
				else std::copy(win + 6, win + 8, reinterpret_cast<uint8_t*>(&p.w));
				if (p.center) {
					p.x = 160 - p.w / 2;
				}
				if (p.x != null) copy(p.x, win + 2);
				if (p.y != null) copy(p.y, win + 4);
				if (p.h != null) copy(ffString::pxHeight(p.h), win + 8);
				if (p.e != null) copy(p.e, win + 8);
				if (tmpSpecial[wId].size()) {
					bvit& sp = tmpSpecial[wId].back();
					if (p.sx != null) sp[3] = p.sx;
					if (p.sh != null) sp[4] = p.sh;
				}
				// Reuse last params in case they have not been provided
				if (windows[tId].p.size() > 1)
					windows[tId].p.erase(windows[tId].p.begin());
			}
		}
		if (*op == ask && windows[tId].first != null) {
			// Set vars for updating if necessary
			qvars.push_back({op[1], op[6], static_cast<u8>(windows[tId].first),
				static_cast<u8>(windows[tId].last), op[5]});
			op[4] = windows[tId].first;
			op[5] = windows[tId].last;
		}
		tmpWindows[wId].clear();
		tmpSpecial[wId].clear();
	}
	called[ent].insert(script);
}

void windowPatch::action() {
	byteVec& scr = s[stack.back().first][stack.back().second];
	vec16 labels = getLabels(scr);
	for (auto op = scr.cbegin(); op < scr.cend(); op += opSize(op)) {
		scriptCall(op);
		u8 wId = 0;
		switch (*op) {
		case wsizw:
		case fwindow:
			active[op[1]] = true;
			copy(scr, op + 2, windows[op[1]]);
			continue;
		case message:
			wId = op[1];
			break;
		case ask:
			wId = op[2];
			break;
		default: continue;
		}
		if (active[wId]) {
			active[wId] = false;
			continue;
		}
		u16 pos = distance(scr.cbegin(), op);
		u8* w = static_cast<u8*>(static_cast<void*>(&windows[wId]));
		scr.insert(scr.begin() + pos, fwindow);
		scr.insert(scr.begin() + pos + 1, wId);
		scr.insert(scr.begin() + pos + 2, w, w + sizeof(window));
		op = scr.begin() + pos + opSizes[fwindow];
		for (u16& label : labels)
			if (label > pos) label += opSizes[fwindow];
	}
	setLabels(scr, labels);
}

void autosize::action() {
	byteVec& scr = s[stack.back().first][stack.back().second];
	for (auto op = scr.begin(); op < scr.end(); op += opSize(op)) {
		scriptCall(op);
		switch (*op) {
		case wsizw:
		case fwindow:
			windows[op[1]].push_back(op);
			continue;
		case message:
			if (!windows[op[1]].size())
				continue;
			break;
		case ask:
			if (!windows[op[2]].size())
				continue;
			break;
		case wspcl:
			special[op[1]].clear();
			if (op[2])
				special[op[1]].push_back(op);
			continue;
		case wnumb:
			wn[op[2]] = op[7];
			continue;
		default: continue;
		}
		u8 const wId = *op == ask ? op[2] : op[1];
		u8 const tId = *op == ask ? op[3] : op[2];
		for (auto win : windows[wId]) {
			u16 w = ffString::width(s.getText()[tId], sp);
			u16 h = ffString::height(s.getText()[tId]);
			if (special[wId].size()) {
				auto& spParms = special[wId].back();
				w = (spParms[2] == 1 ? 72 : wn[wId] * 16) + 8;
				h = sp.back() == 0x0E ? 38 : 29;
				spParms[3] = 4;
				spParms[4] = 4;
			}
			copy(w, win + 6);
			copy(h, win + 8);
		}
		windows[wId].clear();
		special[wId].clear();
	}
}

vec16 getLabels(byteVec const& scr) {
	vec16 labels;
	for (auto op = scr.cbegin(); op < scr.cend(); op += opSize(op)) {
		u8 jp = 0;
		switch (*op) {
		case jmpf: case jmpb:
		case jmpfl: case jmpbl:
			jp = 1;
			break;
		case ifub: case ifubl:
			jp = 5;
			break;
		case ifsw: case ifuw:
		case ifswl: case ifuwl:
			jp = 7;
			break;
		case ifkey: case ifkeyon: case ifkeyoff:
			jp = 3;
			break;
		case ifprtyq: case ifmembq:
			jp = 2;
			break;
		default: continue;
		}
		u16 const jmp = isLong(*op) ?
			op[jp] | (op[jp+1] << 8) :
			op[jp];
		labels.push_back(distance(scr.cbegin(), op));
		if (*op == jmpb || *op == jmpbl)
			labels.back() -= jmp;
		else if (*op == jmpf || *op == jmpfl)
			labels.back() += jmp + sizeof *op;
		else
			labels.back() += jmp + jp;
	}
	return labels;
}

void setLabels(byteVec& scr, vec16& labels) {
	auto label = labels.cbegin();
	for (auto op = scr.begin(); op < scr.end(); op += opSize(op)) {
		u8 jp;
		switch (*op) {
		case jmpf: case jmpfl:
		case jmpb: case jmpbl:
			jp = 1;
			break;
		case ifub: case ifubl:
			jp = 5;
			break;
		case ifsw: case ifuw:
		case ifswl: case ifuwl:
			jp = 7;
			break;
		case ifkey: case ifkeyon: case ifkeyoff:
			jp = 3;
			break;
		case ifprtyq: case ifmembq:
			jp = 2;
			break;
		default: continue;
		}
		u16 pos, jmp;
		if ((*op == jmpb || *op == jmpbl)) {
			pos = distance(scr.begin(), op);
			jmp = pos - *label;
		} else {
			pos = distance(scr.begin(), op + jp);
			jmp = *label - pos;
		}
		if (isShort(*op) && jmp > 0xFF) {
			*op += 0x01; // change opcode to long version
			scr.insert(op + jp + sizeof *op, 0x00);
			for (u16& label: labels)
				if (label > pos) label += sizeof(u8);
			return setLabels(scr, labels); // Start from top as jumps may need recalculating
		}
		op[jp] = jmp & 0x00FF;
		if (isLong(*op))
			op[jp+1] = (jmp & 0xFF00) >> 8;
		++label;
	}
}

bool isJmp(u8 const op) {
	return (op == jmpf || op == jmpfl || op == jmpb ||
		op == jmpbl || op == ifub || op == ifubl ||
		op == ifsw || op == ifuw || op == ifswl ||
		op == ifuwl || op == ifkey || op == ifkeyon ||
		op == ifkeyoff || op == ifprtyq || op == ifmembq);
}

inline bool isLong(u8 const op) {
	return (op == jmpfl || op == jmpbl || op == ifubl
		|| op == ifswl || op == ifuwl);
}
inline bool isShort(u8 const op) {
	return (op == jmpf || op == jmpb || op == ifub
		|| op == ifsw || op == ifuw);
}

void parseFunc::parseCalled() {
	for (u8 ent = 0; ent < s.size(); ++ent) {
		if(s[ent][0].size()) call(ent, 0);
		this->inter();
	}
	for (u8 const& loc : s.getLocs())
		for (int i = 1; i < 7; ++i) {
			if(s[loc][i].size()) call(loc, i); // Line related scripts
			this->inter();
		}
	for (u8 const& mod : s.getMods()) {
		if(s[mod][1].size()) call(mod, 1); // Talk
		if(s[mod][2].size()) call(mod, 2); // Contact
		this->inter();
	}
}

void parseFunc::parseAll() {
	for (u8 ent = 0; ent < s.size(); ++ent)
		for (u8 script = 0; script < 32; ++script)
			if (s[ent][script].size()) {
				stack.push_back({ent, script});
				this->action();
				stack.pop_back();
			}
}

inline void parseFunc::scriptCall(bvCit const op) {
	switch (*op) {
	case req: case reqsw: case reqew:
		call(op[1], op[2] & 0x1F);
		break;
	case preq: case prqsw: case prqew:
		for (u8 const& pc : s.getPcs())
			call(pc, op[2] & 0x1F);
		break;
	case retto:
		call(stack.back().first, op[1] & 0x1F);
		break;
	}
}

void parseFunc::call(u8 ent, u8 script) {
	while (!s[ent].at(script).size()) { // Rewind to available script
		if (!script) {
			--ent;
			script = 31;
		} else
			--script;
	}
	auto entScr = std::make_pair(ent, script);
	if (count(stack.cbegin(), stack.cend(), entScr))
		return;	// Already on stack, possible loop
	stack.push_back(entScr);
	this->action();
	stack.pop_back();
}

u8 opSize(bvCit const op) {
	u8 opT;
	switch (*op) {
	case kawai:
		return op[1];
	case special:
		opT = op[1] - arrow;
		if (!spOpSizes.at(opT)) break;
		return spOpSizes[opT];
	default:
		opT = *op;
		if (!opSizes[opT]) break;
		return opSizes[opT];
	}
	std::ostringstream err;
	err << "Unknown (special) opcode: "
		<< std::hex << opT;
	throw runErr(err.str());
}

byteVec const opSizes({
	1, 3, 3, 3, 3, 3, 3, 2, 2, 15, 6, 6, 0, 0, 2, 0,
	2, 3, 2, 3, 6, 7, 8, 9, 8, 9, 10, 3, 0, 0, 0, 0,
	11, 2, 5, 3, 3, 9, 2, 2, 0, 1, 2, 2, 5, 7, 2, 10,
	4, 4, 4, 2, 2, 4, 5, 8, 6, 6, 6, 4, 1, 1, 1, 1,
	3, 5, 6, 2, 0, 5, 0, 5, 7, 4, 2, 2, 0, 5, 0, 5,
	10,	6, 4, 2, 2, 3, 7, 7, 5, 5, 5, 7, 8, 10, 8, 1,
	10,	2, 5, 6, 6, 1, 9, 1, 9, 2, 7, 9, 1, 4, 3, 6,
	4, 2,	3, 4, 4, 8, 4, 5, 4, 5, 3, 3, 3, 3, 2, 3,
	4, 5, 4, 4, 4, 4, 5, 4, 5, 4, 5, 4, 5, 4, 5, 4,
	5, 4, 5, 4,	5, 3, 3, 3, 3, 3, 4, 5, 6, 7, 7, 11,
	2, 2, 3, 3, 2, 11, 9, 9, 6, 6, 2, 4, 1, 6, 3, 3,
	5, 5, 4, 3, 6, 6,	2, 4, 5, 4, 3, 5, 5, 4, 0, 2,
	11,	8, 15, 12, 1, 3, 3, 2, 2, 2, 4, 3, 3, 3, 2, 2,
	13,	2, 2, 16, 10, 10,	4, 4, 3, 1, 15, 2, 4, 1, 1, 11,
	4, 4, 3, 3, 3, 5, 5, 5, 7, 10, 10, 5, 5, 8, 8, 11,
	2, 5, 14, 2, 2, 2, 2, 4, 2, 1, 3, 2, 2, 8, 3, 1
});

byteVec const spOpSizes({
	3, 0, 0, 4, 2, 2, 3, 3, 4, 2, 2
});

}
}
