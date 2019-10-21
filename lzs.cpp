#include "lzs.h"

namespace ts {
namespace lzs {

byteVec text_buf;
int match_position;
int match_length;
tree lson;
tree rson;
tree dad;

byteVec decompress(byteVec const& cmp) {
	get<u32>(cmp, cmp.cbegin());

	text_buf.assign(bufSize + maxMatch - 1, 0);
	int r = bufSize - maxMatch;
	u16 flags = 0;

	byteVec unc;
	unc.reserve(cmp.size() * 2);
	
	for (auto cmpIt = cmp.cbegin() + sizeof(u32); cmpIt < cmp.cend(); ++cmpIt) {
	
		if (!((flags >>= 1) & 0x0100))
			flags = *cmpIt++ | 0xFF00;
			
		if (flags & 0x0001) {
			if (cmpIt >= cmp.cend())
				return unc;
				
			unc.push_back(text_buf[r++] = *cmpIt);
			r &= (bufSize - 1);
			
		} else {
		
			u16 i = *cmpIt++;
			
			if (cmpIt >= cmp.cend())
				return unc;
				
			i |= (*cmpIt & 0xF0) << 4;
			u16 j = (*cmpIt & 0x0F) + minMatch;
			
			for (u16 k = 0; k <= j; k++) {
				unc.push_back(text_buf[r++] = text_buf[(i + k) & (bufSize - 1)]);
				r &= (bufSize - 1);
			}
		}
	}
	return unc;
}

byteVec compress(byteVec const& unc) {
	byteVec cmp(4);
	cmp.reserve(unc.size() * 1.2);
	auto uncIt = unc.cbegin();

	int len, last_match_length;
	u8 mask = 1;
	byteVec code_buf(17);
	auto bufIt = code_buf.begin() + 1;

	text_buf.assign(bufSize + maxMatch - 1, 0);
	match_position = 0;
	match_length = 0;
	lson.assign(bufSize + 1, 0);
	rson.assign(bufSize + 257, nil);
	dad.assign(bufSize + 1, nil);

	int s = 0;
	int r = bufSize - maxMatch;

	for (len = 0; len < maxMatch && uncIt < unc.end(); ++len)
		text_buf[r + len] = *uncIt++;
	for (int i = 1; i <= maxMatch; i++)
		insertNode(r - i);
	insertNode(r);
	do {
		if (match_length > len) match_length = len;
		if (match_length <= minMatch) {
			match_length = 1;
			code_buf[0] |= mask;
			*bufIt++ = text_buf[r];
		} else {
			*bufIt++ = match_position;
			*bufIt++ =
				(((match_position >> 4) & 0xF0) | (match_length - (minMatch + 1)));
		}
		if ((mask <<= 1) == 0) {
			for (int i = 0; i < bufIt - code_buf.begin(); i++)
				cmp.push_back(code_buf[i]);
			code_buf[0] = 0;
			bufIt = code_buf.begin() + 1;
			mask = 1;
		}
		last_match_length = match_length;
		int i;
		for (i = 0; i < last_match_length && uncIt < unc.end(); i++) {
			deleteNode(s);
			text_buf[s] = *uncIt++;
			if (s < maxMatch - 1) text_buf[s + bufSize] = text_buf[s];
			s = (s + 1) & (bufSize - 1);
			r = (r + 1) & (bufSize - 1);
			insertNode(r);
		}
		while (i++ < last_match_length) {
			deleteNode(s);
			s = (s + 1) & (bufSize - 1);
			r = (r + 1) & (bufSize - 1);
			if (--len) insertNode(r);
		}
	} while (len > 0);
	if (bufIt > code_buf.begin() + 1)
		for (int i = 0; i < bufIt - code_buf.begin(); i++)
			cmp.push_back(code_buf[i]);

	copy(static_cast<u32>(cmp.size() - 4), cmp.begin());
	return cmp;
}

inline void insertNode(int r) {
	int cmp = 1;
	auto key = text_buf.cbegin() + r;
	int p = bufSize + 1 + *key;

	rson[r] = lson[r] = nil;
	match_length = 0;

	for ( ; ; ) {
		if (cmp >= 0) {
			if (rson[p] != nil) p = rson[p];
			else {
				rson[p] = r;
				dad[r] = p;
				return;
			}
		} else {
			if (lson[p] != nil) p = lson[p];
			else {
				lson[p] = r;
				dad[r] = p;
				return;
			}
		}
		int i;
		for (i = 1; i < maxMatch; ++i)
			if ((cmp = key[i] - text_buf[p + i]) != 0) break;
		if (i > match_length) {
			match_position = p;
			if ((match_length = i) >= maxMatch)  break;
		}
	}
	dad[r] = dad[p];
	lson[r] = lson[p];
	rson[r] = rson[p];
	dad[lson[p]] = r;
	dad[rson[p]] = r;
	if (rson[dad[p]] == p) rson[dad[p]] = r;
	else lson[dad[p]] = r;
	dad[p] = nil;
}

inline void deleteNode(int p) {
	int  q;

	if (dad[p] == nil) return;
	if (rson[p] == nil) q = lson[p];
	else if (lson[p] == nil) q = rson[p];
	else {
		q = lson[p];
		if (rson[q] != nil) {
			do {
				q = rson[q];
			} while (rson[q] != nil);
			rson[dad[q]] = lson[q];
			dad[lson[q]] = dad[q];
			lson[q] = lson[p];
			dad[lson[p]] = q;
		}
		rson[q] = rson[p];
		dad[rson[p]] = q;
	}
	dad[q] = dad[p];
	if (rson[dad[p]] == p) rson[dad[p]] = q;
	else lson[dad[p]] = q;
	dad[p] = nil;
}

}
}
