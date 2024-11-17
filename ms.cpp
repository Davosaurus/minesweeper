#include "ms.h"

int main()
{
	int xSize = 2;
	int ySize = 2;
  int mines = 1;
  bool ended;
  bool exit;
  
  SetConsoleTitle("Minesweeper: \"Instructions included!\"");
  system("cls");
  
  //Grab font settings so they can be re-applied later
  CONSOLE_FONT_INFOEX originalFont = {sizeof(originalFont)};
  GetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), false, &originalFont);
  
  //Disable QuickEdit mode (to prevent pausing when clicking in window)
  DWORD prev_mode;
  GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &prev_mode);
  SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_EXTENDED_FLAGS | (prev_mode & ~ENABLE_QUICK_EDIT_MODE));
  
  //Lock window resizing
  SetWindowLong(GetConsoleWindow(), GWL_STYLE, GetWindowLong(GetConsoleWindow(), GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);
  
  while(1)
  {
    //Set window size
    SetWindow(1, 1);
    SetWindow(50, 12);
    
    ShowConsoleCursor(1, false);
    system("cls");
    
    cout << "\n   Controls:" << endl;
	cout << "\t" + BLUE + "ARROW KEYS" + END + "\tMove cursor" << endl;
	cout << "\t" + BLUE + "ENTER" + END + "\t\tReveal space" << endl;
	cout << "\t" + BLUE + "SPACE BAR" + END + "\tFlag space" << endl;
	cout << "\t" + BLUE + "C" + END + "\t\tChord a number" << endl;
    cout << "\t" + RED + "ESCAPE" + END + "\t\tExit" << endl;
    cout << "   Choose difficulty by pressing a number key..." << endl;
    cout << GREEN + "\t1" + END + "\t\tBeginner" << endl;
    cout << YELLOW + "\t2" + END + "\t\tIntermediate" << endl;
    cout << RED + "\t3" + END + "\t\tExpert";
    
    ended = false;
    exit = false;
    bool input = false;
    do
    {
      DWORD cNumRead, fdwMode, i;
      INPUT_RECORD* inputBuffer;
      ReadConsoleInput( 
        GetStdHandle(STD_INPUT_HANDLE),      // input buffer handle
        inputBuffer,                         // buffer to read into
        1,                                   // size of read buffer
        &cNumRead);                          // number of records read
      
      //For each input in the buffer...
      for(i = 0; i < cNumRead; i++)
      {
        if(inputBuffer[i].EventType == KEY_EVENT && inputBuffer[i].Event.KeyEvent.bKeyDown)
        {
          switch(inputBuffer[i].Event.KeyEvent.wVirtualKeyCode)
          {
            case VK_ESCAPE:
              return 0;
              break;
            case 49: case 50: case 51:
              xSize = ySize = difficulty[inputBuffer[i].Event.KeyEvent.wVirtualKeyCode - 49].first;
              mines = difficulty[inputBuffer[i].Event.KeyEvent.wVirtualKeyCode - 49].second;
              input = true;
              system("cls");
          }
        }
      }
    }
    while(!input);
    
    //Set font size
    CONSOLE_FONT_INFOEX font={0};
    font.cbSize=sizeof(font);
    font.dwFontSize.X = 18;
    font.dwFontSize.Y = 18;
    SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), false, &font);
    
    //Set window size
    SetWindow(1, 1);
    SetWindow(xSize + 2, ySize + 2);
    
    ShowConsoleCursor(100);
    int ** field = buildMatrix(xSize, ySize);	
    printScreen(0, 0, toString(field, xSize, ySize));
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{short(xSize/2), short(ySize/2)});
    
    while(!exit)
    {
      DWORD cNumRead, fdwMode, i;
      INPUT_RECORD inputBuffer[128];
      ReadConsoleInput( 
        GetStdHandle(STD_INPUT_HANDLE),      // input buffer handle
        inputBuffer,                         // buffer to read into
        128,                                 // size of read buffer
        &cNumRead);                          // number of records read
      
      //For each input in the buffer...
      for(i = 0; i < cNumRead; i++)
      {
        if(inputBuffer[i].EventType == KEY_EVENT && inputBuffer[i].Event.KeyEvent.bKeyDown)
        {
          HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
          CONSOLE_SCREEN_BUFFER_INFO cbsi;
          GetConsoleScreenBufferInfo(hOut, &cbsi);
          COORD position = cbsi.dwCursorPosition;
          
          switch(inputBuffer[i].Event.KeyEvent.wVirtualKeyCode)
          {
            case VK_ESCAPE:
              deleteMatrix(field, xSize);
              firstMove = true;
              exit = true;
              break;
            case VK_UP:
              if(position.Y > 1)
                position.Y--;
              break;
            case VK_RIGHT:
              if(position.X < xSize)
                position.X++;
              break;
            case VK_DOWN:
              if(position.Y < ySize)
                position.Y++;
              break;
            case VK_LEFT:
              if(position.X > 1)
                position.X--;
              break;
            case VK_RETURN:
              if(!ended)
                ended = revealSpace(position, field, xSize, ySize, mines);
              break;
            case VK_SPACE:
              if(!ended)
                markSpace(position, field);
              break;
            case 0x43: //"C"
              if(!ended)
                chordSpace(position, field, xSize, ySize);
              break;
          }
          SetConsoleCursorPosition(hOut, position);
        }
      }
    }
    system("cls");
    
    //Apply old font settings
    SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), false, &originalFont);
  }
	return 0;
}