#ifndef TOUPHSCRIPT_SCENE
#define TOUPHSCRIPT_SCENE

#include "common.h"

namespace ts {
namespace scene {

u16 const count = 256;
u8 const nofsts = 16;
u16 const uncSz = 0x1E80;
u16 const blocksz = 0x2000;
u32 const nullPtr = 0xFFFFFFFF;

byteVec2d toScene(string const& s);
byteVec toVec(byteVec2d const& scene);

class sceneFile {
public:
	sceneFile(byteVec const& in);
	byteVec2d getText() const;
	void setText(byteVec2d& text);
	byteVec save() const;

private:
	typedef vector<byteVec2d> scripts;

	void setScript(scripts& s, byteVec const& in);
	void getScriptText(scripts const& s, byteVec2d& text) const;
	void setScriptText(scripts& s, bv2dIt txtIt);
	vec16 getLabels(byteVec const& scr) const;
	void setLabels(byteVec& scr, vec16 const& labels);
	byteVec saveScript(scripts const& s) const;

	enum txtOfsts {
		enemy1 = 0x298, enemy2 = 0x350, enemy3 = 0x408, moves = 0x880,
		fScript = 0xC80, eScript = 0xE80
	};
	enum opcodes {
		jmp0 = 0x70, jmpne, jmp, end, fftext = 0x93, debug = 0xA0
	};
	static byteVec const oplen;
	static u16 const nullPtr = 0xFFFF;

	byteVec buf;

	scripts fScripts;
	scripts eScripts;
};

}
}

#endif
