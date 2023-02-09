#ifndef TOUPHSCRIPT_SCRIPT
#define TOUPHSCRIPT_SCRIPT

#include "common.h"
#include <map>
#include <set>

namespace ts {
namespace script {

class section0 {
	typedef std::set<u8> entSet;
public:
	explicit section0(byteVec const& buf);
	byteVec2d const& operator[](u8 const i) const { return (*this)[i]; }
	byteVec2d& operator[](u8 const i) {
		if (i < scripts.size())
			return scripts[i];
		throw runErr{"invalid script index: " + std::to_string(i)};
	}
	byteVec getScript() const;
	entSet const& getPcs() const { return pcs; }
	entSet const& getMods() const { return mods; }
	entSet const& getLocs() const { return locs; }
	byteVec2d const& getText() const { return text; }
	void setText(byteVec2d const& t) { text = t; }
	void setText(int const i, byteVec const& v) { text[i] = v; }
	u8 size() const { return scripts.size(); }
	byteVec save() const;

private:
	void init();
	u16 scriptOfst() const;
	u16 textOfst() const;
	u32 akaoOfst() const;
	u32 intSize() const;

	static u8 const nmSz = 8;
	static u8 const nScr = 32;
	// count, one pointer and single 0xFF
	static u8 const minTblSz = sizeof(u16) * 2 + sizeof(u8);

#pragma pack(push, 1)
	struct header {
		u16 unk1;
    u8 nEnts;
    u8 nMods;
    u16 strOfst;
    u16 akOfsts;
    u16 scale;
    u16 unk2[3];
    char szCreator[nmSz];
    char szName[nmSz];
	} hdr;
#pragma pack(pop)

	byteVec2d entNms;
	byteVec2d akaoBlks;
	vector<byteVec2d> scripts;
	byteVec2d text;
	entSet pcs;
	entSet mods;
	entSet locs;
};

u8 opSize(bvCit const op);

enum opcodes : u8 {
	ret, req, reqsw, reqew, preq, prqsw, prqew, retto, join, fsplit,
	sptye, gtpye, dskcg = 0x0E, special, jmpf, jmpfl, jmpb, jmpbl, ifub,
	ifubl, ifsw, ifswl, ifuw, ifuwl, minigame = 0x20, tutor, btmd2, btrld,
	wait, nfade, blink, bgmovie, kawai, kawiw, pmova, slip, bgpdh, bgscr,
	wcls, wsizw, ifkey, ifkeyon, ifkeyoff, uc, pdira, ptura, wspcl, wnumb,
	sttim, goldu, goldd, chgld, hmpmax1, hmpmax2, mhmmx, hmpmax3, message,
	mpara, mpra2, mpnam, mpu = 0x45, mpd = 0x47, ask, menu, menu2, btltb,
	hpu = 0x4D, hpd = 0x4F, fwindow, wmove, wmode, wrest, wclse, wrow,
	gwcol, swcol, stitm, dlitm, ckitm, smtra, dmtra, cmtra, shake, nop,
	mapjump, scrlo, scrlc, scrla, scr2d, scrcc, scr2dc, scrlw, scr2dl,
	mpdsp, vwoft, fade, fadew, idlck, lstmp, scrlp, battle, btlon, btlmd,
	pgtdr, getpc, pxyzi, plusx, plus2x, minusx, minus2x, incx, inc2x, decx,
	dec2x, tlkon, rdmsd, setbyte, setword, biton, bitoff, bitxor, plus,
	plus2, minus, minus2, mul, mul2, fdiv, fdiv2, mod, mod2, fand, and2,
	ffor, or2, fxor, xor2, inc, inc2, dec, dec2, random, lbyte, hbyte,
	tbyte, setx, getx, searchx, pc, fchar, dfanm, anime1, visi, xyzi, xyi,
	xyz, move, cmove, mova, tura, animw, fmove, anime2, animx1, canim1,
	canmx1, msped, ffdir, turngen, turn, dira, getdir, getaxy, getai,
	animx2, canim2, canmx2, asped, cc = 0xBF, jump, axyzi, lader, ofst,
	ofstw, talkr, slidr, solid, prtyp, prtym, prtye, ifprtyq, ifmembq,
	mmbud, mmblk, mmbuk, line, linon, mpjpo, sline, fsin, fcos, tlkr2,
	sldr2, pmjmp, pmjmp2, akao2, fcfix, ccanm, animb, turnw, mppal, bgon,
	bgoff, bgrol, bgrol2, bgclr, stpal, ldpal, cppal, rtpal, adpal, mppal2,
	stpls, ldpls, cppal2, rtpal2, adpal2, music, sound, akao, musvt, musvm,
	mulck, bmusc, chmph, pmvie, movie, mvief, mvcam, fmusc, cmusc, chmst,
	gameover
};
enum spOpcodes : u8 {
	arrow = 0xF5, pname, gmspd, smspd, flmat, flitm, btlck, mvlck, spcnm,
	rsglb, clitm
};

extern byteVec const opSizes;
extern byteVec const spOpSizes;

vec16 getLabels(byteVec const& scr);
void setLabels(byteVec& scr, vec16& labels);
bool isLong(u8 const op);
bool isShort(u8 const op);
bool isJmp(u8 const op);

class parseFunc {
protected:
	typedef std::set<u8> entSet;

	parseFunc(section0& scr): s(scr), stack() { }
	virtual void action() =0;
	void parseCalled();
	void parseAll();
	void call(u8 ent, u8 script);
	void scriptCall(bvCit const op);
	virtual void inter() { }
	virtual ~parseFunc() { }

	section0& s;
	vector<std::pair<u8, u8>> stack;
};

class getTextIds : public parseFunc {
public:
	explicit getTextIds(section0& s): parseFunc(s), ids() {
		parseAll();
	}
	entSet ids;

protected:
	void action();
};

class textStrip : public parseFunc {
public:
	explicit textStrip(section0& s): parseFunc(s), get(s) {
		try {
			parseAll();
			strip();
		} catch (runErr const& e) {
			throw runErr(string("text strip failed, ") + e.what());
		}
	}
private:
	void action();
	void strip();

	getTextIds get;
};

class removeUnused : public parseFunc {
public:
	removeUnused(section0& scr): parseFunc(scr), called(s.size()) {
		try {
			parseCalled();
			removeOrphans();
		} catch (runErr const& e) {
			throw runErr(string("remove scripts failed, ") + e.what());
		}
	}
	void action();

private:
	void removeOrphans();

	std::vector<std::set<u8>> called;
};

class getWin : public parseFunc {
public:
	explicit getWin(section0& scr): parseFunc(scr), windows(),
	tmpWindows(4), called(scr.size()) {
		try {
			parseCalled();
		} catch (runErr const& e) {
			throw runErr(string("couldn't get windows from script, ") + e.what());
		}
	}

	static u16 const null = 0xFFFF;

	struct params {
		int16_t x, y;
		u16 w, h;
		u16 sx, sh;
		params(): x(null), y(null), w(null), h(null), sx(null), sh(null) { }
	};
	struct window {
		vector<params> p;
		u16 first, last;
		window(): p(), first(null), last(null) { }
	};
	std::map<u8, window> windows;

private:
	void action();
	void inter() { tmpWindows.assign(4, vector<params>()); }
	vector<vector<params>> tmpWindows;
	std::vector<std::set<u8>> called;
};

class setWin : public parseFunc {
public:
	struct params {
		int16_t x, y;
		u16 w, h, e;
		u16 sx, sh;
		bool center;
		params(): x(null), y(null), w(null), h(null), e(null), sx(null), sh(null), center(false) { }
	};
	struct window {
		vector<params> p;
		u16 first, last;
		window(): p(), first(null), last(null) { }
	};

	explicit setWin(section0& scr, std::map<u8, window> w):
	parseFunc(scr), windows(w), tmpWindows(4), tmpSpecial(4),
	qvars(), called(scr.size()) {
		try {
			parseCalled();
		} catch (runErr const& e) {
			throw runErr(string("set windows failed, ") + e.what());
		}
	}

	static u16 const null = 0xFFFF;

private:
	void action();
	void inter() {
		tmpWindows.assign(4, vector<bvit>());
		tmpSpecial.assign(4, vector<bvit>());
		qvars.clear();
	}
	struct lineVar {
		u8 bank, address, first, last, plast;
	};	
	std::map<u8, window> windows;
	vector<vector<bvit>> tmpWindows;
	vector<vector<bvit>> tmpSpecial;
	vector<lineVar> qvars;
	vector<std::set<u8>> called;
};

class windowPatch : public parseFunc {
public:
	explicit windowPatch(section0& scr):
	parseFunc(scr), added(), windows(4, {80, 80, 150, 57}), active(4) {
		try {
			parseCalled();
		} catch (runErr const& e) {
			throw runErr(string("window patch failed, ") + e.what());
		}
	}
	int added;
private:
	void action();
	void inter() { active.assign(4, false); }

	struct window {
		u16 x, y, w, h;
	};

	vector<window> windows;
	vector<bool> active;
};

class autosize : public parseFunc {
public:
	explicit autosize(section0& scr, byteVec const& spacing):
	parseFunc(scr), sp(spacing), windows(4), special(4), wn(4) {
		try {
			parseCalled();
		} catch (runErr const& e) {
			throw runErr(string("autosize failed, ") + e.what());
		}
	}

private:
	void action();
	void inter() {
		windows.assign(4, vector<bvit>());
		special.assign(4, vector<bvit>());
		wn.assign(4, 0);
	}

	byteVec const sp;
	vector<vector<bvit>> windows;
	vector<vector<bvit>> special;
	byteVec wn;
};

}
}

#endif
