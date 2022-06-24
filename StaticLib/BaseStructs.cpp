#include "BaseStructs.h"

bool operator==(const uid& UID1, const uid& UID2) { return UID1.id == UID2.id; }

size_t UTItem::read(std::fstream& disk) {
	disk >> id;
	size_t groupsSize;
	disk >> groupsSize;

	unsigned number = groupsSize / sizeof(gid);
	gid* buffer = new gid[number];
	for (unsigned i = 0; i < number; i++) {
		disk >> buffer[i];
	}
	groups.assign(buffer, buffer + number);
	delete[] buffer;

	size_t stringSize;
	disk >> stringSize;

	name.resize(stringSize / sizeof(char));
	disk >> name;

	return sizeof(uid) + sizeof(size_t) + groupsSize + sizeof(size_t) + stringSize;
}

void UTItem::wright(std::fstream& disk, size_t paddingSize) {
	disk << id;
	disk << groups.size() * sizeof(gid);
	for (gid GID : groups) {
		disk << GID;
	}
	disk << name.size() * sizeof(char) + paddingSize;
	disk << name;

	writePadding(disk, paddingSize);
}

size_t GTItem::read(std::fstream& disk) {
	disk >> id;
	size_t stringSize;
	disk >> stringSize;
	name.resize(stringSize / sizeof(char));
	disk >> name;

	return sizeof(size_t) + sizeof(size_t) + stringSize;
}

void GTItem::wright(std::fstream& disk, size_t paddingSize) {
	disk << id;
	disk << name.size() * sizeof(char) + paddingSize;
	disk << name;

	writePadding(disk, paddingSize);
}