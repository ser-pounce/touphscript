#include "config.h"

namespace ts {

config::config(string const& file, string const& FFpath):
opReg(), nameReg(nameKeys.size(), "OOOOOOOOO"), pathReg(), spacingTable(256) {
	pathReg["text"]			= ".\\text\\";
	pathReg["flevel"]		= FFpath + "data\\field\\flevel.lgp";
	pathReg["world"]		= FFpath + "data\\wm\\world_us.lgp";
	pathReg["scene"]		= FFpath + "data\\battle\\scene.bin";
	pathReg["kernel"]		= FFpath + "data\\kernel\\KERNEL.BIN";
	pathReg["kernel2"]	    = FFpath + "data\\kernel\\kernel2.bin";
	pathReg["window"]		= FFpath + "data\\kernel\\WINDOW.BIN";
	pathReg["exe"]			= FFpath + "ff7.exe";
	pathReg["char_file"]	= "";

	ifstream f(file);
	if (!f.good()) {
		clog << ".ini not found / unreadable, options disabled\n";
		return;
	}
	removeBOM(f);
	string line;

	while (getline(f, line)) {
		line = string(line, 0, line.find('#')); // strip comments
		if (!trim(line).size())
			continue;

		string key(line, 0, line.find('='));
		trim(key);
		string value(line, line.find('=') + 1);
		trim(value);

		if (key == "font_spacing") {
			std::stringstream valstream(value);
			for (int i = 0; i < 256; ++i) {
				int j;
				valstream >> std::hex >> j;
				if (valstream)
					spacingTable[i] = j;
			}
			opReg["font_spacing"] = true;
		} else if (key == "max") {
			max = std::stoi(value);
			opReg["max"] = true;
		} else if (key == "choice") {
			choice = std::stoi(value);
			opReg["choice"] = true;
		} else if (key == "tab") {
			tab = std::stoi(value);
			opReg["tab"] = true;
		} else if (key == "box_padding") {
			padding = std::stoi(value);
			opReg["padding"] = true;
		} else if (find(keys.begin(), keys.end(), key) != keys.end()) {
			opReg[key] = (value == "1" || value == "true");
		} else if (find(paths.begin(), paths.end(), key) != paths.end()) {
			pathReg[key] = value;
		} else {			
			auto const name = find(nameKeys.cbegin(), nameKeys.cend(), key);			
			if (name != nameKeys.cend())
				nameReg[distance(nameKeys.cbegin(), name)] = value;			
			else
				clog << "Unknown ini key: " << key << '\n';
		}
	}
}

sVec const config::keys({
	"autosize_width", "autosize_height", "text_strip", "window_patch",
	"dump_x", "dump_y", "dump_w", "dump_h", "dump_sp", "dump_question", "script_strip", 
	"ealin_2", "chrin_2", "kuro_4", "goson", "elmin4_2", "psdun_2", 
	"gongaga", "cosin1_1", "games_2", "mtcrl_9", "elminn_1", "blin67_2", 
	"min51_1", "lastmap", "frcyo", "fship_4", "dump_flevel", "dump_world", "dump_scene",
	"dump_kernel", "dump_kernel2", "dump_exe"
});

sVec const config::nameKeys({
	"cloud", "barret", "tifa", "aerith", "redxiii", "yuffie", "caitsith",
	"vincent", "cid"
});

sVec const config::paths({
	"text", "flevel", "world", "scene", "kernel", "kernel2", "exe", "window", "char_file"
});


int config::choice;
int config::tab;
int config::max;
int config::padding;


}
