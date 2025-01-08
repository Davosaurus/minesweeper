#include "ms.h"

int main()
{
	SetConsoleTitle("Minesweeper: \"Custom Difficulty, and a User Interface to do it in!\"");
	
	COORD windowSize;
	
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
		
		cout << BLACK;
		cout << BG_DARK_GRAY;
		window::printEdgeBorders(windowSize, 2, 2, short(inputLocation.Y - 1));
		cout << END;
		cout << END;
		
		window::printInRectangle("Main Menu", COORD{1, 1});
		window::printInRectangle(">", COORD{short(inputLocation.X - 1), inputLocation.Y});
		window::Table()
				.atPosition(COORD{4, 3})
				.addRow()
						.addCell(window::FlexibleString().addText(GREEN, 0).addText("1").addText(END, 0), COORD{6, 1})
						.addCell("Beginner Game")
				.addRow()
						.addCell(window::FlexibleString().addText(YELLOW, 0).addText("2").addText(END, 0))
						.addCell("Intermediate Game")
				.addRow()
						.addCell(window::FlexibleString().addText(RED, 0).addText("3").addText(END, 0))
						.addCell("Expert Game")
				.addRow()
						.addCell(window::FlexibleString().addText(BLUE, 0).addText("4").addText(END, 0))
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
			menuChoice = window::getNumericInput(inputLocation, 1, window::inNumericRange(1, 5));
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
					
					cout << BLACK;
					cout << BG_DARK_GRAY;
					window::printEdgeBorders(windowSize, 1, 2);
					cout << END;
					cout << END;
					
					window::printInRectangle("Custom Game", COORD{1, 1});
					
					//Get user input for board dimensions
					window::printInRectangle("Board width?", COORD{4, 3});
					window::printInRectangle(">", COORD{24, 3});
					short boardWidth = window::getNumericInput(COORD{25, 3}, 4, window::inNumericRange(1, 9999));
					
					window::printInRectangle("Board height?", COORD{4, 4});
					window::printInRectangle(">", COORD{24, 4});
					short boardHeight = window::getNumericInput(COORD{25, 4}, 4, window::inNumericRange(1, 9999));
					
					//Get user input for number of mines, and set the custom difficulty accordingly
					window::printInRectangle("Number of mines?", COORD{4, 5});
					window::printInRectangle(">", COORD{24, 5});
					difficulty = Difficulty (
							COORD{boardWidth, boardHeight},
							window::getNumericInput(COORD{25, 5}, 4, window::inNumericRange(0, boardWidth * boardHeight))
					);
					break;
				}
				case 5: //controls
					window::initialize(menuFont, windowSize);
					
					cout << BLACK;
					cout << BG_DARK_GRAY;
					window::printEdgeBorders(windowSize, 2, 2, short(inputLocation.Y - 1));
					cout << END;
					cout << END;
					
					window::printInRectangle("Controls", COORD{1, 1});
					window::printInRectangle(">", COORD{short(inputLocation.X - 1), inputLocation.Y});
					window::Table()
							.atPosition(COORD{4, 3})
							.addRow()
									.addCell(window::FlexibleString().addText(BLUE, 0).addText("ARROW KEYS").addText(END, 0), COORD{11, 1})
									.addCell("Move cursor (hold ALT to    jump to screen edge)", COORD{27, 2})
							.addRow()
									.addCell(window::FlexibleString().addText(BLUE, 0).addText("ENTER").addText(END, 0))
									.addCell("Reveal a space")
							.addRow()
									.addCell(window::FlexibleString().addText(BLUE, 0).addText("SPACE BAR").addText(END, 0))
									.addCell("Flag a space")
							.addRow()
									.addCell(window::FlexibleString().addText(BLUE, 0).addText("C").addText(END, 0))
									.addCell("Chord a number (reveal all  unflagged adjacent spaces)", COORD{27, 2})
							.addRow()
									.addCell(window::FlexibleString().addText(BLUE, 0).addText("ESC").addText(END, 0))
									.addCell("Exit")
							.print();
					window::getNumericInput(inputLocation, 1, window::inNumericRange(1, -1));
					break;
				case 6: //high scores
					break;
			}
		} catch(window::UserExitException e) {
			continue;
		}
		
		windowSize.X = difficulty.dimensions.X >= 9 ? difficulty.dimensions.X + 2 : 11;
		windowSize.Y = difficulty.dimensions.Y + 4;
		
		window::scaleFontSizeToFit(gameFont, windowSize);
		
		window::initialize(gameFont, windowSize);
		
		cout << BLACK;
		cout << BG_DARK_GRAY;
		window::printEdgeBorders(windowSize, 1, 2);
		cout << END;
		cout << END;
		
		//Create the minefield
		Field minefield (
				difficulty.dimensions,
				difficulty.mines,
				COORD{1, 3},
				RandomFieldEvaluator()
		);
		
		//Initialize cursor
		COORD position = minefield.initializeCursor();
		
		//Set minimum and maximum cursor bounds
		COORD min = minefield.getPositionMin();
		COORD max = minefield.getPositionMax();
		
		bool exit = false;
		short lastKnownMineCount = -1;
		short lastKnownTime = -1;
		field::GameStatus lastKnownGameStatus = field::UNSTARTED;
		do {
			//Display UI elements if they need updating
			short currentMineCount = minefield.getMineCount();
			short currentTime = minefield.getElapsedTime() / 1000;
			field::GameStatus currentStatus = minefield.getGameStatus();
			if(currentMineCount != lastKnownMineCount) {
				window::hideCursor();
				char uiOutputString[4];
				
				sprintf(uiOutputString, "%04d", currentMineCount);
				window::printInRectangle(RED + uiOutputString + END, COORD{1, 1});
				lastKnownMineCount = currentMineCount;
				
				SetConsoleCursorPosition(window::handleOut, position);
				window::showCursor();
			}
			
			if(currentTime != lastKnownTime) {
				window::hideCursor();
				char uiOutputString[4];
				
				sprintf(uiOutputString, "%04d", currentTime);
				window::printInRectangle(RED + uiOutputString + END, COORD{short(windowSize.X - 5), 1});
				lastKnownTime = currentTime;
				
				SetConsoleCursorPosition(window::handleOut, position);
				window::showCursor();
			}
			
			if(currentStatus != lastKnownGameStatus) {
				window::hideCursor();
				char uiOutputString[4];
				
				if(currentStatus == field::GameStatus::LOST) {
					window::printInRectangle("☹", COORD{short(windowSize.X / 2), 1});
				}
				else if(currentStatus == field::GameStatus::WON) {
					window::printInRectangle("☺", COORD{short(windowSize.X / 2), 1});
				}
				
				lastKnownGameStatus = currentStatus;
				
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
						minefield.revealSpace();
						break;
					case VK_SPACE:
						minefield.flagSpace();
						break;
					case 'C':
						minefield.chordSpace();
						break;
				}
				
				//Set cursor position in case it changed
				SetConsoleCursorPosition(window::handleOut, position);
				
				//Show Cursor
				window::showCursor();
			}
		} while(!exit);
	}
}