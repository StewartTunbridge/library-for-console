//////////////////////////////////////////////////////////////////////////////
//
// INTERACTIVELY SHOW A PAGE OF GENERIC DATA
// =========================================
//
// Display a scrollable page of arbitary data
// Caller must provide a line formatter for an element of the data


typedef void _ShowPageItem (void *Data, int Index, int xOffset);
typedef bool _ShowPageSeek (void *Data, int Index, char *Target);
//typedef bool _ShowPageProcess (void *Data, int Index, byte Command);

int ShowPageHelpFGMask = ColBright;

void ShowHelpLine (char *HelpLine, int xOffset)
  {
    char c;
    int x;
    byte Col;
    const int Column = 14;
    //
    Col = ConsoleFG;
    if (*HelpLine == '_')
      {
        ConsoleColourFG (ConsoleFG |= ColUnderline);
        HelpLine++;
      }
    x = 0;
    while (*HelpLine)
      {
        c = *HelpLine++;
        if (c == '|')   // Hot key on/off
          if (*HelpLine == '|')   // double ||
            {
              PutChar (c);
              HelpLine++;
            }
          else
            ConsoleColourFG (ConsoleFG ^ ShowPageHelpFGMask);
        else if (c == tab)
          do
            {
              if (x >= xOffset)
                PutChar (' ');
              x++;
            }
          while (x % Column);
        else
          {
            if (x >= xOffset)
              PutChar (c);
            x++;
          }
      }
    ConsoleColourFG (Col);
  }

void ShowPageItemHelp (void *Data, int Index, int xOffset)
  {
    char *HelpLine;
    //
    HelpLine = ((char **) Data) [Index];
    ShowHelpLine (HelpLine, xOffset);
  }

void ShowPageItemHelpFile (void *Data, int Index, int xOffset)
  {
    char *HelpLine;
    //
    HelpLine = TextFileSeakln ((_TextFile *) Data, Index, false);
    ShowHelpLine (HelpLine, xOffset);
  }

bool ShowPageSeekHelp (void *Data, int Index, char *Target)
  {
    char *HelpLine;
    //
    HelpLine = ((char **) Data) [Index];
    return (int) StrPos_ (HelpLine, Target) >= 0;
  }

bool ShowPageSeekHelpFile (void *Data, int Index, char *Target)
  {
    char *HelpLine;
    //
    HelpLine = TextFileSeakln ((_TextFile *) Data, Index, false);
    return (int) StrPos_ (HelpLine, Target) >= 0;
  }


//////////////////////////////////////////////////////////////////////////////
//

#define Next(Sel,Size) {if (++Sel >= Size) Sel = 0;}

int ShowGenericPage (int Head, int Foot, _ShowPageItem *SPI, void *Data, int Size, int ColFG, int ColBG, int ColBGSel, int *ySel, _ShowPageSeek *SPS) //, _ShowPageProcess *SPP)
  {
    int xOffset, yOffset;
    int xOffset_, yOffset_;
    int Sel, Sel_;
    int y;   // ??vertical position within the data window
    int c, cprev;
    bool Redraw;
    char Seek [32];
    //
    GetKeyMacro = NULL;
    //ConsoleTab = Column;
    Sel = xOffset = yOffset = 0;
    xOffset_ = yOffset_ = 0;
    if (ySel)
      Sel = *ySel;
    Sel_ = -1;
    Seek [0] = 0;
    Redraw = true;
    c = 0;
    while (true)
      {
        if ((xOffset != xOffset_) || (yOffset != yOffset_))
          Redraw = true;
        // Draw page
        if (Redraw)
          {
            if (!GetKeyBuffered ())
              {
                for (y = 0; y < ConsoleSizeY - Head - Foot; y++)
                  {
                    if ((y == Sel - yOffset) && ySel)
                      ConsoleLine (y + Head, ColFG, ColBGSel);
                    else
                      ConsoleLine (y + Head, ColFG, ColBG);
                    if (y + yOffset < Size)
                      SPI (Data, y + yOffset, xOffset);
                  }
                Redraw = false;
              }
          }
        else if (Sel != Sel_)
          {
            if ((Sel_ >= 0) && (Sel_ < Size))
              {
                ConsoleLine (Sel_ - yOffset + Head, ColFG, ColBG);
                if (Sel_ < Size)
                  SPI (Data, Sel_, xOffset);
              }
            if ((Sel >= 0) && (Sel < Size))
              {
                ConsoleLine (Sel - yOffset + Head, ColFG, ColBGSel);
                SPI (Data, Sel, xOffset);
              }
          }
        y = ConsoleSizeY - Head - Foot;
        DrawScrollBar (Head, Head + y - 1,
                       yOffset, yOffset + y, Size,
                       ColFG, ColBG);
        // process user input
        //ConsoleCursor (0, y1);
        Sel_ = Sel;
        xOffset_ = xOffset;
        yOffset_ = yOffset;
        cprev = c;
        c = GetKeyWait (false);
        if (c == GetKeyWaitResizeOccured)
          return c;
        else if (c >= 0)
          /*if (SPP && SPP (Data, Sel, c))
            {
              ConsoleLine (Sel - yOffset + Head, ColFG, ColBGSel);
              SPI (Data, Sel, xOffset);
            }
          else*/
          if ((c > ' ') && (c < 0x80))
            {
              // Update Seek
              if (cprev <= ' ' || cprev >= 0x80)
                Seek [0] = 0;
              if (cprev == '=' && c != '=')
                Seek [0] = 0;
              if (c == '=')
                {
                  if (Seek [0])
                    Next (Sel, Size);
                }
              else
                if (StrLength (Seek) + 1 < sizeof (Seek))
                  StrAppend (Seek, c);
              // Find GenericPageSeek in Data
              while (true)
                {
                  if (SPS)
                    if (SPS (Data, Sel, Seek))
                      break;
                  Next (Sel, Size)
                  if (Sel == Sel_)
                    {
                      ConsoleBeep ();
                      Seek [0] = 0;
                      break;
                    }
                }
            }
          else
            switch (c)
              {
                case KeyHome:
                  Sel = 0;
                  xOffset = 0;
                  break;
                case KeyEnd:
                  Sel = Size - 1;
                  xOffset = 0;
                  break;
                case KeyUp:
                  Sel--;
                  break;
                case KeyDown:
                  Sel++;
                  break;
                case KeyPageUp:
                  Sel -= ConsoleSizeY - Head - Foot;
                  break;
                case KeyPageDown:
                  Sel += ConsoleSizeY - Head - Foot;
                  break;
                case KeyLeft:
                  if (xOffset > 0)
                    xOffset -= 8; //Column / 2;
                  break;
                case KeyRight:
                  xOffset += 8; //Column / 2;
                  break;
                default:
                  if (ySel)
                    *ySel = Sel;
                  return c;
              }
        // Check ranges and offsets
        if (Sel < 0)
          Sel = 0;
        else if (Sel >= Size)
          Sel = Size - 1;
        if (ySel == NULL)
          yOffset = Sel;
        while ((Sel - yOffset < 0) && (yOffset > 0))
          yOffset--;
        while (Sel - yOffset >= ConsoleSizeY - Head - Foot)
          yOffset++;
      }
  }


//////////////////////////////////////////////////////////////////////////////
//

char ShowHelpPageFile (char *Filename, int Head, int Foot, int ColFG, int ColBG)
  {
    _TextFile File;
    char c;
    //
    c = esc;
    if (TextFileOpen (&File, Filename, foRead))
      {
        while (TextFileReadln (&File, false))   // Find Size
          ;
        c = ShowGenericPage (Head, Foot, ShowPageItemHelpFile, &File, File.Line, ColFG, ColBG, ColBG ^ ColBright, NULL, ShowPageSeekHelpFile);
        TextFileClose (&File);
      }
    return c;
  }
