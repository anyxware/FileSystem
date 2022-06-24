#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include "Menu.h"
#include <chrono>
#include <thread>

using namespace std::this_thread;     // sleep_for, sleep_until
using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
using std::chrono::system_clock;

#define delay 150ms

using namespace sf;

void remove_first_line(std::string& str) {
	int found = str.find_first_of('\n');
	std::cout << str.substr(0, found + 1);
	str.erase(0, found + 1);
}

void remove_first_two_lines(std::string& str) {
	int found = str.find_first_of('\n');
	std::cout << str.substr(0, found + 1);
	str.erase(0, found + 1);
	found = str.find_first_of('\n');
	std::cout << str.substr(0, found + 1);
	str.erase(0, found + 1);
}

void remove_extra_lines(std::string& MyText) {
	size_t n = std::count(MyText.begin(), MyText.end(), '\n');
	if (n > 26) {
		remove_first_two_lines(MyText);
	}
	else if (n == 26) {
		remove_first_line(MyText);
	}
}

int main()
{
	RenderWindow window(VideoMode(1000, 700), "Shell");

	sf::Text text;
	sf::Font font;
	if (!font.loadFromFile("../resources/sansation.ttf")) {
		std::cout << "Error loading font\n";
	}
	text.setFont(font); // font is a sf::Font
	std::string MyText = "Use existing Filesystem[1] / Create new[2]: ";
	text.setString(MyText);
	text.setCharacterSize(24); // in pixels, not points!
	text.setFillColor(sf::Color::Cyan);

	window.clear(Color(0, 0, 0, 0));
	window.draw(text);
	window.display();

	bool isExist;

	while(window.isOpen()) {
		Event event;
		while (window.pollEvent(event)) {
			if (event.type == Event::Closed) {
				window.close();
				return 0;
			}
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num1)) {
			isExist = true;
			break;
		}
		else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num2)){
			isExist = false;
			break;
		}
	}

	sleep_for(delay);

	MyText = "Input disk name: ";
	text.setString(MyText);
	window.clear(Color(0, 0, 0, 0));
	window.draw(text);
	window.display();
	char currentToken = 0;
	size_t count = 0;
	std::string diskName;
	bool firstCycle = true;

	// read disk name
	while (1)
	{
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed) {
				window.close();
				return 0;
			}
		}
		

		if (event.type != sf::Event::TextEntered) {
			continue;
		}
		
		currentToken = static_cast<char>(event.text.unicode);
		
		if (firstCycle) {
			firstCycle = false;
			continue;
		}

		if (currentToken == 8) {
			if(MyText.size() > 17)
				MyText.pop_back();
		}
		else if (currentToken == 13) {
			diskName = MyText.substr(17, MyText.size() - 17);
			break;
		}
		else if (currentToken >= 128 || currentToken < 32) {
			continue;
		}
		else {
			MyText += currentToken;
		}
		text.setString(MyText);
		if (MyText.size() % 80 == 0) {
			MyText += '\n';
		}
		
		window.clear(Color(0, 0, 0, 0));
		window.draw(text);
		window.display();
		sleep_for(delay);
	}

	Menu menu;
	if (!menu.checkParam(diskName, isExist))
		return 0;

	MyText = "Input user name: ";
	text.setString(MyText);
	window.clear(Color(0, 0, 0, 0));
	window.draw(text);
	window.display();
	count = 0;
	std::string userName;

	// read user name
	while (1)
	{
		Event event;
		while (window.pollEvent(event)) {
			if (event.type == Event::Closed) {
				window.close();
				return 0;
			}
		}
		

		if (event.type != sf::Event::TextEntered) {
			continue;
		}

		currentToken = static_cast<char>(event.text.unicode);

		if (currentToken == 8) {
			if(MyText.size() > 17)
				MyText.pop_back();
		}
		else if (currentToken == 13) {
			userName = MyText.substr(17, MyText.size() - 17);
			break;
		}
		else if (currentToken >= 128 || currentToken < 32) {
			continue;
		}
		else {
			MyText += currentToken;
		}
		text.setString(MyText);
		if (MyText.size() % 80 == 0) {
			MyText += '\n';
		}
		window.clear(Color(0, 0, 0, 0));
		window.draw(text);
		window.display();
		sleep_for(delay);
	}

	FileSystem fs;
	if (!fs.start(diskName, isExist))
		return 0;
	if (!fs.login(userName)) {
		fs.end();
		return 0;
	}

	MyText = userName + "@localhost> ";
	text.setString(MyText);
	window.clear(Color(0, 0, 0, 0));
	window.draw(text);
	window.display();
	count = 0;
	std::string unpreparedString;
	size_t startPos = MyText.size();

	//read command
	while (1) {
		Event event;
		while (window.pollEvent(event)) {
			if (event.type == Event::Closed) {
				window.close();
				break;
			}
		}

		if (event.type != sf::Event::TextEntered) {
			continue;
		}
		
		currentToken = static_cast<char>(event.text.unicode);

		if (currentToken == 26) { // eof
			break;
		}
		else if (currentToken == 8) { // backspace
			if(MyText.size() > startPos)
				MyText.pop_back();
		}
		else if (currentToken == 13) { // \n
			unpreparedString = MyText.substr(startPos, MyText.size() - startPos);
			std::cout << unpreparedString << "\n";
			MyText += "\n";
			MyText += menu.execute(fs, unpreparedString);
			MyText += userName + "@localhost> ";
			remove_extra_lines(MyText);
			startPos = MyText.size();
		}
		else if (currentToken >= 128 || currentToken < 32) {
			continue;
		}
		else {
			MyText += currentToken;
		}
		text.setString(MyText);
		window.clear(Color(0, 0, 0, 0));
		window.draw(text);
		window.display();
		
		sleep_for(delay);
	}

	fs.end();

	return 0;
}



//if (event.text.unicode < 128)
			//std::cout << "ASCII character typed: " << (int)static_cast<char>(event.text.unicode) << " " << static_cast<char>(event.text.unicode) << std::endl;
		/*if (currentToken == static_cast<char>(event.text.unicode) && count < speed) {
			count++;
			continue;
		}
		else {
			count = 0;
		}*/

//#include <iostream>
//#include "../StaticLib/FileSystem.h"
//#include "Menu.h"
//#include <string>
//#include <map>
//#include <fstream>
//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>
//#include "../TextEditor/Editor.h"
//
//
//// read time std::asctime(std::localtime(&result));
//void print() { std::cout << '\n'; }
//
//template <typename Head, typename... Tail>
//void print(const Head& head, const Tail&... tail) {
//	std::cout << head << " ";
//	print(tail...);
//}
//
//
//int main()
//{
//	// start
//	 
//	Menu menu;
//	bool isExist = menu.isExist();
//
//	std::string diskName = menu.getDiskName();
//	if (!menu.checkParam(diskName, isExist))
//		return 0;
//
//
//	FileSystem fs;
//	if (!fs.start(diskName, isExist))
//		return 0;
//	std::string userName = menu.login();
//	if (!fs.login(userName)) {
//		fs.end();
//		return 0;
//	}
//
//	// running
//
//
//	//menu.ReadAndExecute(fs);
//
//	fs.end();
//
//
//
//}



