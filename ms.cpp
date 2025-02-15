#include "ms.h"

COORD windowSize;

struct Difficulty {
	public:
		COORD dimensions;
		int mines;
		
		Difficulty() {}
		Difficulty(const COORD& desiredDimensions, const int& desiredMines) : dimensions(desiredDimensions), mines(desiredMines) {}
};

Difficulty difficulties[3] = {
	Difficulty(COORD{9, 9}, 10),
	Difficulty(COORD{16, 16}, 40),
	Difficulty(COORD{30, 16}, 99)
};

/**
 * Print the given cell to the screen
 */
void print(Cell* const cell, const Field& minefield, const bool& losingMove = false) {
	FlexibleString output;
	
	switch(cell->getState()) {
		case FAIL: //this only means we are not allowed to know the state, i.e. the cell is hidden
			if(cell->isFlagged()) {
				output = color(character::FLAG, text::RED, background::GRAY);
			}
			else {
				output = color(character::BLANK, background::GRAY);
			}
			break;
		case BLANK:
			output = color(character::BLANK, cell->isFlagged() ? background::DARK_RED : background::BLACK);
			break;
		case NUMBER:
			output = color(to_string(cell->getNumber()), text::WHITE, cell->isFlagged() ? background::DARK_RED : background::BLACK);
			break;
		case MINE:
			if(losingMove) {
				output = color(character::MINE, text::WHITE, background::DARK_RED); //mine that was revealed (causing a loss)
			}
			else if(cell->isFlagged()) {
				output = color(character::FLAG, text::RED, background::GRAY); //mine that was correctly flagged
			}
			else {
				output = color(character::MINE, text::WHITE, background::BLACK); //mine that was never revealed or flagged
			}
			break;
	}
	
	window::printInRectangle(output, minefield.getPositionOf(cell));
}

/**
 * Print all the given cells to the screen
 */
template<typename ContainerType>
void print(ContainerType* const cells, const Field& minefield) {
	for(Cell* cell : *cells) {
		print(cell, minefield);
	}
}

void checkForGameEnd(Cell* const cell, const Field& minefield) {
	GameStatus currentGameStatus = minefield.getGameStatus();
	if(currentGameStatus == GameStatus::LOST) {
		window::printInRectangle(color(character::INDICATOR_LOST, text::WHITE, background::RED), COORD{short(windowSize.X / 2), 1});
		
		//Print extra bad space to show which move lost the game
		print(cell, minefield, true);
	}
	else if(currentGameStatus == GameStatus::WON) {
		window::printInRectangle(color(character::INDICATOR_WON, text::WHITE, background::GREEN), COORD{short(windowSize.X / 2), 1});
	}
}

int main() {
	SetConsoleTitle("Minesweeper: \"Graphical!\"");
	
	//Font settings
	CONSOLE_FONT_INFOEX menuFont = {};
	menuFont.cbSize=sizeof(CONSOLE_FONT_INFOEX);
	wcscpy(menuFont.FaceName, L"MS Gothic");
	menuFont.dwFontSize.X = 19;
	menuFont.dwFontSize.Y = 36;
	menuFont.FontWeight = 800;
	
	CONSOLE_FONT_INFOEX defaultGameFont = {};
	defaultGameFont.cbSize=sizeof(CONSOLE_FONT_INFOEX);
	wcscpy(defaultGameFont.FaceName, L"MS Gothic");
	defaultGameFont.dwFontSize.X = 18;
	defaultGameFont.dwFontSize.Y = 18;
	defaultGameFont.FontWeight = 1000;
	
    //Set console code page to UTF-8
    SetConsoleOutputCP(CP_UTF8);
	
	//Disable QuickEdit mode (to prevent pausing when clicking in window)
	DWORD prev_mode;
	GetConsoleMode(window::handleIn, &prev_mode);
	SetConsoleMode(window::handleIn, ENABLE_EXTENDED_FLAGS | (prev_mode & ~ENABLE_QUICK_EDIT_MODE));
	
	//Lock window resizing
	SetWindowLong(GetConsoleWindow(), GWL_STYLE, GetWindowLong(GetConsoleWindow(), GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);
	
	//Set cursor size (full block character)
	window::setCursorSize(100);
	
	while(1)
	{
		windowSize.X = 44;
		windowSize.Y = 13;
		
		COORD inputLocation = COORD{4, short(windowSize.Y - 2)};
		
		window::initialize(menuFont, windowSize);
		window::printEdgeBorders(windowSize, text::BLACK, background::DARK_GRAY, 2, 2, short(inputLocation.Y - 1));
		
		window::printInRectangle("Main Menu", COORD{1, 1});
		window::printInRectangle(">", COORD{short(inputLocation.X - 1), inputLocation.Y});
		window::Table()
				.atPosition(COORD{4, 3})
				.addRow()
						.addCell(color("1", text::GREEN), COORD{6, 1})
						.addCell("Beginner Game")
				.addRow()
						.addCell(color("2", text::YELLOW))
						.addCell("Intermediate Game")
				.addRow()
						.addCell(color("3", text::RED))
						.addCell("Expert Game")
				.addRow()
						.addCell(color("4", text::BLUE))
						.addCell("Custom Game")
				.addRow()
						.addCell("5")
						.addCell("Controls")
				// .addRow()
						// .addCell("6")
						// .addCell("High Scores")
				.print();
		
		Difficulty difficulty;
		CONSOLE_FONT_INFOEX gameFont = defaultGameFont;
		
		short menuChoice;
		try {
			menuChoice = window::getNumericInput(inputLocation, 1, inNumericRange(1, 5));
		} catch(window::UserExitException e) {
			return 0;
		}
		
		try {
			switch(menuChoice) {
				case 1:
				case 2:
				case 3: //preset difficulties
					difficulty = difficulties[menuChoice - 1];
					break;
				case 4: { //custom difficulty
					window::initialize(menuFont, windowSize);
					window::printEdgeBorders(windowSize, text::BLACK, background::DARK_GRAY, 1, 2);
					
					window::printInRectangle("Custom Game", COORD{1, 1});
					
					//Get user input for board dimensions
					window::printInRectangle("Board width?", COORD{4, 3});
					window::printInRectangle(">", COORD{24, 3});
					short boardWidth = window::getNumericInput(COORD{25, 3}, 4, inNumericRange(1, 9999));
					
					window::printInRectangle("Board height?", COORD{4, 4});
					window::printInRectangle(">", COORD{24, 4});
					short boardHeight = window::getNumericInput(COORD{25, 4}, 4, inNumericRange(1, 9999));
					
					//Get user input for number of mines, and set the custom difficulty accordingly
					window::printInRectangle("Number of mines?", COORD{4, 5});
					window::printInRectangle(">", COORD{24, 5});
					difficulty = Difficulty (
							COORD{boardWidth, boardHeight},
							window::getNumericInput(COORD{25, 5}, 8, inNumericRange(0, boardWidth * boardHeight))
					);
					break;
				}
				case 5: { //controls
					window::initialize(menuFont, windowSize);
					window::printEdgeBorders(windowSize, text::BLACK, background::DARK_GRAY, 2, 2, short(inputLocation.Y - 1));
					
					window::printInRectangle("Controls", COORD{1, 1});
					window::printInRectangle(">", COORD{short(inputLocation.X - 1), inputLocation.Y});
					const int controlsKeyColor = text::DARK_BLUE;
					const int controlsDescColor = text::DARK_GRAY;
					window::Table()
							.atPosition(COORD{4, 3})
							.addRow()
									.addCell(color("ARROW KEYS", controlsKeyColor), COORD{11, 1})
									.addCell(color("Move cursor (hold ALT to    jump to screen edge)", controlsDescColor), COORD{27, 2})
							.addRow()
									.addCell(color("ENTER", controlsKeyColor))
									.addCell(color("Reveal a space", controlsDescColor))
							.addRow()
									.addCell(color("SPACE BAR", controlsKeyColor))
									.addCell(color("Flag a space", controlsDescColor))
							.addRow()
									.addCell(color("C", controlsKeyColor))
									.addCell(color("Chord a number (reveal all  unflagged adjacent spaces)", controlsDescColor), COORD{27, 2})
							.addRow()
									.addCell(color("ESC", controlsKeyColor))
									.addCell(color("Exit", controlsDescColor))
							.print();
					window::getNumericInput(inputLocation, 1, inNumericRange(1, -1));
					break;
				}
				case 6: {//settings
					//TODO
					break;
				}
				case 7: {//high scores
					//TODO
					break;
				}
			}
		} catch(window::UserExitException e) {
			continue;
		}
		
		windowSize.X = difficulty.dimensions.X >= 9 ? difficulty.dimensions.X + 2 : 11;
		windowSize.Y = difficulty.dimensions.Y + 4;
		
		window::scaleFontSizeToFit(gameFont, windowSize);
		
		system("color 70");
		window::initialize(gameFont, windowSize);
		window::printEdgeBorders(windowSize, text::BLACK, background::DARK_GRAY, 1, 2);
		
		//Create the minefield
		Field minefield (
				difficulty.dimensions,
				difficulty.mines,
				COORD{short((windowSize.X / 2) - (difficulty.dimensions.X / 2)), 3},
				fieldEvaluators::safeStartPlus
		);
		
		//Initialize cursor
		COORD position = COORD {
				short((minefield.getPositionMin().X + minefield.getPositionMax().X) / 2),
				short((minefield.getPositionMin().Y + minefield.getPositionMax().Y) / 2)
		};
		SetConsoleCursorPosition(window::handleOut, position);
		window::showCursor();
		
		//Set minimum and maximum cursor bounds
		COORD min = minefield.getPositionMin();
		COORD max = minefield.getPositionMax();
		
		//Print header bar (blank)
		window::printInRectangle(color(character::BLANK, background::BLACK), COORD{5, 1}, COORD{short(windowSize.X - 6), 1}, true);
		
		//Display initial mine count
		window::printInRectangle(color(getTextFromNumericField(minefield.getMineCount(), 4), text::RED), COORD{1, 1});
		
		//Display blank sidebars (small boards only)
		if(difficulty.dimensions.X < 9) {
			window::printInRectangle(color(character::BLANK, background::BLACK), COORD{1, 3}, COORD{short(min.X - 1), short(windowSize.Y - 2)}, true);
			window::printInRectangle(color(character::BLANK, background::BLACK), COORD{short(max.X + 1), 3}, COORD{short(windowSize.X - 2), short(windowSize.Y - 2)}, true);
		}
		
		bool exit = false;
		short lastKnownTime = -1;
		do {
			//Display UI timer if it needs updating
			short currentTime = minefield.getElapsedTime() / 1000;
			if(currentTime != lastKnownTime) {
				window::hideCursor();
				window::printInRectangle(color(getTextFromNumericField(currentTime, 4), text::RED), COORD{short(windowSize.X - 5), 1});
				lastKnownTime = currentTime;
				SetConsoleCursorPosition(window::handleOut, position);
				window::showCursor();
			}
			
			//Gather keyboard input
			DWORD cNumRead;
			INPUT_RECORD inputBuffer[128];
			window::ReadConsoleInputExA(
				window::handleIn,	// input buffer handle
				inputBuffer,		// buffer to read into
				128,				// size of read buffer
				&cNumRead,			// number of records read
				0x0002);			// do not wait for input before returning
			
			//For each input in the buffer...
			for(int i = 0; i < cNumRead; i++) {
				KEY_EVENT_RECORD keyEvent;
				if(inputBuffer[i].EventType == KEY_EVENT) {
					keyEvent = inputBuffer[i].Event.KeyEvent;
				}
				else {
					continue;
				}
				
				if(!keyEvent.bKeyDown) {
					continue;
				}
				
				//Hide cursor
				window::hideCursor();
				
				//Initialize empty cell set to hold the result of the action, if any
				unordered_set<Cell*> resultSet;
				unordered_set<Cell*>* const result = &resultSet;
				Cell* resultingCell;
				
				//Handle input
				switch(keyEvent.wVirtualKeyCode) {
					case VK_ESCAPE:
						exit = true;
						break;
					case VK_UP:
						if(position.Y > min.Y)
							if(keyEvent.dwControlKeyState & LEFT_ALT_PRESSED)
								position.Y = min.Y;
							else
								position.Y--;
						break;
					case VK_DOWN:
						if(position.Y < max.Y)
							if(keyEvent.dwControlKeyState & LEFT_ALT_PRESSED)
								position.Y = max.Y;
							else
								position.Y++;
						break;
					case VK_LEFT:
						if(position.X > min.X)
							if(keyEvent.dwControlKeyState & LEFT_ALT_PRESSED)
								position.X = min.X;
							else
								position.X--;
						break;
					case VK_RIGHT:
						if(position.X < max.X)
							if(keyEvent.dwControlKeyState & LEFT_ALT_PRESSED)
								position.X = max.X;
							else
								position.X++;
						break;
					case VK_RETURN:
						resultingCell = minefield.revealSpace(position, result);
						break;
					case VK_SPACE:
						resultingCell = minefield.flagSpace(position, result);
						
						//Display mine count
						window::printInRectangle(color(getTextFromNumericField(minefield.getMineCount(), 4), text::RED), COORD{1, 1});
						break;
					case 'C':
						resultingCell = minefield.chordSpace(position, result);
						break;
				}
				
				//Print all new cells from the result of the operation
				print(result, minefield);
				
				//Check for win/loss and display results
				checkForGameEnd(resultingCell, minefield);
				
				//Set cursor position in case it changed
				SetConsoleCursorPosition(window::handleOut, position);
				
				//Show Cursor
				window::showCursor();
			}
		} while(!exit);
	}
}