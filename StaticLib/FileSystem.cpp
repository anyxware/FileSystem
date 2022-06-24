#include "FileSystem.h"
#include <new>

#pragma warning(disable:4996) // for working with time

// FileSystem

void FileSystem::createEmptyFS() {
	TableDescriptor UTDescriptor, GTDescriptor, FMDescriptor;

	UTDescriptor = { BLOCK_SZ, 0 };
	GTDescriptor = { 2 * BLOCK_SZ, 0 };
	FMDescriptor = { 3 * BLOCK_SZ, 0 };
	size = 4 * BLOCK_SZ; // 3 blocks are used (for 3 sys tables)
	rootAddress = 0; // address of root dir

	Disk << UTDescriptor << GTDescriptor << FMDescriptor;
	Disk << size;
	Disk << rootAddress;

	Disk.seekg(0, Disk.beg);
}

void FileSystem::readFS() {
	TableDescriptor UTDescriptor, GTDescriptor, FMDescriptor;

	Disk >> UTDescriptor >> GTDescriptor >> FMDescriptor; // magic
	Disk >> size;
	Disk >> rootAddress;
	currentAddress = rootAddress; // mb user's own dir
	// read users

	Disk.seekg(UTDescriptor.address, Disk.beg);
	FreeMap.push_back(UTDescriptor.address);
	int i = 0;
	for (size_t curSize = 0; curSize < UTDescriptor.size;) {
		UTItem item;
		curSize += item.read(Disk);
		UserTable.push_back(item);

		if (curSize % TBLOCK_SZ == 0 && curSize < UTDescriptor.size) { // if it's an end of block
			size_t nextBlockAdress;
			Disk >> nextBlockAdress;
			Disk.seekg(nextBlockAdress, Disk.beg);
			FreeMap.push_back(nextBlockAdress);
		}
	}
	// read groups

	Disk.seekg(GTDescriptor.address, Disk.beg);
	FreeMap.push_back(GTDescriptor.address);
	for (size_t curSize = 0; curSize < GTDescriptor.size;) {
		GTItem item;
		curSize += item.read(Disk);
		GroupTable.push_back(item);

		if (curSize % TBLOCK_SZ == 0 && curSize < GTDescriptor.size) { // if it's an end of block
			size_t nextBlockAdress;
			Disk >> nextBlockAdress;
			Disk.seekg(nextBlockAdress, Disk.beg);
			FreeMap.push_back(nextBlockAdress);
		}
	}

	// read free map

	Disk.seekg(FMDescriptor.address, Disk.beg);
	FreeMap.push_back(FMDescriptor.address);
	for (size_t curSize = 0; curSize < FMDescriptor.size;) {
		size_t item;
		Disk >> item;
		FreeMap.push_back(item);

		curSize += sizeof(size_t); // increase by size of read data
		if (curSize % TBLOCK_SZ == 0 && curSize < FMDescriptor.size) { // if it's an end of block
			size_t nextBlockAdress;
			Disk >> nextBlockAdress;
			Disk.seekg(nextBlockAdress, Disk.beg);
			FreeMap.push_back(nextBlockAdress);
		}
	}
}

bool createImage(const std::string& diskName) {
	std::ofstream createFile(diskName);
	if (!createFile.is_open()) return false;
	createFile.close();
	return true;
}

bool FileSystem::start(const std::string& diskName, bool isExist)
{
	if (!isExist && !createImage(diskName)) return false; // read only

	Disk.open(diskName, std::ios::in | std::ios::out | std::ios::binary);
	if (!Disk.is_open()) return false;

	if (!isExist) createEmptyFS();
	readFS();
	if (!isExist) {
		useradd("root");
		mkdir("");
	}

	currentAddress = rootAddress;

	return true;
}

bool FileSystem::login(const std::string& userName)
{
	for (const auto& i : UserTable) {
		if (i.name == userName) {
			currentUID = i.id;
			return true;
		}
	}
	return false;
}

size_t FileSystem::getWriteAddress(){

	size_t writeAddress;

	if (FreeMap.size() != 0) {
		writeAddress = FreeMap.front();
		FreeMap.pop_front();
	}
	else {
		writeAddress = size;
		size += BLOCK_SZ;
	}

	return writeAddress;
}

void FileSystem::writeFS() {

	TableDescriptor UTDescriptor = { 0,0 }, GTDescriptor = { 0,0 }, FMDescriptor = { 0,0 };

	// write users

	UTDescriptor.address = getWriteAddress();
	Disk.seekg(UTDescriptor.address, Disk.beg);

	for (unsigned count = 0; count < UserTable.size(); ) {
		UTItem item = UserTable[count];

		size_t paddingSize = 0;
		size_t freeSpaceSize = TBLOCK_SZ - UTDescriptor.size % TBLOCK_SZ;

		if (count + 1 < UserTable.size() && freeSpaceSize - item.size() < UserTable[count + 1].size()) {
			paddingSize = freeSpaceSize - item.size();
		}

		item.wright(Disk, paddingSize);
		UTDescriptor.size += item.size() + paddingSize;

		++count;

		if (UTDescriptor.size % TBLOCK_SZ == 0 && count < UserTable.size()) {
			size_t nextBlockAddress = getWriteAddress();
			Disk << nextBlockAddress;
			Disk.seekg(nextBlockAddress, Disk.beg);
		}
	}

	// write groups

	GTDescriptor.address = getWriteAddress();
	Disk.seekg(GTDescriptor.address, Disk.beg);

	for (unsigned count = 0; count < GroupTable.size(); ) {
		GTItem item = GroupTable[count];

		size_t paddingSize = 0;
		size_t freeSpaceSize = TBLOCK_SZ - GTDescriptor.size % TBLOCK_SZ;

		if (count + 1 < GroupTable.size() && freeSpaceSize - item.size() < GroupTable[count + 1].size()) {
			paddingSize = freeSpaceSize - item.size();
		}

		item.wright(Disk, paddingSize);
		GTDescriptor.size += item.size() + paddingSize;

		++count;

		if (GTDescriptor.size % TBLOCK_SZ == 0 && count < GroupTable.size()) {
			size_t nextBlockAddress = getWriteAddress();
			Disk << nextBlockAddress;
			Disk.seekg(nextBlockAddress, Disk.beg);
		}
	}

	// write free map

	FMDescriptor.address = getWriteAddress();

	Disk.seekg(GTDescriptor.address, Disk.beg);
	for (;!FreeMap.empty();) {
		size_t item = FreeMap.front();
		FreeMap.pop_front();
		Disk << item;
		FMDescriptor.size += sizeof(size_t);
		
		if (FMDescriptor.size % TBLOCK_SZ == 0 && !FreeMap.empty()) {
			size_t nextBlockAddress = getWriteAddress();
			Disk << nextBlockAddress;
			Disk.seekg(nextBlockAddress, Disk.beg);
		}
	}

	Disk.seekg(0, Disk.beg);

	Disk << UTDescriptor << GTDescriptor << FMDescriptor;
	Disk << size;
	Disk << rootAddress;
}

void FileSystem::end() {
	writeFS();
	Disk.close();
}

bool FileSystem::useradd(const std::string& userName) { // false if user already exists
	for (UTItem item : UserTable) {
		if (item.name == userName) return false;
	}

	UTItem item;
	if (UserTable.size() == 0) {
		item.id = uid(0);
	}
	else {
		int lastIndex = UserTable.size() - 1;
		item.id = UserTable[lastIndex].id + 1;
	}
	item.name = userName;

	if (!groupadd(userName)) return false; // create first user's group
	UserTable.push_back(item);
	usermod(userName, userName); // add user to a first group

	return true;
}

bool FileSystem::groupadd(const std::string& groupName) { // false if group already exists
	for (GTItem item : GroupTable) {
		if (item.name == groupName) return false;
	}

	GTItem item;
	if (GroupTable.size() == 0) {
		item.id = gid(0);
	}
	else {
		int lastIndex = GroupTable.size() - 1;
		item.id = GroupTable[lastIndex].id + 1;
	}
	item.name = groupName;
	GroupTable.push_back(item);

	return true;
}

bool FileSystem::usermod(const std::string& userName, const std::string& groupName) { // false if haven't found user or group
	gid GID;
	bool found = false;
	for (GTItem item : GroupTable) {
		if (item.name == groupName) {
			GID = item.id;
			found = true;
			break;
		}
	}

	if (!found) return false;

	found = false;
	for (UTItem& item : UserTable) {
		if (item.name == userName) {
			item.groups.push_back(GID);
			found = true;
		}
	}

	return found;
}

void FileSystem::pwd(size_t parentAddress, std::string& out) {
	if (parentAddress == 0) return;
	Directory dir(parentAddress, *this);
	pwd(dir.getParent(), out);
	dir.name(out);
}

void FileSystem::recursivelyRemove(size_t fileAddress)
{
	const bool& DirType = true;

	Entry entry(fileAddress, *this);

	if (entry.type() == DirType) {
		Directory dir(fileAddress, *this);
		for (const auto& it : dir) {
			recursivelyRemove(it.second);
		}
	}

	entry.remove();
}

void FileSystem::pwd(std::string& out)
{
	Directory dir(currentAddress, *this);
	pwd(dir.getParent(), out);
	dir.name(out);
	std::cout << "\n";
	out += "\n";
}

bool FileSystem::cd1(const std::string& path, size_t address)
{
	if (path == "") {
		currentAddress = rootAddress;
		return true;
	}

	size_t curAddress = address == 0 ? currentAddress : address;
	size_t found = path.find_first_of("/");

	if (found == 0) { // absolute path
		std::string newPath = path.substr(found + 1, path.size() - found - 1);
		return cd(newPath, rootAddress);
	}

	std::string name;
	if(found == std::string::npos){
		name = path;
	}
	else {
		name = path.substr(0, found);
	}

	Directory dir(curAddress, *this);
	dir.open();
	size_t newAddress = dir.getFileAddress(name);
	if (name == ".." && address == rootAddress) newAddress = rootAddress;
	dir.close();

	if (newAddress == 0) {
		return false;
	}
	else if (found == std::string::npos || found == path.size() - 1) { // no slash or it in back of name
		Entry entry(newAddress, *this);
		if (!entry.type()) return false;
		currentAddress = newAddress;
		return true;
	}
	else {
		Entry entry(newAddress, *this);
		if (!entry.type()) return false;
		std::string newPath = path.substr(found + 1, path.size() - found - 1);
		return cd(newPath, newAddress);
	}
}

bool FileSystem::cd(const std::string& path, size_t address)
{
	if (path.size() == 0 && address != 0) {
		currentAddress = address;
		return true;
	}
	else if (path.size() == 0 && address == 0) {
		return false;
	}

	std::string newPath = path[path.size() - 1] == '/' ? path : (path + '/');
	size_t found = newPath.find_first_of('/');

	if (found == 0) { // absolute path
		cd(path.substr(1, path.size() - 1), rootAddress);
	}
	else {
		std::string name = newPath.substr(0, found);
		newPath = newPath.substr(found + 1, newPath.size() - found - 1);

		size_t curAddress = address == 0 ? currentAddress : address;

		Directory curDir(curAddress, *this);
		curDir.open();
		size_t newAddress = curDir.getFileAddress(name);
		curDir.close();

		if (newAddress == 0 && address == rootAddress)
			return cd(newPath, rootAddress);
		else if (newAddress == 0) {
			return false;
		}

		Entry entry(newAddress, *this);
		if (!entry.type()) {
			return false;
		}

		return cd(newPath, newAddress);
	}

	return true;
}

bool FileSystem::mkdir(const std::string& path) {

	std::string newPath;
	if (path.size() != 0 && path[path.size() - 1] == '/') {
		newPath = path.substr(0, path.size() - 1);
	}
	else {
		newPath = path;
	}

	size_t found = newPath.find_last_of('/');

	std::string name;
	size_t oldCurAddress = currentAddress;

	if (found == std::string::npos) {
		name = newPath;
	}
	else {
		name = newPath.substr(found + 1, newPath.size() - found - 1);
		newPath = newPath.substr(0, found);
		if (!cd(newPath))
			return false;
	}
	// create new dir descriptor

	INode inode;
	inode.createTime = time(nullptr);
	inode.modifyTime = inode.createTime;
	inode.UID = currentUID;
	for (UTItem i : UserTable) {
		if (i.id == inode.UID) {
			inode.GID = i.groups[0];
			break;
		}
	}
	bool mode[10] = { 1,1,1,1,1,0,1,1,0,1 }; //drwxr-xr-x
	std::copy(&(mode[0]), &(mode[9]), inode.mode);
	inode.size = 0;
	inode.address = getWriteAddress();
	inode.parentAddress = currentAddress;
	inode.name = name;

	// create dir

	Directory dir(inode, *this);

	if (currentAddress == 0) { // creating root dir
		rootAddress = inode.address;
		currentAddress = rootAddress;
	}
	else {
		Directory curDir(currentAddress, *this);
		curDir.open();
		curDir.addFile(inode.address);
		curDir.close();
	}

	currentAddress = oldCurAddress;
	return true;
}

bool FileSystem::list(std::string& out, const std::string& path)
{
	size_t oldCurAddress = currentAddress;
	if (!cd(path))
		return false;

	Directory dir(currentAddress, *this);
	dir.open();
	dir.list(out);
	dir.close();

	currentAddress = oldCurAddress;

	return true;
}

bool FileSystem::touch(const std::string& path)
{
	std::string newPath;
	if (path[path.size() - 1] == '/') {
		newPath = path.substr(0, path.size() - 1);
	}
	else {
		newPath = path;
	}

	size_t found = newPath.find_last_of('/');

	std::string name;
	size_t oldCurAddress = currentAddress;

	if (found == std::string::npos) {
		name = newPath;
	}
	else {
		name = newPath.substr(found + 1, newPath.size() - found - 1);
		newPath = newPath.substr(0, found);
		if (!cd(newPath))
			return false;
	}

	// create new file descriptor

	INode inode;
	inode.createTime = time(nullptr);
	inode.modifyTime = inode.createTime;
	inode.UID = currentUID;
	for (const auto& i : UserTable) {
		if (i.id == currentUID) {
			inode.GID = i.groups[0];
			break;
		}
	}
	bool mode[10] = { 0,1,1,0,1,1,0,1,0,0 }; //-rw-rw-r--
	std::copy(&(mode[0]), &(mode[9]), inode.mode);
	inode.size = 0;
	inode.address = getWriteAddress();
	inode.parentAddress = currentAddress;
	inode.name = name;

	// create file

	File file(inode, *this);

	Directory curDir(currentAddress, *this);
	curDir.open();
	curDir.addFile(inode.address);
	curDir.close();

	currentAddress = oldCurAddress;
	return true;
}

bool FileSystem::rm(const std::string& path)
{
	std::string newPath;
	if (path[path.size() - 1] == '/') {
		newPath = path.substr(0, path.size() - 1);
	}
	else {
		newPath = path;
	}

	size_t found = newPath.find_last_of('/');

	std::string name;
	size_t oldCurAddress = currentAddress;

	if (found == std::string::npos) {
		name = newPath;
	}
	else {
		name = newPath.substr(found + 1, newPath.size() - found - 1);
		newPath = newPath.substr(0, found);
		if (!cd(newPath))
			return false;
	}

	Directory curDir(currentAddress, *this);
	curDir.open();
	size_t fileAddress = curDir.getFileAddress(name);

	if (fileAddress == 0) {
		curDir.close();
		return false;
	}

	recursivelyRemove(fileAddress);
	
	curDir.rmvFile(fileAddress);
	curDir.close();

	currentAddress = oldCurAddress;
	return true;
}

bool FileSystem::mv(const std::string& path1, const std::string& path2)
{
	std::string newPath;
	if (path1[path1.size() - 1] == '/') {
		newPath = path1.substr(0, path1.size() - 1);
	}
	else {
		newPath = path1;
	}

	size_t found = newPath.find_last_of('/');

	std::string name;
	size_t oldCurAddress = currentAddress;

	if (found == std::string::npos) {
		name = newPath;
	}
	else {
		name = newPath.substr(found + 1, newPath.size() - found - 1);
		newPath = newPath.substr(0, found);
		if (!cd(newPath))
			return false;
	}

	Directory srcDir(currentAddress, *this);

	currentAddress = oldCurAddress;

	if (!cd(path2))
		return false;

	Directory dstDir(currentAddress, *this);

	currentAddress = oldCurAddress;

	srcDir.open();
	dstDir.open();

	size_t fileAddress1 = srcDir.getFileAddress(name);
	size_t fileAddress2 = dstDir.getFileAddress(name);

	if (fileAddress1 == 0 || fileAddress2 != 0)
		return false;

	srcDir.rmvFile(fileAddress1);
	dstDir.addFile(fileAddress1);

	srcDir.close();
	dstDir.close();

	return true;
}

Editor FileSystem::open(const std::string& path)
{
	std::string newPath;
	if (path[path.size() - 1] == '/') {
		newPath = path.substr(0, path.size() - 1);
	}
	else {
		newPath = path;
	}

	size_t found = newPath.find_last_of('/');

	std::string name;
	size_t oldCurAddress = currentAddress;

	if (found == std::string::npos) {
		name = newPath;
	}
	else {
		name = newPath.substr(found + 1, newPath.size() - found - 1);
		newPath = newPath.substr(0, found);
		if (!cd(newPath))
			throw std::invalid_argument("Invalid path");
	}

	Directory curDir(currentAddress, *this);
	curDir.open();
	size_t fileAddress = curDir.getFileAddress(name);
	if (fileAddress == 0) {
		curDir.close();
		currentAddress = oldCurAddress;
		throw std::invalid_argument("Invalid name");
	}
	
	Editor editor(fileAddress, *this);
	Entry& e = editor;
	if (e.type()) { // if it is directory
		curDir.close();
		currentAddress = oldCurAddress;
		throw std::invalid_argument("Invalid name");
	}

	curDir.close();

	currentAddress = oldCurAddress;

	return editor;
}

//bool FileSystem::cp(const std::string& path1, const std::string& path2)
//{
//	std::string newPath;
//	if (path1[path1.size() - 1] == '/') {
//		newPath = path1.substr(0, path1.size() - 1);
//	}
//	else {
//		newPath = path1;
//	}
//
//	size_t found = newPath.find_last_of('/');
//
//	std::string name;
//	size_t oldCurAddress = currentAddress;
//
//	if (found == std::string::npos) {
//		name = newPath;
//	}
//	else {
//		name = newPath.substr(found + 1, newPath.size() - found - 1);
//		newPath = newPath.substr(0, found);
//		if (!cd(newPath))
//			return false;
//	}
//
//	Directory srcDir(currentAddress, *this);
//
//	currentAddress = oldCurAddress;
//
//	if (!cd(path2))
//		return false;
//
//	Directory dstDir(currentAddress, *this);
//
//	currentAddress = oldCurAddress;
//
//	srcDir.open();
//	dstDir.open();
//
//	size_t fileAddress1 = srcDir.getFileAddress(name);
//	size_t fileAddress2 = dstDir.getFileAddress(name);
//
//	if (fileAddress1 == 0 || fileAddress2 != 0)
//		return false;
//
//	// recursively copy
//
//	dstDir.addFile(fileAddress1);
//
//	srcDir.close();
//	dstDir.close();
//
//	return true;
//}

bool FileSystem::read(const std::string& name, size_t size, size_t position)
{
	Directory curDir(currentAddress, *this);
	curDir.open();
	size_t fileAddress = curDir.getFileAddress(name);

	if (fileAddress == 0) {
		curDir.close();
		return false;
	}

	Entry entry(fileAddress, *this);
	if (entry.type()) {
		curDir.close();
		return true;
	}

	File file(fileAddress, *this);
	file.fseek(position);
	file.fread(size);

	file.fclose();
	curDir.close();
	return true;
}

bool FileSystem::write(const std::string& name, const std::string& text)
{
	Directory curDir(currentAddress, *this);
	curDir.open();
	size_t fileAddress = curDir.getFileAddress(name);

	if (fileAddress == 0) {
		curDir.close();
		return false;
	}

	Entry entry(fileAddress, *this);
	if (entry.type()) {
		curDir.close();
		return true;
	}

	File file(fileAddress, *this);
	file.fwrite(text.c_str(), text.size());

	file.fclose();
	curDir.close();
	return true;
}
