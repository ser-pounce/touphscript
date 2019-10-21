#ifndef TOUPHSCRIPT_LZS
#define TOUPHSCRIPT_LZS

#include "common.h"

namespace ts {
namespace lzs {

typedef vector<int> tree;

byteVec decompress(byteVec const& cmp);
byteVec compress(byteVec const& unc);

void insertNode(int r);
void deleteNode(int p);

int const bufSize = 4096;
int const maxMatch = 18;
int const minMatch = 2;
int const trees = 256;
int const nil = bufSize;

extern byteVec text_buf;
extern int match_position;
extern int match_length;
extern tree lson;
extern tree rson;
extern tree dad;

}
}

#endif
