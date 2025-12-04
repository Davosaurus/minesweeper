#include "solver.h"

namespace field {

namespace evaluators {

bool random(const Minefield& field, const short& row, const short& col) {
	return true;
}

bool safeStart(const Minefield& field, const short& row, const short& col) {
	if(field.getMines() >= field.getRows() * field.getCols()) {
		return true;
	}
	
	Minefield prospectiveField = Minefield(field);
	prospectiveField.revealSpace(row, col);
	return prospectiveField.getGameStatus() != LOST;
}

bool safeStartPlus(const Minefield& field, const short& row, const short& col) {
	int numCells = field.getRows() * field.getCols();
	int numRevealedCells = 0;
	
	Minefield prospectiveField = Minefield(field);
	
	for(short rowModifier = -1; rowModifier <= 1; rowModifier++) {
		for(short colModifier = -1; colModifier <= 1; colModifier++) {
			if(prospectiveField.isValidSpace(row + rowModifier, col + colModifier)) {
				numRevealedCells++;
				prospectiveField.revealSpace(row + rowModifier, col + colModifier);
			}
		}
	}
	
	if(prospectiveField.getMines() > numCells - numRevealedCells) {
		return true;
	}
	
	return prospectiveField.getGameStatus() != LOST;
}

bool noGuess(const Minefield& field, const short& row, const short& col) {
	Minefield prospectiveField = Minefield(field);
	return solver::Solver(prospectiveField).solve();
}

}

} using namespace field;