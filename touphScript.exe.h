#ifndef TOUPHSCRIPT
#define TOUPHSCRIPT

#include "config.h"
#include "script.h"
#include <windows.h>

int main(int argc, char* argv[]);

namespace ts {

extern config conf;
extern string const delimiter;
extern byteVec spacing;

void dumpFlevel();
void encodeFlevel();
void readText(script::section0& s, string const& name);
void writeText(script::section0& s, string const& name);

void dumpTutorial(string const& name, byteVec const& buf);
void encodeTutorial(string const& name, byteVec& buf);

void dumpWorld();
void encodeWorld();

void dumpScene();
void encodeScene();

void dumpKernel();
void encodeKernel();

void dumpKernel2();
void encodeKernel2();

void dumpExe();
void encodeExe();

void vecPatch(byteVec& vec);
void insert(byteVec& s, u32 const pos, byteVec&& ins);
void erase(byteVec& s, u32 const pos, script::opcodes const op);
void setJmptmp(u8& op);

byteVec getSpacingTable();

string const getTempFile();
string const getFFPath();
void makeDir(string const& dir);
void copyFile(string const& where, string const& to);

class tmpFile {
public:
	tmpFile(): tmp(getTempFile()) { }
	~tmpFile() { DeleteFile(tmp.c_str()); }
	operator string() const { return tmp; }

private:
	tmpFile(tmpFile const&) =delete;
	void operator= (tmpFile const&) =delete;

	string tmp;
};

class logger {
public:
	logger():
		bk(clog.rdbuf()), out("touphScript.log", out.out | out.trunc) {
			out.exceptions(out.failbit | out.badbit | out.eofbit);
			clog.rdbuf(out.rdbuf());
		}
	~logger() {
		clog.rdbuf(bk);
		if (out.tellp()) {
			out.close();
			//ShellExecute(nullptr, "open", "log.txt", nullptr, nullptr, SW_SHOW);
		} else
			out.close();
	}
private:
	logger(logger const&) =delete;
	void operator= (logger const&) =delete;

	std::streambuf* const bk;
	ofstream out;
};

enum vechashes : DWORD {
	loslake2 = 0x5CC26D0F, blue_1 = 0x1D4027B6, shpin_22 = 0xF13E7598, junpb_tut = 0x7769423D
};

enum scriptHashes : DWORD {
	ealin_2 = 0xD46FE046, chrin_2 = 0xD90FF69B,  kuro_4 = 0x4D47F168,   goson = 0xE0A456C6,
	gongaga = 0x5C805E5D, elmin4_2 = 0x68D8E65A, psdun_2 = 0xC1945F77,  cosin1_1 = 0xB19C4C37,
	games_2 = 0xA2E29B8F, mtcrl_9 = 0xB116B636,  blin67_2 = 0x70D7746E, elminn_1 = 0xE6A5CD4E,
	min51_1 = 0x2AE2D829, lastmap = 0x2D1B2506,  frcyo = 0x5D25806B,    fship_4 = 0xEF5F4A27
};

}

#endif
