#include "ms.h"

namespace solver {

class Solver final {
	private:
		Minefield& minefield;
		
		/**
		 * Reveal one truly random space on the board. Used only for the first move of the game.
		 * @param result points to a set where pointers to newly revealed cells will be placed.
		 * @return the cell that was revealed.
		 */
		template<typename ContainerType = unordered_set<Minecell*>>
		Minecell* revealRandomSpace(ContainerType* const result = nullptr) {
			//Construct a distribution which, when given a randomizer, can produce a number that refers to a unique cell in the field space
			uniform_int_distribution<> generateNumberInRangeUsing(0, (minefield.getRows() * minefield.getCols()) - 1);
			int index = generateNumberInRangeUsing(randomizer);
			short candidateRow = index / minefield.getCols();
			short candidateCol = index % minefield.getCols();
			
			return minefield.revealSpace(candidateRow, candidateCol, result);
		}
	
	public:
		Solver(Minefield& desiredMinefield) : minefield(desiredMinefield) {}
	
		/**
		 * Advance the solver one step, i.e. make one game move
		 * @param result points to a set where pointers to newly revealed cells will be placed.
		 * @return the cell that was acted upon.
		 */
		template<typename ContainerType = unordered_set<Minecell*>>
		Minecell* step(ContainerType* const result = nullptr) {
			if(minefield.getGameStatus() == UNSTARTED) {
				return revealRandomSpace(result);
			}
		}
		
		/**
		 * Continue advancing the solver until the game is won or lost
		 * @return whether the game was won.
		 */
		bool solve() {
			//TODO implement
			return true;
		}
};

}