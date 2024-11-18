////////////////////////////////////////////////////////////////////////////
//
// EDIT FIELDS
// -----------
// Author: Stewart Tunbridge, Pi Micros
// Email:  stewarttunbridge@gmail.com
//
// Edits a set of parameters
// Acceps up/down/.. to move between the fields
// Specify a list of FieldNames and a function FieldEdit(item)
//

typedef int _FieldEdit (int Field);   // Return unprocessed key OR -1 for error

bool EditFields (const char **FieldNames, _FieldEdit *FieldEdit, byte ColEdit)
  {
    int ColFG, ColBG;
    int cx, cy;
    int y;
    int c, cPrev;
    //
    ColFG = ConsoleFG;
    ColBG = ConsoleBG;
    cx = ConsoleX;
    cy = ConsoleY;
    y = 0;
    c = KeyDown;
    while (true)
      {
        ConsoleCursor (cx, cy);
        ConsoleClearEOL ();
        PutChar ('[');
        PutInt (y + 1, 0);
        PutChar (']');
        //PutChar ('/');
        //PutInt (FieldCount, 0);
        //PutString (": ");
        PutChar (' ');
        PutStringHighlight ((char *) FieldNames [y], ColEdit);
        PutString (": ");
        ConsoleColourFG (ColEdit);
        cPrev = c;
        c = FieldEdit (y);
        if (c < 0)
          c = cPrev;
        ConsoleColourFG (ColFG);
        ConsoleColourBG (ColBG);
        switch (c)
          {
            case 0:
              break;
            case KeyUp:
            case KeyPageUp:
              if (y)
                y--;
              else
                while (FieldNames [y + 1])
                  y++;
              break;
            case KeyDown:
            case KeyPageDown:
            case tab:
              if (FieldNames [y + 1])
                y++;
              else
                y = 0;
              break;
            case KeyEnter:
              return true;
            case esc:
              return false;
            default:
              ConsoleBeep ();
          }
      }
  }

