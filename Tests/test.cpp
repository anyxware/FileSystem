#include <gtest/gtest.h>
#include "../StaticLib/FileSystem.h"

TEST(FileSystem_, start_end) {
	FileSystem fs;
	EXPECT_TRUE(fs.start("d.img", false));
	fs.end();
	EXPECT_TRUE(fs.start("d.img", true));
	fs.end();
	EXPECT_FALSE(fs.start("unexist.img", true));
	fs.end();
}

TEST(FileSystem_, WorkingWithUsers) {
	FileSystem& fs = *(new FileSystem); // test if null
	fs.start("d.img", false);
	EXPECT_TRUE(fs.login("root"));
	EXPECT_FALSE(fs.login("groot"));
	EXPECT_FALSE(fs.useradd("root"));
	EXPECT_TRUE(fs.useradd("groot"));
	EXPECT_TRUE(fs.login("groot")); // dont remove 
	
	for (int i = 1; i < 200; i++) {
		fs.useradd(std::to_string(i));
		fs.groupadd("group_for_all_users");
		fs.usermod(std::to_string(i), "group_for_all_users");
	}
	fs.end();
	delete &fs;
	
	FileSystem& newfs = *(new FileSystem);

	newfs.start("d.img", true);
	newfs.show();
	newfs.end();
	delete &newfs;
}

TEST(FileSystem_, Commands) {
	FileSystem& fs = *(new FileSystem);

	fs.start("d.img", true);
	for (int i = 1; i < 201; i++) {
		fs.mkdir(std::to_string(i));
	}
	std::cout << "------------LIST ROOT---------------\n";
	fs.list();
	fs.end();
	delete& fs;

	FileSystem& newfs = *(new FileSystem);
	newfs.start("d.img", true);
	std::cout << "------------LIST ROOT---------------\n";
	newfs.list();
	newfs.cd("1");
	for (int i = 1; i < 201; i++) {
		newfs.mkdir(std::to_string(i));
	}
	newfs.end();
	delete &newfs;

	
	FileSystem& newnewfs = *(new FileSystem);
	newnewfs.start("d.img", true);
	std::cout << "------------LIST ROOT---------------\n";
	newnewfs.list();
	newnewfs.cd("1");
	std::cout << "------------LIST DIR---------------\n";
	newnewfs.list();

	newnewfs.cd("/");
	newnewfs.rm("1");
	newnewfs.list();

	newnewfs.end();
	delete& newnewfs;


}


TEST(FileSystem_, DirectoryTraversal){
	FileSystem& fs = *(new FileSystem);
	fs.start("d.img", false);
	fs.mkdir("1");
	fs.cd("2");
	fs.pwd();
	fs.cd("1");
	fs.mkdir("2");
	fs.cd("2");
	fs.pwd();
	fs.end();
	delete& fs;

	FileSystem& newfs = *(new FileSystem);
	newfs.start("d.img", true);
	newfs.cd("1");
	newfs.cd("2");
	newfs.pwd();
	newfs.end();
	delete& newfs;
}

// remove group

// dont read uninitalize data

TEST(FileSystem_, Commands_2) {
	FileSystem& fs = *(new FileSystem);
	fs.start("d.img", false);
	fs.mkdir("home");
	fs.cd("home");
	fs.touch("file.txt");
	fs.list();
	fs.cd("file.txt"); // cd file.txt???
	fs.pwd();
	fs.mkdir("newdir");
	fs.cd("newdir");
	fs.mkdir("newdir"); ///////////////////////////////
	fs.pwd();

	fs.cd("..");
	fs.cd("..");
	fs.cd("..");
	fs.rm("home");
	fs.list();
	fs.cd("home");
	fs.pwd();
	fs.end();
	delete& fs;
}

TEST(File_, File_) {
	FileSystem& fs = *(new FileSystem);
	fs.start("d.img", false);
	fs.touch("test.txt");
	fs.write("test.txt", "lol kek cheburek\n");
	fs.write("test.txt", "wasap bro\n");
	fs.read("test.txt");
	fs.read("test.txt", 17);
	fs.read("test.txt", 10, 17);
	fs.end();
	delete& fs;
}

TEST(FileSystem_, CompositePath) {
	FileSystem& fs = *(new FileSystem);
	fs.start("d.img", false);
	fs.mkdir("dir1");
	fs.cd("dir1");
	fs.mkdir("dir2");

	fs.cd("dir2");
	fs.pwd();
	
	fs.cd("/dir1");
	fs.pwd();

	fs.cd("dir2");
	fs.cd1("../../../");
	fs.pwd();

	fs.cd("dir1/dir2");
	fs.cd("../lola");
	fs.pwd();

	fs.end();
	delete& fs;
}


TEST(FileSystem_, CompositePath2) {
	FileSystem& fs = *(new FileSystem);
	fs.start("d.img", false);

	fs.mkdir("dir1");
	fs.mkdir("dir1/dir2");
	fs.mkdir("dir1/dir3");
	fs.touch("dir1/dir2/file.txt");
	fs.mv("dir1/dir2/file.txt", "dir1/dir3");
	fs.list("dir1/dir3/");
	fs.rm("/dir1/dir3/file.txt");
	fs.list("dir1/dir3/");

	fs.end();
	delete& fs;
}


TEST(Map_, Map_) {
	Map<std::string, size_t> m = { {"1",1}, {"2",2},{"3",3}, {"4",4}, {"5",5}, {"6",6} };
	Map<std::string, size_t> m2 = m;
	Map<std::string, size_t> m1;
	m1 = m;
	m1.insert("7", 7);
	m1.insert("6", 6);
	size_t a;
	EXPECT_TRUE(m1.at("5"));
	EXPECT_FALSE(m1.at("9"));
	EXPECT_FALSE(m1.search("9", a));
	EXPECT_TRUE(m1.search("5", a));
	EXPECT_EQ(a, 5);
	m1.remove("2");
	EXPECT_FALSE(m1.at("2"));
	
	for (auto& it : m1) {
		std::cout << it.first << " " << it.second << "\n";
		it.second = 13;
	}
	for (const auto& it : m1) {
		std::cout << it.first << " " << it.second << "\n";
	}

}



int main(int argc, char* argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
