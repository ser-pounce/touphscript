#ifndef TOUPHSCRIPT_GZIP
#define TOUPHSCRIPT_GZIP

#include "common.h"
#include "zlib.h"

namespace ts {
namespace gzip {

inline byteVec decompress(byteVec const& cmp, u32 const size) {
	byteVec unc(size);
	
	z_stream gzStrm;
	gzStrm.next_in = const_cast<u8*>(cmp.data());;
	gzStrm.avail_in = cmp.size();
	gzStrm.total_in = 0;
	
	gzStrm.next_out = unc.data();
	gzStrm.avail_out = unc.size();
	gzStrm.total_out = 0;	
	
	gzStrm.zalloc = nullptr;
	gzStrm.zfree = nullptr;
	gzStrm.opaque = nullptr;


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"	
	if (inflateInit2(&gzStrm, MAX_WBITS + 16)
		|| inflate(&gzStrm, Z_FINISH) != Z_STREAM_END
		|| inflateEnd(&gzStrm))
		throw runErr("gzip decompression error");
	return unc;
#pragma GCC diagnostic pop
}

inline byteVec compress(byteVec const& unc) {
	byteVec cmp(unc.size() * 1.5);
	
	z_stream gzStrm;
	gzStrm.next_in = const_cast<u8*>(unc.data());;
	gzStrm.avail_in = unc.size();
	gzStrm.total_in = 0;
	
	gzStrm.next_out = cmp.data();
	gzStrm.avail_out = cmp.size();
	gzStrm.total_out = 0;	
	
	gzStrm.zalloc = nullptr;
	gzStrm.zfree = nullptr;
	gzStrm.opaque = nullptr;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"	
	if (deflateInit2(&gzStrm, Z_BEST_COMPRESSION, Z_DEFLATED,
			MAX_WBITS + 16, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY)
		|| deflate(&gzStrm, Z_FINISH) != Z_STREAM_END
		|| deflateEnd(&gzStrm))
		throw runErr("gzip compression error");
#pragma GCC diagnostic pop

	cmp.resize(cmp.size() - gzStrm.avail_out);
	return cmp;
}

}
}

#endif
