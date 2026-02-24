#include "ms.h"
// #include "value_reorderable_list.h"
#include <fstream>

namespace solver {

class NoValidMoveException : public exception {
public:
	const char* what() const noexcept override {
		return "No move is available except by making a random guess";
	}
};

class Solvercell;

struct PossibilitySet {
	short numAdjacentMines;
	unordered_set<Solvercell*> possibilities;
	
	PossibilitySet(const short& desiredNumAdjacentMines
	) :		numAdjacentMines(desiredNumAdjacentMines) {}
};

class Solvercell final : public Cell {
	friend class Solver;
	private:
		State state;
		PossibilitySet* possibilitySet;
		
		Solvercell(	const short& desiredRow,
					const short& desiredCol
		) :		Cell(desiredRow, desiredCol),
				state(UNINITIALIZED),
				possibilitySet(nullptr) {}
	
	public:
		~Solvercell() {
			delete possibilitySet;
		}
};

class Solver final : public Field {
	private:
		Minefield& minefield;
		// value_reorderable_list<Solvercell*> workingList;
		
		Solvercell* at(const short& row, const short& col) const {
			return static_cast<Solvercell*>(Field::at(row, col));
		}
		
		unordered_set<Solvercell*> getAdjacentCells(Solvercell* cell) const {
			unordered_set<Solvercell*> result;
			for(auto& baseCell : Field::getAdjacentCells(cell)) {
				result.insert(static_cast<Solvercell*>(baseCell));
			}
			return result;
		}
		
		unordered_set<Solvercell*> getAllCells() const {
			unordered_set<Solvercell*> result;
			for(auto& baseCell : Field::getAllCells()) {
				result.insert(static_cast<Solvercell*>(baseCell));
			}
			return result;
		}
		
		/**
		 * Reveal one truly random space on the board. Used only for the first move of the game.
		 * @param result points to a set where pointers to newly revealed cells will be placed.
		 * @return the cell that was revealed.
		 */
		template<typename ContainerType = unordered_set<Minecell*>>
		Minecell* revealRandomSpace(ContainerType* const result) {
			//Construct a distribution which, when given a randomizer, can produce a number that refers to a unique cell in the field space
			uniform_int_distribution<> generateNumberInRangeUsing(0, (minefield.getRows() * minefield.getCols()) - 1);
			int index = generateNumberInRangeUsing(randomizer);
			short candidateRow = index / minefield.getCols();
			short candidateCol = index % minefield.getCols();
			
			return minefield.revealSpace(candidateRow, candidateCol, result);
		}
		
		/**
		 * When a cell is revealed to be a number or a blank space, remove it as a possibility for all other possibility sets, because we now know it cannot be a mine.
		 * Reveal operations can turn cells from UNINITIALIZED to NUMBER, and we might be in the middle of a reveal operation. So we must be careful to only remove from possibility sets that are already initialized (check against nullptr)
		 */
		void removeSpaceFromAllAdjacentSolverCellPossibilitySets(Solvercell* cell) {
			unordered_set<Solvercell*> neighbors = getAdjacentCells(cell);
			for(Solvercell* neighbor : neighbors) {
				if(neighbor->state == NUMBER && neighbor->possibilitySet != nullptr) {
					neighbor->possibilitySet->possibilities.erase(cell);
					
					if(neighbor->possibilitySet->numAdjacentMines > neighbor->possibilitySet->possibilities.size()) {
						throw logic_error("Possibility set contains more mines than possible cells after removing a cell from possibilites");
					}
					
					//If possibilities size is reduced to (and now equals) mine count, we know the remaining possibilities (if any) are all mines and can be flagged.
					//N mines in N spaces, move this solver cell to the front of the working list so that it gets processed next.
					if(neighbor->possibilitySet->possibilities.size() == neighbor->possibilitySet->numAdjacentMines) {
						//TODO: move this solver cell to the front of the working list
					}
				}
			}
		}
		
		/**
		 * When a cell is flagged, update it as a mine in all other possibility sets (credit each set with one mine, then remove the flagged space from each set).
		 */
		void flagSpaceInAllAdjacentSolverCellPossibilitySets(Solvercell* cell) {
			unordered_set<Solvercell*> neighbors = getAdjacentCells(cell);
			for(Solvercell* neighbor : neighbors) {
				if(neighbor->state == NUMBER && neighbor->possibilitySet != nullptr) {
					neighbor->possibilitySet->numAdjacentMines--; //decrement the number of adjacent mines (we found one)
					neighbor->possibilitySet->possibilities.erase(cell); //remove the cell from the possibilites (the remaining mines must be in the other remaining cells)
					
					if(neighbor->possibilitySet->numAdjacentMines < 0) {
						throw logic_error("Possibility set contains negative mine count after flagging a cell");
					}
					
					//If mine count is reduced to 0, we know the remaining possibilities (if any) are safe to reveal.
					//0 mines in N spaces, move this solver cell to the front of the working list so that it gets processed next.
					if(neighbor->possibilitySet->numAdjacentMines == 0) {
						//TODO: move this solver cell to the front of the working list
					}
				}
			}
		}
		
		template<typename ContainerType = unordered_set<Minecell*>>
		void processResults(ContainerType* const result) {
			//Loop through results, set all cells with the appropriate state, and check how newly revealed or flagged cells affect PREEXISTING cells
			for(Minecell* cell : *result) {
				Solvercell* solvercell = at(cell->row, cell->col);
				
				State currentState = cell->getState();
				solvercell->state = currentState;
				if(currentState == BLANK || currentState == NUMBER) {
					removeSpaceFromAllAdjacentSolverCellPossibilitySets(solvercell);
				}
				else if(currentState == FAIL && cell->isFlagged()) {
					flagSpaceInAllAdjacentSolverCellPossibilitySets(solvercell);
				}
			}
			
			//Loop through results and initialize all NUMBER cells with possibility sets
			//Pre-requirement: all result cells have a state. This is because we need to examine neighbors to build each possibility set.
			for(Minecell* cell : *result) {
				Solvercell* solvercell = at(cell->row, cell->col);
				if(solvercell->state == NUMBER) {
					PossibilitySet* newPossibilitySet = new PossibilitySet(cell->getNumber());
					unordered_set<Solvercell*> neighbors = getAdjacentCells(solvercell);
					for(Solvercell* neighbor : neighbors) {
						if(neighbor->state == UNINITIALIZED) {
							newPossibilitySet->possibilities.insert(neighbor);
						}
					}
					
					if(newPossibilitySet->numAdjacentMines > newPossibilitySet->possibilities.size()) {
						throw logic_error("Possibility set contains more mines than possible cells after construction");
					}
					
					solvercell->possibilitySet = newPossibilitySet;
					
					//TODO: add this solver cell to the front of the working list
				}
			}
			
			//Log results
			ofstream logFile("log.txt");
			logFile << "|\n|\n|\n|\n|" << endl;
			for(const auto& minecell : *result) {
				auto cell = at(minecell->row, minecell->col);
				logFile << "================================================" << endl;
				if(cell->state == NUMBER) {
					logFile << "Data for cell [" << cell->row << ", " << cell->col << "]: Possibility Set with " << cell->possibilitySet->numAdjacentMines << " mines in the following cells:" << endl;
					for(const auto& possibility : cell->possibilitySet->possibilities) {
						logFile << "\t[" << possibility->row << ", " << possibility->col << "]" << endl;
					}
				}
			}
			logFile.close();
		}
	
	public:
		Solver(Minefield& desiredMinefield) : Field(desiredMinefield.getRows(), desiredMinefield.getCols()), minefield(desiredMinefield) {
			cells = vector<vector<Cell*>>();
			for(short row = 0; row < rows; row++) {
				cells.push_back(vector<Cell*>());
				for(short col = 0; col < cols; col++) {
					cells.at(row).push_back(new Solvercell(row, col));
				}
			}
		}
	
		/**
		 * Advance the solver one step, i.e. make one game move
		 * @param result points to a set where pointers to newly revealed cells will be placed.
		 * @return the cell that was acted upon.
		 */
		template<typename ContainerType = unordered_set<Minecell*>>
		Minecell* step(ContainerType* const result) {
			if(minefield.getGameStatus() == UNSTARTED) {
				Minecell* resultingCell = revealRandomSpace(result);
				processResults(result);
				return resultingCell;
			}
			
			//What order to search through cells/possibility sets?
			//    - Solution: keep some custom list of cells to look at.
			//			Store pointers to SolverCell s - that way we can pull adjacent cells and reorder them to the top when relevant
			
			//Loop through all possibility sets - 4 possible cases to check:
			//case 1: set has mine count of 0 && possibilities of size 0
			//     action: cell is solved, remove it from working set, and continue looping
			//case 2: set has mine count of 0
			//     action: reveal any (aka first) cell in the set, process results, and return
			//case 3: set has positive mine count of N && set has possibilities of size N
			//     action: flag any (aka first) cell in the set, process results, and return
			//case 4: otherwise
			//     action: check if the set is a subset of another set, and continue looping
			//          keep track of some kind of "isDirty" marker or similar, to track whether a given set has been evaluated already for being a subset?
			//               **only need to recheck for being a subset when nearby cells are updated**
			//			or, instead of skipping already checked cells, just keep them at the back of the list and keep the front populated with cells that have had neighbor updates recently
			//     can try having this be a "do nothing" case at first, to see how well the other cases do at advancing the board
		}
		
		/**
		 * Continue advancing the solver until the game is won or lost or guessing is required
		 * @return whether the game was won.
		 */
		bool solve() {
			return true;
			// try {
				// do {
					// unordered_set<Minecell*> resultSet;
					// step(&resultSet);
				// } while(minefield.getGameStatus() == PLAYING);
				// return minefield.getGameStatus() == WON;
			// } catch(NoValidMoveException e) {
				// return false;
			// }
		}
};

}