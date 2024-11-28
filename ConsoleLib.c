////////////////////////////////////////////////////////////////////////////
//
// CONSOLE LIBRARY
// ===============
//
// Author: Stewart Tunbridge, Pi Micros
// Email:  stewarttunbridge@gmail.com
// Copyright (c) 2000-2024 Stewart Tunbridge, Pi Micros
//
////////////////////////////////////////////////////////////////////////////

void PutCharN (char ch, int n)
  {
    while (n-- > 0)
      PutChar (ch);
  }

bool PutNewLine (void)
  {
    bool Res;
    //
    Res = (ConsoleY + 1 >= ConsoleSizeY);
    PutCR ();
    PutLF ();
    return Res;
  }

void PutStringN (const char *St, int n)
  {
    if (St)
      while (n--)
        if (*St == 0)
          PutCharPlain (' ');
        else
          PutCharPlain (*St++);
  }

void PutIntLengthBase (longint n, byte Length, byte Base)   // Write Integer
  {
    char Result [32], *x;
    //
    x = Result;
    IntToStrBase (&x, n, Length, Base);
    *x = 0;
    PutString (Result);
  }

void PutInt (longint n, byte l)
  {
    PutIntLengthBase (n, l, 10);
  }

void PutIntScaled (longint n)
  {
    char Res [16], *x;
    //
    x = Res;
    IntToStrScaled (&x, n);
    *x = 0;
    PutString (Res);
  }

void PutIntCommas (int Val)
  {
    char St [24], *s;
    //
    s = St;
    IntToStrBase (&s, Val, IntToLengthCommas, 10);
    *s = 0;
    PutString (St);
  }

void PutIntDecimals (longint n, byte Decimals)
  {
    char St [24], *s;
    //
    s = St;
    IntToStrDecimals (&s, n, Decimals);
    *s = 0;
    PutString (St);
  }

void PutHH (int n)
  {
    PutIntLengthBase (n & 0xFF, 2 | 0x80, 16);
  }

void PutHHHH (int n)
  {
    PutIntLengthBase (n & 0xFFFF, 4 | 0x80, 16);
  }

void PutHHHHHH (int n)
  {
    PutIntLengthBase (n & 0xFFFFFF, 6 | 0x80, 16);
  }

void PutOOO (int n)
  {
    PutIntLengthBase (n & 0x1FF, 3 | 0x80, 8);
  }

void PutDateTimeLocalize (time_t DateTime, char *Format)   // Format eg "%Y %b %d %H:%M:%S"
  {
    char ts [64], *tsp;
    //
    tsp = ts;
    DateTimeToStrLocalize (&tsp, DateTime, Format);
    *tsp = 0;
    PutString (ts);
  }

void PutStringHighlight (char *St, byte FG2)
  {
    //byte Col;
    byte Mask;
    //
    //Col = ConsoleFG;
    Mask = ConsoleFG ^ FG2;
    while (*St)
      {
        if (*St == '|')
          ConsoleColourFG (ConsoleFG ^ Mask);
        else
          PutChar (*St);
        St++;
      }
    //ConsoleColourFG (Col);
  }

int StrLengthHighlight (const char * St)
  {
    int Res;
    //
    Res = 0;
    while (*St)
      {
        if (*St != '|')
          Res++;
        St++;
      }
    return Res;
  }


/////////////////////////////////////////////////////////////////////////
//
// EDIT DATA TYPES
//
/////////////////////////////////////////////////////////////////////////

bool EditChanged;

int EditString (char *St, int Size, int Width)
  {
    int x0;     // Starting position
    int xOff;   // x Offset
    int PosX;   // Position in the string
    int c;
    int Len;
    bool First;
    char *StOld;
    //
    StOld = (char *) malloc (Size);
    StrCopy (StOld, St);
    x0 = ConsoleX;
    PosX = 0;
    xOff = 0;
    First = true;
    if (Width >= ConsoleSizeX - ConsoleX)
      Width = ConsoleSizeX - ConsoleX - 1;
    c = esc;
    if (Width > 0)
      while (true)
        {
          ConsoleCursor (x0, ConsoleY);
          PutStringN (&St [xOff], Width);
          Len = StrLength (St); //ConsoleX - x0;   // current length of string
          ConsoleCursor (x0 + PosX - xOff, ConsoleY);
          c = GetKeyWaitCursor ();
          if (c == KeyLeft)
            {
              if (PosX)
                PosX--;
            }
          else if (c == KeyRight)   // Right
            {
              if (St [PosX])   // Not end of string
                PosX++;
            }
          else if (c == KeyHome)   // Home
            PosX = 0;
          else if (c == KeyEnd)   // End
            PosX = Len;
          else if (c == KeyBackSpace)   // Backspace
            {
              if (PosX)
                StrDelete (&St [--PosX], 1);
            }
          else if (c == KeyDel)   // Delete
            {
              if (St [PosX])
                StrDelete (&St [PosX], 1);
            }
          else if ((c == KeyAltDel) || (c == KeyCntrlDel) || (c == Cntrl ('L')))   // Delete End of Line
            St [PosX] = 0;
          else if (c == KeyShiftDel)
            {
              PosX = 0;
              St [PosX] = 0;
            }
          else if (c == KeyIns)
            {
              if (Len < Size)
                StrInsertN (&St [PosX], 1);
              else
                ConsoleBeep ();
            }
          else if ((c >= ' ') && (c < 0x7F))   // Insert Character
            {
              if (First)   // First typed is a character => Delete all
                {
                  St [0] = 0;
                  Len = 0;
                }
              if (Len < Size)
                {
                  StrInsertN (&St [PosX], 1);
                  St [PosX++] = c;
                }
              else
                ConsoleBeep ();
            }
          else
            break;
          First = false;
          // Adjust x offset
          while (PosX - xOff >= Width)
            xOff++;
          while ((PosX - xOff < Width) && (xOff))
            xOff--;
          //while (PosX - xOff < 0)
          //  xOff--;
        }
    if (StrCompare (St, StOld))
      EditChanged = true;
    free (StOld);
    ConsoleCursor (x0, ConsoleY);
    return c;
  }

int EditLongint (longint *Value)   // Returns terminating key
  {
    char s [16], *p;
    longint v;
    int c;
    //
    while (true)
      {
        p = s;
        IntToStr (&p, *Value);
        *p = 0;
        c = EditString (s, sizeof (s), sizeof (s));
        if (c == esc)
          break;
        p = s;
        //StrGetError = false;
        v = StrGetInt (&p);
        if (v == StrGetIntError || *p)
          ConsoleBeep ();
        else
          {
            *Value = v;
            break;
          }
      }
    return c;
  }

byte EditInt (int *Value, int Min, int Max)   // Returns terminating key
  {
    longint Val;
    byte c;
    //
    while (true)
      {
        Val = *Value;
        c = EditLongint (&Val);
        if (c == esc)
          break;
        if ((Val >= Min) && (Val <= Max))
          {
            *Value = Val;
            break;
          }
        ConsoleBeep ();
      }
    return c;
  }

byte EditMask (int *Mask, const char *MaskTab, int Size)
  {
    int x0, x;
    int i;
    byte c;
    //
    x0 = ConsoleX;
    x = 0;
    while (true)
      {
        ConsoleCursor (x0, ConsoleY);
        for (i = 0; i < Size; i++)
          if (*Mask & Bit [Size - i - 1])
            PutChar (MaskTab [i]);
          else
            PutChar ('-');
        ConsoleCursor (x0 + x, ConsoleY);
        c = GetKeyWaitCursor ();
        switch (c)
          {
            case KeyLeft:
              if (x > 0)
                x--;
              break;
            case KeyRight:
              if (x + 1 < Size)
                x++;
              break;
            case KeyHome:
              x = 0;
              break;
            case KeyEnd:
              x = Size - 1;
              break;
            default:
              if ((c >= ' ') && (c < 0x80))
                {
                  *Mask ^= Bit [Size - x - 1];
                  if (x + 1 < Size)
                    x++;
                  EditChanged = true;
                }
              else
                return c;
          }
      }
  }

typedef enum {whatever} _enum;

byte EditEnum (_enum *Value, const char *EnumNames [])
//byte EditEnum (int *Value, const char *EnumNames [])
  {
    int x0, x1;
    byte c;
    _enum Value_;
    _enum ValueOld;
    //
    ValueOld = *Value;
    x0 = ConsoleX;
    x1 = x0;
    while (true)
      {
        ConsoleCursor (x0, ConsoleY);
        PutString (EnumNames [*Value]);
        if (ConsoleX > x1)
          x1 = ConsoleX;
        while (ConsoleX < x1)
          PutChar (' ');
        ConsoleCursor (x0, ConsoleY);
        c = UpCase (GetKeyWaitCursor ());
        Value_ = *Value;
        if (IsAlpha (c))
          while (true)
            {
              (*(int *) &Value_)++; // ++Value_;
              if (EnumNames [Value_] == NULL)
                Value_ = (_enum) 0;
              if (Value_ == *Value)   // no match
                {
                  ConsoleBeep ();
                  break;
                }
              if (UpCase (c) == UpCase (EnumNames [Value_] [0]))   // match
                {
                  *Value = Value_;
                  break;
                }
            }
        else if ((c == ' ') || (c == KeyRight))
          {
            (*(int *) Value)++;
            if (EnumNames [*Value] == NULL)
              *Value = (_enum) 0;
          }
        else if (c == KeyLeft)
          if (*Value == 0)
            while (EnumNames [*Value + 1])
              (*(int *) Value)++;
          else
            (*(int *) Value)--;
        else
          break;
      }
    if (*Value != ValueOld)
      EditChanged = true;
    return c;
  }

const char *BoolName [] = {"No", "Yes", NULL};

byte EditBool (bool *Value)
  {
    return EditEnum ((_enum*) Value, BoolName);
  }

byte EditBit (unsigned int *Value, int Mask)
  {
    bool v;
    byte c;
    //
    v = (*Value & Mask) != 0;
    c = EditBool (&v);
    if (v)
      *Value = *Value | Mask;
    else
      *Value = *Value & ~Mask;
    return c;
  }

int EditDateTime (struct tm *DateTime)
  {
    struct tm DateTime_;
    char s [32], *sx;
    int c;
    //
    while (true)
      {
        // Build Date Time String
        sx = s;
        IntToStrFill (&sx, DateTime->tm_year + 1900, 2 | IntToLengthZeros);
        CharToStr (&sx, '-');
        IntToStrFill (&sx, DateTime->tm_mon + 1, 2 | IntToLengthZeros);
        CharToStr (&sx, '-');
        IntToStrFill (&sx, DateTime->tm_mday, 2 | IntToLengthZeros);
        CharToStr (&sx, ' ');
        IntToStrFill (&sx, DateTime->tm_hour, 2 | IntToLengthZeros);
        CharToStr (&sx, ':');
        IntToStrFill (&sx, DateTime->tm_min, 2 | IntToLengthZeros);
        *sx = 0;
        // Edit String
        c = EditString (s, sizeof (s), sizeof (s));
        if (c == esc)
          return esc;
        // Parse String
        DateTime_ = *DateTime;
        sx = s;
        //StrGetError = false;
        DateTime_.tm_year = StrGetInt (&sx);
        if (DateTime_.tm_year != StrGetIntError)
          if (StrGetSpecificChar (&sx, '-'))
            {
              DateTime_.tm_mon = StrGetNum (&sx);
              if ((DateTime_.tm_mon > 0) && (DateTime_.tm_mon <= 12))
                if (StrGetSpecificChar (&sx, '-'))
                  {
                    DateTime_.tm_mday = StrGetNum (&sx);
                    if (DateTime_.tm_mday > 0)
                      if (StrGetSpecificChar (&sx, ' '))
                        {
                          DateTime_.tm_hour = StrGetNum (&sx);
                          if (DateTime_.tm_hour > 0)
                            if (StrGetSpecificChar (&sx, ':'))
                              {
                                DateTime_.tm_min = StrGetNum (&sx);
                                if (DateTime_.tm_min > 0)
                                  if (*sx == 0)
                                    {
                                      DateTime_.tm_year -= 1900;
                                      DateTime_.tm_mon -= 1;
                                      DateTime_.tm_sec = 0;
                                      *DateTime = DateTime_;
                                      return c;
                                    }
                              }
                        }
                  }
            }
        ConsoleBeep ();
      }
  }

byte EditColours (byte *Col, bool Foreground)   // Returns terminating key
  {
    int x0;
    byte c, cOld;
    byte ColOld;
    //
    ColOld = *Col;
    cOld = 0;
    x0 = ConsoleX;
    while (true)
      {
        ConsoleCursor (x0, ConsoleY);
        if (Foreground)
          ConsoleColourFG (*Col);
        else
          ConsoleColourBG (*Col);
        PutString (" Sample ");
        PutHH (*Col);
        PutChar (' ');
        ConsoleCursor (x0, ConsoleY);
        c = UpCase (GetKeyWaitCursor ());
        if (c == 'B' && Foreground)
          *Col ^= ColBold;
        else if (c == 'I' && Foreground)
          *Col ^= ColItalic;
        else if (c == 'U' && Foreground)
          *Col ^= ColUnderline;
        else if (c == ' ' || c == KeyRight|| c == '+')
          *Col = (((*Col + 1) ^ *Col) & 0x0F) ^ *Col;
        else if (c == KeyLeft || c == '-')
          *Col = (((*Col - 1) ^ *Col) & 0x0F) ^ *Col;
        else if (c >= '0' && c <= '7')
          if (c == cOld)
            *Col ^= ColBright;
          else
            *Col = (*Col & ~0x0F) | (c - '0');
        else if (c == '8')
          *Col ^= ColBright;
        else
          break;
        cOld = c;
      }
    if (*Col != ColOld)
      EditChanged = true;
    return c;
  }

// Edit a string allowing a selection from a list -
//   Default is the initial string values (if provided)
//   If the result string is not in the StringArray, it will be added to it -

int EditStringArray (char **StringArray, int Size, int StLength, char *Default)
  {
    char *Item;
    int Index;
    int c;
    int x;
    //
    Item = (char *) malloc (StLength);
    x = ConsoleX;
    Index = -1;
    while (true)
      {
        ConsoleCursor (x, ConsoleY);
        ConsoleClearEOL ();
        if (Index < 0)
          {
            Item [0] = 0;
            if (Default)
              StrCopyN (Item, Default, StLength);
          }
        else
          StringArrayRetrieve (StringArray, Size, Index, Item, StLength);
        c = EditString (Item, StLength, StLength);
        if (c == KeyUp)
          {
            if (Index < 0 || (Index + 1 < Size && StringArray [Index + 1]))
              Index++;
          }
        else if ((c == KeyDown) || (c == tab))
          {
            if (Index >= 0)
              Index--;
          }
        else
          break;
      }
    if (c != esc)
      if (Item [0])
        StringArrayAdd (StringArray, Size, Item);
    free (Item);
    return c;
  }

/*
bool EditStringArray (char **StringArray, int Size, int StLength, char *Default)
  {
    int Index;
    //
    Index = -1;
    return EditStringArray_ (StringArray, Size, StLength, Default, &Index);
  }

bool EditStringArray_ (char **StringArray, int Size, int StLength, char *Default)
  {
    byte c;
    //
    while (true)
      {
        c = EditStringArray (StringArray, Size, StLength, Default);
        if (c == KeyEnter)
          return true;
        if (c == esc)
          return false;
        ConsoleBeep ();
      }
  }
*/

int SelectStringArray (char **StringArray, int Size, int Start)
  {
    int x;
    byte c;
    //
    x = ConsoleX;
    while (true)
      {
        ConsoleCursor (x, ConsoleY);
        ConsoleClearEOL ();
        PutString (StringArray [Start]);
        c = UpCase (GetKeyWaitCursor ());
        if (c == KeyEnter)
          return Start;
        if (c == esc)
          return -1;
        if (c == KeyDown)
          Start = StringArrayNext (StringArray, Size, Start, +1);
        else if (c == KeyUp)
          Start = StringArrayNext (StringArray, Size, Start, -1);
        else
          ConsoleBeep ();
      }
  }

byte GetOption (char *List, int FG2)
  {
    byte c;
    int i;
    char Res;
    //
    PutStringHighlight (List, FG2);
    //PutString (": ");
    //PutString (List);
    PutString (": ");
    while (true)
      {
        c = UpCase (GetKeyWaitCursor ());
        if ((c == esc) || (c == KeyEnter))
          return c;
        i = 0;
        while (true)
          {
            if (List [i] == 0)   // not found
              {
                ConsoleBeep ();
                break;
              }
            if (IsAlphaUpper (List [i]))
              {
                Res = List [i];
                if ((Res == c) || (KeyCntrl (Res) == c) || (KeyAlt (Res) == c))   // found
                  {
                    while (true)   // show full word reply
                      {
                        if (IsAlpha (List [i]))
                          PutChar (List [i]);
                        else if (List [i] != '|')
                          break;
                        i++;
                      }
                    return Res;
                  }
              }
            i++;
          }
      }
  }

bool YesNo (int FG2)
  {
    byte c;
    //
    c = GetOption ("|Y|es/|N|o", FG2);
    return ((c == 'Y') || (c == KeyEnter));
  }


////////////////////////////////////////////////////////////////////////////
//
// CONSOLE APPLICATION SUPPORT
//
////////////////////////////////////////////////////////////////////////////

void DrawScrollBar (int yTop, int yBot,
                    unsigned int PosTop, unsigned int PosBot, unsigned int PosSize,
                    byte Col1, byte Col2)
  {
    int y1, y2, y;
    //
    if (PosSize)
      {
        y1 = yTop + (yBot - yTop + 1) * PosTop / PosSize;
        y2 = yTop + (yBot - yTop + 1) * PosBot / PosSize;
        ConsoleColourFG (Col1 & !(ColItalic | ColUnderline));   // no silly attributes please
        //ConsoleColourBG (Col2);
        for (y = yTop; y <= yBot; y++)
          {
            ConsoleCursor (ConsoleSizeX - 1, y);
            if ((y >= y1) && (y <= y2))
              {
                //ConsoleColourFG (Col2);
                ConsoleColourBG (Col1);
                PutChar (' ');
              }
            else
              {
                //ConsoleColourFG (Col1);
                ConsoleColourBG (Col2);
                PutChar ('|');
              }
          }
      }
  }

void PutDataHex (byte *Data, unsigned int DataLen, bool ASCII)
  {
    char *st;
    unsigned int stl;
    //
    if (ASCII)
      st = DataToHexAscii (Data, DataLen, &stl, ConsoleSizeX - ConsoleX);
    else
      st = DataToHex (Data, DataLen, &stl, ConsoleSizeX - ConsoleX);
    StrIndent (&st, ConsoleX);
    PutString (st);
    free (st);
  }

/*
void PutBox (int x1, int y1, int x2, int y2)
  {
    int x, y;
    int i;
    //
    x = ConsoleX;
    y = ConsoleY;
    ConsoleCursor (x1, y1);
    PutChar (scTL);
    PutCharN (scHL, x2 - x1 - 1);
    PutChar (scTR);
    for (i = y1 + 1; i < y2; i++)
      {
        ConsoleCursor (x1, i);
        PutChar (scVL);
        PutCharN (' ', x2 - x1 - 1);
        PutChar (scVL);
      }
    ConsoleCursor (x1, y2);
    PutChar (scBL);
    PutCharN (scHL, x2 - x1 - 1);
    PutChar (scBR);
    ConsoleCursor (x, y);
  }
*/

void StringExtent (char *St, int *dx, int *dy)
  {
    int x;
    int i;
    //
    *dx = *dy = 0;
    x = 0;
    i = 0;
    while (true)
      {
        if (x > *dx)
          *dx = x;
        if (St [i] == 0)
          break;
        if (*dy == 0)
          *dy = 1;
        if (St [i] != cr)
          if (St [i] == '\n')
            {
              x = 0;
              (*dy)++;
            }
          else
            x++;
        i++;
      }
  }

/*
void PutStringMulti (char *St)
  {
    int x;
    int y;
    int i;
    //
    x = ConsoleX;
    y = ConsoleY;
    i = 0;
    while (St [i])
      {
        if (St [i] != cr)
          if (St [i] == '\n')
            ConsoleCursor (x, ++y);
          else
            PutChar (St [i]);
        i++;
      }
  }
*/

void PutBoxStr (char *St)
  {
    //int x, y;
    int dx, dy;
    //int i;
    //
    //x = ConsoleX;
    //y = ConsoleY;
    StringExtent (St, &dx, &dy);
    PutCharN ('-', dx);
    PutNewLine ();
    //for (i = 0; i < dy + 2; i++)
    //  if (PutNewLine ())
    //    y--;
    //PutBox (x, y, x + dx + 3, y + dy + 1);
    //ConsoleCursor (x + 2, y + 1);
    PutString (St);
    PutNewLine ();
    PutCharN ('-', dx);
    //ConsoleCursor (x, y + dy + 2);
    PutNewLine ();
    PutNewLine ();
  }
