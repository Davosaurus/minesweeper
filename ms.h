#define _WIN32_WINNT 0x0600

#include <algorithm>
#include <functional>
#include <iostream>
#include <random>
#include <stdexcept>
#include <time.h>
#include <unordered_set>
#include <vector>
#include <windows.h>
using namespace std;

mt19937 randomizer{(unsigned int)time(0)};

struct FlexibleStringComponent {
	short length;
	const string text;
	
	FlexibleStringComponent(const string& desiredText, const short& desiredLength) : text(desiredText), length(desiredLength) {}
	
	FlexibleStringComponent(const string& desiredText) : text(desiredText) {
		FlexibleStringComponent(desiredText, desiredText.length());
	}
};

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
		for(char c : text) {
			addText(string(1, c), 1);
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

FlexibleString color(const string& text, const int& colorCode) {
	return FlexibleString()
			.addText(ansiSequence(colorCode), 0)	//text style
			.addText(text)
			.addText(ansiSequence(0), 0);			//default style
}

FlexibleString color(const string& text, const int& colorCodeText, const int& colorCodeBackground) {
	return FlexibleString()
			.addText(ansiSequence(colorCodeText), 0)		//text style
			.addText(ansiSequence(colorCodeBackground), 0)	//background style (the order of the two actually doesn't matter, both styles will be applied)
			.addText(text)
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

bool isDigits(const string& input) {
	return all_of(input.begin(), input.end(), ::isdigit);
}

const function<bool(const string&)> inNumericRange(const int& min, const int& max) {
	return [min, max](const string& input) { return isDigits(input) && stoi(input) >= min && stoi(input) <= max; };
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
			if(cursor.Y > endPosition.Y && component.length > 0) {
				continue;
			}
			else {
				cout << component.text;
				cursor.X += component.length;
				
				//Check for end of line
				if(cursor.X > endPosition.X) {
					//Update cursor to new position
					cursor.Y++;
					cursor.X = beginPosition.X;
					
					//Move screen cursor to print cursor
					SetConsoleCursorPosition(handleOut, cursor);
				}
			}
		}
	} while(fillAll && cursor.Y <= endPosition.Y);
}

void printInRectangle(const string& text, const COORD& beginPosition, const COORD& endPosition = COORD{SHRT_MAX, SHRT_MAX}, const bool& fillAll = false) {
	printInRectangle(FlexibleString().addText(text), beginPosition, endPosition, fillAll);
}

void printEdgeBorders(const COORD& windowSize, const int& colorCodeText, const int& colorCodeBackground, const int& numHorizontalSeparators = 0, ...) {
	COORD max = COORD{short(windowSize.X - 1), short(windowSize.Y - 1)};
	
	const FlexibleString borderCorner = color(FlexibleStringComponent("╬", 1), colorCodeText, colorCodeBackground);
	const FlexibleString borderEdgeHorizontal = color(FlexibleStringComponent("═", 1), colorCodeText, colorCodeBackground);
	const FlexibleString borderEdgeVertical = color(FlexibleStringComponent("║", 1), colorCodeText, colorCodeBackground);
	
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
		Table& atPosition(const COORD& desiredPosition) {
			position = desiredPosition;
			return *this;
		}
		
		Table& addRow() {
			cells.push_back(vector<FlexibleString>());
			currentColumn = 0;
			return *this;
		}
		
		Table& addCell(const string& text, COORD cellSize = COORD{0, 1}) {
			return addCell(FlexibleString().addText(text), cellSize);
		}
		
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
};

class UserExitException : public exception {
public:
	const char* what() const noexcept override {
		return "User exited before supplying input";
	}
};

/**
 * Attempt to get user input from the keyboard. Wait for RETURN key to be pressed to submit. If ESC key is pressed, return immediately. Entered characters must be graphical (non-control, non-whitespace) and optionally must meet a separately defined condition.
 * @param screenPosition is the starting position for the input field.
 * @param maxLength is the maximum allowable length for the input field. If user attempts to type more characters than this, subsequent inputs are discarded.
 * @param validate is a function that determines if the input is valid. User input will continue to be gathered until the submission meets validation criteria (or the user exits). Default value is unconditional pass; that is, perform no additional validation on the string.
 * @return a string containing the inputted characters.
 * @throws UserExitException if the user presses ESC key to exit
 */
string getTextInput(const COORD& screenPosition, const short& maxLength, const function<bool(const string&)>& validate = [](const string& input) { return true; }) {
	string result;
	COORD cursorPosition = screenPosition;
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
		for(int i = 0; i < cNumRead; i++)
		{
			if(inputBuffer[i].EventType == KEY_EVENT && inputBuffer[i].Event.KeyEvent.bKeyDown)
			{
				switch(inputBuffer[i].Event.KeyEvent.wVirtualKeyCode)
				{
					case VK_ESCAPE:
						throw UserExitException();
					case VK_RETURN:
						if(result.length() > 0 && validate(result)) {
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

short getInput(const COORD& screenPosition, const short& maxLength, const function<bool(const string&)>& validate = [](const string& input) { return true; }) {
	return stoi(getTextInput(screenPosition, maxLength, validate));
}

}

const string CHARACTER_BLANK = " ";
const string CHARACTER_HIDDEN = ansiSequence(47) + CHARACTER_BLANK + ansiSequence(0);
const string CHARACTER_FLAG = ansiSequence(91) + ansiSequence(47) + "ľ" + ansiSequence(0);
const string CHARACTER_MINE = "☼";

namespace field {

enum State {
	BLANK,
	MINE,
	NUMBER,
	UNINITIALIZED,
	FAIL
};

class Cell {
	friend class Field;
	private:
		bool hidden = true;
		bool flagged = false;
		State state;
		short number;
	
		Cell(	const short& desiredRow,
				const short& desiredCol
		) :		row(desiredRow),
				col(desiredCol),
				state(UNINITIALIZED) {}
		
		/*
		 *  Attempt to initialize the cell
		 *  Returns whether attempt was successful
		 */
		const bool init(const State& desiredState, const short& desiredNumber = 0) {
			if(state != UNINITIALIZED) {
				return false;
			}
			else switch(desiredState) {
				case MINE:
					state = MINE;
					return true;
				case NUMBER:
					if(desiredNumber == 0) {
						state = BLANK;
					}
					else {
						state = NUMBER;
						number = desiredNumber;
					}
					return true;
				default:
					return false;
			}
		}
		
		/**
		 * Attempt to flag the cell
		 * @return the number of flags placed by this action:
		 *	-1		a flag was removed
		 *	 0		no flag was placed or removed
		 *	 1		a flag was placed
		 */
		const short toggleFlag() {
			if(flagged) {
				flagged = false;
				return -1;
			}
			
			if(hidden) {
				flagged = true;
				return 1;
			}
			
			return 0;
		}
		
		/**
		 * Attempt to reveal the cell
		 * @return the state, if it was newly revealed. Else, return FAIL State to indicate that no reveal was performed.
		 */
		const State reveal() {
			if(state == UNINITIALIZED) {
				throw logic_error("Cell must be initialized before call to reveal()");
			}
			
			if(hidden && !flagged) {
				hidden = false;
				return state;
			}
			else {
				return FAIL;
			}
		}
	
	public:
		const short row;
		const short col;
		
		/**
		 * Attempt to get the cell's state
		 * @return the state, if it is public knowledge. Else, return FAIL State to indicate that the requested information is not available.
		 */
		const State getState() const {
			if(!hidden) {
				return state;
			}
			else {
				return FAIL;
			}
		}
		
		/**
		 * Attempt to get the cell's number
		 * @return the number, if it is public knowledge. Else, return 0 to indicate that the requested information is not available.
		 */
		const short getNumber() const {
			if(!hidden && state == NUMBER) {
				return number;
			}
			else {
				return 0;
			}
		}
};

class Field;

class FieldEvaluator {
	public:
		/**
		 * @return whether the given Field is acceptable; that is, whether or not it meets the specific criteria defined by this particular Evaluator
		 */
		virtual bool evaluate(const Field& field) const = 0;
};

enum GameStatus {
	UNSTARTED,
	PLAYING,
	WON,
	LOST
};

class Field {
	private:
		//set at construction
		const short rows;					//number of rows
		const short cols;					//number of columns
		const short mines;					//number of mines
		const COORD positionOffset;			//position of the board's top-left corner in the screen coordinate system
		const FieldEvaluator& evaluator;	//arbitrary decisionmaking engine that determines if a field is valid to play, based on the state at initialization

		//updated thoughout the game
		GameStatus gameStatus = UNSTARTED;	//current status of the game
		short remainingSpaces;				//number of safe spaces (not mines) that remain hidden
		short mineCount;					//number of mines minus the number of placed flags
		ULONGLONG startTime;				//system time when the game was started
		ULONGLONG endTime;					//system time when the game was completed
		vector<vector<Cell>> cells;			//holds all cells
		
		void printToScreen() const {
			for(short row = 0; row < rows; row++) {
				moveCursorTo(row, 0);
				for(short col = 0; col < cols; col++) {
					print(cells.at(row).at(col));
				}
			}
		}
		
		void resetBoard() {
			cells = vector<vector<Cell>>();
			for(short row = 0; row < rows; row++) {
				cells.push_back(vector<Cell>());
				for(short col = 0; col < cols; col++) {
					cells.at(row).push_back(Cell(row, col));
				}
			}
		}
		
		void init() {
			//Construct a distribution which, when given a randomizer, can produce a number that refers to a unique cell in the field space
			uniform_int_distribution<> generateNumberInRangeUsing(0, rows * cols - 1);
			
			do {
				resetBoard();
				
				//Generate mines
				for(short numberOfMinesPlaced = 0; numberOfMinesPlaced < mines; ) {
					int index = generateNumberInRangeUsing(randomizer);
					Cell& candidate = cells.at(index/cols).at(index%cols);
					if(candidate.init(MINE)) {
						numberOfMinesPlaced++;
					}
				}
			} while(!evaluator.evaluate(*this));
			
			//Generate adjacent numbers
			for(short row = 0; row < rows; row++) {
				for(short col = 0; col < cols; col++) {
					Cell& cell = cells.at(row).at(col);
					
					short sumOfNeighboringMines = 0;
					unordered_set<const Cell*> neighbors = getAdjacentCells(row, col);
					for(const Cell* neighbor : neighbors) {
						if(neighbor->state == MINE) {
							sumOfNeighboringMines++;
						}
					}
					
					cell.init(NUMBER, sumOfNeighboringMines);
				}
			}
			
			gameStatus = PLAYING;
			startTime = GetTickCount64();
		}
		
		void moveCursorTo(const short& desiredRow, const short& desiredCol) const {
			SetConsoleCursorPosition(
				window::handleOut,
				COORD{short(desiredCol + positionOffset.X), short(desiredRow + positionOffset.Y)}
			);
		}
		
		void getCoordsFromCursorPosition(short& row, short& col) const {
			COORD cursorPosition = window::getCursorPosition();
			row = cursorPosition.Y - positionOffset.Y;
			col = cursorPosition.X - positionOffset.X;
		}
		
		/**
		 * Print the given cell to the screen
		 */
		void print(const Cell& cell) const {
			if(cell.hidden) {
				if(cell.flagged) {
					if(gameStatus != PLAYING) {
						if(cell.state == MINE) {
							cout << CHARACTER_MINE; //display good mine
						}
						else {
							cout << CHARACTER_FLAG; //display bad flag
						}
					}
					else {
						cout << CHARACTER_FLAG;
					}
				}
				else { //not flagged
					if(gameStatus != UNSTARTED && gameStatus != PLAYING && cell.state == MINE) { //game over scenario, unexploded mine
						cout << CHARACTER_MINE; //display neutral mine
					}
					else {
						cout << CHARACTER_HIDDEN;
					}
				}
			}
			else if(cell.state == BLANK) {
				cout << CHARACTER_BLANK;
			}
			else if(cell.state == NUMBER) {
				cout << to_string(cell.number);
			}
			else if(cell.state == MINE) {
				cout << CHARACTER_MINE; //display extra bad mine
			}
			else {
				cout << "?";
			}
		}
		
		const bool isValidSpace(const short& row, const short& col) const {
			return row >= 0 && row < rows
				&& col >= 0 && col < cols;
		}
		
		const bool isValidCurrentCursorPosition(short& row, short& col) const {
			getCoordsFromCursorPosition(row, col);
			return isValidSpace(row, col);
		}
		
		unordered_set<const Cell*> getAdjacentCells(const short& rowPos, const short& colPos) const {
			unordered_set<const Cell*> result;
			for(short rowModifier = -1; rowModifier <= 1; rowModifier++)
			{
				for(short colModifier = -1; colModifier <= 1; colModifier++)
				{
					short row = rowPos + rowModifier;
					short col = colPos + colModifier;
					if(    isValidSpace(row, col)				//coordinates requested are within field bounds
						&& !(row == rowPos && col == colPos))	//coordinates are not the same as the original space
						result.insert(&cells.at(row).at(col));
				}
			}
			return result;
		}
		
		void lose() {
			gameStatus = LOST;
			endTime = GetTickCount64();
			printToScreen();
		}
		
		void win() {
			gameStatus = WON;
			endTime = GetTickCount64();
			printToScreen();
		}
	
	public:
		Field(
				const COORD& desiredSize,
				const short& desiredMines,
				const COORD& desiredPosition,
				const FieldEvaluator& desiredFieldEvaluator
		) :
				rows(desiredSize.Y),
				cols(desiredSize.X),
				mines(desiredMines),
				positionOffset(desiredPosition),
				evaluator(desiredFieldEvaluator)
		{
			remainingSpaces = rows * cols - mines;
			mineCount = mines;
			resetBoard();
			printToScreen();
		}
		
		/**
		 * @return a coordinate that holds the position of the board's top-left corner, in the screen coordinate system.
		 */
		const COORD getPositionMin() const {
			return positionOffset;
		}
		
		/**
		 * @return a coordinate that holds the position of the board's bottom-right corner, in the screen coordinate system.
		 */
		const COORD getPositionMax() const {
			return COORD{short(positionOffset.X + cols - 1), short(positionOffset.Y + rows - 1)};
		}
		
		/**
		 * @return the current game status.
		 */
		const GameStatus getGameStatus() const {
			return gameStatus;
		}
		
		/**
		 * @return the current "Mine Count" of the game. Mine Count refers to the total number of mines in the game, minus the number of placed flags.
		 */
		const short getMineCount() const {
			return mineCount;
		}
		
		/**
		 * @return the number of milliseconds since the game started. If the game has finished, return the game duration in milliseconds.
		 */
		const ULONGLONG getElapsedTime() const {
			switch(gameStatus) {
				case UNSTARTED:
					return 0;
				case PLAYING:
					return GetTickCount64() - startTime;
				default:
					return endTime - startTime;
			}
		}
		
		/**
		 * Move the cursor to the center of the field, and ensure it is not hidden
		 * @return a coordinate that holds the new screen position of the cursor
		 */
		const COORD initializeCursor() const {
			moveCursorTo(rows / 2, cols / 2);
			window::showCursor();
			return window::getCursorPosition();
		}
		
		/**
		 * Attempt to flag the currently selected cell
		 * @param result points to a set where pointers to newly flagged/unflagged cells will be placed.
		 * @return the current game status.
		 */
		GameStatus flagSpace(unordered_set<const Cell*>* result = nullptr) {
			short row, col;
			if(gameStatus == PLAYING && isValidCurrentCursorPosition(row, col)) {
				Cell& cell = cells.at(row).at(col);
				short flagActionStatus = cell.toggleFlag();
				mineCount -= flagActionStatus;
				print(cell);
				
				if(result != nullptr && flagActionStatus != 0) {
					result->insert(&cell);
				}
			}
			
			return getGameStatus();
		}
		
		/**
		 * Attempt to reveal the currently selected cell
		 * @param result points to a set where pointers to newly revealed cells will be placed
		 * @return the current game status.
		 */
		GameStatus revealSpace(unordered_set<const Cell*>* result = nullptr) {
			short row, col;
			if(isValidCurrentCursorPosition(row, col)) {
				if(gameStatus == UNSTARTED) {
					init();
				}
				
				if(gameStatus == PLAYING) {
					Cell& cell = cells.at(row).at(col);
					State cellState = cell.reveal();
					print(cell);
					
					if(result != nullptr && cellState != FAIL) {
						result->insert(&cell);
					}
					
					switch(cellState) {
						case MINE: {
							lose();
							break;
						}
						case BLANK: {
							unordered_set<const Cell*> neighbors = getAdjacentCells(row, col);
							for(const Cell* neighbor : neighbors) {
								moveCursorTo(neighbor->row, neighbor->col);
								revealSpace(result);
							}
						}
						case NUMBER: { //fall-through because these are common to both BLANK and NUMBER cases
							if(--remainingSpaces == 0) {
								win();
							}
						}
					}
				}
			}
			
			return getGameStatus();
		}
		
		/**
		 * Attempt to chord the currently selected cell
		 * @param result points to a set where pointers to newly revealed cells will be placed
		 * @return the current game status.
		 */
		GameStatus chordSpace(unordered_set<const Cell*>* result = nullptr) {
			short row, col;
			if(gameStatus == PLAYING && isValidCurrentCursorPosition(row, col)) {
				Cell& cell = cells.at(row).at(col);
				short number = cell.getNumber();
				if(number) {
					//Count adjacent flags
					unordered_set<const Cell*> neighbors = getAdjacentCells(row, col);
					short numNeighboringFlags = 0;
					for(const Cell* neighbor : neighbors) {
						if(neighbor->flagged)
							numNeighboringFlags++;
					}
					
					//If count is correct, reveal all non-flagged cells
					if(numNeighboringFlags == number) {
						for(const Cell* neighbor : neighbors) {
							moveCursorTo(neighbor->row, neighbor->col);
							revealSpace(result);
						}
					}
				}
			}
			
			return getGameStatus();
		}
};

} using namespace field;

namespace evaluator {

class RandomFieldEvaluator : public FieldEvaluator {
	bool evaluate(const Field& field) const {
		return true;
	}
};

class SafeStartFieldEvaluator : public FieldEvaluator {
	bool evaluate(const Field& field) const {
		//TODO
		return true;
	}
};

} using namespace evaluator;