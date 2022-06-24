#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "DiskIO.h"

struct uid {
	unsigned id;

	explicit uid(unsigned id = 0) : id(id) {};
	explicit operator unsigned() { return id; };
	uid operator+(int i) { return uid(id + i); }
};

bool operator==(const uid& UID1, const uid& UID2);

struct gid {
	unsigned id;

	explicit gid(unsigned id = 0) : id(id) {};
	explicit operator unsigned() { return id; };
	gid operator+(int i) { return gid(id + i); }
};

struct TableDescriptor {
	size_t address;
	size_t size;
};

template <class T>
struct TItem {
	T id;
	std::string name;

	virtual size_t read(std::fstream& disk) = 0;
	virtual void wright(std::fstream& disk, size_t padding) = 0;
	virtual size_t size() = 0;
};

struct UTItem : public TItem<uid>{
	std::vector<gid> groups;

	// in file:
	//
	// uid - id
	// size_t - user's groups size
	// gid - gid
	// ...
	// gid - gid
	// size_t - size with padding
	// chars - string

	size_t read(std::fstream& disk);
	void wright(std::fstream& disk, size_t padding);
	size_t size() {
		return sizeof(uid) + sizeof(size_t) + groups.size() * sizeof(gid) + sizeof(size_t) + name.size();
	}
};

struct GTItem : public TItem<gid> {

	// in file:
	//
	// gid - id
	// size_t - size with padding
	// chars - string

	size_t read(std::fstream& disk);
	void wright(std::fstream& disk, size_t padding);
	size_t size() {
		return sizeof(gid) + sizeof(size_t) + name.size();
	}
};



//std::fstream& operator<<(std::fstream& out, const size_t& T);
//std::fstream& operator>>(std::fstream& in, size_t& T);
//std::fstream& operator<<(std::fstream& out, const time_t& T);
//std::fstream& operator>>(std::fstream& in, time_t& T);
//std::fstream& operator<<(std::fstream& out, const int& T);
//std::fstream& operator>>(std::fstream& in, int& T);
//std::fstream& operator<<(std::fstream& out, const bool& T);
//std::fstream& operator>>(std::fstream& in, bool& T);

//inline std::fstream& operator<<(std::fstream& out, const size_t& T) {
//	out.write((const char*)&T, sizeof(size_t));
//	return out;
//}
//inline std::fstream& operator>>(std::fstream& in, size_t& T) {
//	in.read((char*)&T, sizeof(size_t));
//	return in;
//}
//inline std::fstream& operator<<(std::fstream& out, const time_t& T) {
//	out.write((const char*)&T, sizeof(time_t));
//	return out;
//}
//inline std::fstream& operator>>(std::fstream& in, time_t& T) {
//	in.read((char*)&T, sizeof(time_t));
//	return in;
//}
//inline std::fstream& operator<<(std::fstream& out, const int& T) {
//	out.write((const char*)&T, sizeof(int));
//	return out;
//}
//inline std::fstream& operator>>(std::fstream& in, int& T) {
//	in.read((char*)&T, sizeof(int));
//	return in;
//}
//inline std::fstream& operator<<(std::fstream& out, const bool& T) {
//	out.write((const char*)&T, sizeof(bool));
//	return out;
//}
//inline std::fstream& operator>>(std::fstream& in, bool& T) {
//	in.read((char*)&T, sizeof(bool));
//	return in;
//}

