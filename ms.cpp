#include "ms.h"

int main()
{
	SetConsoleTitle("Minesweeper: \"Object Oriented!\"");
	system("cls");
	
	//Get input and output handles for the console
	HANDLE handleIn = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE handleOut = GetStdHandle(STD_OUTPUT_HANDLE);
	
    //Set console code page to UTF-8
    SetConsoleOutputCP(CP_UTF8);
	
	//Font settings
	CONSOLE_FONT_INFOEX menuFont = {0};
	menuFont.cbSize=sizeof(CONSOLE_FONT_INFOEX);
	wcscpy(menuFont.FaceName, L"MS Gothic");
	menuFont.dwFontSize.X = 12;
	menuFont.dwFontSize.Y = 18;
	menuFont.FontWeight = 800;
	
	CONSOLE_FONT_INFOEX gameFont = {0};
	gameFont.cbSize=sizeof(CONSOLE_FONT_INFOEX);
	wcscpy(gameFont.FaceName, L"MS Gothic");
	gameFont.dwFontSize.X = 18;
	gameFont.dwFontSize.Y = 18;
	gameFont.FontWeight = 1000;
	
	//Disable QuickEdit mode (to prevent pausing when clicking in window)
	DWORD prev_mode;
	GetConsoleMode(handleIn, &prev_mode);
	SetConsoleMode(handleIn, ENABLE_EXTENDED_FLAGS | (prev_mode & ~ENABLE_QUICK_EDIT_MODE));
	
	//Lock window resizing
	SetWindowLong(GetConsoleWindow(), GWL_STYLE, GetWindowLong(GetConsoleWindow(), GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);
	
	while(1)
	{
		//Set font settings
		SetCurrentConsoleFontEx(handleOut, false, &menuFont);
		
		//Set window size
		setWindow(50, 14);
		
		//Hide cursor
		setConsoleCursorVisibility(1, false);
		system("cls");
		
		cout << "\n   Controls:" << endl;
		cout << "\t" + BLUE + "ARROW KEYS" + END + "\tMove cursor (hold ALT to" << endl << "\t\t\t    jump to screen edge)" << endl;
		cout << "\t" + BLUE + "ENTER" + END + "\t\tReveal a space" << endl;
		cout << "\t" + BLUE + "SPACE BAR" + END + "\tFlag a space" << endl;
		cout << "\t" + BLUE + "C" + END + "\t\tChord a number" << endl;
		cout << "\t" + RED + "ESCAPE" + END + "\t\tExit" << endl << endl;
		cout << "   Choose difficulty by pressing a number key..." << endl;
		cout << GREEN + "\t1" + END + "\t\tBeginner" << endl;
		cout << YELLOW + "\t2" + END + "\t\tIntermediate" << endl;
		cout << RED + "\t3" + END + "\t\tExpert" << endl;
		
		Difficulty difficulty;
		
		bool input = false;
		do
		{
			DWORD cNumRead;
			INPUT_RECORD inputBuffer[128];
			ReadConsoleInput( 
				handleIn,		// input buffer handle
				inputBuffer,	// buffer to read into
				128,				// size of read buffer
				&cNumRead);		// number of records read
			
			//For each input in the buffer...
			for(int i = 0; i < cNumRead; i++)
			{
				if(inputBuffer[i].EventType == KEY_EVENT && inputBuffer[i].Event.KeyEvent.bKeyDown)
				{
					switch(inputBuffer[i].Event.KeyEvent.wVirtualKeyCode)
					{
						case VK_ESCAPE:
							return 0;
						case '1': case '2': case '3':
							difficulty = difficulties[inputBuffer[i].Event.KeyEvent.wVirtualKeyCode - '1'];
							input = true;
							break;
						//Debug option
						case VK_OEM_5:
							difficulty = Difficulty(COORD{10, 5}, 5);
							input = true;
							break;
					}
				}
			}
		}
		while(!input);
		
		//Clear the screen
		system("cls");
		
		//Set font settings
		SetCurrentConsoleFontEx(handleOut, false, &gameFont);
		
		//Set window size
		const short windowSizeX = difficulty.dimensions.X + 2;
		const short windowSizeY = difficulty.dimensions.Y + 4;
		setWindow(windowSizeX, windowSizeY);
		
		//Create the minefield
		Field minefield (
				difficulty.dimensions,
				difficulty.mines,
				COORD{1, 3},
				RandomFieldEvaluator()
		);
		
		//Initialize cursor
		COORD position = minefield.initializeCursor();
		
		bool altHeld = false;
		bool exit = false;
		do {
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
				if(inputBuffer[i].EventType != KEY_EVENT) {
					continue;
				}
				
				if(inputBuffer[i].Event.KeyEvent.bKeyDown) //key press
				{
					//Hide cursor
					setConsoleCursorVisibility(1, false);
					
					//Handle input
					switch(inputBuffer[i].Event.KeyEvent.wVirtualKeyCode)
					{
						case VK_ESCAPE:
							exit = true;
							break;
						case VK_UP:
							if(position.Y > 0)
								if(altHeld)
									position.Y = 0;
								else
									position.Y--;
							break;
						case VK_RIGHT:
							if(position.X < windowSizeX - 1)
								if(altHeld)
									position.X = windowSizeX - 1;
								else
									position.X++;
							break;
						case VK_DOWN:
							if(position.Y < windowSizeY - 1)
								if(altHeld)
									position.Y = windowSizeY - 1;
								else
									position.Y++;
							break;
						case VK_LEFT:
							if(position.X > 0)
								if(altHeld)
									position.X = 0;
								else
									position.X--;
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
						case VK_MENU:
							altHeld = true;
							break;
					}
					
					//Set cursor position in case it changed
					SetConsoleCursorPosition(handleOut, position);
					
					//Show Cursor
					setConsoleCursorVisibility(100);
				}
				else if(inputBuffer[i].Event.KeyEvent.wVirtualKeyCode == VK_MENU) { //key release
					altHeld = false;
				}
			}
		} while(!exit);
	}
}