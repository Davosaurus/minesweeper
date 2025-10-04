#include "settings.h"

COORD windowSize;

//Font settings
CONSOLE_FONT_INFOEX menuFont = {
	sizeof(CONSOLE_FONT_INFOEX),	//cbSize
	0,								//nFont
	COORD{19, 36},					//dwFontSize
	FF_MODERN,						//FontFamily
	800,							//FontWeight
	L"MS Gothic"					//FaceName
};

CONSOLE_FONT_INFOEX gameFont = {
	sizeof(CONSOLE_FONT_INFOEX),
	0,
	COORD{18, 18},
	FF_MODERN,
	1000,
	L"MS Gothic"
};

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

text::text numberColors[9] = {
	text::WHITE,
	text::BLUE,
	text::DARK_GREEN,
	text::RED,
	text::DARK_BLUE,
	text::DARK_RED,
	text::DARK_CYAN,
	text::DARK_GRAY,
	text::GRAY
};

background::background numberColorsBackgrounds[9] = {
	background::WHITE,
	background::BLUE,
	background::DARK_GREEN,
	background::RED,
	background::DARK_BLUE,
	background::DARK_RED,
	background::DARK_CYAN,
	background::DARK_GRAY,
	background::GRAY
};

/**
 * Print the given cell to the screen
 */
void print(Cell* const cell, const Minefield& minefield, Settings& settings, const bool& losingMove = false) {
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
			output = color(
					to_string(cell->getNumber()),
					numberColors[cell->getNumber()],
					cell->isFlagged()
							? background::DARK_RED
							: gameFont.dwFontSize.X < settings.getPixelDisplayThreshold()
									? numberColorsBackgrounds[cell->getNumber()]
									: background::BLACK
			);
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
void print(ContainerType* const cells, const Minefield& minefield, Settings& settings) {
	for(Cell* cell : *cells) {
		print(cell, minefield, settings);
	}
}

void checkForGameEnd(Cell* const cell, const Minefield& minefield, Settings& settings, bool& gameInProgress) {
	GameStatus currentGameStatus = minefield.getGameStatus();
	if(gameInProgress && currentGameStatus == GameStatus::LOST) {
		gameInProgress = false;
		
		window::printEdgeBorders(windowSize, text::BLACK, background::RED, 1, 2);
		window::printInRectangle(color(character::INDICATOR_LOST, text::WHITE, background::RED), COORD{short(windowSize.X / 2), 1});
		
		//Print extra bad space to show which move lost the game
		print(cell, minefield, settings, true);
	}
	else if(gameInProgress && currentGameStatus == GameStatus::WON) {
		gameInProgress = false;
		
		window::printEdgeBorders(windowSize, text::BLACK, background::GREEN, 1, 2);
		window::printInRectangle(color(character::INDICATOR_WON, text::WHITE, background::GREEN), COORD{short(windowSize.X / 2), 1});
		
		string playerName = settings.getPlayerName();
		auto addHighScoreResult = settings.addHighScore(minefield, playerName);
		if(addHighScoreResult.second) {
			window::printInRectangle(color("High Score!", text::WHITE, background::GREEN), COORD{0, 0});

			if(playerName == "") {
				window::printInRectangle(color("Name: ⎕⎕⎕", text::WHITE, background::GREEN), COORD{1, 2});
				auto removeHighScoreResult = settings.removeHighScore(addHighScoreResult.first);
				try {
					settings.addHighScore(removeHighScoreResult, minefield, window::getTextInput(COORD{7, 2}, 3, isNotBlank() && isAlpha()));
				} catch(window::UserExitException e) {
					return;
				}
			}
			
			settings.save();
		}
	}
}

int main() {
	SetConsoleTitle("Minesweeper");
	
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
	
	//Load settings
	Settings settings = Settings();
	
	while(1)
	{
		windowSize.X = 44;
		windowSize.Y = 13;
		
		COORD inputLocation = COORD{4, short(windowSize.Y - 2)};
		
		window::initialize(menuFont, windowSize);
		window::printEdgeBorders(windowSize, text::BLACK, background::DARK_GRAY, 2, 2, short(inputLocation.Y - 1));
		
		window::printInRectangle("Main Menu", COORD{1, 1});
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
				.addRow()
						.addCell("6")
						.addCell("High Scores")
				.addRow()
						.addCell("7")
						.addCell("Settings")
				.print();
		window::printInRectangle(">", COORD{short(inputLocation.X - 1), inputLocation.Y});
		
		Difficulty difficulty;
		
		short menuChoice;
		try {
			menuChoice = window::getNumericInput(inputLocation, 1, isInNumericRange(1, 7));
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
					short boardWidth = window::getNumericInput(COORD{25, 3}, 4, isInNumericRange(1, 9999));
					
					window::printInRectangle("Board height?", COORD{4, 4});
					window::printInRectangle(">", COORD{24, 4});
					short boardHeight = window::getNumericInput(COORD{25, 4}, 4, isInNumericRange(1, 9999));
					
					//Get user input for number of mines, and set the custom difficulty accordingly
					window::printInRectangle("Number of mines?", COORD{4, 5});
					window::printInRectangle(">", COORD{24, 5});
					difficulty = Difficulty (
							COORD{boardWidth, boardHeight},
							window::getNumericInput(COORD{25, 5}, 8, isInNumericRange(0, boardWidth * boardHeight))
					);
					break;
				}
				case 5: { //controls
					window::initialize(menuFont, windowSize);
					window::printEdgeBorders(windowSize, text::BLACK, background::DARK_GRAY, 2, 2, short(inputLocation.Y - 1));
					
					window::printInRectangle("Controls", COORD{1, 1});
					const int controlsDescColor = text::DARK_GRAY;
					window::Table()
							.atPosition(COORD{4, 3})
							.addRow()
									.addCell("ARROW KEYS", COORD{12, 1})
									.addCell("Move cursor (hold ALT to jump to screen edge)", COORD{27, 2})
							.addRow()
									.addCell(color("ENTER", text::DARK_GRAY))
									.addCell(color("Reveal a space", text::DARK_GRAY))
							.addRow()
									.addCell("SPACE BAR")
									.addCell("Flag a space")
							.addRow()
									.addCell(color("C", text::DARK_GRAY))
									.addCell(color("Chord a number (reveal all unflagged adjacent spaces)", text::DARK_GRAY), COORD{27, 2})
							.addRow()
									.addCell("ESC")
									.addCell("Exit")
							.print();
					window::printInRectangle(">", COORD{short(inputLocation.X - 1), inputLocation.Y});
					window::printInRectangle("Press any key to exit...", inputLocation);
					window::waitForUserInput();
				}
				case 6: { //high scores
					window::initialize(menuFont, windowSize);
					window::hideCursor();
					window::printEdgeBorders(windowSize, text::BLACK, background::DARK_GRAY, 2, 2, short(inputLocation.Y - 1));
					
					window::printInRectangle("High Scores", COORD{1, 1});
					
					window::printInRectangle(">", COORD{short(inputLocation.X - 1), inputLocation.Y});
					window::printInRectangle("Press any key to exit...", inputLocation);
					
					window::Table highScoreTable = window::Table()
						.atPosition(COORD{4, 3});
					
					for(HighScore highScore : settings.getHighScores()) {
						highScoreTable.addRow(highScore.getDisplayString(windowSize.X - 8));
					}
					
					short offset = -6;
					while(1) {
						window::printInRectangle(character::STRING_BLANK, COORD{4, 3}, COORD{short(windowSize.X - 4), short(inputLocation.Y - 2)}, true);
						highScoreTable
								.subTable(offset, offset + 7)
								.atPosition(COORD{4, short(3 - (offset < 0 ? offset : 0))})
								.print();
						window::waitForUserInput(2000);
						offset++;
						if(offset >= highScoreTable.getRowCount()) {
							offset = -6;
						}
					}
				}
				case 7: { //settings
					while(1) {
						window::initialize(menuFont, windowSize);
						window::printEdgeBorders(windowSize, text::BLACK, background::DARK_GRAY, 2, 2, short(inputLocation.Y - 1));
						
						window::printInRectangle("Settings", COORD{1, 1});
						window::printInRectangle(color("To save changes, exit to main menu (ESC). To revert changes, close the app instead.", text::DARK_GRAY), COORD{1, 3}, COORD{42, 4});
						window::Table()
								.atPosition(COORD{4, 5})
								.addRow()
										.addCell("1", COORD{6, 1})
										.addCell("Board Generation Rules")
								.addRow()
										.addCell("2")
										.addCell("Cell Reveal Pattern")
								.addRow()
										.addCell("3")
										.addCell("Pixel Display Threshold")
								.addRow()
										.addCell("4")
										.addCell("Player Username")
								.print();
						window::printInRectangle(">", COORD{short(inputLocation.X - 1), inputLocation.Y});
						
						try {
							menuChoice = window::getNumericInput(inputLocation, 1, isInNumericRange(1, 4));
						} catch(window::UserExitException e) {
							break;
						}
						
						try {
							switch(menuChoice) {
								case 1:
									window::initialize(menuFont, windowSize);
									window::printEdgeBorders(windowSize, text::BLACK, background::DARK_GRAY, 2, 2, short(inputLocation.Y - 1));
									
									window::printInRectangle("Board Generation Rules", COORD{1, 1});
									window::printInRectangle(color("This controls how the board generates mines upon first move", text::DARK_GRAY), COORD{1, 3}, COORD{42, 4});
									window::Table()
											.atPosition(COORD{4, 5})
											.addRow()
													.addCell("1", COORD{6, 1})
													.addCell("True random (can lose instantly)")
											.addRow()
													.addCell("2")
													.addCell("First move no mine")
											.addRow()
													.addCell("3")
													.addCell("First move no mine or number")
											// .addRow()
													// .addCell("4")
													// .addCell("Winnable with no blind guesses")
											.print();
									window::printInRectangle(">", COORD{short(inputLocation.X - 1), inputLocation.Y});
									
									menuChoice = window::getNumericInput(inputLocation, 1, isInNumericRange(1, 3), to_string(settings.getEvaluatorOrdinal()));
									settings.setEvaluatorByOrdinal(menuChoice);
									break;
								case 2:
									window::initialize(menuFont, windowSize);
									window::printEdgeBorders(windowSize, text::BLACK, background::DARK_GRAY, 2, 2, short(inputLocation.Y - 1));
									
									window::printInRectangle("Cell Reveal Pattern", COORD{1, 1});
									window::printInRectangle(color("This controls how the screen displays new cells being revealed. Generally only noticeable with VERY large board sizes.", text::DARK_GRAY), COORD{1, 3}, COORD{42, 6});
									window::Table()
											.atPosition(COORD{4, 6})
											.addRow()
													.addCell("1", COORD{6, 1})
													.addCell("Fragmented")
											.addRow()
													.addCell("2")
													.addCell("Random")
											.addRow()
													.addCell("3")
													.addCell("Ordered (fastest)")
											.print();
									window::printInRectangle(">", COORD{short(inputLocation.X - 1), inputLocation.Y});
									
									menuChoice = window::getNumericInput(inputLocation, 1, isInNumericRange(1, 3), to_string(settings.getContainerTypeOrdinal()));
									settings.setContainerTypeByOrdinal(menuChoice);
									break;
								case 3:
									window::initialize(menuFont, windowSize);
									window::printEdgeBorders(windowSize, text::BLACK, background::DARK_GRAY, 2, 2, short(inputLocation.Y - 1));
									
									window::printInRectangle("Pixel Display Threshold", COORD{1, 1});
									window::printInRectangle(color("Cells will be rendered in solid colors when the size in pixels is below this value. This allows play with enormous board sizes that would otherwise be limited by illegible text. Advanced players can use this option to play by color.", text::DARK_GRAY), COORD{1, 3}, COORD{42, 9});
									window::printInRectangle(">", COORD{short(inputLocation.X - 1), inputLocation.Y});
									
									menuChoice = window::getNumericInput(inputLocation, 4, isInNumericRange(1, 9999), to_string(settings.getPixelDisplayThreshold()));
									settings.setPixelDisplayThreshold(menuChoice);
									break;
								case 4:
									window::initialize(menuFont, windowSize);
									window::printEdgeBorders(windowSize, text::BLACK, background::DARK_GRAY, 2, 2, short(inputLocation.Y - 1));
									
									window::printInRectangle("Player Username", COORD{1, 1});
									window::printInRectangle(color("This (alphabetic) value will be used automatically for any high scores achieved while it is set. Enter a blank value and save to return to the default behavior (ask when high score is achieved).", text::DARK_GRAY), COORD{1, 3}, COORD{42, 8});
									window::printInRectangle(">", COORD{short(inputLocation.X - 1), inputLocation.Y});
									
									string newName = window::getTextInput(inputLocation, 3, isAlpha(), settings.getPlayerName());
									settings.setPlayerName(newName);
									break;
							}
						} catch(window::UserExitException e) {
							continue;
						}
					}
					settings.save();
					continue;
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
		Minefield minefield (
				difficulty.dimensions,
				difficulty.mines,
				COORD{short((windowSize.X / 2) - (difficulty.dimensions.X / 2)), 3},
				settings.getEvaluator()
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
		
		//Initialize empty cell containers to hold the results of the user's actions, if any
		vector<Cell*> resultVector;
		unordered_set<Cell*> resultSet;
		
		//Flag to ensure end of game is only checked once
		bool gameInProgress = true;
		
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
				
				//Clear result container
				if(settings.getContainerType() == "fragmented") {
					resultSet.clear();
				}
				else {
					resultVector.clear();
				}
				
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
						settings.getContainerType() == "fragmented"
								? resultingCell = minefield.revealSpace(position, &resultSet)
								: resultingCell = minefield.revealSpace(position, &resultVector);
						break;
					case VK_SPACE:
						settings.getContainerType() == "fragmented"
								? resultingCell = minefield.flagSpace(position, &resultSet)
								: resultingCell = minefield.flagSpace(position, &resultVector);
						
						//Display mine count
						window::printInRectangle(color(getTextFromNumericField(minefield.getMineCount(), 4), text::RED), COORD{1, 1});
						break;
					case 'C':
						settings.getContainerType() == "fragmented"
								? resultingCell = minefield.chordSpace(position, &resultSet)
								: resultingCell = minefield.chordSpace(position, &resultVector);
						break;
				}
				
				//Print all new cells from the result of the operation
				if(settings.getContainerType() == "fragmented") {
					print(&resultSet, minefield, settings);
				}
				else if(settings.getContainerType() == "random") {
					shuffle(resultVector.begin(), resultVector.end(), randomizer);
					print(&resultVector, minefield, settings);
				}
				else if(settings.getContainerType() == "ordered") {
					print(&resultVector, minefield, settings);
				}
				
				//Check for win/loss and display results
				checkForGameEnd(resultingCell, minefield, settings, gameInProgress);
				
				//Set cursor position in case it changed
				SetConsoleCursorPosition(window::handleOut, position);
			}
		} while(!exit);
	}
}