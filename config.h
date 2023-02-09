#ifndef TOUPHSCRIPT_CONFIG
#define TOUPHSCRIPT_CONFIG

#include "common.h"

#include <map>

namespace ts {

class config {
public:
	config(): opReg(), nameReg(), pathReg(), spacingTable() { }
	config(std::string const& path, std::string const& FFpath);
	bool operator[](string const& key) const {
		return opReg.count(key) ? opReg.find(key)->second : false;
	}
	sVec const& names() const { return nameReg; }
	string const& path(string const& key) const {
		if (!pathReg.count(key)) throw runErr("Unknown path: " + key);
		return pathReg.find(key)->second;
	}

	byteVec getSpacing() const {
		byteVec newSpacing;
		for (auto i = 0; i < 256; ++i)
			// Up to 0x1F px for each char, each 0x20 adds + 1 px
			newSpacing.push_back((spacingTable[i] & 0x1F) + spacingTable[i] / 0x20);
		return newSpacing;
	}

	static int choice;
	static int tab;
	static int max;
	static int padding;
	static int rowH1;
	static int rowH2;

private:
	static sVec const keys;
	static sVec const nameKeys;
	static sVec const paths;

	std::map<string, bool> opReg;
	sVec nameReg;
	std::map<string, string> pathReg;
	byteVec spacingTable;
};

}

#endif
