#include <iostream>
#include <windows.h>
#include <time.h>
#include <vector>
#include <functional>
#include <unordered_set>
#include <stdexcept>
using namespace std;

const string CHARACTER_HIDDEN = "\x1B[47m \033[0m";
const string CHARACTER_BLANK = " ";
const string CHARACTER_FLAG = "\x1B[91m\x1B[47mľ\033[0m\033[0m";
const string CHARACTER_MINE = "☼";

const string RED = "\x1B[91m";
const string GREEN = "\x1B[92m";
const string YELLOW = "\x1B[93m";
const string BLUE = "\x1B[94m";
const string END = "\033[0m";

struct Difficulty {
	public:
		COORD dimensions;
		short mines;
		
		Difficulty() {}
		Difficulty(COORD desiredDimensions, short desiredMines) : dimensions(desiredDimensions), mines(desiredMines) {}
};

Difficulty difficulties[3] = {
	Difficulty(COORD{9, 9}, 10),
	Difficulty(COORD{16, 16}, 40),
	Difficulty(COORD{24, 24}, 99)
};

typedef struct _CONSOLE_FONT_INFOEX
{
		ULONG cbSize;
		DWORD nFont;
		COORD dwFontSize;
		UINT  FontFamily;
		UINT  FontWeight;
		WCHAR FaceName[LF_FACESIZE];
}CONSOLE_FONT_INFOEX, *PCONSOLE_FONT_INFOEX;

#ifdef __cplusplus
extern "C" {
#endif
BOOL WINAPI SetCurrentConsoleFontEx(HANDLE hConsoleOutput, BOOL bMaximumWindow, PCONSOLE_FONT_INFOEX lpConsoleCurrentFontEx);
BOOL WINAPI GetCurrentConsoleFontEx(HANDLE hConsoleOutput, BOOL bMaximumWindow, PCONSOLE_FONT_INFOEX lpConsoleCurrentFontEx);
#ifdef __cplusplus
}
#endif

void setWindowRaw(const short& width, const short& height) {
	_COORD coord; 
	coord.X = width; 
	coord.Y = height; 

	_SMALL_RECT rect;
	rect.Top = 0; 
	rect.Left = 0; 
	rect.Bottom = height - 1; 
	rect.Right = width - 1; 

	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);      // Get Handle 
	SetConsoleScreenBufferSize(handle, coord);            // Set Buffer Size 
	SetConsoleWindowInfo(handle, true, &rect);            // Set Window Size 
}

void setWindow(const short& width, const short& height) {
	setWindowRaw(1, 1); //Avoids issues with text wrapping and scroll bars
	setWindowRaw(width, height);
}

void setConsoleCursorVisibility(const int& size, const bool& showFlag = true)
{
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_CURSOR_INFO cursorInfo;

	GetConsoleCursorInfo(out, &cursorInfo);
	cursorInfo.bVisible = showFlag;	//cursor visibility
	cursorInfo.dwSize = size;		//portion of the character to fill
	SetConsoleCursorInfo(out, &cursorInfo);
}

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
		const short row;
		const short col;
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
		bool init(const State& desiredState, const short& desiredNumber = 0) {
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
			}
		}
		
		void toggleFlag() {
			if(flagged) {
				flagged = false;
			}
			else if(hidden) {
				flagged = true;
			}
		}
		
		/**
		 * Attempt to reveal the cell
		 * @return the state, if it was newly revealed. Else, return FAIL State to indicate that no reveal was performed.
		 */
		State reveal() {
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
		/**
		 * Attempt to get the cell's state
		 * @return the state, if it is public knowledge. Else, return FAIL State to indicate that the requested information is not available.
		 */
		State getState() {
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
		short getNumber() const {
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
		const short verticalOffset;			//position of the board downwards from the screen origin
		const short horizontalOffset;		//position of the board to the right from the screen origin
		const FieldEvaluator& evaluator;	//arbitrary decisionmaking engine that determines if a field is valid to play, based on the state at initialization

		//updated thoughout the game
		GameStatus gameStatus = UNSTARTED;	//current status of the game
		short remainingSpaces;				//number of safe spaces (not mines) that remain hidden
		vector<vector<Cell>> cells;			//holds all cells
		
		void printToScreen() const {
			for(short row = 0; row < rows; row++) {
				moveCursorTo(row, 0);
				for(short col = 0; col < cols; col++) {
					print(cells[row][col]);
				}
			}
		}
		
		void resetBoard() {
			cells = vector<vector<Cell>>();
			for(short row = 0; row < rows; row++) {
				cells.push_back(vector<Cell>());
				for(short col = 0; col < cols; col++) {
					cells[row].push_back(Cell(row, col));
				}
			}
		}
		
		void init() {
			do {
				resetBoard();
				
				//Seed board with mines
				srand(time(0));
				for(short numberOfMinesPlaced = 0; numberOfMinesPlaced < mines; )
				{
					int randomNumber = rand() % (rows * cols);
					Cell& candidate = cells[randomNumber/cols][randomNumber%cols];
					if(candidate.init(MINE)) {
						numberOfMinesPlaced++;
					}
				}
			} while(!evaluator.evaluate(*this));
			
			//Generate adjacent numbers
			for(short row = 0; row < rows; row++)
			{
				for(short col = 0; col < cols; col++)
				{
					Cell& cell = cells[row][col];
					
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
		}
		
		void moveCursorTo(const short& desiredRow, const short& desiredCol) const {
			SetConsoleCursorPosition(
				GetStdHandle(STD_OUTPUT_HANDLE),
				COORD{short(desiredCol + horizontalOffset), short(desiredRow + verticalOffset)}
			);
		}
		
		COORD getCursorPosition() const {
			CONSOLE_SCREEN_BUFFER_INFO csbi;
			GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
			return csbi.dwCursorPosition;
		}
		
		void getCoordsFromCursorPosition(short& row, short& col) const {
			COORD cursorPosition = getCursorPosition();
			row = cursorPosition.Y - verticalOffset;
			col = cursorPosition.X - horizontalOffset;
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
		
		bool isValidSpace(const short& row, const short& col) const {
			return row >= 0 && row < rows
				&& col >= 0 && col < cols;
		}
		
		bool isValidCurrentCursorPosition(short& row, short& col) const {
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
						result.insert(&cells[row][col]);
				}
			}
			return result;
		}
		
		void lose() {
			gameStatus = LOST;
			printToScreen();
		}
		
		void win() {
			gameStatus = WON;
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
				verticalOffset(desiredPosition.Y),
				horizontalOffset(desiredPosition.X),
				evaluator(desiredFieldEvaluator)
		{
			remainingSpaces = rows * cols - mines;
			resetBoard();
			printToScreen();
		}
		
		COORD getDimensions() const {
			return COORD{cols, rows};
		}
		
		COORD initializeCursor() const {
			moveCursorTo(rows / 2, cols / 2);
			setConsoleCursorVisibility(100);
			return getCursorPosition();
		}
		
		void flagSpace() {
			if(gameStatus != PLAYING) {
				return;
			}
			
			short row, col;
			if(!isValidCurrentCursorPosition(row, col)) {
				return;
			}
			
			Cell& cell = cells[row][col];
			cell.toggleFlag();
			print(cell);
		}
		
		/**
		 * Attempt to reveal the currently selected cell
		 * @return a set containing all newly revealed cells
		 */
		unordered_set<const Cell*> revealSpace() {
			unordered_set<const Cell*> result;
			short row, col;
			if(!isValidCurrentCursorPosition(row, col)) {
				return result;
			}
			
			if(gameStatus == UNSTARTED) {
				init();
			}
			else if(gameStatus != PLAYING) {
				return result;
			}
		
			Cell& cell = cells[row][col];
			result.insert(&cell);
			State cellState = cell.reveal();
			print(cell);
			
			switch(cellState) {
				case MINE: {
					lose();
					break;
				}
				case BLANK: {
					unordered_set<const Cell*> neighbors = getAdjacentCells(row, col);
					for(const Cell* neighbor : neighbors) {
						moveCursorTo(neighbor->row, neighbor->col);
						unordered_set<const Cell*> newlyRevealedCells = revealSpace();
						result.insert(newlyRevealedCells.begin(), newlyRevealedCells.end());
					}
				}
				case NUMBER: { //fall-through because these are common to both BLANK and NUMBER cases
					if(--remainingSpaces == 0) {
						win();
					}
				}
			}
			
			return result;
		}
		
		/**
		 * Attempt to chord the currently selected cell
		 * @return a set containing all newly revealed cells
		 */
		unordered_set<const Cell*> chordSpace() {
			unordered_set<const Cell*> result;
			if(gameStatus != PLAYING) {
				return result;
			}
			
			short row, col;
			if(!isValidCurrentCursorPosition(row, col)) {
				return result;
			}
			
			Cell& cell = cells[row][col];
			short number = cell.getNumber();
			if(!number) {
				return result;
			}
			
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
					unordered_set<const Cell*> newlyRevealedCells = revealSpace();
					result.insert(newlyRevealedCells.begin(), newlyRevealedCells.end());
				}
			}
		}
};

class RandomFieldEvaluator : public FieldEvaluator {
	bool evaluate(const Field& field) const {
		return true;
	}
};

class SafeStartFieldEvaluator : public FieldEvaluator {
	bool evaluate(const Field& field) const {
		//TODO
	}
};
