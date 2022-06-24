#include "File.h"

size_t INode::read(std::fstream& disk) {
	disk >> createTime;
	disk >> modifyTime;
	disk >> UID;
	disk >> GID;
	for (int i = 0; i < 10; ++i) {
		disk >> mode[i];
	}
	disk >> size;
	disk >> address;
	for (int i = 0; i < 8; i++) {
		disk >> externAddresses[i];
	}
	disk >> parentAddress;
	disk >> hash;
	size_t stringSize;
	disk >> stringSize;
	name.resize(stringSize / sizeof(char));
	disk >> name;

	return 2 * sizeof(time_t) + sizeof(uid) + sizeof(gid) + 10 * sizeof(bool) +
		13 * sizeof(size_t) + stringSize;
}

size_t INode::write(std::fstream& disk) {
	disk << createTime;
	disk << modifyTime;
	disk << UID;
	disk << GID;
	for (int i = 0; i < 10; ++i) {
		disk << mode[i];
	}
	disk << size;
	disk << address;
	for (int i = 0; i < 8; i++) {
		disk << externAddresses[i];
	}
	disk << parentAddress;
	disk << hash;

	size_t paddingSize = 0;
	size_t inodeSize = 2 * sizeof(time_t) + sizeof(uid) + sizeof(gid) + 10 * sizeof(bool) +
		13 * sizeof(size_t) + name.size() * sizeof(char);
	size_t freeSpaceSize = FileSystem::getBlockSize() - inodeSize;
	if (mode[0] && freeSpaceSize % sizeof(size_t) != 0) { // if it's dir
		paddingSize = freeSpaceSize % sizeof(size_t);
	}
	disk << name.size() * sizeof(char) + paddingSize;
	disk << name;

	writePadding(disk, paddingSize);

	return 2 * sizeof(time_t) + sizeof(uid) + sizeof(gid) + 10 * sizeof(bool) +
		12 * sizeof(size_t) + name.size() * sizeof(char) + paddingSize;
}



uid Entry::getCurrentUID()
{
	return fs.currentUID;
}

uid Entry::getOwnUID()
{
	return inode.UID;
}

size_t Entry::getMaxSize()
{
	return 7 * FileSystem::BLOCK_SZ + 2 * (FileSystem::BLOCK_SZ / sizeof(size_t)) * FileSystem::BLOCK_SZ;
}

size_t Entry::freeAddress(size_t curSize)
{
	const size_t& BS = BLOCK_SZ; // as an alias

	size_t retAddress;

	if (curSize < 7 * BS) {
		retAddress = inode.externAddresses[curSize / BS - 1];
	}
	else if (curSize  < 7 * BS + (BS / sizeof(size_t)) * BS) {
		size_t addressIndexInBlock = curSize / BS - 7;
		if(addressIndexInBlock == 0) putBlock(inode.externAddresses[6]);
		read(inode.externAddresses[6] + addressIndexInBlock * sizeof(size_t), (char*)&retAddress, sizeof(size_t));
	}
	else if (curSize < 7 * BS + 2 * (BS / sizeof(size_t)) * BS) {
		size_t addressIndexInBlock = curSize / BS - 7 - (BS / sizeof(size_t));
		if (addressIndexInBlock == 0) putBlock(inode.externAddresses[7]);
		read(inode.externAddresses[7] + addressIndexInBlock * sizeof(size_t), (char*)&retAddress, sizeof(size_t));
	}

	putBlock(retAddress);

	return retAddress;
}

size_t Entry::takeAddress(size_t curSize)
{
	const size_t& BS = BLOCK_SZ;

	size_t recAddress = getBlock();

	if (curSize < 7 * BS) {
		inode.externAddresses[curSize / BS - 1] = recAddress;
	}
	else if (curSize < 7 * BS + (BS / sizeof(size_t)) * BS) {
		size_t addressIndexInBlock = curSize / BS - 7;
		if (addressIndexInBlock == 0) inode.externAddresses[6] = getBlock();
		write(inode.externAddresses[6] + addressIndexInBlock * sizeof(size_t), (const char*)&recAddress, sizeof(size_t));
	}
	else if (curSize < 7 * BS + 2 * (BS / sizeof(size_t)) * BS) {
		size_t addressIndexInBlock = curSize / BS - 7 - (BS / sizeof(size_t));
		if (addressIndexInBlock == 0) inode.externAddresses[7] = getBlock();
		write(inode.externAddresses[7] + addressIndexInBlock * sizeof(size_t), (const char*)&recAddress, sizeof(size_t));
	}

	return recAddress;
}

size_t Entry::getAddress(size_t curSize)
{
	const size_t& BS = BLOCK_SZ; // as an alias

	size_t retAddress;

	if (curSize < 7 * BS) {
		retAddress = inode.externAddresses[curSize / BS - 1];
	}
	else if (curSize < 7 * BS + (BS / sizeof(size_t)) * BS) {
		size_t addressIndexInBlock = curSize / BS - 7;
		read(inode.externAddresses[6] + addressIndexInBlock * sizeof(size_t), (char*)&retAddress, sizeof(size_t));
	}
	else if (curSize < 7 * BS + 2 * (BS / sizeof(size_t)) * BS) {
		size_t addressIndexInBlock = curSize / BS - 7 - (BS / sizeof(size_t));
		read(inode.externAddresses[7] + addressIndexInBlock * sizeof(size_t), (char*)&retAddress, sizeof(size_t));
	}

	return retAddress;
}

void Entry::read(size_t address, char* buffer, size_t BUF_SZ)
{
	fs.Disk.seekg(address, fs.Disk.beg);
	fs.Disk.read(buffer, BUF_SZ);
}

void Entry::write(size_t address, const char* buffer, size_t BUF_SZ)
{
	fs.Disk.seekg(address, fs.Disk.beg);
	fs.Disk.write(buffer, BUF_SZ);
}

size_t Entry::getBlock()
{
	return fs.getWriteAddress();
}

void Entry::putBlock(size_t address)
{
	fs.FreeMap.push_back(address);
}

void Entry::writeInode()
{
	fs.Disk.seekg(inode.address, fs.Disk.beg);
	inode.write(fs.Disk);
}

Entry::Entry(const INode& inInode, FileSystem& fs) : inode(inInode), fs(fs) 
{
	fs.Disk.seekg(inode.address, fs.Disk.beg);
	inodeSize = inode.write(fs.Disk);
}

Entry::Entry(size_t inodeAddress, FileSystem& fs) : fs(fs)
{
	fs.Disk.seekg(inodeAddress, fs.Disk.beg);
	inodeSize = inode.read(fs.Disk);
}

void Entry::remove()
{
	const size_t& BS = BLOCK_SZ;
	putBlock(inode.address);
	for (size_t curSize = BS; curSize < inode.size; curSize += BS) {
		freeAddress(curSize);
	}
}

std::string Entry::readName(size_t fileAddress) {
	size_t nameSizeAddress = fileAddress + 2 * sizeof(time_t) + sizeof(uid) + sizeof(gid) + 10 * sizeof(bool) + 12 * sizeof(size_t);
	size_t nameAddress = nameSizeAddress + sizeof(size_t);

	size_t nameSize;
	read(nameSizeAddress, (char*)&nameSize, sizeof(size_t));

	char* name_buffer = new char[nameSize + 1]{ 0 };
	read(nameAddress, name_buffer, nameSize);
	std::string fileName = name_buffer;
	delete[] name_buffer;
	return fileName;
}


void Directory::open()
{

	opened = true;

	size_t inodeSize = getInodeSize();
	size_t startAddress = getAddress();
	size_t dirSize = getSize() + inodeSize;

	const size_t& BLOCK_SIZE = BLOCK_SZ;

	int count = 0;
	for (size_t curSize = inodeSize; curSize < dirSize;) {
		size_t fileAddress;
		read(startAddress + curSize % BLOCK_SIZE, (char*)&fileAddress, sizeof(size_t));
		std::string fileName = readName(fileAddress);

		table.insert(fileName, fileAddress);

		curSize += sizeof(size_t);

		if (curSize % BLOCK_SIZE == 0 && curSize < dirSize) {
			startAddress = freeAddress(curSize);
		}
	}

	table.insert(".", getAddress());
	table.insert("..", getParent());
}

void Directory::close()
{
	opened = false;

	size_t inodeSize = getInodeSize();
	size_t startAddress = getAddress();
	size_t dirSize = getSize() + inodeSize;

	const size_t& BLOCK_SIZE = BLOCK_SZ;

	auto it = table.begin();

	for (size_t curSize = inodeSize; curSize < dirSize; ) {
		if ((*it).first == "." || (*it).first == "..") {
			++it;
			continue;
		}

		write(startAddress + curSize % BLOCK_SIZE, (const char*)&((*it).second), sizeof(size_t));

		curSize += sizeof(size_t);

		if (curSize % BLOCK_SIZE == 0 && curSize < dirSize) {
			startAddress = takeAddress(curSize);
		}

		++it;
	}

	writeInode();
}

void Directory::list(std::string& out)
{
	if (!opened) return;

	for (auto it : table) {
		std::cout << it.first << ' ';
		out += it.first + " ";
	}
	std::cout << "\n";
	out += "\n";
}

size_t Directory::getFileAddress(const std::string& fileName)
{
	if (!opened) return 0;

	size_t fileAddress;
	if (table.search(fileName, fileAddress)) {
		return fileAddress;
	}
	else {
		return 0;
	}
}

void Directory::addFile(size_t fileAddress)
{
	if (!opened) return;
	if (getSize() + getInodeSize() + sizeof(size_t) >= getMaxSize()) return;

	size_t fileToErasing;

	std::string fileName = readName(fileAddress);
	if (!table.at(fileName)) {
		table.insert(fileName, fileAddress);
		incrSize(sizeof(size_t));
	}

	modifyTime();
}

void Directory::rmvFile(size_t fileAddress)
{
	if (!opened) return;

	std::string fileName = readName(fileAddress);
	table.remove(fileName);
	decrSize(sizeof(size_t));

	modifyTime();
}

ConstIterator<std::string, size_t> Directory::begin()
{
	return table.begin();
}

ConstIterator<std::string, size_t> Directory::end()
{
	return table.end();
}

void File::fread(size_t size)
{
	if (size > getSize() + position && size != size_t(-1)) return;

	if (size == size_t(-1)) size = getSize() - position;

	size_t inodeSize = getInodeSize();
	size_t startAddress = getAddress(inodeSize + position);

	const size_t& BLOCK_SIZE = BLOCK_SZ;
	const size_t BUF_SZ_MAX = 256;
	size_t buf_sz;

	size_t maxSize = inodeSize + position + size;

	for (size_t curSize = inodeSize + position; curSize < maxSize; ) {
		size_t freeSpaceSize = BLOCK_SIZE - curSize % BLOCK_SIZE;
		buf_sz = std::min(maxSize - curSize, std::min(freeSpaceSize, BUF_SZ_MAX));

		char* buffer = new char[buf_sz + 1];
		buffer[buf_sz] = 0;
		read(startAddress + curSize % BLOCK_SIZE, buffer, buf_sz);
		std::cout << buffer;
		delete[] buffer;

		position += buf_sz;
		curSize += buf_sz;

		if (curSize % BLOCK_SIZE == 0 && curSize < maxSize) {
			startAddress = getAddress(curSize);
		}
	}

	modifyTime();
}

void File::fwrite(const char* buffer, size_t size)
{
	if (getSize() + getInodeSize() + size > getMaxSize()) return;

	size_t inodeSize = getInodeSize();
	size_t fileSize = getSize() + inodeSize;
	size_t startAddress = getAddress(fileSize);

	const size_t& BLOCK_SIZE = BLOCK_SZ;
	const size_t BUF_SZ_MAX = 256;
	size_t buf_sz;

	for(size_t curSize = fileSize, count = 0; curSize < fileSize + size; count++){
		size_t freeSpaceSize = BLOCK_SIZE - curSize % BLOCK_SIZE;
		buf_sz = std::min(fileSize + size - curSize, std::min(freeSpaceSize, BUF_SZ_MAX));

		char* s_buffer = new char[buf_sz + 1];
		std::copy(buffer + count, buffer + count + buf_sz, s_buffer);
		write(startAddress + curSize % BLOCK_SIZE, s_buffer, buf_sz);
		delete[] s_buffer;

		curSize += buf_sz;

		if (curSize % BLOCK_SIZE == 0 && curSize < fileSize + size) {
			startAddress = takeAddress(curSize);
		}
	}

	incrSize(size);
	modifyTime();
	
}

void File::fseek(size_t newPosition)
{
	if (newPosition > getSize()) return;
	position = newPosition;
}

void File::fclose()
{
	writeInode();
}

void Editor::oldinsert(size_t pos, const char* buffer, size_t size)
{
	size_t fileAddress = getAddress();
	size_t fileSize = getSize();
	size_t inodeSize = getInodeSize();

	if (fileSize + size > BLOCK_SZ - inodeSize) {
		return;
	}

	size_t buffer_size = fileSize - pos;
	char* mbuffer = new char[buffer_size];

	write(fileAddress + inodeSize + pos, mbuffer, buffer_size);
	read(fileAddress + inodeSize + pos + size, mbuffer, buffer_size);

	write(fileAddress + inodeSize + pos, buffer, size);

	incrSize(size);
	modifyTime();
}

void Editor::oldremove(size_t pos, size_t size)
{
	size_t fileAddress = getAddress();
	size_t fileSize = getSize();
	size_t inodeSize = getInodeSize();

	if (fileSize > BLOCK_SZ - inodeSize) {
		return;
	}

	size_t buffer_size = fileSize - pos;
	char* mbuffer = new char[buffer_size];

	read(fileAddress + inodeSize + pos + size, mbuffer, buffer_size);
	write(fileAddress + inodeSize + pos, mbuffer, buffer_size);

	decrSize(size);
	modifyTime();
}

void Editor::open() {
	if (!(getCurrentUID() == getOwnUID())) {
		return;
	}

	size_t fileAddress = getAddress();
	size_t fileSize = getSize();
	size_t inodeSize = getInodeSize();

	if (fileSize > BLOCK_SZ - inodeSize) {
		return;
	}

	for (size_t pos = fileAddress + inodeSize, count = 0; count < fileSize; ++count, ++pos) {
		char token;
		read(pos, &token, sizeof(char));
		Mytext += token;
	}
}

void Editor::insert(size_t pos, const std::string& data)
{
	Mytext.insert(pos, data);
}

void Editor::insert(size_t pos, char token)
{
	Mytext.insert(pos, std::string(1,token));
}

void Editor::remove(size_t pos, size_t size)
{
	Mytext.erase(pos, size);
}

const std::string& Editor::show()
{
	return Mytext;
}

void Editor::save() {

	setSize(Mytext.size());

	size_t fileAddress = getAddress();
	size_t fileSize = getSize();
	size_t inodeSize = getInodeSize();

	if (fileSize > BLOCK_SZ - inodeSize) {
		return;
	}

	for (size_t pos = fileAddress + inodeSize, count = 0; count < fileSize; ++count, ++pos) {
		char token = Mytext[count];
		write(pos, &token, sizeof(char));
	}

	modifyTime();
}

void Editor::close()
{
	writeInode();
}

void Editor::oldshow(size_t begin, size_t end)
{
	if (begin == size_t(-1)) {
		begin = 0;
	}
	if (end == size_t(-1)) {
		end = getSize();
	}

	size_t fileAddress = getAddress();
	size_t fileSize = getSize();
	size_t inodeSize = getInodeSize();

	if (fileSize > BLOCK_SZ - inodeSize) {
		return;
	}

	if (begin > fileSize || end - begin > fileSize) {
		return;
	}

	for (size_t pos = fileAddress + inodeSize + begin, count = 0; count < end - begin; ++count, ++pos) {
		char token;
		read(pos, &token, sizeof(char));
		std::cout << token;
	}
}
