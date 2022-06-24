#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include "File.h"
#include "BaseStructs.h"
#include "DiskIO.h"

class Editor;

class FileSystem
{
	friend class Entry;
private:

	// FS Descriptors
	// write magic number on disk
	// check read only flag
	size_t size; // size of used blocks
	size_t rootAddress;

	static constexpr size_t BLOCK_SZ = 1024;
	static constexpr size_t TBLOCK_SZ = BLOCK_SZ - sizeof(size_t); // size of data in table block(last value is pointer to the next block)

	// Loaded info

	std::vector<UTItem> UserTable;
	std::vector<GTItem> GroupTable;
	std::list<size_t> FreeMap;

	// Current info

	std::fstream Disk;
	size_t currentAddress = 0;
	uid currentUID = uid(0);
protected:
	size_t getWriteAddress();
	void createEmptyFS();
	void readFS();
	void writeFS();
	void pwd(size_t parrentAddress, std::string& out);
	void recursivelyRemove(size_t fileAddress);
public:
	FileSystem() = default;
	virtual ~FileSystem() {};

	bool start(const std::string& diskName, bool isExist); // if false FS not valid
	bool login(const std::string& userName);
	void end();

	static size_t getBlockSize() { return BLOCK_SZ; }

	bool useradd(const std::string& userName);
	bool groupadd(const std::string& groupName);
	bool usermod(const std::string& userName, const std::string& groupName);

	void pwd(std::string& out);
	bool cd(const std::string& path, size_t address = 0);
	bool cd1(const std::string& path, size_t address = 0);
	bool mkdir(const std::string& path);
	bool list(std::string& out, const std::string& path = ".");
	bool touch(const std::string& path);
	bool rm(const std::string& path);
	bool mv(const std::string& path1, const std::string& path2);
	Editor open(const std::string& path);
	
	bool read(const std::string& name, size_t size = -1, size_t position = 0);
	bool write(const std::string& name, const std::string& text);

	//testing

	void show() {
		std::cout << UserTable.size() << "\n";
		for (auto item : UserTable) {
			std::cout << "user: " << item.name << " id: " << unsigned(item.id) << " groups: ";
			for (const auto& it : item.groups) {
				std::cout << it.id << ' ';
			}
			std::cout << "\n";

		}
		for (auto item : GroupTable) {
			std::cout << "group: " << item.name << " " << unsigned(item.id) << '\n';
		}
	}
};

