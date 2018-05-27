/* crc.c -- Fast CRC table construction and rolling CRC hash calculation
2009-11-23 : Igor Pavlov : Public domain
2013-03-27 : Bulat.Ziganshin@gmail.com : Public domain
2018-05-26 : Bart Massey <bart.massey@gmail.com> : Public domain */
 
#include <stdio.h>
 
#define kCrcPoly     0xEDB88320
#ifndef CRC_INIT_VAL
#define CRC_INIT_VAL 0 /* 0xFFFFFFFF for zip/rar/7-zip quasi-CRC */
#endif

// For rolling CRC hash demo
#define WINSIZE      100
#define TESTSIZE     200
 
// Fast CRC table construction algorithm
void FastTableBuild (unsigned CRCTable[256], unsigned seed)
{
  unsigned i, j, r;
 
  CRCTable[0]   = 0;
  CRCTable[128] = r = seed;
  for (i=64; i; i/=2)
    CRCTable[i] = r = (r >> 1) ^ (kCrcPoly & ~((r & 1) - 1));
 
  for (i=2; i<256; i*=2)
    for (j=1; j<i; j++)
      CRCTable[i+j] = CRCTable[i] ^ CRCTable[j];
}
 
 
#define init_CRC()                  CRC_INIT_VAL
#define update_CRC(crc,CRCTable,c)  (CRCTable[((crc)^(c)) & 0xFF] ^ ((crc)>>8))
#define finish_CRC(crc)             ((crc) ^ CRC_INIT_VAL)
 
unsigned calcCRC (unsigned char *buffer, unsigned len, unsigned CRCTable[256])
{
  unsigned crc = init_CRC(), i;
  for (i=0; i<len; i++)
    crc = update_CRC(crc,CRCTable,buffer[i]);
  return finish_CRC(crc);
}
 
 
void BuildRollingCRCTable (const unsigned CRCTable[256], unsigned RollingCRCTable[256]) 
{ 
    unsigned i,c,x,y; 
    for(c=0;c<256;c++) 
    { 
        x = init_CRC(); 
        y = init_CRC(); 
        x = update_CRC(x,CRCTable,c);
        y = update_CRC(y,CRCTable,0);
        for(i=0;i<WINSIZE-1;i++) 
        { 
            x = update_CRC(x,CRCTable,0);
            y = update_CRC(y,CRCTable,0);
        } 
        x = update_CRC(x,CRCTable,0);
        RollingCRCTable[c] = x ^ y;
    }
}

int main()
{
  unsigned i, j, r, CRCTab[256],
      FastCRCTab[256], RollingCRCTab[256], SlowRollingCRCTab[256], crc1, crc2;
 
  // Fast CRC table construction
  FastTableBuild (FastCRCTab, kCrcPoly);
 
  // Classic CRC table construction algorithm
  for (i=0; i<256; i++)
  {
    r = i;
    for (j = 0; j < 8; j++)
      r = (r >> 1) ^ (kCrcPoly & ~((r & 1) - 1));
    CRCTab[i] = r;
    if (FastCRCTab[i] != CRCTab[i])   // unit testing :)
      printf("c-crc: %02x %08x %08x\n", i, r, (r & 1) ? (r>>1)^kCrcPoly : (r>>1));
  }
 
 
#if CRC_INIT_VAL == 0
  // Rolling CRC table construction
  for (i=0; i<256; i++)
  {
    unsigned crc = init_CRC();
    crc = update_CRC(crc,CRCTab,i);
    for (j=0; j<WINSIZE; j++)
      crc = update_CRC(crc,CRCTab,0);
    RollingCRCTab[i] = finish_CRC(crc);
//    printf("r-crc: %02x %08x %08x\n",i,r, (r & 1) ? (r>>1)^kCrcPoly : (r>>1));
  }

  // Check slow rolling CRC build.
  BuildRollingCRCTable(FastCRCTab, SlowRollingCRCTab);
  for (i=0; i<256; i++)
    if (RollingCRCTab[i] != SlowRollingCRCTab[i])   // unit testing :)
      printf("sr-crc: *%02x %08x %08x\n",i,RollingCRCTab[i],SlowRollingCRCTab[i]);

  // Fast table construction for rolling CRC
  FastTableBuild (FastCRCTab, RollingCRCTab[128]);
  for (i=0; i<256; i++)
    if (FastCRCTab[i] != RollingCRCTab[i])   // unit testing :)
      printf("fr-crc: %02x %08x %08x\n",i,FastCRCTab[i],RollingCRCTab[i]);

#else
  BuildRollingCRCTable(FastCRCTab, RollingCRCTab);
#endif
 
  // Example of rolling CRC calculation and unit test simultaneously
  unsigned char buffer[WINSIZE+TESTSIZE];
  for (i=0; i<WINSIZE+TESTSIZE; i++)
    buffer[i] = 11 + i*31 + i/17;   // random :)
 
  // Let's calc CRC(buffer+TESTSIZE,WINSIZE) in two ways
  crc1 = calcCRC(buffer+TESTSIZE,WINSIZE,CRCTab);
  crc2 = calcCRC(buffer,WINSIZE,CRCTab);
  crc2 = finish_CRC(crc2);
  for (i=0; i<TESTSIZE; i++)
  {
    crc2 = update_CRC(crc2,CRCTab,buffer[WINSIZE+i]) ^
      RollingCRCTab[buffer[i]];
  }
  crc2 = finish_CRC(crc2);
  printf("roll: %08x and %08x %s\n", crc1, crc2, crc1==crc2? "are equal":"ARE NOT EQUAL!");
  return 0;
}
