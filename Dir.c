////////////////////////////////////////////////////////////////////////////
//
// DIRECTORY
// =========
//
// Author: Stewart Tunbridge, Pi Micros
// Email:  stewarttunbridge@gmail.com
// Copyright (c) 2000-2024 Stewart Tunbridge, Pi Micros
//
// 24 Mar 2017 Sort (sPath): Fix bug
//  8 Sep 2017 DirEntry: Add SymLinkTarget and fill in ReadDirSearch()
//              Tidy functions re Item path
//  1 May 2018 Add MakePath, DirectoryExists, PathDelimiter
//  3 Jun 2018 ReadDirSearch: Add Containing
//  7 Jul 2018 Separate DirEntryFromFilename () from ReadDirSearch ()

#include <dirent.h>
#include <sys/stat.h>
#ifdef _Windows
  #include <dir.h>
#endif

typedef struct DirEntry
  {
    struct DirEntry *Next;
    char *Path;
    char *Name;
    char *SymLinkTarget;
    bool Directory;
    bool SymLink;
    //off_t Size;
    longint Count;
    longint Size;
    time_t DateTime;
    mode_t Attrib;
    //unsigned int Position;
    #ifndef _Windows
    uid_t UID;
    gid_t GID;
    #endif
    //
    bool Tagged;
  } _DirEntry;

typedef enum {sName, sExt, sPath, sAttr, sOwnerGroup, sDateTime, sCount, sSize, sZZZZ} _SortMode;

//bool ReadDirAbort = false;
//bool ReadDirDebug = false;
//bool SortDirDebug = false;

//const int MaxPath = 1024;
#define MaxPath 1024

char *GetCurrentWorkingDirectory (void)   // Caller must free Result
  {
    char *p1, *p2;
    //
    p2 = NULL;
    if (chdir (".") == 0)   // Try to be in a valid place (####)
      {
        p1 = (char *) malloc (MaxPath + 1);
        getcwd (p1, MaxPath + 1);
        #ifndef _Windows
        if (p1 [0] == PathDelimiter)   // Valid path, not "(unreachable)..."
        #endif // _Windows
          StrAssign (&p2, p1);
        free (p1);
      }
    return p2;
  }

char *GetItemPathFrom (char *Path, _DirEntry *Item)   // Caller must free Result
  {
    char *Res, *r;
    //
    Res = (char *) malloc (StrLength (Path) + StrLength (Item->Name) + 2);
    r = Res;
    if (Path && Path [0])
      {
        StrToStr (&r, Path);
        if (*(r - 1) != PathDelimiter)
          CharToStr (&r, PathDelimiter);
      }
    StrToStr (&r, Item->Name);
    *r = 0;
    //strcpy (Res, Path);
    //StrAppend (Res, PathDelimiter);
    //strcat (Res, Item->Name);
    return Res;
  }

char *GetItemPath (_DirEntry *Item)   // Caller must free Result
  {
    return GetItemPathFrom (Item->Path, Item);
  }

#ifdef _Windows
  #define lstat stat
#endif

time_t FileDateTimeRead (int File, bool *SymLink)
  {
    struct stat st;
    //
    if (fstat (File, &st) == 0)
      {
        #ifndef _Windows
        if (SymLink)
          *SymLink = S_ISLNK (st.st_mode) != 0;
        #endif
        return st.st_mtime;
      }
    return -1;
  }

bool FileDateTimeWrite (char *Filename, time_t DateTime)
  {
    #ifdef _Windows
    HANDLE f;
    longint DateTime_;
    struct _FILETIME ft;
    //
    // Convert Linux time to magic windows time
    DateTime_ = DateTime;
    // Calculate Windows time. Offset 369 years and resolution 100nS
    //DateTime_ = (DateTime_ + 11644473600L) * 10000000L;
    DateTime_ = DateTime_ + 11644473600L;  // 134774 * 24 * 60 * 60
    DateTime_ = DateTime_ + 3600L;   // fudge ######
    DateTime_ = DateTime_ * 10000000L;  // scale 1 second to 100nS
    f = CreateFile (Filename, FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    ft.dwLowDateTime = (DWORD) DateTime_;
    ft.dwHighDateTime = DateTime_ >> 32;
    SetFileTime (f, &ft, &ft, &ft);
    CloseHandle (f);
    #else
    struct utimbuf utb;
    //
    utb.actime = DateTime;
    utb.modtime = DateTime;
    if (utime (Filename, &utb) != 0)   // fail
      return false;
    #endif // _Windows
    return true;
  }

bool DirEntryFromFilename (char *Filename, _DirEntry *Item)
  {
    struct stat st;
    char *lnk;
    int lnksz;
    //
    if (lstat (Filename, &st) == 0)
      {
        //memset (Item, 0, sizeof (_DirEntry));
        free (Item->Path);
        Item->Path = GetCurrentWorkingDirectory ();
        StrAssign (&Item->Name, Filename);
        Item->Directory = S_ISDIR (st.st_mode) != 0;
        //Item->Position = -1;
        #ifdef _Windows
        Item->SymLink = false;
        if (Item->Directory)
          Item->Size = st.st_size;
        else
          {
            DWORD szlo, szhi;   // 64 bit file sizes
            HANDLE f = CreateFile (Item->Name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (f == INVALID_HANDLE_VALUE)
              Item->Size = st.st_size;   // Use 32 bit size if we can't open the file
            else
              {
                szlo = GetFileSize (f, &szhi);
                CloseHandle (f);
                Item->Size = ((longint) szhi << (longint) 32) | szlo;
              }
          }
        #else
        Item->SymLink = S_ISLNK (st.st_mode) != 0;
        Item->Size = st.st_size;
        Item->UID = st.st_uid;
        Item->GID = st.st_gid;
        if (Item->SymLink)   // This is a symbolic link
          {
            lnk = (char *) malloc (Item->Size + 1);
            lnksz = readlink (Item->Name, lnk, Item->Size + 1);   // read link target
            if (lnksz == Item->Size)   // correct length of target string
              {
                lnk [lnksz] = 0;   // terminate string
                Item->SymLinkTarget = lnk;
              }
            else
              free (lnk);
          }
        #endif
        Item->DateTime = st.st_mtime;
        Item->Attrib = st.st_mode;   // Keep for permissions etc
        Item->Tagged = false;
        return true;
      }
    return false;
  }

void FreeDirItemContents (_DirEntry *Item)
  {
    free (Item->Path);
    Item->Path = NULL;
    free (Item->Name);
    Item->Name = NULL;
    free (Item->SymLinkTarget);
    Item->SymLinkTarget = NULL;
  }

void FreeDir (_DirEntry *Dir)
  {
    _DirEntry *Dir_;
    //
    while (Dir)
      {
        Dir_ = Dir->Next;
        FreeDirItemContents (Dir);
        free (Dir);
        Dir = Dir_;
      }
  }


//////////////////////////////////////////////////////////////////////////////////
//
// Read Directory into linked list of _DirEntry **List
//   List: Pointer to Address of first Item
//   Recurse: Read all sub directories

#define ReadDirInList 0x01
#define ReadDirInStats 0x02

typedef byte _ReadDirCallback (_DirEntry *Item, int Depth);

void ReadDir (_DirEntry **List, bool Recurse, _ReadDirCallback CallBack)
  {
    _DirEntry *New;
    struct dirent *de;
    DIR *Dir;
    byte Res;
    static int Depth;
    static bool Abort;
    static int TotalCount;
    static longint TotalSize;
    int TotalCount_;
    longint TotalSize_;
    //
    if (Depth == 0)
      Abort = false;
    Depth++;
    New = NULL;
    Dir = opendir (".");
    if (Dir != NULL)
      {
        while (true)
          {
            if (GetKey () == esc)
              Abort = true;
            if (Abort)
              break;
            de = readdir (Dir);
            if (de == NULL)
              break;
            if (StrCompare (de->d_name, ".") && StrCompare (de->d_name, ".."))
              {
                if (New == NULL)
                  {
                    New = (_DirEntry *) malloc (sizeof (_DirEntry));
                    MemSet (New, 0, sizeof (_DirEntry));
                  }
                if (DirEntryFromFilename (de->d_name, New))
                  {
                    if (New->Directory && Recurse)
                      {
                        if (chdir (New->Name) == 0)
                          {
                            TotalCount_ = TotalCount;
                            TotalSize_ = TotalSize;
                            ReadDir (List, Recurse, CallBack);
                            New->Count = TotalCount - TotalCount_;
                            New->Size = TotalSize - TotalSize_;
                            chdir ("..");
                          }
                      }
                    Res = ReadDirInList | ReadDirInStats;
                    if (CallBack)
                      Res = CallBack (New, Depth);  // Is this wanted?
                    if (Res & ReadDirInStats)    // Do stats
                      if (S_ISREG (New->Attrib))
                        {
                          TotalCount++;
                          TotalSize += New->Size;
                        }
                    if (Res & ReadDirInList)   // Add to top of list
                      {
                        New->Next = *List;
                        *List = New;
                        New = NULL;
                      }
                    else   // not wanted
                      FreeDirItemContents (New);   // free stuff in New
                  }
              }
          }
        closedir (Dir);
      }
    if (New)
      free (New);
    Depth--;
  }

int GetDirLength (_DirEntry *Dir)
  {
    int Len;
    //
    Len = 0;
    while (Dir)
      {
        Dir = Dir->Next;
        Len++;
      }
    return Len;
  }

int DirSortCompareName (_DirEntry *A, _DirEntry *B)
  {
    int Result;
    char *S1, *S2;
    //
    if (A->Directory != B->Directory)
      return (B->Directory - A->Directory);
    Result = StrCompareCase (A->Name, B->Name, false);
    if (Result)
      return Result;
    // Same so far so compare full file path
    S1 = GetItemPath (A);
    S2 = GetItemPath (B);
    Result = StrCompareCase (S1, S2, false);
    free (S1);
    free (S2);
    return Result;
  }

int DirSortCompareExt (_DirEntry *A, _DirEntry *B)
  {
    int Result;
    char *S1, *S2;
    //
    if (A->Directory != B->Directory)
      return (B->Directory - A->Directory);
    S1 = StrGetFileExtension (A->Name);
    S2 = StrGetFileExtension (B->Name);
    Result = StrCompareCase (S1, S2, false);
    if (Result)
      return Result;
    return DirSortCompareName (A, B);
  }

int DirSortComparePath (_DirEntry *A, _DirEntry *B)
  {
    int Result;
    //
    Result = StrCompareCase (A->Path, B->Path, false);
    if (Result)
      return Result;
    return DirSortCompareName (A, B);
  }

int DirSortCompareDateTime (_DirEntry *A, _DirEntry *B)
  {
    if (B->DateTime > A->DateTime)   // Newest items first
      return +1;
    if (B->DateTime < A->DateTime)
      return -1;
    return DirSortCompareName (A, B);
  }

int DirSortCompareCount (_DirEntry *A, _DirEntry *B)
  {
    //if (A->Directory != B->Directory)
    //  return (B->Directory - A->Directory);
    if (A->Count > B->Count)
      return +1;
    if (A->Count < B->Count)
      return -1;
    return DirSortCompareName (A, B);
  }

int DirSortCompareSize (_DirEntry *A, _DirEntry *B)
  {
    //if (A->Directory != B->Directory)
    //  return (B->Directory - A->Directory);
    if (A->Size > B->Size)
      return +1;
    if (A->Size < B->Size)
      return -1;
    return DirSortCompareName (A, B);
  }

#ifndef _Windows
int DirSortCompareOwnerGroup (_DirEntry *A, _DirEntry *B)
  {
    int Result;
    //
    Result = (int) (A->UID - B->UID);
    if (Result == 0)
      Result = (int) (A->GID - B->GID);
    if (Result == 0)
      Result = DirSortCompareName (A, B);
    return Result;
  }

int DirSortCompareAttr (_DirEntry *A, _DirEntry *B)
  {
    int Result;
    //
    Result = (int) (A->Attrib - B->Attrib);
    if (Result == 0)
      Result = DirSortCompareName (A, B);
    return Result;
  }
#endif // _Windows

void SortDir (_DirEntry **Dir, _SortMode Mode)
  {
    switch (Mode)
      {
        case sName:
          *Dir = (_DirEntry *) SortList ((void **) *Dir, (_SortListCompare *) &DirSortCompareName);
          break;
        case sExt:
          *Dir = (_DirEntry *) SortList ((void **) *Dir, (_SortListCompare *) &DirSortCompareExt);
          break;
        case sPath:
          *Dir = (_DirEntry *) SortList ((void **) *Dir, (_SortListCompare *) &DirSortComparePath);
          break;
        case sDateTime:
          *Dir = (_DirEntry *) SortList ((void **) *Dir, (_SortListCompare *) &DirSortCompareDateTime);
          break;
        case sCount:
          *Dir = (_DirEntry *) SortList ((void **) *Dir, (_SortListCompare *) &DirSortCompareCount);
          break;
        case sSize:
          *Dir = (_DirEntry *) SortList ((void **) *Dir, (_SortListCompare *) &DirSortCompareSize);
          break;
        #ifndef _Windows
        case sAttr:
          *Dir = (_DirEntry *) SortList ((void **) *Dir, (_SortListCompare *) &DirSortCompareAttr);
          break;
        case sOwnerGroup:
          *Dir = (_DirEntry *) SortList ((void **) *Dir, (_SortListCompare *) &DirSortCompareOwnerGroup);
          break;
        #endif
      }
  }

#ifndef _Windows
  #include <sys/statvfs.h>
#endif // _Windows

bool DiskSpace (char *Path, longint *Capacity, longint *Free)
  {
#ifdef _Windows
    DWORD SectorsPerCluster;
    DWORD BytesPerSector;
    DWORD FreeClusters;
    DWORD Clusters;
    if (GetDiskFreeSpace (Path, &SectorsPerCluster, &BytesPerSector, &FreeClusters, &Clusters))
      {
        *Capacity = (longint) SectorsPerCluster * (longint) BytesPerSector * (longint) Clusters;
        *Free = (longint) SectorsPerCluster * (longint) BytesPerSector * (longint) FreeClusters;
        return true;
      }
#else
    struct statvfs DriveStats;
    //
    if (statvfs (Path, &DriveStats) == 0)
      {
        *Capacity = (longint) DriveStats.f_blocks * (longint) DriveStats.f_bsize;
        *Free = (longint) DriveStats.f_bfree * (longint) DriveStats.f_bsize;
        return true;
      }
#endif // _Windows
    return false;
  }

bool DirectoryExists (char *Path)
  {
    DIR *Dir;
    //
    Dir = opendir (Path);
    if (Dir)   // Directory exists
      {
        closedir (Dir);
        return true;
      }
    return false;
  }

bool MakeDirectory (char *Path)
  {
    #ifdef _Windows
    return (mkdir (Path) == 0);
    #else
    return (mkdir (Path, S_IRWXU | S_IRWXG | S_IRWXO) == 0);
    #endif
  }

bool MakePath (char *Path)
  {
    int l, i, j;
    //
    i = 1;
    l = StrLength (Path);
    if (l)
      while (true)
        {
          j = StrPosChFrom (Path, i, PathDelimiter);   // break Path into sub strings
          if (j < 0)
            j = l;
          Path [j] = 0;   // terminate sub string
          if (!DirectoryExists (Path))
            if (!MakeDirectory (Path))
              break;
          if (j < l)
            Path [j] = PathDelimiter;
          else
            return true;
          i = j + 1;
        }
    return false;
  }

// Ensure Path exists
bool MakeFilePath (char *FilePath)
  {
    int i;
    //
    i = StrPosLastCh (FilePath, PathDelimiter);
    if (i >= 0)
      {
        FilePath [i] = 0;
        MakePath (FilePath);
        FilePath [i] = PathDelimiter;
      }
  }

char *StrPathHome (char **St, char *Filename)
  {
#ifdef _Windows
    StrToStr (St, getenv ("HOMEPATH"));
    //SHGetFolderPath (NULL, CSIDL_PROFILE, NULL, 0, path);
    //StrToStr (St, path);
#else
    StrToStr (St, getenv ("HOME"));
#endif
    CharToStr (St, PathDelimiter);
    if (Filename)
      StrToStr (St, Filename);
    **St = 0;
  }

char *StrPathConfig (char **St, char *Filename, char *Appname)
  {
    #ifndef _Windows
    if (Appname)
      {
        StrPathHome (St, NULL);
        StrToStr (St, ".config");
        CharToStr (St, PathDelimiter);
        StrToStr (St, Appname);
        CharToStr (St, PathDelimiter);
        StrToStr (St, Filename);
      }
    else
    #endif // _Windows
      StrPathHome (St, Filename);
    **St = 0;
  }

/*
// Create full pathname to file in $HOME directory
// caller must free
//
char *PathHome (char *Filename)
  {
    char *Path, *p;
    char *Res;
    //
    Path = malloc (MaxPath);
    p = Path;
    StrHomePath (&p);
    if (Filename)
      {
        CharToStr (&p, PathDelimiter);
        StrToStr (&p, Filename);
      }
    *p = 0;
    Res = NULL;
    StrAssign (&Res, Path);
    free (Path);
    return Res;
  }

char *PathConfig (char **St, char *Appname)
  {
    char *s;
    //
    s = *St;
    StrToStr (St, getenv ("HOME"));
    CharToStr (St, PathDelimiter);
    if (Appname)
      {
        StrToStr (St, ".config");
        CharToStr (St, PathDelimiter);
        StrToStr (St, Appname);
        CharToStr (St, PathDelimiter);
        **St = 0;
        MakePath (s);
      }
  }
*/

// Search $PATH for Filename:
// Returns full pathname
// Caller must free
//
#ifdef _Windows
  #define PathsDelimiter ';'
#else
  #define PathsDelimiter ':'
#endif // _Windows
//
char *FindFileInPath (char *Filename)
  {
    char *Res;
    char *path;
    char *FullPath;
    char *p;
    int a, b;
    //
    Res = NULL;
    FullPath = malloc (MaxPath);
    // Search along $PATH
#ifdef _Windows
    path = getenv ("PATH");
#else
    path = getenv ("PATH");
#endif
    if (path && path [0])
      {
        a = -1;
        while (true)
          {
            b = StrPosChFrom (path, a + 1, PathsDelimiter);
            if (b < 0)   // no more
              b = StrLength (path);
            p = FullPath;
            StrToStrN (&p, &path [a + 1], b - a - 1);
            CharToStr (&p, PathDelimiter);
            StrToStr (&p, Filename);
            *p = 0;
            if (FileExists (FullPath))
              {
                StrAssign (&Res, FullPath);
                break;
              }
            if (path [b] == 0)
              break;
            a = b;
          }
      }
    free (FullPath);
    return Res;
  }


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
void SortDir (_DirEntry **Dir, int Mode)
  {
    int Unsorted;
    _DirEntry **x, *b, *c, *d;
    int Compare;
    char *S1, *S2;
    int ConsoleX_;
    int Tick;
    int Size;
    //
    if (*Dir == NULL)
      return;
    ConsoleX_ = ConsoleX;
    Tick = ClockMS ();
    do
      {
        Size = 0;
        Unsorted = 0;
        x = Dir;
        while (true)
          {
            if ((*x)->Next == NULL)
              break;
            Size++;
            Compare = 0;
            switch (Mode)
              {
                case 0: // Name
                  if ((*x)->Directory != (*x)->Next->Directory)
                    Compare = ((*x)->Next->Directory - (*x)->Directory);
                  else
                    Compare = StrCmp ((*x)->Name, (*x)->Next->Name);
                  break;
                case 1: // Extension
                  if ((*x)->Directory != (*x)->Next->Directory)
                    Compare = ((*x)->Next->Directory - (*x)->Directory);
                  else
                    {
                      S1 = StrFindFileExtension ((*x)->Name);
                      S2 = StrFindFileExtension ((*x)->Next->Name);
                      Compare = StrCmp (S1, S2);
                      if (Compare == 0)
                        Compare = StrCmp ((*x)->Name, (*x)->Next->Name);
                    }
                  break;
                case 2: // Size
                  if ((*x)->Directory != (*x)->Next->Directory)
                    Compare = ((*x)->Next->Directory - (*x)->Directory);
                  else
                    Compare = (*x)->Size - (*x)->Next->Size;
                  break;
                case 3: // Date
                  Compare = (*x)->Next->Date - (*x)->Date;   // Newest items first
                  //Compare = (*x)->Date - (*x)->Next->Date;   // Newest items first
                  break;
                case 4: // Path
                  break;
              }
            if (Compare == 0)   // Same so far so compare full file path
              {
                S1 = GetFullPath (*x);
                S2 = GetFullPath ((*x)->Next);
                Compare = StrCmp (S1, S2);
                free (S1);
                free (S2);
              }
            if (Compare > 0)   // Out of order
              {
                Unsorted++;
                b = *x;
                c = b->Next;
                d = c->Next;
                *x = c;
                b->Next = d;
                c->Next = b;
              }
            x = (_DirEntry **) *x;
          }
        if (GetKey () == esc)
          break;
        if (SortDirDebug)
          if ((int) (ClockMS () - Tick) >= 1000)
            {
              ConsoleCursor (ConsoleX_, ConsoleY);
              //PutInt (ClockMS () - Tick, -8);//####
              PutInt (Unsorted, 9);
              PutChar ('/');
              PutInt (Size, 9);
              Tick = ClockMS ();
            }
      }
    while (Unsorted > 0);
  }
*/

/*
bool SortDir (_DirEntry **Dir, int Mode)
  {
    int Size, Step;
    bool Unsorted, Swap;
    _DirEntry *a, **x, **y, *b, *c, *d;
    int i;
    char *Ext1, *Ext2;
    //
    if (*Dir == NULL)
      return true;
    // Determine Size & Step
    Size = GetDirSize (*Dir);
    Step = 1;
    while (Step + Step <= Size)
      Step = Step << 1;
    // Sort in ever halving steps
    do
      {
        do
          {
            x = Dir;
            y = x;
            // Find Start of swap chunk
            i = Step;
            while (true)
              {
                if (i == 0)
                  break;
                if ((*y)->Next == NULL)
                  break;
                i--;
                y = (_DirEntry **) *y;
              }
            Unsorted = false;
            while (true)
              {
                if ((*y)->Next == NULL)
                  break;
                if ((*x)->Next == NULL)
                  break;
                if ((*x)->Directory != (*y)->Directory)
                  Swap = (*x)->Directory < (*y)->Directory;
                else
                  switch (Mode)
                    {
                      case 0: // Name
                        Swap = StrCmp ((*x)->Name, (*y)->Name) > 0;
                        break;
                      case 1: // Extension
                        Ext1 = StrFindFileExtension ((*x)->Name);
                        Ext2 = StrFindFileExtension ((*y)->Name);
                        i = StrCmp (Ext1, Ext2);
                        if (i == 0)
                          i = StrCmp ((*x)->Name, (*y)->Name);
                        Swap = i > 0;
                        break;
                      case 2: // Size
                        Swap = (*x)->Size > (*y)->Size;
                        break;
                      case 3: // Date
                        Swap = (*x)->Date > (*y)->Date;
                        break;
                      case 4: // Path
                        i = StrCmp ((*x)->Path, (*y)->Path);
                        if (i == 0)
                          i = StrCmp ((*x)->Name, (*y)->Name);
                        Swap = i > 0;
                        break;
                    }
                if (Swap)   // Out of order
                  {
                    Unsorted = true;
                    //aa = GetDirSize (*Dir);
                    //**x = *y;
                    //SwapBytes ((*x)->Path, (*y)->Path, sizeof (char *));
                    //SwapBytes ((*x)->Name, (*y)->Name, sizeof (char *));
                    StrSwap ((*x)->Path, (*y)->Path);
                    StrSwap ((*x)->Name, (*y)->Name);
                    SwapByte (&(*x)->Tagged, &(*y)->Tagged);
                    SwapByte (&(*x)->Directory, &(*y)->Directory);
                    SwapBytes (&(*x)->Size, &(*y)->Size, sizeof (int));
                    SwapBytes (&(*x)->Date, &(*y)->Date, sizeof (int));
                  }
                x = (_DirEntry **) *x;
                y = (_DirEntry **) *y;
              }
          }
        while (Unsorted);
        Step = Step >> 1;
      }
    while (Step > 0);
    return true;
  }
*/

/*
_DirEntry **ReadDir (_DirEntry **Tail)
  {
    _DirEntry *New;
    struct dirent *entry;
    struct stat st;
    DIR *Dir;
    //
    Dir = opendir (".");
    if (Dir != NULL)
      {
        while (true)
          {
            entry = readdir (Dir);
            if (entry == NULL)
              break;
            if (strcmp (entry->d_name, ".") && strcmp (entry->d_name, ".."))
              {
                New = (_DirEntry *) malloc (sizeof (_DirEntry));
                memset (New, 0, sizeof (_DirEntry));
                New->Name = (char *) malloc (strlen (entry->d_name) + 1);
                strcpy (New->Name, entry->d_name);
                if (stat (entry->d_name, &st) == 0)
                  {
                    New->Directory = (S_IFDIR & st.st_mode) != 0;
                    New->Size = st.st_size;
                    New->Date = st.st_mtime;
                  }
                *Tail = New;
                Tail = (_DirEntry **) New;
              }
          }
        closedir (Dir);
      }
    return Tail;
  }

                      New->Path = GetCurrentWorkingDirectory ();
                      StrAssign (&New->Name, entry->d_name);
                      New->Directory = S_ISDIR (st.st_mode) != 0;
                      New->Position = Pos;
                      #ifdef _Windows
                      New->SymLink = false;
                      if (New->Directory)
                        New->Size = st.st_size;
                      else
                        {
                          DWORD szlo, szhi;   // 64 bit file sizes
                          HANDLE f = CreateFile (New->Name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                          szlo = GetFileSize (f, &szhi);
                          CloseHandle (f);
                          New->Size = ((longint) szhi << (longint) 32) | szlo;
                        }
                      #else
                      New->SymLink = S_ISLNK (st.st_mode) != 0;
                      New->Size = st.st_size;
                      New->UID = st.st_uid;
                      New->GID = st.st_gid;
                      if (New->SymLink)   // This is a symbolic link
                        {
                          lnk = (char *) malloc (New->Size + 1);
                          lnksz = readlink (New->Name, lnk, New->Size + 1);   // read link target
                          if (lnksz == New->Size)   // correct length of target string
                            {
                              lnk [lnksz] = 0;   // terminate string
                              New->SymLinkTarget = lnk;
                            }
                          else
                            free (lnk);
                        }
                      #endif
                      New->DateTime = st.st_mtime;
                      New->Attrib = st.st_mode;   // Keep for permissions etc
                      New->Tagged = false;
*/

/*
void SortDir (_DirEntry **Dir, _SortMode Mode)
  {
    int Size;
    _DirEntry **DirTable;
    int Offset;
    int Unsorted;
    int i;
    _DirEntry *a;
    //
    int ConsoleX_;
    //int Tick;
    //
    SortMode = Mode;
    if (*Dir == NULL)
      return;
    ConsoleX_ = ConsoleX + 1;
    //Tick = ClockMS ();
    Size = GetDirSize (*Dir);
    DirTable = (_DirEntry **) malloc (sizeof (_DirEntry *) * Size);
    for (i = 0, a = (_DirEntry *) Dir; i < Size; i++, a = a->Next)
      DirTable [i] = a->Next;
    //Offset = 1;
    //while (Offset << 1 < Size)
    //  Offset = Offset << 1;
    Offset = Size / 2;
    while (Offset)
      {
        while (true)
          {
            Unsorted = 0;
            for (i = 0; i + Offset < Size; i++)
              if (DirSortCompare (DirTable [i], DirTable [i + Offset]) > 0)   // Out of order
                {
                  Unsorted++;   // note the problem
                  a = DirTable [i];
                  DirTable [i] = DirTable [i + Offset];
                  DirTable [i + Offset] = a;
                }
            if (Unsorted == 0)
              break;
          }
        if (GetKey () == esc)
          break;
        if (SortDirDebug)
          //if ((int) (ClockMS () - Tick) >= 1000)
            {
              ConsoleCursor (ConsoleX_, ConsoleY);
              PutInt (Offset, 0 | 0x40); //Unsorted, 9);
              PutChar ('/');
              PutInt (Size, 0 | 0x40);
              PutChar (' ');
              //Tick = ClockMS ();
            }
        Offset = Offset / 2;
      }
    a = (_DirEntry *) Dir;
    for (i = 0; i < Size; i++)
      {
        a->Next = DirTable [i];
        a = a->Next;
      }
    a->Next = NULL;
    free (DirTable);
  }
*/

/*int DirSortCompare (_DirEntry *A, _DirEntry *B) //, _SortMode Mode)
  {
    int Result;
    char *S1, *S2;
    //
    Result = 0;
    switch (SortMode)
      {
        case sName:
        case sInvalid:
          if (A->Directory != B->Directory)
            Result = (B->Directory - A->Directory);
          else
            Result = StrCmp (A->Name, B->Name);
          break;
        case sExt:
          if (A->Directory != B->Directory)
            Result = (B->Directory - A->Directory);
          else
            {
              S1 = StrFindFileExtension (A->Name);
              S2 = StrFindFileExtension (B->Name);
              Result = StrCmp (S1, S2);
              if (Result == 0)
                Result = StrCmp (A->Name, B->Name);
            }
          break;
        case sPath:
          Result = StrCmp (A->Path, B->Path);
          if (Result == 0)
            Result = StrCmp (A->Name, B->Name);
          break;
        case sDateTime:
          if (B->DateTime > A->DateTime)   // Newest items first
            Result = +1;
          else if (B->DateTime < A->DateTime)
            Result = -1;
          break;
        case sSize:
          if (A->Directory != B->Directory)
            Result = (B->Directory - A->Directory);
          else
            if (A->Size > B->Size)
              Result = +1;
            else if (A->Size < B->Size)
              Result = -1;
          break;
      }
    if (Result == 0)   // Same so far so compare full file path
      {
        S1 = GetItemPath (A);
        S2 = GetItemPath (B);
        Result = StrCmp (S1, S2);
        free (S1);
        free (S2);
      }
    return Result;
  }*/

