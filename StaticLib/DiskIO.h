#pragma once

#include <string>
#include <fstream>

inline void writePadding(std::fstream& disk, size_t paddingSize) {
	if (paddingSize == 0) return;
	char* padding = new char[paddingSize / sizeof(char)]{ 0 };
	disk.write(padding, paddingSize);
	delete[] padding;
}

template <class T>
inline std::fstream& operator<<(std::fstream& out, const T& item) {
	out.write((const char*)&item, sizeof(T));
	return out;
}

template <class T>
inline std::fstream& operator>>(std::fstream& in, T& item) {
	in.read((char*)&item, sizeof(T));
	return in;
}

inline std::fstream& operator>>(std::fstream& in, std::string& str) {
	char* buffer = new char[str.size() + sizeof(char)]{ 0 };
	in.read(buffer, str.size());
	str = buffer;
	delete[] buffer;
	return in;
}

inline std::fstream& operator<<(std::fstream& out, const std::string& str) {
	char* buffer = new char[str.size()];
	str.copy(buffer, str.size());
	out.write((const char*)buffer, str.size());
	delete[] buffer;
	return out;
}