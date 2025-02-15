////////////////////////////////////////////////////////////////////////////
//
// ANNOUNCEMENT
//

void About (const char *Name, const char *Revision, const char *Description)
  {
    int i;
    //
    PutNewLine ();
    for (i = 0; i <= 6; i++)
      {
        //ConsoleClearEOL ();
        switch (i)
          {
            case 1:  PutString (Name);
                     if (Revision)
                       {
                         PutString (" [");
                         PutString (Revision);
                         PutString (" - ");
                         PutString (__DATE__);
                         PutChar (']');
                       }
                     if (Description)
                       {
                         PutString (": ");
                         PutString (Description);
                       }
                     break;
            case 3:  PutString ("(C)opyright 2013 2019 Pi Micros");
                     break;
            case 4:  PutString ("Freely use and distribute UNMODIFIED");
                     break;
            case 5:  PutString ("http://pimicros.com.au");
                     break;
            default: PutCharN ('-', ConsoleSizeX);
                     break;
          }
        PutNewLine ();
      }
  }


            //case 6: if (Wait)
            //          PutString ("<press a key>");
            //        break;
