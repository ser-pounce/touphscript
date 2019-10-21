#ifndef TOUPHSCRIPT_COMMON
#define TOUPHSCRIPT_COMMON

#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>

namespace ts {

using std::cin;
using std::clog;
using std::cout;
using std::istream;
using std::ostream;
using std::ifstream;
using std::ofstream;
using std::string;
using std::vector;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef vector<u8> byteVec;
typedef byteVec::iterator bvit;
typedef byteVec::const_iterator bvCit;

typedef vector<byteVec> byteVec2d;
typedef byteVec2d::iterator bv2dIt;

typedef vector<u16> vec16;
typedef vector<u32> vec32;

typedef vector<string> sVec;
typedef string::const_iterator sCit;

typedef std::runtime_error runErr;

inline string& ltrim(string& s) {
	s.erase(s.begin(), find_if(s.begin(), s.end(),
		std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

inline string& rtrim(string& s) {
	s.erase(find_if(s.rbegin(), s.rend(),
		std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}

inline string& trim(string& s) {
	return ltrim(rtrim(s));
}

inline void removeBOM(ifstream& f) {
	if (f.peek() == 0xEF) {
		f.get();
		if (f.peek() == 0xBB) {
			f.get();
			if (f.peek() == 0xBF)
				f.get();
		}
	}
}

template<typename T>
inline typename vector<T>::size_type vsize(vector<T> const& v) {
	return v.size() * sizeof(T);
}
template<typename T>
inline typename vector<T>::size_type vsize(vector<vector<T>> const& v) {
	typename vector<T>::size_type sz = 0;
	for (vector<T> const& vec : v)
		sz += vsize(vec);
	return sz;
}

inline bvCit copy(byteVec const& in, bvCit const pos, size_t const size, void* const out) {
	if (pos + size > in.cend()) {
		std::ostringstream err;
		err << "Attempted to read " << std::hex << size << " bytes from "
				<< distance(in.cbegin(), pos) << '\n';
		throw runErr(err.str());
	}
	std::copy(pos, pos + size, static_cast<u8*>(out));
	return pos + size;
}
inline bvit copy(void const* const in, size_t const size, bvit out) {
	u8 const* const p = static_cast<u8 const*>(in);
	std::copy(p, p + size, out);
	return out += size;
}
template<typename T>
inline bvCit copy(byteVec const& in, bvCit const pos, T& out) {
	return copy(in, pos, sizeof(T), &out);
}
template<typename T>
inline bvCit copy(byteVec const& in, bvCit const pos, vector<T>& out) {
	return copy(in, pos, vsize(out), out.data());
}
template<typename T>
inline bvCit copy(byteVec const& in, bvCit pos, vector<vector<T>>& out) {
	for (vector<T>& v : out)
		pos = copy(in, pos, v);
	return pos;
}
template<typename T>
inline bvit copy(T const in, bvit out) {
	out = copy(&in, sizeof(T), out);
	return out;
}
template<typename T>
inline bvit copy(vector<T> const& in, bvit out) {
	out = copy(in.data(), vsize(in), out);
	return out;
}
template<typename T>
inline bvit copy(vector<vector<T>> const& in, bvit out) {
	for (vector<T> const& v : in)
		out = copy(v, out);
	return out;
}

template<typename T>
inline T get(byteVec const& in, bvCit pos) {
	T val;
	copy(in, pos, sizeof(T), &val);
	return val;
}

inline void read(istream& in, void* const p, int const size) {
	in.read(static_cast<char*>(p), size);
}
template<typename T>
inline void read(istream& in, T& out) {
	read(in, &out, sizeof(T));
}
template<typename T>
inline void read(istream& in, vector<T>& out) {
	read(in, out.data(), vsize(out));
}
template<typename T>
inline void read(istream& in, vector<vector<T>>& out) {
	for (vector<T>& v : out)
		read(in, v);
}

template<typename T>
inline T get(istream& in) {
	T val;
	read(in, &val, sizeof(T));
	return val;
}

inline void write(ostream& out, void const* const p, int const size) {
	out.write(static_cast<char const*>(p), size);
}
template<typename T>
inline void write(ostream& out, T const& in) {
	write(out, &in, sizeof(T));
}
template<typename T>
inline void write(ostream& out, vector<T> const& in) {
	write(out, in.data(), vsize(in));
}
template<typename T>
inline void write(ostream& in, vector<vector<T>>& out) {
	for (vector<T> const& v : out)
		write(in, v);
}

}

#endif
