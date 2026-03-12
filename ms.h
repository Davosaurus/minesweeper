#include "window.h"

namespace field {

enum State {
	BLANK,
	MINE,
	NUMBER,
	UNINITIALIZED,
	FAIL
};

class Cell {
	protected:
		Cell(	const short& desiredRow,
				const short& desiredCol
		) :		row(desiredRow),
				col(desiredCol) {}
	
	public:
		const short row;
		const short col;
};

class Minecell final : public Cell {
	friend class Minefield;
	private:
		bool hidden = true;
		bool flagged = false;
		State state;
		short number;
	
		Minecell(	const short& desiredRow,
					const short& desiredCol
		) :		Cell(desiredRow, desiredCol),
				state(UNINITIALIZED) {}
		
		/**
		 * Attempt to initialize the cell
		 * Returns whether attempt was successful
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
		
		/**
		 * Whether the cell is flagged or not
		 * @return true if the cell is flagged.
		 */
		const bool isFlagged() const {
			return flagged;
		}
};

enum GameStatus {
	UNSTARTED,
	PLAYING,
	WON,
	LOST
};

class Field {
	protected:
		const short rows;					//number of rows
		const short cols;					//number of columns
		vector<vector<Cell*>> cells;		//holds pointers to all cells
		
		unordered_set<Cell*> getAdjacentCells(Cell* cell) const {
			unordered_set<Cell*> result;
			for(short rowModifier = -1; rowModifier <= 1; rowModifier++) {
				for(short colModifier = -1; colModifier <= 1; colModifier++) {
					const short row = cell->row + rowModifier;
					const short col = cell->col + colModifier;
					if(!(row == cell->row && col == cell->col)	//coordinates are not the same as the original space
						&& isValidSpace(row, col)) {			//coordinates requested are within field bounds
						result.insert(at(row, col));
					}
				}
			}
			return result;
		}
		
		unordered_set<Cell*> getAllCells() const {
			unordered_set<Cell*> result;
			for(const vector<Cell*>& row : cells) {
				for(Cell* cell : row) {
					result.insert(cell);
				}
			}
			return result;
		}
	
	public:
		/**
		 * Default Constructor
		 */
		Field(const short& desiredRows, const short& desiredCols)
				: rows(desiredRows), cols(desiredCols) {}
		
		/**
		 * @return pointer to the cell at the given coordinates.
		 */
		Cell* at(const short& row, const short& col) const {
			return cells.at(row).at(col);
		}
		
		/**
		 * @return number of rows.
		 */
		const short getRows() const {
			return rows;
		}
		
		/**
		 * @return number of cols.
		 */
		const short getCols() const {
			return cols;
		}
		
		/**
		 * @return whether the given row and column refer to a space within the bounds of the field.
		 */
		const bool isValidSpace(const short& row, const short& col) const {
			return row >= 0 && row < rows
				&& col >= 0 && col < cols;
		}
		
		virtual ~Field() {
			for(vector<Cell*>& row : cells) {
				for(Cell* cell_ptr : row) {
					delete cell_ptr;
				}
			}
			cells.clear();
		}
};

class Minefield;

namespace evaluators {

bool random(const Minefield& field, const short& row, const short& col);
bool safeStart(const Minefield& field, const short& row, const short& col);
bool safeStartPlus(const Minefield& field, const short& row, const short& col);
bool noGuess(const Minefield& field, const short& row, const short& col);

unordered_map<string, const function<bool(const Minefield& field, const short& row, const short& col)>> evaluator = {
	{"random", random},
	{"safeStart", safeStart},
	{"safeStartPlus", safeStartPlus},
	{"noGuess", noGuess}
};

}

class Minefield final : public Field {
	private:
		//set at construction
		const int mines;					//number of mines
		const COORD positionOffset;			//position of the board's top-left corner in the screen coordinate system
		const function<bool(const Minefield& field, const short& row, const short& col)>* const evaluate;
											//arbitrary function that determines if a field is valid to play, based on the state at initialization
		
		//updated thoughout the game
		GameStatus gameStatus = UNSTARTED;	//current status of the game
		int remainingSpaces;				//number of safe spaces (not mines) that remain hidden
		int mineCount;						//number of mines minus the number of placed flags
		ULONGLONG startTime;				//system time when the game was started
		ULONGLONG endTime;					//system time when the game was completed
		
		/**
		 * Reset the field by reinitializing the cells vector with newly created cells (uninitialized state)
		 */
		void resetBoard() {
			cells = vector<vector<Cell*>>();
			for(short row = 0; row < rows; row++) {
				cells.push_back(vector<Cell*>());
				for(short col = 0; col < cols; col++) {
					cells.at(row).push_back(new Minecell(row, col));
				}
			}
		}
		
		unordered_set<Minecell*> getAdjacentCells(Minecell* cell) const {
			unordered_set<Minecell*> result;
			for(auto& baseCell : Field::getAdjacentCells(cell)) {
				result.insert(static_cast<Minecell*>(baseCell));
			}
			return result;
		}
		
		unordered_set<Minecell*> getAllCells() const {
			unordered_set<Minecell*> result;
			for(auto& baseCell : Field::getAllCells()) {
				result.insert(static_cast<Minecell*>(baseCell));
			}
			return result;
		}
		
		/**
		 * Initialize the field by placing mines and generating all blank and number spaces
		 * @param row is the row of the cell about to be revealed.
		 * @param col is the column of the cell about to be revealed.
		 * @return a pointer to the cell about to be revealed.
		 */
		Minecell* init(const short row, const short col) {
			int numCells = rows * cols;
			//Construct a distribution which, when given a randomizer, can produce a number that refers to a unique cell in the field space
			uniform_int_distribution<> generateNumberInRangeUsing(0, numCells - 1);
			
			gameStatus = PLAYING;
			
			do {
				resetBoard();
				
				//Generate mines
				for(int numberOfMinesPlaced = 0; numberOfMinesPlaced < mines; ) {
					int index = generateNumberInRangeUsing(randomizer);
					short candidateRow = index / cols;
					short candidateCol = index % cols;
					
					//Optimizations for some evaluators
					if(evaluate != &field::evaluators::evaluator["random"]) {
						if(mines < numCells
								&& candidateRow == row
								&& candidateCol == col) {
							continue;
						}
						
						if(evaluate == &field::evaluators::evaluator["safeStartPlus"]
								|| evaluate == &field::evaluators::evaluator["noGuess"]) {
							short numRevealedCells = 0;
							for(short rowModifier = -1; rowModifier <= 1; rowModifier++) {
								for(short colModifier = -1; colModifier <= 1; colModifier++) {
									if(isValidSpace(row + rowModifier, col + colModifier)) {
										numRevealedCells++;
									}
								}
							}
							
							if(mines <= numCells - numRevealedCells
									&& candidateRow >= row - 1 && candidateRow <= row + 1
									&& candidateCol >= col - 1 && candidateCol <= col + 1) {
								continue;
							}
						}
					}
					
					Minecell* candidate = at(candidateRow, candidateCol);
					if(candidate->init(MINE)) {
						numberOfMinesPlaced++;
					}
				}
				
				//Generate adjacent numbers
				for(Minecell* cell : getAllCells()) {
					short sumOfNeighboringMines = 0;
					unordered_set<Minecell*> neighbors = getAdjacentCells(cell);
					for(Minecell* neighbor : neighbors) {
						if(neighbor->state == MINE) {
							sumOfNeighboringMines++;
						}
					}
					
					cell->init(NUMBER, sumOfNeighboringMines);
				}
			} while(!(*evaluate)(*this, row, col));
			
			startTime = GetTickCount64(); //reset timer to eliminate delay from board generation and validation
			return at(row, col);
		}
		
		void getFieldCoordinatesFromScreenCoord(const COORD& screenPosition, short& row, short& col) const {
			row = screenPosition.Y - positionOffset.Y;
			col = screenPosition.X - positionOffset.X;
		}
		
		template<typename ContainerType = unordered_set<Minecell*>>
		void lose(ContainerType* const result = nullptr) {
			gameStatus = LOST;
			endTime = GetTickCount64();
			
			if(result != nullptr) {
				for(Minecell* cell : getAllCells()) {
					if(cell->state == MINE || (cell->hidden && cell->flagged)) {
						cell->hidden = false;
						result->insert(result->end(), cell);
					}
				}
			}
		}
		
		void win() {
			gameStatus = WON;
			endTime = GetTickCount64();
		}
	
	public:
		Minefield(
				const COORD& desiredSize,
				const int& desiredMines,
				const COORD& desiredPosition,
				const function<bool(const Minefield& field, const short& row, const short& col)>* const desiredFieldEvaluator
		) :
				Field(desiredSize.Y, desiredSize.X),
				mines(desiredMines),
				mineCount(desiredMines),
				positionOffset(desiredPosition),
				evaluate(desiredFieldEvaluator)
		{
			remainingSpaces = rows * cols - mines;
			resetBoard();
		}
		
		//Copy constructor
		Minefield(const Minefield& other) :
				Field(other.rows, other.cols),
				mines(other.mines),
				mineCount(other.mines),
				positionOffset(other.positionOffset),
				evaluate(other.evaluate),
				gameStatus(other.gameStatus),
				remainingSpaces(other.remainingSpaces),
				startTime(other.startTime),
				endTime(other.endTime)
		{
			cells = vector<vector<Cell*>>();
			for(short row = 0; row < rows; row++) {
				cells.push_back(vector<Cell*>());
				for(short col = 0; col < cols; col++) {
					cells.at(row).push_back(new Minecell(*other.at(row, col)));
				}
			}
		}
		
		/**
		 * @return pointer to the cell at the given coordinates.
		 */
		Minecell* at(const short& row, const short& col) const {
			return static_cast<Minecell*>(Field::at(row, col));
		}
		
		/**
		 * @return pointer to the cell at the given screen coordinates.
		 */
		Minecell* at(const COORD& screenPosition) {
			short row, col;
			getFieldCoordinatesFromScreenCoord(screenPosition, row, col);
			return at(row, col);
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
		 * @return a coordinate that holds the position of the given cell, in the screen coordinate system.
		 */
		const COORD getPositionOf(Minecell* cell) const {
			return COORD{short(cell->col + positionOffset.X), short(cell->row + positionOffset.Y)};
		}
		
		/**
		 * @return the current game status.
		 */
		const GameStatus getGameStatus() const {
			return gameStatus;
		}
		
		/**
		 * @return the current number of mines in the game. This refers to the total, regardless of placed flags or revealed spaces.
		 */
		const int getMines() const {
			return mines;
		}
		
		/**
		 * @return the current "Mine Count" of the game. Mine Count refers to the total number of mines in the game, minus the number of placed flags.
		 */
		const int getMineCount() const {
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
		 * Attempt to flag the given cell
		 * @param cell must point to a valid cell in this field.
		 * @param result points to a set where pointers to newly flagged/unflagged cells will be placed.
		 * @return the given cell.
		 */
		template<typename ContainerType = unordered_set<Minecell*>>
		Minecell* flagSpace(Minecell* cell, ContainerType* const result = nullptr) {
			if(gameStatus == PLAYING) {
				const short flagActionStatus = cell->toggleFlag();
				mineCount -= flagActionStatus;
				
				if(result != nullptr && flagActionStatus != 0) {
					result->insert(result->end(), cell);
				}
			}
			
			return cell;
		}
		
		template<typename ContainerType = unordered_set<Minecell*>>
		Minecell* flagSpace(const short& row, const short& col, ContainerType* const result = nullptr) {
			return flagSpace(at(row, col), result);
		}
		
		template<typename ContainerType = unordered_set<Minecell*>>
		Minecell* flagSpace(const COORD& screenPosition, ContainerType* const result = nullptr) {
			return flagSpace(at(screenPosition), result);
		}
		
		/**
		 * Attempt to reveal the given cell
		 * @param cell must point to a valid cell in this field.
		 * @param result points to a set where pointers to newly revealed cells will be placed.
		 * @return the given cell.
		 */
		template<typename ContainerType = unordered_set<Minecell*>>
		Minecell* revealSpace(Minecell* cell, ContainerType* const result = nullptr) {
			if(gameStatus == UNSTARTED) {
				cell = init(cell->row, cell->col);
			}
			
			if(gameStatus == PLAYING) {
				const State cellState = cell->reveal();
				
				if(result != nullptr && cellState != FAIL) {
					result->insert(result->end(), cell);
				}
				
				switch(cellState) {
					case MINE: {
						lose(result);
						break;
					}
					case BLANK: {
						unordered_set<Minecell*> neighbors = getAdjacentCells(cell);
						for(Minecell* neighbor : neighbors) {
							revealSpace(neighbor, result);
						}
					}
					case NUMBER: { //fall-through because these are common to both BLANK and NUMBER cases
						if(--remainingSpaces == 0) {
							win();
						}
					}
				}
			}
			
			return cell;
		}
		
		template<typename ContainerType = unordered_set<Minecell*>>
		Minecell* revealSpace(const short& row, const short& col, ContainerType* const result = nullptr) {
			return revealSpace(at(row, col), result);
		}
		
		template<typename ContainerType = unordered_set<Minecell*>>
		Minecell* revealSpace(const COORD& screenPosition, ContainerType* const result = nullptr) {
			return revealSpace(at(screenPosition), result);
		}
		
		/**
		 * Attempt to chord the given cell
		 * @param cell must point to a valid cell in this field.
		 * @param result points to a set where pointers to newly revealed cells will be placed.
		 * @return the given cell.
		 */
		template<typename ContainerType = unordered_set<Minecell*>>
		Minecell* chordSpace(Minecell* cell, ContainerType* const result = nullptr) {
			if(gameStatus == PLAYING) {
				const short number = cell->getNumber();
				if(number) {
					//Count adjacent flags
					unordered_set<Minecell*> neighbors = getAdjacentCells(cell);
					short numNeighboringFlags = 0;
					for(Minecell* neighbor : neighbors) {
						if(neighbor->isFlagged()) {
							numNeighboringFlags++;
						}
					}
					
					//If count is correct, reveal all non-flagged cells (attempt to reveal all adjacent cells, flagged cells will be ignored)
					if(numNeighboringFlags == number) {
						for(Minecell* neighbor : neighbors) {
							revealSpace(neighbor, result);
						}
					}
				}
			}
			
			return cell;
		}
		
		template<typename ContainerType = unordered_set<Minecell*>>
		Minecell* chordSpace(const short& row, const short& col, ContainerType* const result = nullptr) {
			return chordSpace(at(row, col), result);
		}
		
		template<typename ContainerType = unordered_set<Minecell*>>
		Minecell* chordSpace(const COORD& screenPosition, ContainerType* const result = nullptr) {
			return chordSpace(at(screenPosition), result);
		}
};

} using namespace field;
