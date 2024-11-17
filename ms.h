#include <iostream>
#include <windows.h>
#include <time.h>
using namespace std;

const char UNKNOWN = 176;
const char FLAG = 244;
const char MINE = '*';

const string RED = "\x1B[91m";
const string GREEN = "\x1B[92m";
const string YELLOW = "\x1B[93m";
const string BLUE = "\x1B[94m";
const string END = "\033[0m";

pair<int, int> difficulty[3] = {{9, 10}, {16, 40}, {24, 99}};

bool firstMove = true;

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

void SetWindow(int Width, int Height) 
{ 
	_COORD coord; 
	coord.X = Width; 
	coord.Y = Height; 

	_SMALL_RECT Rect; 
	Rect.Top = 0; 
	Rect.Left = 0; 
	Rect.Bottom = Height - 1; 
	Rect.Right = Width - 1; 

	HANDLE Handle = GetStdHandle(STD_OUTPUT_HANDLE);      // Get Handle 
	SetConsoleScreenBufferSize(Handle, coord);            // Set Buffer Size 
	SetConsoleWindowInfo(Handle, true, &Rect);            // Set Window Size 
}

void ShowConsoleCursor(int size, bool showFlag = true)
{
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_CURSOR_INFO     cursorInfo;

	GetConsoleCursorInfo(out, &cursorInfo);
	cursorInfo.bVisible = showFlag; // set the cursor visibility
	cursorInfo.dwSize = size; // set the cursor visibility
	SetConsoleCursorInfo(out, &cursorInfo);
}

void printScreen(int x, int y, string str)
{
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD Position;
	Position.X = x;
	Position.Y = y;
	
	SetConsoleCursorPosition(hOut, Position);
	cout << str;
}

string toString(int **matrix, int rows, int cols)
{
	string output = "";
  output += 201;
	for(int i = 0; i < cols; i++)
	{
		output += 205;
	}
  output += 187;
	output += "\n";
	for(int i = 0; i < rows; i++)
	{
		output += 186;
		for(int j = 0; j < cols; j++)
      output += UNKNOWN;
		output += 186;
		output += "\n";
	}
  output += 200;
	for(int i = 0; i < cols; i++)
	{
		output += 205;
	}
  output += 188;
	return output;
}

int ** buildMatrix(int rows, int cols)
{
	int **myArray = new int *[rows];
	for(int i = 0; i < rows; i++)
		myArray[i] = new int[cols];
	return myArray;
}

void fillMatrix(COORD pos, int **matrix, int rows, int cols, int numMines)
{
  //Initialize matrix to zeroes
  for(int i = 0; i < rows; i++)
  {
    for(int j = 0; j < cols; j++)
    {
      matrix[i][j] = 0;
    }
  }
  
  //Seed board with mines
  srand(time(0));
  for(int i = 0; i < numMines; i++)
  {
    bool validPosition = false;
    do
    {
      int minePosition = rand() % (rows * cols);
      if(matrix[minePosition / cols][minePosition % cols] != 9
      && !(minePosition / cols >= pos.Y - 2
        && minePosition / cols <= pos.Y
        && minePosition % cols >= pos.X - 2
        && minePosition % cols <= pos.X))
      {
        matrix[minePosition / cols][minePosition % cols] = 9;
        validPosition = true;
      }
    }while(!validPosition); //Check that it is valid space
  }
  
  //Generate adjacent numbers
  for(int i = 0; i < rows; i++)
  {
    for(int j = 0; j < cols; j++)
    {
      if(matrix[i][j] != 9)
      {
        int sum = 0;
        for(int k = -1; k <= 1; k++)
        {
          for(int m = -1; m <= 1; m++)
          {
            if(i + k >= 0 && i + k < rows && j + m >= 0 && j + m < cols && matrix[i + k][j + m] == 9)
              sum++;
          }
        }
        
        if(sum == 0)
          matrix[i][j] = 10;
        else
          matrix[i][j] = sum;
      }
    }
  }
}

void deleteMatrix(int **matrix, int rows)
{
	for(int i = 0; i < rows; i++)
	{
		delete[] matrix[i];
	}
	delete[] matrix;
	matrix = NULL;
}

void endGame(int** matrix, int rows, int cols)
{
  for(int i = 0; i < rows; i++)
	{
    string output = "";
		for(int j = 0; j < cols; j++)
    {
      switch(matrix[i][j])
      {
        case 19:
          output += RED + MINE + END;
          break;
        case 9:
        case -9:
          output += GREEN + MINE + END;
          break;
        case 10:
        case 20:
          output += " ";
          break;
        case -10:
          output += RED + "_" + END;
          break;
        default:
          if(matrix[i][j] < 0)
          {
            output += RED + to_string(matrix[i][j] * -1) + END;
          }
          else if(matrix[i][j] > 10)
          {
            output += to_string(matrix[i][j] - 10);
          }
          else
          {
            output += to_string(matrix[i][j]);
          }
      }
    }
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{1, short(i + 1)});
    cout << output;
	}
}

bool revealSpace(COORD pos, int** matrix, int rows, int cols, int numMines)
{
  if(firstMove)
    fillMatrix(pos, matrix, rows, cols, numMines);
  firstMove = false;
  
  if(matrix[pos.Y - 1][pos.X - 1] > 0 && matrix[pos.Y - 1][pos.X - 1] < 11)
  {
    switch(matrix[pos.Y - 1][pos.X - 1])
    {
      case 9:
        matrix[pos.Y - 1][pos.X - 1] += 10;
        endGame(matrix, rows, cols);
        firstMove = true;
        return 1;
        break;
      case 10:
        matrix[pos.Y - 1][pos.X - 1] = 20;
        for(int i = -1; i <= 1; i++)
        {
          for(int j = -1; j <= 1; j++)
          {
            if(pos.X + i - 1 >= 0 && pos.X + i - 1 < cols && pos.Y + j - 1 >= 0 && pos.Y + j - 1 < rows)
              revealSpace(COORD{short(pos.X + i), short(pos.Y + j)}, matrix, rows, cols, numMines);
          }
        }
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
        cout << " ";
        break;
      default:
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
        cout << matrix[pos.Y - 1][pos.X - 1];
        matrix[pos.Y - 1][pos.X - 1] += 10;
    }
  }
  
  //Check for win condition
  bool win = true;
  for(int i = 0; i < rows && win; i++)
  {
    for(int j = 0; j < cols && win; j++)
    {
      if((matrix[i][j] > 0 && matrix[i][j] < 9) || matrix[i][j] == 10)
        win = false;
    }
  }
  if(win)
  {
    endGame(matrix, rows, cols);
    firstMove = true;
    return 1;
  }
  
  return 0;
}

void markSpace(COORD pos, int** matrix)
{
  if(!firstMove)
  {
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
    if(matrix[pos.Y - 1][pos.X - 1] <= 10)
    {
      if(matrix[pos.Y - 1][pos.X - 1] > 0)
        cout << BLUE << FLAG << END;
      else if(matrix[pos.Y - 1][pos.X - 1] < 0)
        cout << UNKNOWN;
      matrix[pos.Y - 1][pos.X - 1] *= -1;
    }
  }
}

void chordSpace(COORD pos, int** matrix, int rows, int cols)
{
  //See if space is a revealed number
  if(matrix[pos.Y - 1][pos.X - 1] > 10 && matrix[pos.Y - 1][pos.X - 1] < 20)
  {
    //Count number of adjacent flags
    int numFlags = 0;
    for(int i = -1; i <= 1; i++)
    {
      for(int j = -1; j <= 1; j++)
      {
        if(pos.X + i - 1 >= 0 && pos.X + i - 1 < cols && pos.Y + j - 1 >= 0 && pos.Y + j - 1 < rows //Bound checking
        && matrix[pos.Y + j - 1][pos.X + i - 1] < 0) //Is flagged
          numFlags++;
      }
    }
    
    //If correct # of adjacent flags, chord (reveal all adjacent spaces)
    if(numFlags == matrix[pos.Y - 1][pos.X - 1] - 10)
    {
      for(int i = -1; i <= 1; i++)
      {
        for(int j = -1; j <= 1; j++)
        {
          if(!firstMove && pos.X + i - 1 >= 0 && pos.X + i - 1 < cols && pos.Y + j - 1 >= 0 && pos.Y + j - 1 < rows)
            revealSpace(COORD{short(pos.X + i), short(pos.Y + j)}, matrix, rows, cols, 0);
        }
      }
    }
  }
}











