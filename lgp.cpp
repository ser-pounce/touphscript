#include "lgp.h"
#include <cstring>

namespace ts {

string const lgp::hdtxt("\0\0SQUARESOFT", 12);
string const lgp::endtxt = "FINAL FANTASY7";

lgp::~lgp() { }

ilgp::ilgp(string const& s): lgp(s, data.in | data.binary) {
	data.exceptions(data.failbit | data.badbit | data.eofbit);

	data.ignore(hdtxt.size());
	toc.resize(get<u32>(data));
	ts::read(data, toc);

	sort(toc.begin(), toc.end(), [](tocEnt const& a, tocEnt const& b) {
		return a.ofst < b.ofst;
	});
}

byteVec ilgp::read(tocEnt const& file) {
	data.seekg(file.ofst + sizeof file.name); // Skip name
	byteVec buf(get<u32>(data));
	ts::read(data, buf);
	return buf;
}

byteVec ilgp::operator[](string const& name) {
	auto const ent = find_if(toc.cbegin(), toc.cend(), [&name](tocEnt const& file) {
		return (!strncmp(file.name, name.c_str(), sizeof file.name));
	});
	if (ent >= toc.cend())
		throw runErr(name + "not found");
	return read(*ent);
}

olgp::olgp(string const& s, u32 const nFiles): lgp(s, data.out | data.binary) {
	data.exceptions(data.failbit | data.badbit | data.eofbit);

	data.write(hdtxt.data(), hdtxt.size());
	ts::write(data, nFiles);
	u32 const blank =
		sizeof(tocEnt) * nFiles
		+ sizeof(hashEnt) * hashTblSz
		+ sizeof(u16); // conflicts
	data.write(vector<char>(blank).data(), blank);
}

void olgp::write(byteVec const& buf, string const& name) {
	if (name.find('.') < 2)
		throw runErr("Incompatible filename");
	toc.push_back(tocEnt());
	tocEnt& ent = toc.back();

	ent.ofst = data.tellp();
	ent.unk = 0x0E;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
	strncpy(ent.name, name.c_str(), sizeof ent.name);
#pragma GCC diagnostic pop

	data.write(ent.name, sizeof ent.name);
	ts::write(data, static_cast<u32>(buf.size()));
	ts::write(data, buf);
}

inline u16 olgp::hash(tocEnt const& t) const {
	return (hash(t.name[0]) * nChars + hash(t.name[1]) + 1);
}

inline u8 olgp::hash(char const c) const {
	if (isdigit(c)) return c - '0';
	if (c == '_') return 'k' - 'a';
	if (c == '-') return 'l' - 'a';
	return tolower(c) - 'a';
}

olgp& olgp::save() {
	sort(toc.begin(), toc.end(), [this](tocEnt const& a, tocEnt const& b) {
		return (hash(a) < hash(b));
	});
	vector<hashEnt> table(hashTblSz);
	for (u16 i = 0; i < size(); ++i) {
		hashEnt& ent = table.at(hash(toc[i]));
		++ent.count;
		if (!ent.ofst) ent.ofst = i + 1;
	}
	data.seekp(hdtxt.size() + sizeof(u32));
	ts::write(data, toc);
	ts::write(data, table);
	data.seekp(0, data.end);
	data << endtxt;
	return *this;
}

}

