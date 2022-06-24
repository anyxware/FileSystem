#pragma once

#include <string>
#include <list>
#include <array>
#include <map>
#include "../StaticLib/FileSystem.h"
#include <SFML/Graphics.hpp>

using namespace sf;

using cstr = const std::string&;
using str = std::string&;

void TextEditor(Editor& editor);

inline bool pwd(FileSystem& fs, cstr str_a, cstr str_b, str out) {
	fs.pwd(out);
	return true;
}

inline bool cd(FileSystem& fs, cstr str_a, cstr str_b, str out) {
	return fs.cd(str_a);
}

inline bool mkdir(FileSystem& fs, cstr str_a, cstr str_b, str out) {
	return fs.mkdir(str_a);
}

inline bool list(FileSystem& fs, cstr str_a, cstr str_b, str out) {
	return str_a == std::string() ? fs.list(out) : fs.list(out, str_a);
}

inline bool touch(FileSystem& fs, cstr str_a, cstr str_b, str out) {
	return fs.touch(str_a);
}

inline bool rm(FileSystem& fs, cstr str_a, cstr str_b, str out) {
	return fs.rm(str_a);
}

inline bool mv(FileSystem& fs, cstr str_a, cstr str_b, str out) {
	return fs.mv(str_a, str_b);
}

inline bool useradd(FileSystem& fs, cstr str_a, cstr str_b, str out) {
	return fs.useradd(str_a);
}

inline bool open(FileSystem& fs, cstr str_a, cstr str_b, str out) {
	try {
		Editor editor = fs.open(str_a);
		TextEditor(editor);
	}
	catch (std::exception& e) {
		return false;
	}
	return true;
}



using pfunc = bool(*)(FileSystem&, cstr, cstr, str);


class Menu
{
private:
	std::map<std::string, std::pair<int, pfunc>> Commands = { {"pwd", {0, pwd}}, {"cd", {1, cd}}, {"mkdir", {1, mkdir}}, {"ls", {1, list}}, {"touch", {1, touch}}, {"rm", {1, rm}}, {"mv", {2, mv}},  {"open", {1, open}}, {"useradd", {1, useradd}} };
	std::map<std::string, std::list<std::string>> Aliases;
public:
	bool isExist(); // 0 if eof, 1 if exist, 2 if doesn't
	std::string getDiskName();
	bool checkParam(const std::string& diskName, bool isExist);
	std::string login();
	std::string execute(FileSystem& fs, const std::string& unpreparedString);
	void makeAlias(std::stringstream& args);
};



//struct command {
//	virtual bool operator()(FileSystem& fs, cstr str_a = std::string(), cstr str_b = std::string()) const {}
//};
//
//struct pwd : command {
//	bool operator()(FileSystem& fs, cstr str_a = std::string(), cstr str_b = std::string()) const override {
//		fs.pwd();
//		return true;
//	}
//};

//struct cd : command {
//	bool operator()(FileSystem& fs, cstr str_a = std::string(), cstr str_b = std::string()) const override {
//		return fs.cd(str_a);
//	}
//};
//
//struct mkdir : command {
//	bool operator()(FileSystem& fs, cstr str_a = std::string(), cstr str_b = std::string()) const override {
//		return fs.mkdir(str_a);
//	}
//};
//
//struct list : command {
//	bool operator()(FileSystem& fs, cstr str_a = std::string(), cstr str_b = std::string()) const override {
//		return str_a == std::string() ? fs.list() : fs.list(str_a);
//	}
//};
//
//struct touch : command {
//	bool operator()(FileSystem& fs, cstr str_a = std::string(), cstr str_b = std::string()) const override {
//		return fs.touch(str_a);
//	}
//};
//
//struct rm : command {
//	bool operator()(FileSystem& fs, cstr str_a = std::string(), cstr str_b = std::string()) const override {
//		return fs.rm(str_a);
//	}
//};
//
//struct mv : command {
//	bool operator()(FileSystem& fs, cstr str_a = std::string(), cstr str_b = std::string()) const override {
//		return fs.mv(str_a, str_b);
//	}
//};
//
//struct cp : command {
//	bool operator()(FileSystem& fs, cstr str_a = std::string(), cstr str_b = std::string()) const override {
//		return fs.cp(str_a, str_b);
//	}
//};