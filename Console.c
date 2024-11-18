////////////////////////////////////////////////////////////////////////////
//
// CONSOLE
// =======
//
// Author: Stewart Tunbridge, Pi Micros
// Email:  stewarttunbridge@gmail.com
//
// Encapsulate ANSI terminal functions for "Visual" console apps.
//
// 04 Apr 2018 EditString: Delete: First => delete string
// 19 May 2018 EditString: KeyCntrlDel -> Clear end of string
//                         KeyShiftDel -> Clear string
// 18 Jun 2018 GetKey_: completely table driven
//  2 Oct 2018 EditString: KeyAltDel/^L -> Clear end of string
// 28 Mar 2019 GetKey_: Tolerate key blocks (ie from Paste)
// 20 Jun 2019 EditString: Add horizontal scrolling fo long strings
// 20 Sep 2019 ConsoleCursor[Show/Hide]
// 04 Dec 2019 Default to cursor OFF until GetKeyWait
// 08 Dec 2019 ConsoleInit(): Clear only specific cc_c items
// 08 Feb 2021 Windows: Try writing directly to Screen Buffer
//
////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifdef _Windows
  #include <windows.h>
  #include <conio.h>
  #include <wincon.h>
#else
  #include <sys/ioctl.h>
  #include <termios.h>
  #include <time.h>
  #include <signal.h>
#endif

typedef enum {ColBlack, ColMaroon, ColGreenDark, ColBrown, ColBlueDark, ColPurpleDark, ColCyanDark, ColGray, ColBright} Colour;

#define ColUnderline 0x10
#define ColItalic 0x20
//#define ColInvert 0x40
#define ColBold 0x40
#define ColFGMax 0x7F
#define ColBGMax 0x7

#define ColWhite (ColGray + ColBright)
#define ColYellow (ColBrown + ColBright)
#define ColCyan (ColCyanDark + ColBright)
#define ColRed (ColMaroon + ColBright)
#define ColBlue (ColBlueDark + ColBright)
#define ColGreen (ColGreenDark + ColBright)
#define ColPurple (ColPurpleDark + ColBright)

int ConsoleSizeX = 80;
int ConsoleSizeY = 24;

int ConsoleX = 0;
int ConsoleY = 0;
int ConsoleBG = -1;
int ConsoleFG = -1;
int ConsoleFG_ = -1;
int ConsoleBG_ = -1;
int ConsoleTab = 8;

byte *GetKeyMacro = NULL;
byte *GetKeyMacroRecord = NULL;
int GetKeyMacroRecordSize = 0;

#ifdef _Windows
  HANDLE WinConsoleHandle;
#else
  struct termios TermiosOld;
  int FlagsOld;
#endif

//enum KeyState_ {kFirst, kWaitCommand, kWaitParam} KeyState;
//int KeyParam;

#ifdef _Windows
HANDLE GetConsoleOutputHandle (void)
  {
    return GetStdHandle (STD_OUTPUT_HANDLE);
  }
#endif // _Windows

bool ConsoleGetSize (void)   // returns true if size changes
  {
    int x, y;
    //
    x = ConsoleSizeX;
    y = ConsoleSizeY;
#ifdef _Windows
    HANDLE console = GetStdHandle (STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO screen;
    GetConsoleScreenBufferInfo (console, &screen);
    ConsoleSizeX = screen.srWindow.Right - screen.srWindow.Left + 1;
    ConsoleSizeY = screen.srWindow.Bottom - screen.srWindow.Top + 1;
#else
    struct winsize w;
    //
    if (ioctl (0, TIOCGWINSZ, &w) == 0)
      {
        ConsoleSizeX = w.ws_col;
        ConsoleSizeY = w.ws_row;
      }
#endif
    if (ConsoleSizeX != x || ConsoleSizeY != y)
      return true;
    return false;
  }

void putst (char *st)
  {
    while (*st)
      putchar (*st++);
  }

void ConsoleCursor (int x, int y);   // Move Cursor. Top Left is (0, 0)
void ConsoleCursorShow (bool Show);
void ConsoleCursorHide (void);

#define ENABLE_VIRTUAL_TERMINAL_INPUT 0x0200
//#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004

void ConsoleInit (bool Save)
  {
#ifdef _Windows
    SetConsoleMode (GetStdHandle (STD_INPUT_HANDLE), ENABLE_VIRTUAL_TERMINAL_INPUT);// | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#else
    int fd = fileno (stdin);
    //Settings for stdin (source: svgalib):
    struct termios t;
    //
    //signal (SIGKILL, SignalReceived);
    //
    if (Save)
      {
        //putst ("\e7");   // Save cursor
        putst ("\e[?47h");   // New Pane
      }
    tcgetattr (fd, &t);   // Get Terminal Status
    TermiosOld = t;
    t.c_oflag &= ~(OPOST);
    t.c_cc [VINTR] = 0;
    t.c_cc [VKILL] = 0;
    t.c_cc [VQUIT] = 0;
    t.c_cc [VSUSP] = 0;
    t.c_cc [VEOF] = 0;
    t.c_cc [VEOL] = 0;
    t.c_cc [VSTART] = 0;
    t.c_cc [VSTOP] = 0;
    //for (i = VINTR; i <= VEOL2; i++)
    //  t.c_cc [i] = 0;
    t.c_lflag &= ~(ICANON | ECHO | IXON | IXOFF);
    tcsetattr (fd, TCSANOW, &t);
    FlagsOld = fcntl(fd, F_GETFL, 0);
    fcntl (fd, F_SETFL, O_NONBLOCK);
#endif
    ConsoleGetSize ();
    //ConsoleCursor (0, ConsoleSizeY - 1);
    ConsoleCursorShow (false);
    // Get Cursor position
    /*
    char msg [] = "\e[6n";
    byte buf [20];
    write (fd, msg, sizeof (msg) - 1);
    //putst ("\e[6n");
    usleep (10000);   // this is disgusting
    int s = read (fd, buf, 20);
    buf [s] = 0;
    char *b = &buf [2];
    int y = StrGetInt (&b);
    b++;
    int x = StrGetInt (&b);
    if ((x > 0) && (x <= ConsoleSizeX) && (y > 0) && (y <= ConsoleSizeY))
      {
        ConsoleX = x - 1;
        ConsoleY = y - 1;
      }
    */
  }

void ConsoleUninit (bool Save)
  {
#ifdef _Windows
    CloseHandle (WinConsoleHandle);
#else
    int fd = fileno (stdin);
    fcntl (fd, F_SETFL, FlagsOld);   // Turn Blocking back on
    tcsetattr (fd, TCSANOW, &TermiosOld);
    if (Save)
      {
        putst ("\e[?47l");   // Restore Pane
        //putst ("\e8");   // Restore cursor
      }
    //putst ("\e[0m");   // Reset all attributes
    //####putst ("\ec");   // Reset Terminal
    //putst ("<reset-top>");   // Reset Terminal
#endif
    ConsoleCursorShow (true);
  }

#define KeyArrows 0x80
#define KeyUp 0x80
#define KeyDown 0x81
#define KeyLeft 0x82
#define KeyRight 0x83

#define KeyHome 0x84
#define KeyEnd 0x87
#define KeyIns 0x85
#define KeyDel 0x86
#define KeyPageUp 0x88
#define KeyPageDown 0x89

#define KeyBackSpace 0x08
#define KeyEnter 0x0D

#define KeyCntrlArrows 0x90
#define KeyCntrlUp 0x90
#define KeyCntrlDown 0x91
#define KeyCntrlLeft 0x92
#define KeyCntrlRight 0x93
#define KeyCntrlIns 0x95
#define KeyCntrlDel 0x96
#define KeyCntrlHome 0x94
#define KeyCntrlEnd 0x97
#define KeyCntrlPageUp 0x98
#define KeyCntrlPageDown 0x99

#define KeyCntrlTab 0x9A

#define KeyShiftArrows 0xA0
#define KeyShiftUp 0xA0
#define KeyShiftDown 0xA1
#define KeyShiftLeft 0xA2
#define KeyShiftRight 0xA3
#define KeyShiftIns 0xA5
#define KeyShiftDel 0xA6
#define KeyShiftHome 0xA4
#define KeyShiftEnd 0xA7
#define KeyShiftPageUp 0xA8
#define KeyShiftPageDown 0xA9

#define KeyShiftTab 0xAA

#define KeyAltArrows 0xB0
#define KeyAltUp 0xB0
#define KeyAltDown 0xB1
#define KeyAltLeft 0xB2
#define KeyAltRight 0xB3
#define KeyAltIns 0xB5
#define KeyAltDel 0xB6
#define KeyAltHome 0xB4
#define KeyAltEnd 0xB7
#define KeyAltPageUp 0xB8
#define KeyAltPageDown 0xB9

#define KeyAltEnter 0xBA

#define KeyF1 0xC0  // ..F10

#define KeyAlt0 0xD0
#define KeyAltA 0xE1  // AltA..AltZ = $E1..$FA

//

#define KeyCntrlF1 0x100  // ..F10
#define KeyShiftF1 0x110  // ..F10
//#define KeyAltF1 0x120  // ..F10

/* //not always available realocate numbers
#define KeyShiftCntrlUp 0xC0
#define KeyShiftCntrlDown 0xC1
#define KeyShiftCntrlLeft 0xC2
#define KeyShiftCntrlRight 0xC3
//define KeyShiftCntrlIns 0xC4
#define KeyShiftCntrlDel 0xC6
#define KeyShiftCntrlHome 0xC4
#define KeyShiftCntrlEnd 0xC7
#define KeyShiftCntrlPageUp 0xC8
#define KeyShiftCntrlPageDown 0xC9

#define KeyAltCntrlUp 0xD0
#define KeyAltCntrlDown 0xD1
#define KeyAltCntrlLeft 0xD2
#define KeyAltCntrlRight 0xD3
//define KeyAltCntrlIns 0xD4
#define KeyAltCntrlDel 0xD6
#define KeyAltCntrlHome 0xD4
#define KeyAltCntrlEnd 0xD7
#define KeyAltCntrlPageUp 0xD8
#define KeyAltCntrlPageDown 0xD9
*/

//#define KeyAlt(X) (X>='A'?KeyAltA+X-'A':KeyAlt0+X-0)

void ConsoleColourFG (int n);
void ConsoleColourBG (int n);
void ConsoleBeep ();

/*
int TestKbd = -1;

void TestKbdLog (byte Ch)
  {
    if (TestKbd < 0)
      TestKbd = FileOpen ("TestKbd", foWrite);
    write (TestKbd, &Ch, 1);
  }
*/

int GetKeyRaw (void)
  {
    int c;
    //
    c = -1;
#ifdef _Windows
    if (kbhit ())
      c = getch ();
#else
    c = getchar ();
#endif
    //if (c >= 0)
    //  TestKbdLog (c);
    return c;
  }

typedef struct
  {
    char *Sequence;
    word Key;
  } _KeyMap;

const _KeyMap KeyMap [] =
  {
/*
#ifdef _Windows
    {"\xFFH", KeyUp},
    {"\xFFP", KeyDown},
    {"\xFFK", KeyLeft},
    {"\xFFM", KeyRight},
    {"\xFFR", KeyIns},
    {"\xFFS", KeyDel},
    {"\xFFG", KeyHome},
    {"\xFFO", KeyEnd},
    {"\xFFI", KeyPageUp},
    {"\xFFQ", KeyPageDown},

    {"\xFF;", KeyF1+0},
    {"\xFF<", KeyF1+1},
    {"\xFF=", KeyF1+2},
    {"\xFF>", KeyF1+3},
    {"\xFF?", KeyF1+4},
    {"\xFF@", KeyF1+5},
    {"\xFF""A", KeyF1+6},
    {"\xFF""B", KeyF1+7},
    {"\xFF""C", KeyF1+8},
    {"\xFF""D", KeyF1+9},

    {"\xFF\x8D", KeyCntrlUp},
    {"\xFF\x91", KeyCntrlDown},
    {"\xFFs", KeyCntrlLeft},
    {"\xFFt", KeyCntrlRight},

    {"\xFF\x98", KeyAltUp},
    {"\xFF\xA0", KeyAltDown},
    {"\xFF\x9B", KeyAltLeft},
    {"\xFF\x9D", KeyAltRight},
    {"\xFF\xA3", KeyAltDel},

    {"\xFF\x94", KeyCntrlTab},
    {"\xFF\x93", KeyCntrlDel},

    {"\x0D", KeyEnter},
    //{"\x08", KeyBackSpace},

#else
*/
    {"\e[A", KeyUp},
    {"\e[B", KeyDown},
    {"\e[D", KeyLeft},
    {"\e[C", KeyRight},
    {"\e[2~", KeyIns},
    {"\e[3~", KeyDel},
    {"\e[H", KeyHome},
    {"\e[F", KeyEnd},
    {"\e[5~", KeyPageUp},
    {"\e[6~", KeyPageDown},

    // Raspberry Pi
    {"\e[1~", KeyHome},
    {"\e[4~", KeyEnd},

    {"\eOP", KeyF1+0},   // F1
    {"\eOQ", KeyF1+1},
    {"\eOR", KeyF1+2},
    {"\eOS", KeyF1+3},
    {"\e[15~", KeyF1+4},
    {"\e[17~", KeyF1+5},
    {"\e[18~", KeyF1+6},
    {"\e[19~", KeyF1+7},
    {"\e[20~", KeyF1+8},
    {"\e[21~", KeyF1+9},   // F10
    {"\e[23~", KeyF1+10},  // F11 untested
    {"\e[24~", KeyF1+11},   // F12

    {"\e[1;5P", KeyCntrlF1+0},   // Cntrl F1
    {"\e[1;5Q", KeyCntrlF1+1},   // Cntrl F2
    {"\e[1;5R", KeyCntrlF1+2},   // Cntrl F3
    {"\e[1;5S", KeyCntrlF1+3},   // Cntrl F4
    {"\e[15;5~", KeyCntrlF1+4},   // Cntrl F5
    {"\e[17;5~", KeyCntrlF1+5},   // Cntrl F6
    {"\e[18;5~", KeyCntrlF1+6},   // Cntrl F7
    {"\e[19;5~", KeyCntrlF1+7},   // Cntrl F8
    {"\e[20;5~", KeyCntrlF1+8},   // Cntrl F9
    {"\e[21;5~", KeyCntrlF1+9},   // Cntrl F10
    {"\e[23;5~", KeyCntrlF1+10},   // Cntrl F11
    {"\e[24;5~", KeyCntrlF1+11},   // Cntrl F12

    {"\e[1;2P", KeyShiftF1+0},   // Shift F1
    {"\e[1;2Q", KeyShiftF1+1},   // Shift F2
    {"\e[1;2R", KeyShiftF1+2},   // Shift F3
    {"\e[1;2S", KeyShiftF1+3},   // Shift F4
    {"\e[15;2~", KeyShiftF1+4},   // Shift F5
    {"\e[17;2~", KeyShiftF1+5},   // Shift F6
    {"\e[18;2~", KeyShiftF1+6},   // Shift F7
    {"\e[19;2~", KeyShiftF1+7},   // Shift F8
    {"\e[20;2~", KeyShiftF1+8},   // Shift F9
    {"\e[??;2~", KeyShiftF1+9},   // Shift F10
    {"\e[23;2~", KeyShiftF1+10},   // Shift F11
    {"\e[24;2~", KeyShiftF1+11},   // Shift F12


    // Raspberry Pi
    {"\e[[A", KeyF1+0},
    {"\e[[B", KeyF1+1},
    {"\e[[C", KeyF1+2},
    {"\e[[D", KeyF1+3},
    {"\e[[E", KeyF1+4},

    {"\e[1;5A", KeyCntrlUp},
    {"\e[1;5B", KeyCntrlDown},
    {"\e[1;5D", KeyCntrlLeft},
    {"\e[1;5C", KeyCntrlRight},
    //{"\e[2~", KeyCntrlIns},
    {"\e[3;5~", KeyCntrlDel},
    {"\e[1;5H", KeyCntrlHome},
    {"\e[1;5F", KeyCntrlEnd},
    {"\e[5;5~", KeyCntrlPageUp},
    {"\e[6;5~", KeyCntrlPageDown},

    //{"\e", KeyCntrlTab},

    {"\e[1;2A", KeyShiftUp},
    {"\e[1;2B", KeyShiftDown},
    {"\e[1;2D", KeyShiftLeft},
    {"\e[1;2C", KeyShiftRight},
    {"\e[2;2~", KeyShiftIns},
    {"\e[3;2~", KeyShiftDel},
    {"\e[1;2H", KeyShiftHome},
    {"\e[1;2F", KeyShiftEnd},
    {"\e[5;2~", KeyShiftPageUp},
    {"\e[6;2~", KeyShiftPageDown},

    {"\eOA", KeyShiftUp},
    {"\eOB", KeyShiftDown},
    {"\eOD", KeyShiftLeft},
    {"\eOC", KeyShiftRight},

    {"\e[Z", KeyShiftTab},

    {"\e[1;3A", KeyAltUp},
    {"\e[1;3B", KeyAltDown},
    {"\e[1;3D", KeyAltLeft},
    {"\e[1;3C", KeyAltRight},
    {"\e[2;3~", KeyAltIns},
    {"\e[3;3~", KeyAltDel},
    {"\e[1;3H", KeyAltHome},
    {"\e[1;3F", KeyAltEnd},
    //{"\e[5??~", KeyAltPageUp},
    //{"\e[6??~", KeyAltPageDown},

    {"\e\x0A", KeyAltEnter},
    {"\x7F", KeyBackSpace},

    /*
    {"\e[1;6A", KeyShiftCntrlUp},
    {"\e[1;6B", KeyShiftCntrlDown},
    {"\e[1;6D", KeyShiftCntrlLeft},
    {"\e[1;6C", KeyShiftCntrlRight},
    //{"\e[???", 0}, //KeyShiftCntrlIns},
    {"\e[3;6~", KeyShiftCntrlDel},
    {"\e[1;6H", KeyShiftCntrlHome},
    {"\e[1;6F", KeyShiftCntrlEnd},
    {"\e[5;6~", KeyShiftCntrlPageUp},
    {"\e[6;6~", KeyShiftCntrlPageDown},
    */

    {"\e0", KeyAlt0+0},
    {"\e1", KeyAlt0+1},
    {"\e2", KeyAlt0+2},
    {"\e3", KeyAlt0+3},
    {"\e4", KeyAlt0+4},
    {"\e5", KeyAlt0+5},
    {"\e6", KeyAlt0+6},
    {"\e7", KeyAlt0+7},
    {"\e8", KeyAlt0+8},
    {"\e9", KeyAlt0+9},

    {"\ea", KeyAltA+0},
    {"\eb", KeyAltA+1},
    {"\ec", KeyAltA+2},
    {"\ed", KeyAltA+3},
    {"\ee", KeyAltA+4},
    {"\ef", KeyAltA+5},
    {"\eg", KeyAltA+6},
    {"\eh", KeyAltA+7},
    {"\ei", KeyAltA+8},
    {"\ej", KeyAltA+9},
    {"\ek", KeyAltA+10},
    {"\el", KeyAltA+11},
    {"\em", KeyAltA+12},
    {"\en", KeyAltA+13},
    {"\eo", KeyAltA+14},
    {"\ep", KeyAltA+15},
    {"\eq", KeyAltA+16},
    {"\er", KeyAltA+17},
    {"\es", KeyAltA+18},
    {"\et", KeyAltA+19},
    {"\eu", KeyAltA+20},
    {"\ev", KeyAltA+21},
    {"\ew", KeyAltA+22},
    {"\ex", KeyAltA+23},
    {"\ey", KeyAltA+24},
    {"\ez", KeyAltA+25},

    {"\x0A", KeyEnter}   // Just for Linux
//#endif
  };

#define KeySequenceSize 256   // Power of 2
char KeySequence [KeySequenceSize];
int KeySequenceStart = 0;
int KeySequenceStop = 0;

int KeySequenceIndex (int Index)
  {
    return (Index) & (KeySequenceSize - 1);
  }

//void PutHH (int b);
//bool PutNewLine (void);
//void PutChar (byte ch);

int GetKey_ (void)   // Get Key interpreted into single function
  {
    int c;
    int i, j, k;
    //
    // Collect keys. Fill KeySequence
    while (true)
      {
        // exit if buffer full
        if (KeySequenceIndex (KeySequenceStop + 1) == KeySequenceStart)
          {
            ConsoleBeep ();
            break;
          }
        // Get key
        c = GetKeyRaw ();
        // exit it no more
        if (c < 0)
          break;
        // place in buffer
        KeySequence [KeySequenceStop] = c;
        KeySequenceStop = KeySequenceIndex (KeySequenceStop + 1);
        KeySequence [KeySequenceStop] = 0;
      }
    // return if no keys pressed
    if (KeySequenceStart == KeySequenceStop)
      return -1;
    // Find sequence in KeyMap
    i = 0;
    j = 0;
    while (true)
      {
        if (i >= SIZEARRAY (KeyMap))   // no matches, return first key in
          {
            c = KeySequence [KeySequenceStart];
            KeySequenceStart = KeySequenceIndex (KeySequenceStart + 1);
            return c;
          }
        if (KeyMap [i].Sequence [j] == 0)   // found, return corresponding key code
          {
            KeySequenceStart = KeySequenceIndex (KeySequenceStart + j);
            return KeyMap [i].Key;
          }
        k = KeySequenceIndex (KeySequenceStart + j);
        if (k != KeySequenceStop && KeySequence [k] == KeyMap [i].Sequence [j])
          j++;
        else
          {
            j = 0;
            i++;
          }
      }
  }

bool GetKeyBuffered (void)
  {
    return KeySequenceStart != KeySequenceStop;
  }

int GetKey (void)   // Get key support macro
  {
    int c;
    //
    c = -1;
    // Playing a Macro?
    if (GetKeyMacro)
      if (*GetKeyMacro == 0)
        GetKeyMacro = NULL;
      else
        c = (byte) *GetKeyMacro++;
    if (c < 0)   // No Macro so read keyboard
      c = GetKey_ ();
    // Recording a Macro?
    if (c >= 0)
      if (GetKeyMacroRecord && GetKeyMacroRecordSize)
        {
          *GetKeyMacroRecord++ = c;
          GetKeyMacroRecordSize--;
        }
    return c;
  }

/*
byte GetKeyWait (bool ShowCursor)
  {
    int c;
    //
    if (ShowCursor)
      ConsoleCursorShow ();
    while ((c = GetKey ()) < 0)
#ifdef _Windows
      Sleep (10);
#else
      usleep (10000);
#endif
    if (ShowCursor)
      ConsoleCursorHide ();
    return (byte) c;
  }
*/

#define GetKeyWaitResizeOccured -2

int GetKeyWait (bool ShowCursor)
  {
    int c;
    //
    if (ShowCursor)
      ConsoleCursorShow (true);
    while (true)
      {
        c = GetKey ();
        if (c >= 0)
          break;
        if (ConsoleGetSize ())   // New size
          {
            c = GetKeyWaitResizeOccured;
            break;
          }
#ifdef _Windows
        Sleep (10);
#else
        usleep (10000);
#endif
      }
    //if (ShowCursor)
    ConsoleCursorShow (false);
    return c;
  }

int GetKeyWaitCursor (void)
  {
    return GetKeyWait (true);
  }

#ifdef _Windows

byte WindowColourByte (byte ANSIColour)
  {
    byte Res;
    //
    Res = 0x00;
    if (ANSIColour & ColMaroon)
      Res |= FOREGROUND_RED;
    if (ANSIColour & ColGreenDark)
      Res |= FOREGROUND_GREEN;
    if (ANSIColour & ColBlueDark)
      Res |= FOREGROUND_BLUE;
    if (ANSIColour & ColBright)
      Res |= FOREGROUND_INTENSITY;
    return Res;
  }

word WindowAttribute (void)
  {
    word Res;
    //
    Res = (WindowColourByte (ConsoleFG & 0x0F)) | ((WindowColourByte (ConsoleBG & 0x0F)) << 4);
    if (ConsoleFG & ColUnderline)
      Res |= 0x8000;
    return Res;
  }

/*int WindowsColour (int Col)
  {
    int Res;
    //
    Res = 0;
    switch (Col & 0x07)
      {
        case ColBlack:
          break;
        case ColMaroon:
          Res |= FOREGROUND_RED;
          break;
        case ColGreenDark:
          Res |= FOREGROUND_GREEN;
          break;
        case ColBrown:
          Res |= FOREGROUND_RED | FOREGROUND_GREEN;
          break;
        case ColBlueDark:
          Res |= FOREGROUND_BLUE;
          break;
        case ColPurpleDark:
          Res |= FOREGROUND_RED | FOREGROUND_BLUE;
          break;
        case ColCyanDark:
          Res |= FOREGROUND_BLUE | FOREGROUND_GREEN;
          break;
        case ColGray:
          Res |= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
          break;
      }
    if (Col & ColBright)
      Res |= FOREGROUND_INTENSITY;
    return Res;
  }*/

#endif // _Windows

void ConsoleSetAttributes (void)
  {
    if ((ConsoleFG != ConsoleFG_) || (ConsoleBG != ConsoleBG_))
      {
        putst ("\e[m");   // All attributes off
        putst ("\e[");   // Build attribute command
        /*
        if (ConsoleFG & 8)   // Bold FG
          putst ("1;");
        if (ConsoleFG & ColItalic)
          putst ("3;");
        if (ConsoleFG & ColUnderline)
          putst ("4;");
        //if (ConsoleFG &ColInvert)
        //  puts ("7;");
        putst ("3");
        putchar ((ConsoleFG & 7) + '0');
        putchar (';');
        if (ConsoleBG & 8)
          putst ("10");
        else
          putst ("4");
        putchar ((ConsoleBG & 7) + '0');
        */
        if (ConsoleFG & ColBold)
          putst ("1;");
        if (ConsoleFG & ColItalic)
          putst ("3;");
        if (ConsoleFG & ColUnderline)
          putst ("4;");
        //if (ConsoleFG &ColInvert)
        //  puts ("7;");
        if (ConsoleFG & 8)   // Bright FG
          putst ("9");
        else
          putst ("3");
        putchar ((ConsoleFG & 7) + '0');
        putchar (';');
        if (ConsoleBG & 8)
          putst ("10");
        else
          putst ("4");
        putchar ((ConsoleBG & 7) + '0');
        putchar ('m');
        ConsoleFG_ = ConsoleFG;
        ConsoleBG_ = ConsoleBG;
      }
  }

void PutCharWithAttributes (byte ch)
  {
    #ifdef _Windows
    if ((ConsoleFG != ConsoleFG_) || (ConsoleBG != ConsoleBG_))
      {
        SetConsoleTextAttribute (GetConsoleOutputHandle (), WindowAttribute ());
        ConsoleFG_ = ConsoleFG;
        ConsoleBG_ = ConsoleBG;
      }
    putchar (ch);
    #else
    ConsoleSetAttributes ();
    putchar (ch);
    #endif
  }

void PutCR (void)
  {
    ConsoleX = 0;
    putchar (cr);
    //ConsoleCursor (ConsoleX, ConsoleY);
  }

void PutLF (void)
  {
    putchar (lf);
    if (ConsoleY + 1 < ConsoleSizeY)
      ConsoleY++;
  }

void PutCRLF (void)
  {
    PutLF ();
    PutCR ();
  }

void PutChar (byte ch);

void PutTAB (void)
  {
    //ConsoleX = (((ConsoleX + 1) / ConsoleTab) + 1) * ConsoleTab;
    do
      {
        PutCharWithAttributes (' ');
        ConsoleX++;
      }
    while (ConsoleX % ConsoleTab);
  }

void ConsoleBeep (void)
  {
    putchar (7);
  }

void PutCharPlain (byte ch)
  {
    int SizeX;
    //
    SizeX = ConsoleSizeX;
    if (ConsoleX < SizeX)
      if (ch >= 0x7F)
        {
          ConsoleColourFG (ConsoleFG ^ ColBright);
          PutCharWithAttributes ('$');
          ConsoleColourFG (ConsoleFG ^ ColBright);
        }
      else if (ch >= ' ')
        PutCharWithAttributes (ch);
      else   // control chars shown in italic
        {
          ConsoleColourFG (ConsoleFG ^ ColBright ^ ColItalic);
          PutCharWithAttributes (ch ^ 0x40);
          ConsoleColourFG (ConsoleFG ^ ColBright ^ ColItalic);
        }
    ConsoleX++;
  }

/*
#ifdef _Windows
#define scTL '\''
#define scTR '`'
#define scBL '\\'
#define scBR '/'
#define scHL '-'
#define scVL '|'
#define scBlock 0x7F
#else
#define scTL (0x80|'l')
#define scTR (0x80|'k')
#define scBL (0x80|'m')
#define scBR (0x80|'j')
#define scHL (0x80|'q')
#define scVL (0x80|'x')
#define scBlock (0x80|'a')
#endif
*/

//byte SpecialChar = 0;

void PutChar (byte ch)
  {
    if (ch == '\n')
      PutCRLF ();
    else if (ch == '\r')
      PutCR ();
    else if (ch == '\t')
      PutTAB ();
    else if (ch == '\b')
      {
        if (ConsoleX)
          putchar ('\b');
          //ConsoleCursor (ConsoleX - 1, ConsoleY);
      }
    else if (ch >= ' ')
      PutCharPlain (ch);
  }

void ConsoleColourFG (int n)
  {
    ConsoleFG = n;
    //ConsoleSetAttributes ();
  }

void ConsoleColourBG (int n)
  {
    ConsoleBG = n;
    //ConsoleSetAttributes ();
  }

void StrStr (char **Str, char *s)
  {
    while (*s)
      *(*Str)++ = *s++;
  }

void IntStr (char **Str, int i)
  {
    char s [8], *ps;
    //
    ps = &s [SIZEARRAY (s)];
    *--ps = 0;
    do
      {
        *--ps = (i % 10) + '0';
        i = i / 10;
      }
    while (i);
    StrStr (Str, ps);
  }

void ConsoleCursor (int x, int y)   // Move Cursor. Top Left is (0, 0)
  {
    //
    #ifdef _Windows
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hConsole;
    COORD co;
    //
    hConsole = GetStdHandle (STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo (hConsole, &csbi);
    co.X = x + csbi.srWindow.Left;
    co.Y = y + csbi.srWindow.Top;
    SetConsoleCursorPosition (hConsole, co);
    #else
    char cmd [16], *p;
    p = cmd;
    StrStr (&p, "\e[");
    IntStr (&p, y + 1);
    *p++ = ';';
    IntStr (&p, x + 1);
    *p++ = 'H';
    *p = 0;
    putst (cmd);
    #endif
    ConsoleX = x;
    ConsoleY = y;
  }

bool CursorState = -1;

void ConsoleCursorShow (bool Show)
  {
    if (CursorState != Show)
      {
        CursorState = Show;
        #ifdef _Windows
        HANDLE consoleHandle = GetStdHandle (STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO info;
        info.dwSize = 100;
        info.bVisible = Show;
        SetConsoleCursorInfo (consoleHandle, &info);
        #else
        if (Show)
          putst ("\e[?25h");   // DECTCEM (VT320)
        else
          putst ("\e[?25l");   // DECTCEM (VT320)
        //putst ("\e[?6c");
        //putst ("\e[?1c");
        #endif
      }
  }

/*
void ConsoleCursorHide (void)
  {
    if (CursorState)
      {
        CursorState = false;
        #ifdef _Windows
        #else
        putst ("\e[?25l");   // DECTCEM (VT320)
        #endif
        //putst ("\e[?1c");
      }
  }
*/

#define EOLUseSpaces

void ConsoleClearEOL (void)
  {
    #ifdef _Windows
    int x, y;
    COORD xy;
    DWORD n;
    //
    x = ConsoleX;
    y = ConsoleY;
    xy.X = x;
    xy.Y = y;
    FillConsoleOutputCharacter (GetConsoleOutputHandle (), ' ', ConsoleSizeX - ConsoleX, xy, &n);
    FillConsoleOutputAttribute (GetConsoleOutputHandle (), WindowAttribute (), ConsoleSizeX - ConsoleX, xy, &n);
    #else
    #ifdef EOLUseSpaces
    int x = ConsoleX;
    while (ConsoleX < ConsoleSizeX)
      PutCharPlain (' ');
    ConsoleCursor (x, ConsoleY);
    #else
    ConsoleSetAttributes ();
    putst ("\e[0K");  //?problematic investigate
    #endif // EOLUseSpaces
    #endif
  }

void ConsoleClearEOP (void)
  {
    int y, y0;
    //
    y0 = ConsoleY;
    for (y = y0; y < ConsoleSizeY; y++)
      {
        ConsoleCursor (0, y);
        ConsoleClearEOL ();
      }
    ConsoleCursor (0, y0);
  }

void PutString (const char *St)
  {
    if (St)
      while (*St)
        PutChar (*St++);
  }

void PutStringCRLF (const char *St)
  {
    PutString (St);
    PutCR ();
    PutLF ();
  }

void ConsoleLine (int y, int FG, int BG)   // Clear Line to Colour. Move Cursor to beginning
  {
    ConsoleCursor (0, y);
    ConsoleColourFG (FG);
    ConsoleColourBG (BG);
    ConsoleClearEOL ();
  }

void ConsoleClear (int FG, int BG)
  {
    ConsoleColourFG (FG);
    ConsoleColourBG (BG);
    ConsoleCursor (0, 0);
    ConsoleClearEOP ();
  }

byte KeyCntrl (byte Ch)
  {
    if (Ch >= 'A' && Ch <= 'Z')
      return (Ch - 'A' + 1);
    if (Ch >= 'a' && Ch <= 'z')
      return (Ch - 'a' + 1);
    return 0;
  }

byte KeyAlt (byte Ch)
  {
    if (Ch >= 'A' && Ch <= 'Z')
      return (Ch - 'A' + KeyAltA);
    if (Ch >= 'a' && Ch <= 'z')
      return (Ch - 'a' + KeyAltA);
    if (Ch >= '0' && Ch <= '9')
      return (Ch - '0' + KeyAlt0);
    return 0;
  }
