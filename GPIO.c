//////////////////////////////////////////////////////////////
//
// GPIO - Raspberry Pi 2 & 3
// -------------------------
// Author: Stewart Tunbridge, Pi Micros
// Email:  stewarttunbridge@gmail.com
// Copyright (c) 2000-2024 Stewart Tunbridge, Pi Micros
//
//
// 02 Sep 2016 - Read Channel Mode
// 
//////////////////////////////////////////////////////////////

#include "Lib.c"

// Access from ARM Running Linux
 
//#define BCM2708_PERI_BASE        0x20000000   // Raspberry Pi 1 
#define BCM2708_PERI_BASE  0x3F000000   // Raspberry Pi 2
#define GPIO_BASE          (BCM2708_PERI_BASE + 0x200000) // GPIO controller
 
// GPIO Control Registers

#define GPFSELn 0
#define GPSET 7
#define GPCLR 10
#define GPLEV 13
#define GPPUD 37
#define GPPUDCLK 38

 
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
 
#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)
 
int  mem_fd = -1;
void *gpio_map;
 
// GPIO Register access

volatile unsigned *gpio;
 
 
// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)

//#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
//#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
//#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))
// 
//#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
//#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0
// 
//#define GET_GPIO(g) (*(gpio+13)&(1<<g)) // 0 if LOW, (1<<g) if HIGH
// 
//#define GPIO_PULL *(gpio+37) // Pull up/pull down
//#define GPIO_PULLCLK0 *(gpio+38) // Pull up/pull down clock

// Set up a memory regions to access GPIO

void GPIOInit ()
  {
     // open /dev/mem
    if ((mem_fd = open ("/dev/mem", O_RDWR|O_SYNC) ) < 0) 
      {
        printf ("can't open /dev/mem \n");
        exit (-1);
      }
    // mmap GPIO
    gpio_map = mmap 
      (
        NULL,             //Any adddress in our space will do
        BLOCK_SIZE,       //Map length
        PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
        MAP_SHARED,       //Shared with other processes
        mem_fd,           //File to map
        GPIO_BASE         //Offset to GPIO peripheral
      );
    close (mem_fd); //No need to keep mem_fd open after mmap
    if (gpio_map < 0)
      {
        printf ("mmap error %d\n", (int) gpio_map);   //errno also set!
        exit (-1);
      }
    // Always use volatile pointer!
    gpio = (volatile unsigned *) gpio_map;
  }

void GPIOUninit ()
  {
    //if (mem_fd >= 0)
    //  close (mem_fd);
    //mem_fd = -1;
  }


typedef enum {mInput, mOutput, mAlt0, mAlt1, mAlt2, mAlt3, mAlt4, mAlt5} _Mode;

void GPIOMode (int Chan, _Mode Mode)
  {
    int Offset;
    int Mask;
    int Reg;
    //
    Offset = Chan / 10;
    Mask = Bit [(Chan % 10) * 3];
    Reg = gpio [GPFSELn + Offset];
    Reg &= ~(Mask * 7);   // Clear Function Select bits
    Reg |= Mode * Mask;   // Set new mode
    gpio [GPFSELn + Offset] = Reg;
  }

_Mode GPIOModeRead (int Chan)
  {
    int Offset;
    int Mask;
    int Reg;
    //
    Offset = Chan / 10;
    Mask = Bit [(Chan % 10) * 3];
    Reg = gpio [GPFSELn + Offset];
    return (Reg / Mask) & 0x07;
  }

typedef enum {pNone, pDown, pUp} _Pull;

void GPIOPull (int Chan, _Pull Pull)
  {
    gpio [GPPUD] = Pull;   // GPIO_PULL = Pull;
    usleep (5);
    gpio [GPPUDCLK] = Bit [Chan];   // GPIO_PULLCLK0 = (1 << Chan);
    usleep (5);
    gpio [GPPUD] = 0;   // GPIO_PULL = 0;
    //usleep (5);
    gpio [GPPUDCLK] = 0;   // GPIO_PULLCLK0 = 0;
    usleep (5);
  }

bool GPIORead (int Chan)
  {
    return (gpio [GPLEV] & Bit [Chan]) != 0;
  }

void GPIOWrite (int Chan, bool Value)
  {
    if (Value)
      gpio [GPSET] = Bit [Chan];
    else
      gpio [GPCLR] = Bit [Chan];
  }


//#define Test
//#define TestOut
#define TestIn

#ifdef Test

#define Chan 21

int main (int argc, char **argv)
  {
    bool New, Old;
    //
    GPIOInit ();
    #ifdef TestOut
    //GPIOMode (Chan, false);   //INP_GPIO (Chan);
    GPIOMode (Chan, true);  //OUT_GPIO (Chan);
    New = true;
    while (true)
      {
        GPIOWrite (Chan, New);   // _SET = 1 << Chan;
        New = !New;
        usleep (1000000);   // sleep 1 Sec
      }
    #endif
    #ifdef TestIn
    GPIOMode (Chan, false);
    GPIOPull (Chan, pUp);
    Old = -1;
    while (true)
      {
        New = GPIORead (Chan);
        if (New != Old)
          {
            Old = New;
            printf ("Input - %s\n", New ? "On" : "Off");
          }
        usleep (10000);   // sleep 10mS
      }
    #endif
  }

#endif
