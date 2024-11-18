////////////////////////////////////////////////////////////////////////////
//
// GENERAL SUPPORT LIBRARY
// -----------------------
// Author: Stewart Tunbridge, Pi Micros
// Email:  stewarttunbridge@gmail.com
//
// Maths
// Strings and Formatting
// String Array editing
// Dynamic Arrays
// ..

#ifndef _Lib
#define _Lib

#include <time.h>
#include <utime.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

//#include <string.h>  replacing these ... reasons

#ifdef _Windows
  //#define WINVER 0x0500
  //#define _WIN32_WINNT 0x0500   // See msdn.microsoft.com "GetConsoleWindow"
  #include <sys\stat.h>
  #include <windows.h>
  const char PathDelimiter = '\\';
  //#include <winsock.h>
#else
  #include <unistd.h>
  #include <sys/time.h>
  const char PathDelimiter = '/';
#endif

#ifndef __cplusplus
typedef enum {false, true} bool;
#endif
typedef unsigned short word;
typedef unsigned char byte;

#define SIZEARRAY(X) (sizeof(X)/sizeof(X[0]))

#ifdef _Windows
  //#include <inttypes.h>
  typedef __int64 longint; //typedef int64_t longint;
#else
  //typedef off_t longint;
  typedef int64_t longint;
#endif

const unsigned int Bit [32] =
  {
    0x1, 0x2, 0x4, 0x8,
    0x10, 0x20, 0x40, 0x80,
    0x100, 0x200, 0x400, 0x800,
    0x1000, 0x2000, 0x4000, 0x8000,
    0x10000, 0x20000, 0x40000, 0x80000,
    0x100000, 0x200000, 0x400000, 0x800000,
    0x1000000, 0x2000000, 0x4000000, 0x8000000,
    0x10000000, 0x20000000, 0x40000000, 0x80000000
  };


/////////////////////////////////////////////////////////////////////////////
//
// Math etc Functions

int Max (int v1, int v2)
  {
    if (v1 > v2)
      return v1;
    return v2;
  }

int Min (int v1, int v2)
  {
    if (v1 < v2)
      return v1;
    return v2;
  }

int Sign (int Val)
  {
    if (Val < 0)
      return -1;
    if (Val > 0)
      return +1;
    return 0;
  }

int Abs (int Val)
  {
    if (Val < 0)
      return -Val;
    return Val;
  }

double AbsDouble (double Val)
  {
    if (Val < 0.0)
      return -Val;
    return Val;
  }

void SwapBytes (void *a, void *b, int n)
  {
    byte c;
    //
    while (n--)
      {
        c = ((byte *) a) [n];
        ((byte *) a) [n] = ((byte *) b) [n];
        ((byte *) b) [n] = c;
      }
  }

void SwapByte (byte *a, byte *b)
  {
    byte c;
    //
    c = *a;
    *a = *b;
    *b = c;
  }

int LogN (int Val, int N)   // How many digits are needed?
  {
    int Res;
    //
    if (Val < 0)
      return 0;
    Res = 1;
    while (Val >= N)
      {
        Val = Val / N;
        Res++;
      }
    return Res;
  }

int ClockMS (void)
  {
#ifdef _Windows
    return GetTickCount ();
#else
    struct timeval t;
    //
    gettimeofday (&t, NULL);
    return (t.tv_sec * 1000) + (t.tv_usec / 1000);
#endif
  }

/*
float sqrt_ (float x)   // Yes lets reinvent the wheel yet again
  {
    float res;
    float res_;
    int n;
    //
    res = 1.0;
    n = 32;
    while (true)
      {
        res_ = res;
        res = res - (res * res - x) / (2.0 * res);
        if (res == res_)
          break;
        if (--n == 0)
          break;
        //printf ("\t%.10f\n", res);
      }
    return res;
  }
*/


/////////////////////////////////////////////////////////////////////////////
//
// Character & String Functions

#define Cntrl(X) (X-'@')

#define cr  0x0D
#define lf  0x0A
#define esc 0x1B
#define tab 0x09
#define bel 0x07
#define ff  0x0C
//#define bs  0x08

const char *StrNull = "";

bool IsAlphaLower (char c)
  {
    return (c >= 'a') && (c <= 'z');
  }

bool IsAlphaUpper (char c)
  {
    return (c >= 'A') && (c <= 'Z');
  }

bool IsAlpha (char c)
  {
    return IsAlphaUpper (c) || IsAlphaLower (c);
  }

char UpCase (char c)
  {
    if (IsAlphaLower (c))
      c = c - 'a' + 'A';
    return c;
  }

bool IsDigit (char c)
  {
    return (c >= '0') && (c <= '9');
  }

bool IsPrintable (char c)
  {
    return ((c >= ' ') && (~c & 0x80));
  }

#define NotFound ((unsigned int) -1)

int GetDigit (char c)   // converts to digit values 0-9,A-Z. Returns -1 if invalid
  {
    c = UpCase (c);
    if ((c >= '0') && (c <= '9'))
      return c - '0';
    if ((c >= 'A') && (c <= 'Z'))
      return c - 'A' + 10;
    return NotFound;
  }

unsigned int StrPosCh (char *St, char Target);

int IsBracket (char Ch)
  {
    int i;
    //
    i = StrPosCh ("}]) ([{", Ch);
    if (i < 0)
      return 0;
    return i - 3;
    /*
    switch (Ch)
      {
        case '(': return +1;
        case ')': return -1;
        case '[': return +2;
        case ']': return -2;
        case '{': return +3;
        case '}': return -3;
      }
    */
  }

int IsQuote (char c)
  {
    if (c == '\'')
      return 1;
    if (c == '\"')
      return 2;
    return 0;
  }

int StrLength (const char *St)
  {
    int Res;
    //
    Res = 0;
    if (St)
      while (*St++)
        Res++;
    return Res;
  }

// StrCopy: Copies 0 terminated string. Returns address of the 0.
//
char *StrCopy (char *Dest, const char *Source)
  {
    char c;
    //
    if (Dest && Source)
      while (true)
        {
          c = *Source++;
          *Dest = c;
          if (c == 0)
            break;
          Dest++;
        }
    return Dest;
  }

// StrCopyN: As above, Size limited
//
char *StrCopyN (char *Dest, char *Source, int Size)
  {
    char c;
    //
    if (Dest && Source)
      while (true)
        {
          c = *Source++;
          if (--Size == 0)
            c = 0;
          *Dest = c;
          if (c == 0)
            break;
          Dest++;
        }
    return Dest;
  }

void StrSwap (char *S1, char *S2)
  {
    char c;
    //
    if (S1 && S2)
      while (true)
        {
          if ((*S1 | *S1) == 0)
            break;
          c = *S1;
          *S1 = *S2;
          *S2 = c;
          if (S1)
            S1++;
          if (*S2)
            S2++;
        }
  }

void MemMove (void *Dest, void *Source, unsigned int Size)
  {
    if (Dest < Source)
      while (Size--)
        *(byte *) Dest++ = * (byte *) Source++;
    else
      while (Size--)
        ((byte *) Dest) [Size] = ((byte *) Source) [Size];
  }

void MemSet (void *Dest, byte Value, unsigned int Size)
  {
    while (Size--)
      *(byte *) Dest++ = Value;
  }

// Dynamic String assign from a Static String

void StrAssign (char **Dest, char *Source)   // Dynamic String
  {
    char *d;
    //
    d = NULL;
    if (Source)
      {
        d = (char *) malloc (StrLength (Source) + 1);
        StrCopy (d, Source);
      }
    if (*Dest)
      free (*Dest);
    *Dest = d;
  }

/*
void StrAssign (char **Dest, char *Source)
  {
    if (*Dest)
      free (*Dest);
    *Dest = Source;
  }
*/

void StrAssignAppend (char **Dest, char *Suffix)   // Dynamic String Append
  {
    char *New;
    //
    New = (char *) malloc (StrLength (*Dest) + StrLength (Suffix) + 1);
    StrCopy (StrCopy (New, *Dest), Suffix);
    free (*Dest);
    *Dest = New;
  }


/////////////////////////////////////////////////////////////////////////////
//
// Search String: StrPos, StrPosFrom, StrPosBackwards
// Matching criteria specified in StrPosMode

typedef enum {spmStrict, spmNoCase, spmWild, spmZZZZ} _StrPosMode;
const char *StrPosModeNames [] = {"Strict", "NoCase", "Wild"};

//_StrPosMode StrPosMode = spmNoCase;

unsigned int StrMatch (char *St, char *Target, int TargetLen, _StrPosMode StrPosMode)
  {
    char s, t;
    unsigned int Res;
    //
    Res = 0;
    while (true)
      {
        s = *St++;
        t = *Target++;
        if (t == 0)
          return Res;
        switch (StrPosMode)
          {
            case spmStrict:
              if (s != t)
                return 0;
              break;
            case spmNoCase:
              if (UpCase (s) != UpCase (t))
                return 0;
              break;
            case spmWild:
              if (t != '?')
                if (t == '#')   // any digit
                  {
                    if (!IsDigit (s))
                      return 0;
                  }
                else if (t == '@')   // any alpha
                  {
                    if (!IsAlpha (s))
                      return 0;
                  }
                else if (t == '&')   // any non alpha
                  {
                    if (IsAlpha (s))
                      return 0;
                  }
                else if (t == '~')   // not the next character
                  {
                    t = *Target++;
                    if (UpCase (s) == UpCase (t))
                      return 0;
                  }
                else if (t == '^')   // control character
                  {
                    t = *Target++;
                    if (s != (t & 0x1F))
                      return 0;
                  }
                else
                  if (UpCase (s) != UpCase (t))
                    return 0;
              break;
          }
        if (t == 0)
          return 0;
        Res++;
        if (--TargetLen == 0)
          return Res;
      }
  }

unsigned int StrPosFrom (char *St, unsigned int Start, char *Target, _StrPosMode StrPosMode)
  {
    if (Target == NULL || Target [0] == 0)
      return Start;
    if (St == NULL)
      return -1;
    while (true)
      {
        if (StrMatch (&St [Start], Target, 0, StrPosMode))   // Found
          return Start;
        if (St [Start] == 0)   // Not found
          return -1;
        Start++;
      }
  }

unsigned int StrPosFromBackwards (char *St, unsigned int Start, char *Target, int TargetLen, _StrPosMode StrPosMode)   // returns array index. "-1" if not found
  {
    if (Target == NULL)
      return Start;
    if (St == NULL)
      return -1;
    while (true)
      {
        if (StrMatch (&St [Start], Target, TargetLen, StrPosMode))   // Found
          return Start;
        if (Start == 0)   // Not found
          return -1;
        Start--;
      }
  }

unsigned int StrPos (char *St, char *Target, _StrPosMode StrPosMode)   // returns array index. -1 if not found
  {
    return StrPosFrom (St, 0, Target, StrPosMode);
  }

unsigned int StrPos_ (char *St, char *Target)
  {
    return StrPos (St, Target, spmNoCase);
  }

unsigned int StrPosChFrom (char *St, unsigned int Start, char Target)   // returns array index. "-1" if not found
  {
    if (St)
      while (St [Start])
        {
          if (St [Start] == Target)
            return Start;
          Start++;
        }
    return -1;
  }

unsigned int StrPosCh (char *St, char Target)   // returns array index. "-1" if not found
  {
    return StrPosChFrom (St, 0, Target);
  }

char *StrPosCh_ (char *St, char Target)   // returns pointer to Target or NULL
  {
    while (true)
      {
        if (*St == 0)
          return NULL;
        if (*St == Target)
          return St;
        St++;
      }
  }

unsigned int StrPosLastCh (char *St, char Target)
  {
    int i;
    int Res;
    //
    Res = -1;
    i = 0;
    if (St)
      while (St [i])
        {
          if (St [i] == Target)
            Res = i;
          i++;
        }
    return Res;
  }

unsigned int StrChCount (char *St, char Target, unsigned int Size)
  {
    unsigned int Res;
    //
    Res = 0;
    while (true)
      {
        if (*St == 0)
          break;
        if (*St == Target)
          Res++;
        if (Size)
          if (--Size == 0)
            break;
        St++;
      }
    return Res;
  }

// StrPosMulti: Targets: multiple, space separated
bool StrPosMulti (char *St, char *Targets, _StrPosMode StrPosMode)
  {
    int p1, p2, Res;
    char c;
    //
    p1 = 0;
    while (true)
      {
        // Step space
        while (Targets [p1] == ' ')
          p1++;
        // Are we done?
        if (Targets [p1] == 0)
          break;
        // Find next section in Target
        p2 = StrPosChFrom (Targets, p1, ' ');
        if (p2 < 0)
          p2 = StrLength (Targets);
        // Look for that section in St
        c = Targets [p2];
        Targets [p2] = 0;
        Res = StrPos (St, &Targets [p1], StrPosMode);
        Targets [p2] = c;
        // Is it there?
        if (Res >= 0)   // found?
          return true;
        // Go onto next section
        p1 = p2;
      }
    return false;
  }


/////////////////////////////////////////////////////////////////////////////
//
// Edit String: Insert, Delete etc

void StrRemoveLastChar (char *St)
  {
    int i;
    //
    if (St)
      {
        i = 0;
        while (St [i])
          i++;
        if (i)
          St [i - 1] = 0;
      }
  }

void StrDelete (char *St, int Gap)
  {
    int l;
    //
    l = StrLength (St);
    MemMove (St, &St [Gap], l - Gap + 1);
  }

void StrInsertN (char *St, int Gap)
  {
    MemMove (&St [Gap], St, StrLength (St) + 1);
    MemSet (St, ' ', Gap);
  }

void StrInsertStrN (char *St, char *s, int MaxLen)
  {
    if (St && s)
      {
        if (MaxLen == 0)
          MaxLen = StrLength (s);
        StrInsertN (St, MaxLen);
        MemMove (St, s, MaxLen);
      }
  }

void StrInsertStr (char *St, char *s)
  {
    StrInsertStrN (St, s, 0);
  }

unsigned int StrReplaceCh (char *St, char Target, char Ch)
  {
    unsigned int Count;
    //
    Count = 0;
    while (*St)
      {
        if (*St == Target)
          {
            *St = Ch;
            Count++;
          }
        St++;
      }
    return Count;
  }

char *StrEnd (char *St)
  {
    while (*St)
      St++;
    return St;
  }

char *StrAppend (char *St, char Ch)
  {
    St = StrEnd (St);
    *St++ = Ch;
    *St = 0;
    return St;
  }

char *StrConcat (char *St, const char *Source)
  {
    return StrCopy (StrEnd (St), Source);
  }

void StrTrim (char *St)
  {
    int i;
    //
    i = StrLength (St);
    while (true)
      {
        if (i == 0)
          break;
        i--;
        if ((byte) St [i] > ' ')
          break;
        St [i] = 0;
      }
  }


/*
/////////////////////////////////////////////////////////////////////////////
//
// Compare Strings. CASE INSENSITIVE

int StrCmpN (const char *s1, const char *s2, int n)
  {
    if (!s1)
      s1 = StrNull;
    if (!s2)
      s2 = StrNull;
    while (true)
      {
        if (n-- == 0)   // Match
          return 0;
        if ((*s1 == 0) && (*s2 == 0))   // Match
          return 0;
        if (UpCase (*s1) < UpCase (*s2))
          return -1;
        if (UpCase (*s1) > UpCase (*s2))
          return +1;
        s1++;
        s2++;
      }
  }

int StrCmp (const char *s1, const char *s2)
  {
    return StrCmpN (s1, s2, -1);
  }

bool StrSame (const char *s1, const char *s2)
  {
    return StrCmp (s1, s2) == 0;
  }

int StrInArray (char *Array [], char *Target)
  {
    int Res;
    //
    Res = 0;
    while (true)
      {
        if (Array [Res] == NULL || Array [Res][0] == 0)
          return -1;
        if (StrSame (Array [Res], Target))
          return Res;
        Res++;
      }
  }
*/


/////////////////////////////////////////////////////////////////////////////
//
// String Parsing

// Step past all white space
//
void StepSpace (char **Pos)
  {
    if (*Pos)
      while ((**Pos == ' ') || (**Pos == '\t') || (**Pos == '\n') || (**Pos == '\r'))
        (*Pos)++;
  }

// Get an item from a tab / cr / crlf separated string
// Return the terminating character (after stepping past it
//
char StrGetItem (char **Pos, char *Result, int ResultMax, bool Comma)
  {
    unsigned char c;
    //
    if ((*Pos == NULL) || (Result == NULL))
      return 0;   // Error No string
    while (true)
      {
        c = **Pos;
        (*Pos)++;
        if (c < ' ' || (Comma && c == ','))
          break;
        if (--ResultMax > 0)   // Limited result size but continue parsing
          *Result++ = c;
      }
    *Result = 0;   // Terminate result
    if ((c == cr) && (**Pos == lf))   // step past lf in a crlf pair
      (*Pos)++;
    return c;
  }

bool StrExpectChar (char **St, char Ch)
  {
    StepSpace (St);
    if (**St == Ch)
      {
        if (Ch)
          (*St)++;
        return true;
      }
    return false;
  }

//bool StrGetError;   // Set true if error found in StrGet*

longint StrGetNumBase (char **Pos, int Base)
  {
    longint Result;
    bool OK;
    int Digit;
    //
    OK = false;
    StepSpace (Pos);
    Result = 0;
    while (true)
      {
        Digit = GetDigit (**Pos);
        if (Digit < 0)
          break;
        if (Digit >= Base)
          break;
        Result = (Result * Base) + Digit;
        (*Pos)++;
        OK = true;
      }
    if (!OK)
      return -1;
      //StrGetError = true;
    return Result;
  }

longint StrGetNum (char **Pos)   // Get a cardinal
  {
    return StrGetNumBase (Pos, 10);
  }

#define StrGetIntError 0x8000000000000000   // max negative longint

longint StrGetInt (char **Pos)   // Get an integer
  {
    longint Result;
    bool Neg;
    //
    Neg = false;
    StepSpace (Pos);
    if (**Pos == '-')
      {
        Neg = true;
        (*Pos)++;
        StepSpace (Pos);
      }
    Result = StrGetNumBase (Pos, 10);
    if (Result < 0)   // Error
      return StrGetIntError;
    if (Neg)
      return -Result;
    return Result;
  }

longint StrGetHex (char **Pos)
  {
    return StrGetNumBase (Pos, 16);
  }

double StrGetReal (char **Pos)
  {
    bool n;   // negative?
    bool d;   // decimal point encountered
    int Exp, e;   // exponent
    char c;
    double Result;
    //
    // Note Sign
    StepSpace (Pos);
    n = false;
    if (**Pos == '-')
      {
        (*Pos)++;
        n = true;
      }
    else if (**Pos == '+')
      (*Pos)++;
    // Read value of all mantissa digits. Adjust exponent if decimal point present
    Result = 0.0;
    Exp = 0;
    d = false;
    while (true)
      {
        c = **Pos;
        if (c == 0)
          break;
        if (IsDigit (c))
          {
            (*Pos)++;
            Result = (Result * 10.0) + GetDigit (c);
            if (d)
              Exp--;
          }
        else if (c == '.')
          {
            (*Pos)++;
            d = true;
          }
        else
          {
            if (UpCase (c) == 'E')   // Adjusting exponent if Scientific Notation present
              {
                (*Pos)++;
                e = StrGetInt (Pos);
                if (e == StrGetIntError)
                  return 0.0 / 0.0; //NAN;
                Exp = Exp + e;
              }
            break;
          }
      }
    // Apply multiplier
    while (Exp < 0)
      {
        Result = Result / 10.0;
        Exp++;
      }
    while (Exp > 0)
      {
        Result = Result * 10.0;
        Exp--;
      }
    // Apply Sign
    if (n)
      Result = -Result;
    return Result;
  }

unsigned int StrGetHexAscii (char **Pos, byte *Data, unsigned int MaxData)
// Convert a string of HEX and "ASCII" to raw data
//  Returns "-1" on syntax error
  {
    bool Ascii;
    unsigned int Index;
    int Val;
    int x;
    //
    Index = 0;
    Ascii = false;
    //StrGetError = false;
    while (true)
      {
        /*if (**Pos == 0)
          {
            if (Ascii)
              return -1;   // forgot to close quotes
            return Index;
          }*/
        Val = -1;
        if (**Pos == 0)
          break;
        if (Ascii)
          {
            if (**Pos == '\"')   // Switch mode
              Ascii = false;
            else
              Val = **Pos;
            (*Pos)++;
          }
        else
          {
            StepSpace (Pos);
            if (**Pos == '\"')   // Switch mode
              {
                Ascii = true;
                (*Pos)++;
              }
            else
              {
                x = StrGetNumBase (Pos, 16);
                if (x < 0)
                  break;
                if (x >= 0x100)
                  return -1;
                Val = x;
              }
          }
        if (Val >= 0)
          {
            if (Data)
              if (Index < MaxData)
                Data [Index] = Val;
            Index++;
          }
      }
    if (Ascii)
      return -1;   // forgot to close quotes OR Hex Error
    return Index;
  }

unsigned int StrGetHexAscii_ (char **Pos, byte *Data, unsigned int MaxData, unsigned int StrLen)
  {
    unsigned int Res;
    char c, *p;
    //
    p = *Pos + StrLen;
    c = *p;
    *p = 0;
    Res = StrGetHexAscii (Pos, Data, MaxData);
    *p = c;
    return Res;
  }

bool StrGetSpecificChar (char **St, char c)
  {
    if (c == ' ')
      {
        if (**St == ' ')
          {
            StepSpace (St);
            return true;
          }
        return false;
      }
    StepSpace (St);
    if (**St == c)
      {
        (*St)++;
        return true;
      }
    return false;
  }

int StrToInt (char *St)
  {
    char *s;
    //
    s = St;
    return StrGetInt (&s);
  }


/////////////////////////////////////////////////////////////////////////////
//
// Build String: Format items into a string, adjusting the pointer

int CharToStr (char **Dest, char Source)
  {
    if (Dest)
      if (*Dest)
        *(*Dest)++ = Source;
    return 1;
  }

int CharToStrRepeat (char **Dest, char Source, int n)
  {
    int i;
    //
    if (*Dest)
      for (i = 0; i < n; i++)
        CharToStr (Dest, Source);
    return n;
  }

// Append string to String via pointer.
// Limit to n chars (if n != 0)
// Returns count of characters coppied
//
unsigned int StrToStrN (char **Dest, char *Source, unsigned int n)
  {
    unsigned int j;
    //
    j = 0;
    if (Source)
      while (true)
        {
          if (n-- == 0)
            break;
          if (*Source == 0)
            break;
          if (*Dest)
            *(*Dest)++ = *Source;
          Source++;
          j++;
        }
    return j;
  }

unsigned int StrToStr (char **Dest, char *Source)
  {
    return StrToStrN (Dest, Source, -1);
  }

#define IntToLengthZeros 0x80
#define IntToLengthCommas 0x40
#define IntToLength 0x3F

int IntToStrBase (char **Dest, longint n, byte Length, byte Base)   // Length: b0-b5 Size. + 0x80 pad with '0'. + 0x40 include commas/hyphens
  {
    longint n_;
    int i, j;
    char temp [128];
    int x;
    //
    n_ = n;
    if (n < 0)   // Negative
      {
        n_ = -n;
        Length &= ~IntToLengthZeros;   // Negative numbers pad with spaces
      }
    // Convert to decimal characters (in reverse order)
    i = sizeof (temp);
    temp [--i] = 0;   // terminate
    j = 0;
    do
      {
        x = (n_ % Base);
        if (x < 10)
          temp [--i] = x + '0';
        else
          temp [--i] = x - 10 + 'A';
        j++;
        n_ = n_ / Base;
        if (n_ == 0)
          break;
        if (Length & IntToLengthCommas)   // commas / hyphens
          if (Base == 10)
            {
              if (j % 3 == 0)
                temp [--i] = ',';
            }
          else if (j % 4 == 0)
            temp [--i] = '-';
      }
    while (n_);
    // Negative sign
    if (n < 0)
      temp [--i] = '-';
    // Pad out requested spaces
    while ((byte) (sizeof (temp) - 1 - i) < (Length & 0x3F))
      {
        if (Length & IntToLengthZeros)
          temp [--i] = '0';
        else
          temp [--i] = ' ';
      }
    // Add decimal characters to result (in correct order)
    return StrToStr (Dest, &temp [i]);
  }

int IntToStrFill (char **Dest, longint n, byte Digits)
  {
    return IntToStrBase (Dest, n, Digits, 10);
  }

int IntToStr (char **Dest, longint n)
  {
    return IntToStrBase (Dest, n, 0, 10);
  }

int IntToHex (char **Dest, longint n, byte Digits)
  {
    return IntToStrBase (Dest, n, Digits, 16);
  }

int IntToStrDecimals (char **Dest, longint n, byte Decimals)
  {
    longint m;
    int i;
    int Res;
    //
    Res = 0;
    if (n < 0)
      {
        if (*Dest)
          *(*Dest)++ = '-';
        n = -n;
        Res++;
      }
    m = 1;
    for (i = 0; i < Decimals; i++)
      m = m * 10;
    Res += IntToStr (Dest, n / m);
    if (Decimals)
      {
        if (*Dest)
          *(*Dest)++ = '.';
        Res++;
        Res += IntToStrFill (Dest, n % m, Decimals | IntToLengthZeros);
      }
    return Res;
  }

char Multiplier [] = "kMGTPEZY";

int IntToStrScaled (char **Dest, longint n)
  {
    int x;
    int Res;
    //
    Res = 0;
    if (n < 0)
      {
        if (*Dest)
          *(*Dest)++ = '-';
        Res++;
        n = -n;
      }
    x = 0;
    while (n >= 10000)
      {
        n /= 1000;
        x++;
      }
    Res += IntToStrFill (Dest, n, 0 | IntToLengthZeros | IntToLengthCommas);
    if (x)
      {
        if (*Dest)
          *(*Dest)++ = Multiplier [x - 1];
        Res++;
      }
    return Res;
  }

/*
void RealToStr (char **Dest, double Val, int Decimals)
  {
    int i;
    //
    if (Decimals < 0)
      {
        Decimals = 0;
        while (true)
          {
            if (Val + 1.0 == Val)   // val too big to have a fraction
              break;
            if (Val == (long) Val)   // no fractional part remaining
              break;
            Val = Val * 10.0;
            Decimals++;
          }
      }
    else
      for (i = 0; i < Decimals; i++)
        Val = Val * 10.0;
    if (Val > 0.0)
      Val += 0.5;
    else
      Val -= 0.5;
    if (Decimals)
      IntToStrDecimals (Dest, Val, Decimals);
    else
      IntToStr (Dest, Val);
  }
*/

double Power10 (int Exp)
  {
    double Val;
    //
    Val = 1.0;
    while (Exp > 0)
      {
        Val *= 10.0;
        Exp--;
      }
    while (Exp < 0)
      {
        Val /= 10.0;
        Exp++;
      }
    return Val;
  }

#define FlagsCommas 0x01

int RealToStr (char **Dest, double Val, int Decimals, byte Flags)
  {
    int Exp;
    double Mul, Round;
    int Dig;   // this digit
    int Size;   // resultant size
    //
    Size = 0;
    // Negative sign
    if (Val < 0.0)
      {
        if (*Dest)
          *(*Dest)++ = '-';
        Size++;
        Val = -Val;
      }
    // Scale Mul, Exp, Round: 10.0 > Val * Mul >= 1.0
    Exp = 0;
    Mul = 1.0;
    Round = 1.0;
    if (Val)
      {
        while (Val * Mul >= 10.0)
          {
            Mul /= 10.0;
            Exp++;
          }
        while (Val * Mul < 1.0)
          {
            Mul *= 10.0;
            Exp--;
          }
        // Round up
        if (Decimals < 0)   // decimals unspecified, so use significance
          Round = 5.0 / Mul / Power10 (15);
        else   // specified
          Round = 5.0 / Power10 (Decimals + 1);
        Val += Round;
        if (Val * Mul > 10.0)
          {
            Mul /= 10.0;
            Exp++;
          }
      }
    // Force unscaled display low numbers
    if (Mul > 1.0)
      {
        Mul = 1.0;
        Exp = 0;
      }
    // Calculate succesive digits
    while (true)
      {
        Dig = Val * Mul;
        if (*Dest)
          *(*Dest)++ = Dig + '0';   // Add Digit
        Size++;
        Val = Val - Dig / Mul;   // remove Digit's value
        Mul *= 10;
        if (Decimals < 0)   // "no" limit specified
          {
            if (Exp <= 0)
              if (Val <= Round + Round)
                break;
          }
        else   // decimals specified
          if (Decimals == -Exp)
            break;
        if (Exp == 0)
          {
            if (*Dest)
              *(*Dest)++ = '.';
            Size++;
          }
        else if ((Flags & FlagsCommas) && (Exp > 0) && (Exp % 3 == 0))
          {
            if (*Dest)
              *(*Dest)++ = ',';
            Size++;
          }
        Exp--;
      }
    // Optionally kill trailing zeros
    return Size;
  }

// DateTimeToStrLocalize:
// Format DateTime according to Format
// Format eg "%Y %b %d %H:%M:%S"

bool DateTimeToStrLocalize (char **Dest, time_t DateTime, char *Format)
  {
    char ts [64];
    struct tm *tmp;
    //
    tmp = localtime (&DateTime);
    if (tmp)
      {
        strftime (ts, 32, Format, tmp);
        StrToStr (Dest, ts);
        return true;
      }
    return false;
  }

void KeyName (char **Dest, byte Ch)
  {
    if (Ch < ' ')
      {
        *(*Dest)++ = '^';
        *(*Dest)++ = Ch + '@';
      }
    else if (Ch < 0x80)
      *(*Dest)++ = Ch;
    else
      IntToHex (Dest, Ch, 2);
  }

bool IntStrToStrFirst;

int IntStrToStr (char **Dest, longint Num, char *StrSingular, char *StrPlural)
  {
    int Res;
    //
    Res = 0;
    if (Num)
      {
        if (!IntStrToStrFirst)
          Res += CharToStr (Dest, ' ');
        IntStrToStrFirst = false;
        Res += IntToStrBase (Dest, Num, 0 | IntToLengthCommas, 10);
        Res += CharToStr (Dest, ' ');
        if ((Num > 1) || (Num < -1))
          Res += StrToStr (Dest, StrPlural);
        else
          Res += StrToStr (Dest, StrSingular);
      }
    return Res;
  }


/*
// DataToHex:
// Convert block of raw data to string of Hex broken by spaces and \n
// Always ends on /n
// Caller must free returned result

char *DataToHex (byte *Data, unsigned int DataLen, unsigned int *Size, int Width)
  {
    char *Res, *r;
    unsigned int i;
    int x;
    //
    *Size = DataLen * 3;
    Res = (char *) malloc (*Size + 1);   // Size includes trailing lf but not terminating 0
    r = Res;
    x = 0;
    if (Res)
      {
        if (DataLen)
          {
            i = 0;
            while (true)
              {
                IntToHex (&r, Data [i], 2 | IntToLengthZeros);
                x += 2;
                i++;
                if (i >= DataLen)
                  break;
                if ((Width > 0) && (x + 3 > Width))
                  {
                    *r++ = lf;
                    x = 0;
                  }
                else
                  {
                    *r++ = ' ';
                    x++;
                  }
              }
          }
        *r++ = lf;
        *r = 0;
      }
    return Res;
  }
*/

// DataToHex*:
// Convert block of raw data to string of Hex and "ASCII" broken by spaces and \n
// Always ends on /n
// Caller must free returned result

char *DataToHex_ (byte *Data, unsigned int DataLen, unsigned int *Size, int Width, bool AllowASCII)   // Caller must free returned result
  {
    char *Res, *r;
    unsigned int Line, Break, i;
    byte d;
    bool ASCII, ASCII_;
    //
    Res = NULL;
    while (true)
      {
        *Size = 0;
        r = Res;
        Line = 0;
        Break = 0;
        ASCII = ASCII_ = false;
        i = 0;
        while (i < DataLen)
          {
            d = Data [i];   // Next data byte
            if (AllowASCII)
              ASCII_ = (d >= ' ') && (d < 0x7F) && (d != '\"');   // Is data displayable as ASCII character?
            if (ASCII_ && !ASCII)   // Go to "ASCII" mode?
              {
                (*Size)++;
                if (Res)
                  *r++ = '\"';
              }
            else if (!ASCII_ && ASCII)   // Go to HEX mode?
              {
                (*Size) += 2;
                if (Res)
                  StrToStr (&r, "\" ");
                Break = *Size - 1;
              }
            else if (ASCII)   // Add data as ASCII
              {
                (*Size)++;
                if (Res)
                  *r++ = d;
                i++;
              }
            else   // Otherwise show as HEX
              {
                (*Size) += 3;
                if (Res)
                  {
                    IntToHex (&r, d, 2 | IntToLengthZeros);
                    *r++ = ' ';
                  }
                i++;
                Break = *Size - 1;
              }
            ASCII = ASCII_;
            if (Width)
              if (((int) (*Size - Line) + 1 >= Width) && (Break > Line))   // Line full and breakable
                {
                  if (Res)
                    Res [Break] = lf;
                  Line = Break + 1;
                }
          }
        // End ASCII?
        if (ASCII)
          {
            (*Size)++;
            if (Res)
              *r++ = '\"';
          }
        // End line (if needed)
        if (Line != *Size)
          {
            (*Size)++;
            if (Res)
              *r++ = lf;
          }
        // Terminate
        if (Res)
          *r = 0;
        // Allocate storage
        if (Res == NULL)
          Res = (char *) malloc (*Size + 1);   // Size includes trailing lf but not terminating 0
        else
          return Res;
      }
  }

char *DataToHex (byte *Data, unsigned int DataLen, unsigned int *Size, int Width)
  {
    return DataToHex_ (Data, DataLen, Size, Width, false);
  }

char *DataToHexAscii (byte *Data, unsigned int DataLen, unsigned int *Size, int Width)
  {
    return DataToHex_ (Data, DataLen, Size, Width, true);
  }


// StrIndent: Replace dynamic string with indented version: \n -> \n {space}
void StrIndent (char **St, int Indent)
  {
    char *Res, c;
    int i, j, k;
    //
    Res = NULL;
    i = 0;
    j = 0;
    while (true)
      {
        c = (*St) [i];
        if (Res)
          Res [j] = c;
        i++;
        j++;
        if (c == 0)
          if (Res)
            break;
          else
            {
              Res = (char *) malloc (j);
              i = 0;
              j = 0;
            }
        else
          if ((c == '\n') && (*St) [i])
            for (k = 0; k < Indent; k++)
              {
                if (Res)
                  Res [j] = ' ';
                j++;
              }
      }
    free (*St);
    *St = Res;
  }

// Replace Target with Replacement in new string, returned
// Size is returned. Result will be 0 terminated
// Returns NULL if Target not found
char *StrReplaceAll_ (char *St, char *Target, char *Replacement, unsigned int *Size, _StrPosMode StrPosMode)
  {
    unsigned int Siz;
    unsigned int SizeSt, SizeTarget;
    char *Res, *r;
    unsigned int a, b;
    bool Changed;
    //
    Changed = false;
    SizeSt = StrLength (St);
    SizeTarget = StrLength (Target);
    Siz = a = 0;
    r = Res = NULL;
    while (true)
      {
        b = StrPosFrom (St, a, Target, StrPosMode);
        if ((int) b == -1)   // not found
          b = SizeSt;
        Siz += StrToStrN (&r, St + a, b - a);
        if (b < SizeSt)
          {
            Siz += StrToStr (&r, Replacement);
            Changed = true;
          }
        a = b + SizeTarget;
        if (a >= SizeSt)   // end reached
          {
            if (Res)
              {
                *r = 0;
                if (Size)
                  *Size = Siz;
                return Res;
              }
            if (Changed)
              {
                r = Res = (char *) malloc (Siz + 1);
                Siz = a = 0;
              }
            else
              return NULL;
          }
      }
  }

// As above but replaces dynamic string St
bool StrReplaceAll (char **St, char *Target, char *Replacement, unsigned int *Size, _StrPosMode StrPosMode)
  {
    char *Res;
    //
    Res = StrReplaceAll_ (*St, Target, Replacement, Size, StrPosMode);
    if (Res)
      {
        free (*St);
        *St = Res;
        return true;
      }
    return false;
  }

// Convert letter case
void StrToLower (char *St)
  {
    while (*St)
      {
        if (IsAlphaUpper (*St))
          *St ^= 'A' ^ 'a';
        St++;
      }
  }

void StrToUpper (char *St)
  {
    while (*St)
      {
        if (IsAlphaLower (*St))
          *St ^= 'A' ^ 'a';
        St++;
      }
  }


// StrCRLF: Replace dynamic string with corrected line termination:
//   toCRLF: false: [\n | \r\n] -> \r\n  true: [\r\n | \n] -> \n
/*void StrCRLF (char **St, bool toCRLF)
  {
    char *Res, c;
    int i, j;
    //
    Res = NULL;
    i = 0;
    j = 0;
    while (true)
      {
        c = (*St) [i++];
        if (c != cr)
          if (toCRLF && (c == lf))
            {
              if (Res)
                {
                  Res [j] = cr;
                  Res [j+1] = lf;
                }
              j += 2;
            }
          else
            {
              if (Res)
                Res [j] = c;
              j++;
            }
        if (c == 0)
          if (Res)
            break;
          else
            {
              Res = malloc (j);
              i = 0;
              j = 0;
            }
      }
    free (*St);
    *St = Res;
  }*/

char *StrGetFileExtension (char *FileName)
  {
    int i;
    //
    if (FileName == NULL)
      return NULL;
    i = StrPosLastCh (FileName, '.');
    if (i <= 0)   // no dot OR dot file
      return &FileName [StrLength (FileName)];   // point to end of FileName
    return &FileName [i];   // point to the dot and extension
  }

void StrSetFileExtension (char *FileName, char *Ext, bool Force)
  {
    char *p;
    //
    p = StrGetFileExtension (FileName);
    if (*p == 0 || Force)
      {
        StrToStr (&p, Ext);
        *p = 0;
      }
  }


////////////////////////////////////////////////////////////////////////////
//
// DYNAMIC ARRAY SUPPORT
//

typedef struct
  {
    int Size;
    int SizeAllocated;
    void **Data;
  } _Array;

bool ArrayCheckSize (_Array *Array, int Size)
  {
    int s;
    void *d;
    bool Res;
    //
    Res = true;
    if (Size > Array->SizeAllocated)
      {
        s = Array->SizeAllocated;
        while (s < Size)
          if (s == 0)
            s = 1;
          else
            s = s << 1;
        d = malloc (sizeof (void *) * s);
        if (d)
          {
            MemSet (d, 0, sizeof (void *) * s);
            MemMove (d, Array->Data, sizeof (void *) * Array->SizeAllocated);
            free (Array->Data);
            //memmove (Array->Data, d, sizeof (d));
            Array->Data = (void **) d;
            Array->SizeAllocated = s;
          }
        else
          Res = false;
      }
    return Res;
  }

typedef void _ArrayFreeElement (void *Data);

void ArrayFree (_Array *Array, _ArrayFreeElement *ArrayFreeElement)
  {
    int i;
    //
    for (i = 0; i < Array->SizeAllocated; i++)
      if (Array->Data [i])
        {
          if (ArrayFreeElement)
            ArrayFreeElement (Array->Data [i]);
          free (Array->Data [i]);
        }
    Array->Size = 0;
    Array->SizeAllocated = 0;
    free (Array->Data);
    Array->Data = NULL;
  }

bool ArraySet (_Array *Array, int Index, void *Data)
  {
    if (ArrayCheckSize (Array, Index + 1))
      {
        free (Array->Data [Index]);
        Array->Data [Index] = Data;
        if (Index >= Array->Size)
          Array->Size = Index + 1;
        return true;
      }
    return false;
  }

void *ArrayGet (_Array *Array, int Index)
  {
    if (Index < Array->SizeAllocated)
      return Array->Data [Index];
    return NULL;
  }

bool ArrayAdd (_Array *Array, void *Data)
  {
    return ArraySet (Array, Array->Size, Data);
  }


////////////////////////////////////////////////////////////////////////////
//
// STRING ARRAY SUPPORT
//

int StrCompareCase (char *S1, char *S2, bool Cased)
  {
    int delta;
    //
    if (!S1)
      S1 = (char *) StrNull;
    if (!S2)
      S2 = (char *) StrNull;
    while (true)
      {
        if (*S1 == 0 && *S2 == 0)
          return 0;
        if (*S1 == 0)
          return -1;
        if (*S2 == 0)
          return +1;
        if (Cased)
          delta = *S1 - *S2;
        else
          delta = UpCase (*S1) - UpCase (*S2);
        if (delta)
          return delta;
        S1++;
        S2++;
      }
  }

int StrCompare (char *S1, char *S2)
  {
    return StrCompareCase (S1, S2, true);
  }

int StringArrayAdd (char **StringArray, int Size, char *String)
  {
    int i;
    //
    if (String)
      {
        // Look for entry already in queue
        i = 0;
        while (true)
          {
            if (i == Size - 1)
              break;
            if (StringArray [i])
              if (StrCompare (StringArray [i], String) == 0)   // already in list
                break;
            i++;
          }
        // discard unwanted
        if (StringArray [i])
          StrAssign (&StringArray [i], NULL);
        // Make space in queue
        while (i)
          {
            StringArray [i] = StringArray [i - 1];
            i--;
          }
        StringArray [0] = NULL;
        // Place new one on top
        StrAssign (&StringArray [0], String);
      }
    return i;
  }

void StringArrayDelete (char **StringArray, int Size, int Index)
  {
    if (StringArray)
      if (Index < Size)
        {
          StrAssign (&StringArray [Index], NULL);
          while (Index + 1 < Size)
            {
              StringArray [Index] = StringArray [Index + 1];
              Index++;
            }
          StringArray [Index] = NULL;
        }
  }

int StringArrayNext (char **StringArray, int Size, int Index, int Step)
  {
    int i;
    //
    i = Index;
    if (Size > 1)
      while (true)
        {
          i += Step;
          if (i >= Size)
            i = 0;
          if (i < 0)
            i = Size - 1;
          if (i == Index)
            break;
          if (StringArray [i])
            break;
        }
    return i;
  }

bool StringArrayRetrieve (char **StringArray, int Size, int n, char *Result, int ResultSize)
  {
    if ((n >= 0) && (n < Size))
      if (StringArray [n])
        {
          StrCopyN (Result, StringArray [n], ResultSize);
          return true;
        }
    return false;
  }

const char lf_ = lf;

void StringArrayWrite (char **StringArray, int Size, int file, bool Reverse)
  {
    int i;
    //
    i = 0;
    if (Reverse)
      i = Size - 1;
    while (true)
      {
        if ((i < 0) || (i >= Size))
          break;
        if (StringArray [i])
          {
            write (file, StringArray [i], StrLength (StringArray [i]));
            write (file, &lf_, 1);
          }
        if (Reverse)
          i--;
        else
          i++;
      }
  }

void StringArrayFree (char **StringArray, int Size)
  {
    int i;
    //
    for (i = 0; i < Size; i++)
      if (StringArray [i])
        {
          free (StringArray [i]);
          StringArray [i] = NULL;
        }
  }

int StringArraySearch (char **StringArray, int Size, char *Key)
  {
    int Res;
    char *p;
    int i;
    //
    Res = 0;
    while (true)
      {
        if (Res >= Size)   // Not found
          return -1;
        p = StringArray [Res];
        if (p == NULL)
          return -1;
        i = 0;
        while (true)
          {
            if (Key [i] < ' ')   // Found: Allow for tab/cr/lf/null separation
              return Res;
            if (Key [i] != p [i])
              break;
            i++;
          }
        Res++;
      }
  }

void StringArraySearchReplace (char **StringArray, int Size, char *KeyData)
  {
    int Index;
    //
    Index = StringArraySearch (StringArray, Size, KeyData);
    if (Index >= 0)   // Found
      StrAssign (&StringArray [Index], KeyData);
    else
      StringArrayAdd (StringArray, Size, KeyData);
  }

void StringArraySearchRemove (char **StringArray, int Size, char *Key)
  {
    int Index;
    //
    Index = StringArraySearch (StringArray, Size, Key);
    if (Index >= 0)   // Found
      StringArrayDelete (StringArray, Size, Index);
  }


////////////////////////////////////////////////////////////////////////////
//
// LIST SUPPORT - SORT ...
//

////////////////////////////////////////////////////////////////////////////
//
// Sort ANY List of struct. First item is a pointer to the next. NULL to terminate.
// Uses Merge sort with no memory overhead (other than small stack use)

typedef int _SortListCompare (void *Data1, void *Data2);

void *SortList (void **List, _SortListCompare *SortListCompare)
  {
    void **left, **right;
    void **left_, **right_, **List_;
    void **Res, **Res_;
    unsigned int i;
    //
    // Base case. A list of zero or one elements is sorted, by definition.
    if (List == NULL)
      return NULL;
    if (*List == NULL)
      return List;
    // Divide the list into equal-sized sublists
    left = NULL;
    right = NULL;
    //
    i = 0;
    List_ = List;
    left_ = (void **) &left;
    right_ = (void **) &right;
    while (List_)
      {
        if (i & 1)   // odd index -> add left
          {
            *left_ = List_;
            left_ = List_;
          }
        else   // add right
          {
            *right_ = List_;
            right_ = List_;
          }
        i++;
        List_ = (void **) *List_;
      }
    *left_ = NULL;
    *right_ = NULL;
    // Recursively sort both sublists.
    left = (void **) SortList (left, SortListCompare);
    right = (void **) SortList (right, SortListCompare);
    // Then merge the now-sorted sublists.
    Res = NULL;
    Res_ = (void **) &Res;
    left_ = left;
    right_ = right;
    while (true)
      {
        if (left_ && right_)   // both left and right still have data
          if (SortListCompare (left_, right_) < 0)   // add left
            {
              *Res_ = left_;
              left_ = (void **) *left_;
            }
          else   // add right
            {
              *Res_ = right_;
              right_ = (void **) *right_;
            }
        else if (left_)   // add left
          {
            *Res_ = left_;
            left_ = (void **) *left_;
          }
        else if (right_)   // add right
          {
            *Res_ = right_;
            right_ = (void **) *right_;
          }
        else
          break;
        Res_ = (void **) *Res_;
      }
    return Res;
  }


////////////////////////////////////////////////////////////////////////////
//
// FILING
//

bool FileExists (char *FileName)
  {
    //FILE *f;
    int f;
    //
    if (FileName)
      {
        f = open (FileName, O_RDONLY);
        if (f >= 0)
          {
            close (f);
            return true;
          }
      }
    return false;
  }

typedef enum {foRead, foWrite, foAppend} _FileOpenMode;

int FileOpen (char *Filename, _FileOpenMode FileOpenMode)
  {
    #ifdef _Windows
    if (FileOpenMode == foRead)
      return open (Filename, O_RDONLY | O_BINARY);
    if (FileOpenMode == foWrite)
      return open (Filename, O_RDWR | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);   // Open file for writing
    return open (Filename, O_WRONLY | O_BINARY | O_APPEND | O_CREAT, S_IREAD | S_IWRITE);   // Open file for appending
    #else
    if (FileOpenMode == foRead)
      return open (Filename, O_RDONLY);   // Open file for reading only
    if (FileOpenMode == foWrite)
      return open (Filename, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);   // Open file for writing
    return open (Filename, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);   // Open file for appending
    #endif // _Windows
  }


////////////////////////////////////////////////////////////////////////////
// TEXT FILE "CLASS"

typedef struct
  {
    int ID;
    longint Size;
    byte *Buffer;
    unsigned int BufferPos;
    unsigned int Line;
  } _TextFile;

void TextFileInit (_TextFile *TextFile)
 {
   TextFile->ID = 0;
 }

bool TextFileIsOpen (_TextFile *File)
  {
    return (File->ID > 0);
  }

bool TextFileOpen (_TextFile *File, char *Filename, _FileOpenMode FileOpenMode)
  {
    File->Size = -1;
    File->Buffer = NULL;
    File->BufferPos = 0;
    File->Line = 0;
    File->ID = FileOpen (Filename, FileOpenMode);
    if (File->ID > 0)
      {
        File->Size = lseek (File->ID, 0, SEEK_END);
        lseek (File->ID, 0, SEEK_SET);
      }
    return File->ID > 0;
  }

void TextFileClose (_TextFile *File)
  {
    if (File->ID > 0)
      close (File->ID);
    File->Size = -1;
    if (File->Buffer != NULL)
      free (File->Buffer);
    File->Buffer = NULL;
    File->ID = 0;
  }

// Returns pointer to the next line string in File.Buffer
// NULL if no more
// Span => allow line to connect if terminated with a '\'

typedef enum {eolNone, eolLF, eolCR, eolCRLF, eolNULL} _EndOfLine;

_EndOfLine EOL (char *c)
  {
    if (c [0] == 0)
      return eolNULL;
    if (c [0] == lf)
      return eolLF;
    if (c [0] == cr)
      {
        if (c [1] == lf)
          return eolCRLF;
        return eolCR;
      }
    return eolNone;
  }

char *TextFileReadln (_TextFile *File, bool Span)
  {
    char *Res;
    char *cp;
    _EndOfLine eol;
    //
    Res = NULL;
    if (File->ID > 0)
      {
        // Read Buffer if not yet read
        if (File->Buffer == NULL)
          {
            File->Buffer = (byte *) malloc (File->Size + 1);
            if (read (File->ID, File->Buffer, File->Size) == File->Size)
              File->Buffer [File->Size] = 0;   // Terminate current line in buffer
            else
              {
                free (File->Buffer);
                File->Buffer = NULL;
              }
          }
        if ((File->Buffer) && (File->BufferPos < File->Size))
          {
            Res = (char *) &File->Buffer [File->BufferPos];
            File->Line++;
            while (true)
              {
                cp = (char *) &File->Buffer [File->BufferPos];
                if (File->BufferPos >= File->Size) //(*cp == 0)   // end of file reached
                  break;
                if (Span && (cp [0] == '\\') && cp [1] < ' ')
                  {
                    eol = EOL (cp + 1);
                    if (eol == eolCR || eol == eolLF || eol == eolNULL)
                      {
                        *cp++ = ' ';
                        *cp++ = ' ';
                      }
                    else if (eol == eolCRLF)
                      {
                        *cp++ = ' ';
                        *cp++ = ' ';
                        *cp++ = ' ';
                      }
                  }
                else
                  {
                    eol = EOL (cp);
                    if (eol == eolCR || eol == eolLF || eol == eolNULL)
                      {
                        *cp = 0;   // terminate line
                        File->BufferPos++;   // step past cr/lf
                        return Res;
                      }
                    if (eol == eolCRLF)
                      {
                        *cp = 0;   // terminate line
                        File->BufferPos += 2;   // step past cr + lf
                        return Res;
                      }
                    File->BufferPos++;   // next character
                  }
              }
          }
      }
    return Res;
  }

char *TextFileSeakln (_TextFile *File, int Line, bool Span)
  {
    char *Res;
    //
    Res = NULL;
    if (TextFileIsOpen (File))
      {
        if (Line < File->Line)
          {
            File->BufferPos = 0;
            File->Line = 0;
          }
        while (true)
          {
            Res = TextFileReadln (File, Span);
            if (Res == NULL)   // no more lines
              break;
            if (File->Line > Line)   // found it
              break;
          }
      }
    return Res;
  }

bool TextFileWrite (_TextFile *File, char *St)
  {
    int l;
    //
    l = StrLength (St);
    if (File->ID > 0)
      if (write (File->ID, St, l) == l)
        return true;
    return false;
  }

bool TextFileWriteln (_TextFile *File, char *St)
  {
    if (TextFileWrite (File, St))
      if (write (File->ID, &lf_, 1) == 1)
        return true;
    return false;
  }


////////////////////////////////////////////////////////////////////////////
//
// JUST BECAUSE ..............
//

const longint Order [5] = {1000000000000000, 1000000000000, 1000000000, 1000000, 1000};
const char *OrderWords [5] = {"quadrillion", "trillion", "billion", "million", "thousand"};
const char *TensWords [10] = {"--", "--", "twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety"};
const char *OnesWords [20] = {"zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine",
                              "ten", "eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen", "seventeen", "eighteen", "nineteen"};
void IntToStrWords (char **Dest, longint n)
  {
    char *Dest0;
    int Base;
    //
    if (n < 0)
      {
        StrToStr (Dest, "negative ");
        n = -n;
      }
    Dest0 = *Dest;
    Base = 0;
    while (Base < SIZEARRAY (Order))
      {
        if (n >= Order [Base])
          {
            if (*Dest > Dest0)   // we already have some result
              StrToStr (Dest, ", ");
            IntToStrWords (Dest, n / Order [Base]);
            n = n % Order [Base];
            CharToStr (Dest, ' ');
            StrToStr (Dest, (char *) OrderWords [Base]);
          }
        Base++;
      }
    if (n >= 100)
      {
        if (*Dest > Dest0)   // we already have some result
          StrToStr (Dest, ", ");
        IntToStrWords (Dest, n / 100);
        n = n % 100;
        StrToStr (Dest, " hundred");
      }
    if (n > 0)
      if (*Dest > Dest0)   // we already have some result
        StrToStr (Dest, " and ");
    if (n >= 20)
      {
        StrToStr (Dest, (char *) TensWords [n / 10]);
        n = n % 10;
        if (n)
          {
            CharToStr (Dest, '-');
            StrToStr (Dest, (char *) OnesWords [n]);
          }
      }
    else if ((n) || (*Dest == Dest0))
      StrToStr (Dest, (char *) OnesWords [n]);
  }

char IntToStrRomanNumerals1 [] = "MCXI";
char IntToStrRomanNumerals5 [] = "-DLV";

void IntToStrRomanNumerals (char **Dest, int n)
  {
    int Order;
    int Index;
    //
    CharToStrRepeat (Dest, 'M', n / 1000);
    n = n % 1000;
    Order = 100;
    Index = 1;
    while (n)
      {
        if (n / Order <= 3)
          CharToStrRepeat (Dest, IntToStrRomanNumerals1 [Index], n / Order);
        else if (n / Order <= 5)
          {
            CharToStrRepeat (Dest, IntToStrRomanNumerals1 [Index], 5 - n / Order);
            CharToStr (Dest, IntToStrRomanNumerals5 [Index]);
          }
        else if (n / Order <= 8)
          {
            CharToStr (Dest, IntToStrRomanNumerals5 [Index]);
            CharToStrRepeat (Dest, IntToStrRomanNumerals1 [Index], n / Order - 5);
          }
        else   // 9
          {
            CharToStrRepeat (Dest, IntToStrRomanNumerals1 [Index], 10 - n / Order);
            CharToStr (Dest, IntToStrRomanNumerals1 [Index - 1]);
          }
        n = n % Order;
        Order = Order / 10;
        Index++;
      }
  }


////////////////////////////////////////////////////////////////////////////
//
// IP ADDRESS
//

#ifdef _Windows

bool GetIP (char **Result, bool MultiLine)
  {
    /*  not supported Win10P
    //PHOSTENT HEnt;
    WSADATA WSAData;
    char name [255];
    PHOSTENT hostinfo;
    struct in_addr *InAddr;
    unsigned long IP;
    int x;
    bool OK;
    //
    OK = false;
    if (WSAStartup (0x0101, &WSAData) == 0)
      {
        if (gethostname (name, sizeof (name)) == 0)
          {
            hostinfo = gethostbyname (name);
            if (hostinfo)
              {
                InAddr = (struct in_addr *) *hostinfo->h_addr_list;
                IP = InAddr->S_un.S_addr;
                x = 0;
                while (true)
                  {
                    IntToStr (Result, IP & 0xFF);
                    IP >>= 8;
                    if (++x >= 4)
                      break;
                    StrToStr (Result, ".");
                  }
                OK = true;
              }
          }
        WSACleanup ();
      }
    return OK;
    */
    return false;
  }

#else   // Linux

#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>

#define IncludeName 0x01
#define IncludeIP 0x02
#define IncludeMAC 0x04
#define IncludeLF 0x80

bool StrGetMAC (char **Result, char *Name)
  {
    int fd;
    struct ifreq ifr;
    int i;
    int Res;
    //
    fd = socket (AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    //strncpy (ifr.ifr_name, "eth0", IFNAMSIZ-1);
    StrCopy (ifr.ifr_name, Name);
    Res = ioctl (fd, SIOCGIFHWADDR, &ifr);
    close (fd);
    i = 0;
    while (true)
      {
        IntToHex (Result, (byte) ifr.ifr_hwaddr.sa_data [i], 2 | IntToLengthZeros);
        i++;
        if (i >= 6)
          break;
        CharToStr (Result, '.');
      }
    return Res == 0;
  }

bool StrGetIPMAC (char **Result, byte Include)
  {
    struct ifaddrs *addrs, *addr;
    struct sockaddr_in *sa;
    struct in_addr sia;
    byte *b;
    int i;
    bool OK;
    bool First;
    //
    OK = false;
    First = true;
    if (getifaddrs (&addrs) == 0)
      {
        addr = addrs;
        // Step thru the list of interfaces
        while (addr)
          {
            if (addr->ifa_addr)
              if (addr->ifa_addr->sa_family == AF_INET)
                {
                  OK = true;
                  if (!First)
                    if (Include & IncludeLF)
                      CharToStr (Result, '\n');
                    else
                      CharToStr (Result, ' ');
                  First = false;
                  if (Include & IncludeName)
                    {
                      StrToStr (Result, addr->ifa_name);
                      CharToStr (Result, ':');
                    }
                  if (Include & IncludeIP)
                    {
                      sa = (struct sockaddr_in *) addr->ifa_addr;
                      sia = sa->sin_addr;
                      b = (byte *) &sia;
                      i = 0;
                      while (true)
                        {
                          IntToStr (Result, *b++);
                          i++;
                          if (i == 4)
                            break;
                          CharToStr (Result, '.');
                        }
                      if (Include & IncludeMAC)
                        CharToStr (Result, ' ');
                    }
                  if (Include & IncludeMAC)
                    {
                      StrGetMAC (Result, addr->ifa_name);
                    }
                }
            addr = addr->ifa_next;
          }
        freeifaddrs (addrs);
      }
    return OK;
  }

#endif

#endif   // _Lib
