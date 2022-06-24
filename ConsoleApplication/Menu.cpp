#include <iostream>
#include <fstream>
#include <sstream>
#include "Menu.h"



bool Menu::isExist()
{
	const char* pr = "";
	bool badAnswer = true;
	char answer;
	do {
		std::cout << pr;
		pr = "Bad input\n";
		std::cout << "Use existing Filesystem[1] / Create new[2]: ";
		std::cin >> answer;
		if (std::cin.eof()) {
			break;
		}
		if (answer == '1' || answer == '2') badAnswer = false;
	} while (badAnswer);

	if (std::cin.eof()) exit(0);
	return  answer == '1' ? true : false;
}

std::string Menu::getDiskName()
{
	std::string diskName;
	std::cout << "Input disk name: ";
	std::cin >> diskName;

	if (std::cin.eof()) exit(0);
	return diskName;
}

bool Menu::checkParam(const std::string& diskName, bool isExist)
{
	std::ifstream file;
	file.open(diskName);
	bool status = file.is_open() ? true : false;
	file.close();
	return status;
}

std::string Menu::login()
{
	std::string userName;
	std::cout << "Input user name: ";
	std::cin >> userName;

	if (std::cin.eof()) exit(0);
	return userName;
}

std::string Menu::execute(FileSystem& fs, const std::string& unpreparedString)
{
	std::string command;
	std::string arg1;
	std::string arg2;
	std::string out = "";

	if (unpreparedString == "") {
		return out;
	}

	std::stringstream stream(unpreparedString);

	stream >> command;

	if (command == "makeAlias") {
		makeAlias(stream);
		return out;
	}

	try {
		Commands.at(command);
	}
	catch (std::exception& e) {
		try {
			Aliases.at(command);
			for (auto sub_command : Aliases[command]) {
				if (Commands[sub_command].first == 0) {
					out += execute(fs, sub_command);
				}
				else if (Commands[sub_command].first == 1) {
					stream >> arg1;
					out += execute(fs, sub_command + " " + arg1);
				}
				else if (Commands[sub_command].first == 2) {
					stream >> arg1;
					stream >> arg2;
					out += execute(fs, sub_command + " " + arg1 + " " + arg2);
				}
			}
			return out;
		}
		catch (std::exception q) {
			std::cout << command << ": No such command\n";
			out = command + ": No such command\n";
			return out;
		}
	}

	if (stream.eof() && command == "ls") arg1 = ".";

	if (Commands[command].first > 0) {
		stream >> arg1;
	}

	if (Commands[command].first > 1) {
		stream >> arg2;
	}

	if (!stream.eof()) {
		std::cout << "Bad file or directory name\n";
		out = "Bad file or directory name\n";
	}

	if (!Commands[command].second(fs, arg1, arg2, out)) {
		std::cout << "Bad file or directory name\n";
		out = "Bad file or directory name\n";
	}

	return out;
}

#include <list>

void Menu::makeAlias(std::stringstream& args)
{
	std::string command;
	args >> command;
	std::list<std::string> Args;

	while (!args.eof()) {
		std::string sub_command;
		args >> sub_command;
		std::cout << sub_command << "\n";
		try {
			Commands.at(sub_command);
		}
		catch (std::exception e) {
			return;
		}
		Args.push_back(sub_command);
	}

	Aliases.insert({ command, Args });
}

#include <Windows.h>

#include <chrono>
#include <thread>

using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
using std::chrono::system_clock;

#define delay 150ms

void addCursor(Editor& editor, size_t& cursorPosition) {
	editor.insert(cursorPosition, '|');
}

void moveRight(Editor& editor, size_t& cursorPosition) {
	std::cout << cursorPosition << "\n";
	//editor.remove(cursorPosition - 1, 1);
	cursorPosition+=1;
	editor.insert(cursorPosition - 1, '|');
}

void moveLeft(Editor& editor, size_t& cursorPosition) {
	editor.remove(cursorPosition - 1, 1);
	editor.insert(cursorPosition - 2, '|');
	cursorPosition--;
}

void rmvCursor(Editor& editor, size_t& cursorPosition) {
	editor.remove(cursorPosition, 1);
}

void TextEditor(Editor& editor)
{
	editor.open();

	RenderWindow window(VideoMode(1000, 700), "Text Editor");

	sf::Text text;
	sf::Font font;
	if (!font.loadFromFile("../resources/sansation.ttf")) {
		std::cout << "Error loading font\n";
	}
	text.setFont(font); // font is a sf::Font
	text.setCharacterSize(24); // in pixels, not points!
	text.setFillColor(sf::Color::Cyan);
	size_t cursorPosition = 0;
	addCursor(editor, cursorPosition);
	text.setString(editor.show());
	window.clear(Color(0, 0, 0, 0));
	window.draw(text);
	window.display();

	bool saved = true;

	while (1)
	{
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed) {
				if (!saved && MessageBox(0, L"Save file", L"Save", MB_YESNO | MB_ICONQUESTION) == IDOK)
				{
					rmvCursor(editor, cursorPosition);
					editor.save();
				}
				window.close();
				editor.close();
				return;
			}
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
			if (cursorPosition < editor.show().size() - 1) {
				editor.remove(cursorPosition, 1);
				cursorPosition++;
				editor.insert(cursorPosition, '|');

				text.setString(editor.show());
				window.clear(Color(0, 0, 0, 0));
				window.draw(text);
				window.display();
				std::this_thread::sleep_for(delay);
			}

		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
			if (cursorPosition > 0) {
				editor.remove(cursorPosition, 1);
				cursorPosition--;
				editor.insert(cursorPosition, '|');

				text.setString(editor.show());
				window.clear(Color(0, 0, 0, 0));
				window.draw(text);
				window.display();
				std::this_thread::sleep_for(delay);
			}
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Delete)) {
			if (cursorPosition < editor.show().size() - 1) {
				editor.remove(cursorPosition + 1, 1);

				text.setString(editor.show());
				window.clear(Color(0, 0, 0, 0));
				window.draw(text);
				window.display();
				std::this_thread::sleep_for(delay);

				saved = false;
			}
		}
		
		if (event.type != sf::Event::TextEntered) {
			continue;
		}

		char currentToken = static_cast<char>(event.text.unicode);

		if (currentToken >= 128 || currentToken < 32) {
			if (currentToken == 8) { // backspace
				if (cursorPosition > 0) {
					editor.remove(cursorPosition, 1);
					cursorPosition--;
					editor.remove(cursorPosition, 1);
					editor.insert(cursorPosition, '|');
					saved = false;
				}
			}
			else if (currentToken == 13) { // \n
				editor.remove(cursorPosition, 1);
				editor.insert(cursorPosition, '\n');
				cursorPosition++;
				editor.insert(cursorPosition, '|');
				saved = false;
			}
			else if (currentToken == 19) { // ctrl s
				rmvCursor(editor, cursorPosition);
				editor.save();
				addCursor(editor, cursorPosition);
				saved = true;
			}
			else {
				std::cout << (int)currentToken << "\n";
				continue;
			}
		}
		else {
			editor.remove(cursorPosition, 1);
			editor.insert(cursorPosition, currentToken);
			cursorPosition++;
			editor.insert(cursorPosition, '|');
			saved = false;
		}
		
		std::cout << (int)currentToken << "\n";

		text.setString(editor.show());
		window.clear(Color(0, 0, 0, 0));
		window.draw(text);
		window.display();
		std::this_thread::sleep_for(delay);
	}
}
