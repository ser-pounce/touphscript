#ifndef TOUPHSCRIPT_LGP
#define TOUPHSCRIPT_LGP

#include "common.h"

namespace ts {

class lgp {
public:
	u32 size() const { return toc.size(); }
	void close() { data.close(); }

protected:
	lgp(string const& s, std::fstream::openmode const mode): toc(), data(s, mode) { }
	virtual ~lgp() =0;
	lgp(lgp const&) =delete;
	void operator= (lgp const&) =delete;

	static string const hdtxt;
	static string const endtxt;
#pragma pack(push, 1)
	struct tocEnt {
		char name[20];
		u32 ofst;
		u8 unk;
		u16 confl;
	};
#pragma pack(pop)

	vector<tocEnt> toc;
	std::fstream data;
};

class ilgp : public lgp {
public:
	explicit ilgp(string const& s);
	byteVec operator[](u32 const i) { return read(toc.at(i)); }
	byteVec operator[](string const& name);
	string const getName (u32 const i) const {
		return string(toc.at(i).name, sizeof toc[i].name).c_str();
	}

private:
	byteVec read(tocEnt const& file);
};

class olgp : public lgp {
public:
	explicit olgp(string const& s, u32 const nFiles);
	void write(byteVec const& buf, string const& name);
	olgp& save();

private:
	u16 hash(tocEnt const& t) const;
	u8 hash(char const c) const;

	static u8 const nChars = 30;
	static u16 const hashTblSz = nChars * nChars;

	struct hashEnt {
		u16 ofst;
		u16 count;
	};
};

}

#endif
