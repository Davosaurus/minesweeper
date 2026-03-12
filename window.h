#define _WIN32_WINNT 0x0600

#include <algorithm>
#include <functional>
#include <limits.h>
#include "logger.h"
#include <random>
#include <stdexcept>
#include <time.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <windows.h>
using namespace std;

mt19937 randomizer{(unsigned int)time(0)};

struct FlexibleStringComponent {
	const string text;
	short length;
	
	FlexibleStringComponent(const string& desiredText, const short& desiredLength) : text(desiredText), length(desiredLength) {}
	
	FlexibleStringComponent(const string& desiredText) : text(desiredText) {
		FlexibleStringComponent(desiredText, desiredText.length());
	}
};

namespace stringUtils {
	const string uppercase(const string& str) {
		string temp = str;
		for(auto& c : temp)
			c = toupper(c);
		return temp;
	}
}

namespace character {
	const string DELIMITER = ",";
	const string DATA_END = ";";
	const string KV_SEPARATOR = ":";
	const string STRING_BLANK = " ";
	const string STRING_MINE = "☼";
	const string STRING_CLOCK = "⏱";
	
	const FlexibleStringComponent BORDER_CROSS = FlexibleStringComponent("╬", 1);
	const FlexibleStringComponent BORDER_HORIZONTAL = FlexibleStringComponent("═", 1);
	const FlexibleStringComponent BORDER_VERTICAL = FlexibleStringComponent("║", 1);
	const FlexibleStringComponent BLANK = FlexibleStringComponent(STRING_BLANK, 1);
	const FlexibleStringComponent FLAG = FlexibleStringComponent("ľ", 1);
	const FlexibleStringComponent MINE = FlexibleStringComponent(STRING_MINE, 1);
	const FlexibleStringComponent INDICATOR_WON = FlexibleStringComponent("☺", 1);
	const FlexibleStringComponent INDICATOR_LOST = FlexibleStringComponent("☹", 1);
}

struct FlexibleString {
	vector<FlexibleStringComponent> components = vector<FlexibleStringComponent>();
	
	FlexibleString() {}
	
	FlexibleString& addComponent(const FlexibleStringComponent& component) {
		components.push_back(component);
		return *this;
	}
	
	FlexibleString& addText(const string& text, const short& length) {
		components.push_back(FlexibleStringComponent(text, length));
		return *this;
	}
	
	FlexibleString& addText(const string& text) {
		addText(text, text.length());
		return *this;
	}
	
	FlexibleString& addWordString(const string& wordString) {
		string delimiter = character::STRING_BLANK;
		size_t start = 0;
		size_t end = wordString.find(delimiter);
		
		while(end != string::npos) {
			addText(wordString.substr(start, end - start));
			start = end + 1;
			end = wordString.find(delimiter, start);
			
			if(start != end) {
				addComponent(character::BLANK);
			}
		}
		
		if(start != end) {
			addText(wordString.substr(start));
		}
		
		return *this;
	}
	
	const short length() const {
		short sum = 0;
		for(FlexibleStringComponent component : components) {
			sum += component.length;
		}
		return sum;
	}
};

const string ansiSequence(const int& controlCode) {
	return "\x1B[" + to_string(controlCode) + "m";
}

namespace text { enum text {
	BLACK = 30,
	DARK_RED = 31,
	DARK_GREEN = 32,
	DARK_YELLOW = 33,
	DARK_BLUE = 34,
	DARK_MAGENTA = 35,
	DARK_CYAN = 36,
	GRAY = 37,
	DARK_GRAY = 90,
	RED = 91,
	GREEN = 92,
	YELLOW = 93,
	BLUE = 94,
	MAGENTA = 95,
	CYAN = 96,
	WHITE = 97
}; }

namespace background { enum background {
	BLACK = 40,
	DARK_RED = 41,
	DARK_GREEN = 42,
	DARK_YELLOW = 43,
	DARK_BLUE = 44,
	DARK_MAGENTA = 45,
	DARK_CYAN = 46,
	GRAY = 47,
	DARK_GRAY = 100,
	RED = 101,
	GREEN = 102,
	YELLOW = 103,
	BLUE = 104,
	MAGENTA = 105,
	CYAN = 106,
	WHITE = 107
}; }

FlexibleString color(const string& wordString, const int& colorCode) {
	return FlexibleString()
			.addText(ansiSequence(colorCode), 0)	//text style
			.addWordString(wordString)
			.addText(ansiSequence(0), 0);			//default style
}

FlexibleString color(const string& wordString, const int& colorCodeText, const int& colorCodeBackground) {
	return FlexibleString()
			.addText(ansiSequence(colorCodeText), 0)		//text style
			.addText(ansiSequence(colorCodeBackground), 0)	//background style (the order of the two actually doesn't matter, both styles will be applied)
			.addWordString(wordString)
			.addText(ansiSequence(0), 0);					//default style
}

FlexibleString color(const FlexibleStringComponent& component, const int& colorCode) {
	return FlexibleString()
			.addText(ansiSequence(colorCode), 0)	//text style
			.addComponent(component)
			.addText(ansiSequence(0), 0);			//default style
}

FlexibleString color(const FlexibleStringComponent& component, const int& colorCodeText, const int& colorCodeBackground) {
	return FlexibleString()
			.addText(ansiSequence(colorCodeText), 0)		//text style
			.addText(ansiSequence(colorCodeBackground), 0)	//background style (the order of the two actually doesn't matter, both styles will be applied)
			.addComponent(component)
			.addText(ansiSequence(0), 0);					//default style
}

const function<bool(const string&)> isNotBlank() {
	return [](const string& input) { return input.length() > 0; };
}

const function<bool(const string&)> isInNumericRange(const int& min, const int& max) {
	return [min, max](const string& input) {
		try {
			return stoi(input) >= min && stoi(input) <= max;
		}
		catch(invalid_argument e) {
			return false;
		}
	};
}

const function<bool(const string&)> isAlpha() {
	return [](const string& input) { return all_of(input.begin(), input.end(), ::isalpha); };
}

const function<bool(const string&)> operator&&(const function<bool(const string&)> requirementA, const function<bool(const string&)> requirementB) {
	return [requirementA, requirementB](const string& input) { return requirementA(input) && requirementB(input); };
}

const string getTextFromNumericField(const int& field, const int& length) {
	char outputString[length + 1];
	const char* format = ("%0" + to_string(length) + "d").c_str();
	sprintf(outputString, format, field);
	return outputString;
}

namespace window {

#ifndef CONSOLE_READ_NOWAIT
#define CONSOLE_READ_NOWAIT 0x0002
#endif

#ifdef __cplusplus
extern "C" {
#endif
BOOL WINAPI ReadConsoleInputExA(_In_ HANDLE hConsoleInput, _Out_writes_(nLength) PINPUT_RECORD lpBuffer, _In_ DWORD nLength, _Out_ LPDWORD lpNumberOfEventsRead, _In_ USHORT wFlags);
#ifdef __cplusplus
}
#endif

//Get input and output handles for the console
const HANDLE handleIn = GetStdHandle(STD_INPUT_HANDLE);
const HANDLE handleOut = GetStdHandle(STD_OUTPUT_HANDLE);

void showCursor() {
	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(handleOut, &cursorInfo);
	cursorInfo.bVisible = true;
	SetConsoleCursorInfo(handleOut, &cursorInfo);
}

void hideCursor() {
	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(handleOut, &cursorInfo);
	cursorInfo.bVisible = false;
	SetConsoleCursorInfo(handleOut, &cursorInfo);
}

/**
 * Set the console cursor's size
 * @param size is the portion of the current character to fill, between 1-100
 */
void setCursorSize(const int& size) {
	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(handleOut, &cursorInfo);
	cursorInfo.dwSize = size;
	SetConsoleCursorInfo(handleOut, &cursorInfo);
}

const COORD getCursorPosition() {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(handleOut, &csbi);
	return csbi.dwCursorPosition;
}

void setSize(const COORD& windowSize) {
	SMALL_RECT rect;
	rect.Top = 0;
	rect.Left = 0;
	rect.Bottom = windowSize.Y - 1;
	rect.Right = windowSize.X - 1;

	SetConsoleScreenBufferSize(handleOut, windowSize);
	SetConsoleWindowInfo(handleOut, true, &rect);
}

void initialize(CONSOLE_FONT_INFOEX& font, const COORD& windowSize) {
	//Hide cursor
	hideCursor();
	
	//Clear the screen
	system("cls");
	
	//Set font settings
	SetCurrentConsoleFontEx(handleOut, false, &font);
	
	//Set window size
	setSize(COORD{1, 1}); //avoids issues with text wrapping and scroll bars
	setSize(windowSize);
	
	//Clear the screen
	system("cls");
	
	//Show full-size cursor
	showCursor();
}

void scaleFontSizeToFit(CONSOLE_FONT_INFOEX& font, const COORD& windowSize) {
	//Get screen resolution, minus window borders and taskbar
	int adjustedScreenWidth =
			 (GetSystemMetrics(SM_CXSCREEN)		//original resolution width
			- GetSystemMetrics(SM_CXFRAME) * 2)	//both window borders
			* 0.8;								//small adjustment to handle rounding errors
	int adjustedScreenHeight =
			 (GetSystemMetrics(SM_CYSCREEN)		//original resolution height
			- GetSystemMetrics(SM_CYFRAME) * 2	//both window borders
			- GetSystemMetrics(SM_CYCAPTION))	//taskbar height
			* 0.8;								//small adjustment to handle rounding errors
	
	font.dwFontSize.X = adjustedScreenWidth / windowSize.X;
	font.dwFontSize.Y = adjustedScreenHeight / windowSize.Y;
	
	//Lower the bigger dimension to match so that a square aspect ratio is preserved
	if(font.dwFontSize.X > font.dwFontSize.Y) {
		font.dwFontSize.X = font.dwFontSize.Y;
	}
	else if(font.dwFontSize.Y > font.dwFontSize.X) {
		font.dwFontSize.Y = font.dwFontSize.X;
	}
}

void printInRectangle(const FlexibleString& flexibleString, const COORD& beginPosition, const COORD& endPosition = COORD{SHRT_MAX, SHRT_MAX}, const bool& fillAll = false) {
	COORD cursor = beginPosition;
	
	//Move screen cursor to print cursor
	SetConsoleCursorPosition(handleOut, cursor);
	
	do {
		for(FlexibleStringComponent component : flexibleString.components) {
			//If printing this component would reach out of bounds
			if(cursor.X + component.length > endPosition.X + 1) {
				//Update cursor to position on new line
				cursor.Y++;
				cursor.X = beginPosition.X;
				
				//Move screen cursor to print cursor
				SetConsoleCursorPosition(handleOut, cursor);
				
				//Special case for when a line break coincides with a space (remove the space)
				if(component.text == character::BLANK.text) {
					continue;
				}
			}
			
			if(cursor.Y > endPosition.Y && component.length > 0) {
				continue;
			}
			
			cout << component.text;
			cursor.X += component.length;
		}
	} while(fillAll && cursor.Y <= endPosition.Y);
}

void printInRectangle(const string& wordString, const COORD& beginPosition, const COORD& endPosition = COORD{SHRT_MAX, SHRT_MAX}, const bool& fillAll = false) {
	printInRectangle(FlexibleString().addWordString(wordString), beginPosition, endPosition, fillAll);
}

void printEdgeBorders(const COORD& windowSize, const int& colorCodeText, const int& colorCodeBackground, const int& numHorizontalSeparators = 0, ...) {
	COORD max = COORD{short(windowSize.X - 1), short(windowSize.Y - 1)};
	
	const FlexibleString borderCorner = color(character::BORDER_CROSS, colorCodeText, colorCodeBackground);
	const FlexibleString borderEdgeHorizontal = color(character::BORDER_HORIZONTAL, colorCodeText, colorCodeBackground);
	const FlexibleString borderEdgeVertical = color(character::BORDER_VERTICAL, colorCodeText, colorCodeBackground);
	
	//4 corners
	printInRectangle(borderCorner, COORD{0, 0});
	printInRectangle(borderCorner, COORD{max.X, 0});
	printInRectangle(borderCorner, COORD{max.X, max.Y});
	printInRectangle(borderCorner, COORD{0, max.Y});
	
	//4 outer edges
	printInRectangle(borderEdgeHorizontal, COORD{1, 0}, COORD{short(max.X - 1), 0}, true);
	printInRectangle(borderEdgeVertical, COORD{max.X, 1}, COORD{max.X, short(max.Y - 1)}, true);
	printInRectangle(borderEdgeHorizontal, COORD{1, max.Y}, COORD{short(max.X - 1), max.Y}, true);
	printInRectangle(borderEdgeVertical, COORD{0, 1}, COORD{0, short(max.Y - 1)}, true);
	
	//Arbitrary number of horizontal separators
	va_list args;
	va_start(args, numHorizontalSeparators);
	for(int i = 0; i < numHorizontalSeparators; i++) {
		short row = (short)va_arg(args, int);
		printInRectangle(borderCorner, COORD{0, row});
		printInRectangle(borderCorner, COORD{max.X, row});
		printInRectangle(borderEdgeHorizontal, COORD{1, row}, COORD{short(max.X - 1), row}, true);
	}
	va_end(args);
}

class Table {
	private:
		COORD position;
		vector<short> columnWidths = vector<short>();
		short currentColumn;
		vector<short> rowHeights = vector<short>();
		vector<vector<FlexibleString>> cells = vector<vector<FlexibleString>>();
	public:
		/**
		 * Set the table's position on the screen
		 */
		Table& atPosition(const COORD& desiredPosition) {
			position = desiredPosition;
			return *this;
		}
		
		/**
		 * Add an empty row to the table. Subsequent new cells will be added to this row
		 */
		Table& addRow() {
			cells.push_back(vector<FlexibleString>());
			currentColumn = 0;
			return *this;
		}
		
		/**
		 * Add a row to the table, adding and populating cells from the flexibleString's content. Subsequent new cells will be added to this row
		 */
		Table& addRow(const FlexibleString& flexibleString) {
			addRow();
			for(auto component : flexibleString.components) {
				addCell(FlexibleString().addComponent(component));
			}
			return *this;
		}
		
		/**
		 * Add a cell to the current row using text. Words will be parsed by spaces and wrap to the next line inside the cell (non-breaking)
		 */
		Table& addCell(const string& text, COORD cellSize = COORD{0, 1}) {
			return addCell(FlexibleString().addWordString(text), cellSize);
		}
		
		/**
		 * Add a cell to the current row using a Flexible String. Words will be parsed by the flexibleString's structure and wrap to the next line inside the cell (non-breaking)
		 */
		Table& addCell(const FlexibleString& flexibleString, COORD cellSize = COORD{0, 1}) {
			if(cellSize.X == 0) {
				cellSize = COORD{flexibleString.length(), cellSize.Y};
			}
			
			//Update column width if it's a new column or if it exceeds previous maximum width
			if(currentColumn < columnWidths.size() && cellSize.X > columnWidths[currentColumn]) {
				columnWidths[currentColumn] = cellSize.X;
			}
			else {
				columnWidths.push_back(cellSize.X);
			}
			
			if(cells.size() > rowHeights.size()) { //add a new row height if there's no data for this row yet
				rowHeights.push_back(cellSize.Y);
			}
			else if(cellSize.Y > rowHeights.back()) { //or update row height if it exceeds previous maximum height
				rowHeights.back() = cellSize.Y;
			}
			
			cells.back().push_back(flexibleString);
			currentColumn++;
			return *this;
		}
		
		void print() {
			COORD currentPosition = position;
			vector<short>::iterator rowHeight = rowHeights.begin();
			vector<short>::iterator columnWidth;
			for(vector<FlexibleString> row : cells) {
				columnWidth = columnWidths.begin();
				for(FlexibleString flexibleString : row) {
					printInRectangle(flexibleString, currentPosition, COORD{short(currentPosition.X + *columnWidth - 1), short(currentPosition.Y + *rowHeight - 1)});
					currentPosition = COORD{short(currentPosition.X + *columnWidth), currentPosition.Y};
					columnWidth++;
				}
				currentPosition = COORD{position.X, short(currentPosition.Y + *rowHeight)};
				rowHeight++;
			}
		}
		
		short getRowCount() const {
			return rowHeights.size();
		}
		
		short getColumnCount() const {
			return columnWidths.size();
		}
		
		short getTotalTableHeight() const {
			return accumulate(rowHeights.begin(), rowHeights.end(), 0);
		}
		
		short getTotalTableWidth() const {
			return accumulate(columnWidths.begin(), columnWidths.end(), 0);
		}
		
		Table subTable(short startRow, short endRow) {
			Table subTable;
			subTable.position = position;
			subTable.columnWidths = columnWidths;
			
			if(startRow < 0) {
				startRow = 0;
			}
			if(endRow > cells.size()) {
				endRow = cells.size();
			}
			if(startRow < cells.size() && endRow >= 0 && startRow <= endRow) {
				subTable.rowHeights = vector<short>(rowHeights.begin() + startRow, rowHeights.begin() + endRow);
				subTable.cells = vector<vector<FlexibleString>>(cells.begin() + startRow, cells.begin() + endRow);
			}
			
			return subTable;
		}
};

class UserExitException : public exception {
public:
	const char* what() const noexcept override {
		return "User exited before supplying input";
	}
};

/**
 * Attempt to get any user input from the keyboard. If any key is pressed, exit immediately with exception. If the timeout limit is reached, return normally.
 * @throws UserExitException if the user presses a key to exit
 */
void waitForUserInput(ULONGLONG timeout = -10 - GetTickCount64()) {
	ULONGLONG endTime = GetTickCount64() + timeout;
	FlushConsoleInputBuffer(handleIn);
	
	while(GetTickCount64() < endTime) {
		DWORD consoleEventCount;
		GetNumberOfConsoleInputEvents(handleIn, &consoleEventCount);
		if(consoleEventCount == 0) {
			continue;
		}
		
		DWORD cNumRead;
		INPUT_RECORD inputBuffer[128];
		ReadConsoleInput( 
			handleIn,		// input buffer handle
			inputBuffer,	// buffer to read into
			128,			// size of read buffer
			&cNumRead);		// number of records read
		
		//For each input in the buffer...
		for(int i = 0; i < cNumRead; i++)
		{
			if(inputBuffer[i].EventType == KEY_EVENT && inputBuffer[i].Event.KeyEvent.bKeyDown) {
				throw UserExitException();
			}
		}
	}
}

/**
 * Attempt to get user input from the keyboard. Wait for RETURN key to be pressed to submit. If ESC key is pressed, exit immediately. Entered characters must be graphical (non-control, non-whitespace) and optionally must meet a separately defined condition.
 * @param screenPosition is the starting position for the input field.
 * @param maxLength is the maximum allowable length for the input field. If user attempts to type more characters than this, subsequent inputs are discarded.
 * @param validate is a function that determines if the input is valid. User input will continue to be gathered until the submission meets validation criteria (or the user exits). Default value is unconditional pass; that is, perform no additional validation on the string.
 * @return a string containing the inputted characters.
 * @throws UserExitException if the user presses ESC key to exit
 */
string getTextInput(
		const COORD& screenPosition,
		const short& maxLength,
		const function<bool(const string&)>& validate = [](const string& input) { return true; },
		const string& startingValue = "") {
	string result = startingValue;
	cout << startingValue;
	COORD cursorPosition = COORD{short(screenPosition.X + startingValue.length()), screenPosition.Y};
	while(1) {
		SetConsoleCursorPosition(handleOut, cursorPosition);
		
		DWORD cNumRead;
		INPUT_RECORD inputBuffer[128];
		ReadConsoleInput( 
			handleIn,		// input buffer handle
			inputBuffer,	// buffer to read into
			128,			// size of read buffer
			&cNumRead);		// number of records read
		
		//For each input in the buffer...
		for(int i = 0; i < cNumRead; i++) {
			if(inputBuffer[i].EventType == KEY_EVENT && inputBuffer[i].Event.KeyEvent.bKeyDown) {
				switch(inputBuffer[i].Event.KeyEvent.wVirtualKeyCode) {
					case VK_ESCAPE:
						throw UserExitException();
					case VK_RETURN:
						if(validate(result)) {
							return result;
						}
						else {
							SetConsoleCursorPosition(handleOut, screenPosition);
							for(char c : result) {
								cout << "\x1B[101m" << c << "\x1B[0m";
							}
						}
						break;
					case VK_BACK:
						if(result.length() > 0) {
							result.pop_back();
							cout << "\x08 ";
							cursorPosition.X--;
						}
						break;
					default:
						auto character = inputBuffer[i].Event.KeyEvent.uChar.AsciiChar;
						if(result.length() < maxLength && isgraph(character)) {
							result += character;
							cout << character;
							cursorPosition.X++;
						}
				}
			}
		}
	}
}

int getNumericInput(
		const COORD& screenPosition,
		const short& maxLength,
		const function<bool(const string&)>& validate = [](const string& input) { return true; },
		const string& startingValue = "") {
	return stoi(getTextInput(screenPosition, maxLength, validate, startingValue));
}

}