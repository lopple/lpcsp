/*----------------------------------------------------------------------------/
/  LPCSP - Flash Programmer for LPC1000/2000  R0.02f
/  http://elm-chan.org/
/-----------------------------------------------------------------------------/
/ LPCSP is a flash programming software for NXP LPC1000/2000 family MCUs. It is
/ a Free Software opened under license policy of GNU General Public License.

   Copyright (C) 2011, ChaN, all right reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.

   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE. 
*/



#include <stdio.h>
#include <string.h>
#include <windows.h>


#define INIFILE "lpcsp.ini"
#define MESS(str)	fputs(str, stderr)
#define	LD_DWORD(ptr)		(DWORD)(((DWORD)*((BYTE*)(ptr)+3)<<24)|((DWORD)*((BYTE*)(ptr)+2)<<16)|((WORD)*((BYTE*)(ptr)+1)<<8)|*(BYTE*)(ptr))
#define	ST_DWORD(ptr,val)	*(BYTE*)(ptr)=(BYTE)(val); *((BYTE*)(ptr)+1)=(BYTE)((WORD)(val)>>8); *((BYTE*)(ptr)+2)=(BYTE)((DWORD)(val)>>16); *((BYTE*)(ptr)+3)=(BYTE)((DWORD)(val)>>24)


typedef struct {
	const char	*DeviceName;	/* Device name LPC<string> */
	DWORD		Sign;			/* Device signature */
	DWORD		Class;			/* Device type 0:2xxx, 1:17xx, 2:13xx, 3:12xx/11xx */
	DWORD		FlashSize;		/* User memory size in unit of byte */
	const DWORD	*SectMap;		/* Flash sector organization */
	DWORD		XferSize;		/* Data transfer size per a write transaction */
	DWORD		CRP;			/* CRP address */
} DEVICE;


const char *Usage =
	"LPCSP - LPC1000/2000 Serial Programming tool R0.02f (C)ChaN,2012\n"
	"\n"
	"Write flash memory:    <hex file> [<hex file>] ...\n"
	"Read flash memory:     -R\n"
	"Port number and speed: -P<n>[:<bps>]\n"
	"Oscillator frequency:  -F<n> (used for only LPC21xx/22xx)\n"
	"Signal polarity:       -C<flag> (see lpcsp.ini)\n"
	"Wait on exit:          -W<mode> (see lpcsp.ini)\n"
	"\n"
	"Supported Device:\n"
	"LPC11(02,10,11,12,13,14,15)\n"
	"LPC11C(12,14,22,24)\n"
	"LPC12(24,25,26,27)\n"
	"LPC13(11,13,42,43)\n"
	"LPC17(64,65,66,67,68,69,74,76,77,78,85,86,87,88)\n"
	"LPC21(03,09,19,29,14,24,94,31,32,34,36,38,41,42,44,46,48,94)\n"
	"LPC22(92,94)\n"
	"LPC23(64,65,66,67,68,77,78,87,88)\n"
	"LPC24(58,68,78)\n"
	;



/* Flash sector organizations */
const DWORD Map1[] = { 0, 0x1000, 0x2000, 0x3000, 0x4000, 0x5000, 0x6000, 0x7000, 0x8000, 0x10000, 0x18000, 0x20000, 0x28000, 0x30000, 0x38000, 0x40000, 0x48000, 0x50000, 0x58000, 0x60000, 0x68000, 0x70000, 0x78000, 0x79000, 0x7A000, 0x7B000, 0x7C000, 0x7D000, 0x7E000, 0x7F000, 0x80000 };
const DWORD Map2[] = { 0, 0x2000, 0x4000, 0x6000, 0x8000, 0xA000, 0xC000, 0xE000, 0x10000, 0x12000, 0x14000, 0x16000, 0x18000, 0x1A000, 0x1C000, 0x1E000, 0x20000 };
const DWORD Map3[] = { 0, 0x2000, 0x4000, 0x6000, 0x8000, 0xA000, 0xC000, 0xE000, 0x10000, 0x20000, 0x30000, 0x32000, 0x34000, 0x36000, 0x38000, 0x3A000, 0x3C000, 0x3E000, 0x40000 };
const DWORD Map4[] = { 0, 0x1000, 0x2000, 0x3000, 0x4000, 0x5000, 0x6000, 0x7000, 0x8000, 0x9000, 0xA000, 0xB000, 0xC000, 0xD000, 0xE000, 0xF000, 0x10000, 0x18000, 0x20000, 0x28000, 0x30000, 0x38000, 0x40000, 0x48000, 0x50000, 0x58000, 0x60000, 0x68000, 0x70000, 0x78000, 0x80000 };
const DWORD Map5[] = { 0, 0x1000, 0x2000, 0x3000, 0x4000, 0x5000, 0x6000, 0x7000, 0x8000, 0x9000, 0xA000, 0xB000, 0xC000, 0xD000, 0xE000, 0xF000, 0x10000, 0x11000, 0x12000, 0x13000, 0x14000, 0x15000, 0x16000, 0x17000, 0x18000, 0x19000, 0x1A000, 0x1B000, 0x1C000, 0x1D000, 0x1E000, 0x1F000, 0x20000 };

/* Read code for vector area needed to disable remap (indexed by Device->Class) */
const BYTE RdCode[3][60] = {
	{0x0B,0x4D, 0x01,0x20, 0x28,0x60, 0x0B,0x4D, 0x40,0x22, 0x00,0x23, 0x00,0x24, 0x18,0x78,
	 0x01,0x33, 0x24,0x18, 0x00,0xF0, 0x06,0xF8, 0x01,0x3A, 0xF8,0xD1, 0x20,0x1C, 0x00,0xF0,
	 0x01,0xF8, 0xFE,0xE7, 0x69,0x69, 0x20,0x26, 0x31,0x42, 0xFB,0xD0, 0x28,0x60, 0x70,0x47,
	 0x40,0xC0, 0x1F,0xE0, 0x00,0xC0, 0x00,0xE0, 0x00,0x00, 0x00,0x00},
	{0x0B,0x4D, 0x01,0x20, 0x28,0x60, 0x0B,0x4D, 0x0B,0x4A, 0x00,0x23, 0x00,0x24, 0x18,0x78,
	 0x01,0x33, 0x24,0x18, 0x00,0xF0, 0x06,0xF8, 0x01,0x3A, 0xF8,0xD1, 0x20,0x1C, 0x00,0xF0,
	 0x01,0xF8, 0xFE,0xE7, 0x69,0x69, 0x20,0x26, 0x31,0x42, 0xFB,0xD0, 0x28,0x60, 0x70,0x47,
	 0x40,0xC0, 0x0F,0x40, 0x00,0xC0, 0x00,0x40, 0x00,0x04, 0x00,0x00},
	{0x0B,0x4D, 0x02,0x20, 0x28,0x60, 0x0B,0x4D, 0x0B,0x4A, 0x00,0x23, 0x00,0x24, 0x18,0x78,
	 0x01,0x33, 0x24,0x18, 0x00,0xF0, 0x06,0xF8, 0x01,0x3A, 0xF8,0xD1, 0x20,0x1C, 0x00,0xF0,
	 0x01,0xF8, 0xFE,0xE7, 0x69,0x69, 0x20,0x26, 0x31,0x42, 0xFB,0xD0, 0x28,0x60, 0x70,0x47,
	 0x00,0x80, 0x04,0x40, 0x00,0x80, 0x00,0x40, 0x00,0x02, 0x00,0x00}
};

/* Working buffer address (indexed by Device->Class) */
const DWORD RamAddr[3] = {0x40000200, 0x10000200, 0x10000200};

/* Size of remapped vector area (indexed by Device->Class) */
const DWORD VectSize[3] = {64, 1024, 512};

/* Offset of application check sum (indexed by Device->Class) */
const DWORD AppSum[3] = {20, 28, 28};

/* Device properties */
const DEVICE DevLst[] = {
/*	 *DeviceName   Sign   Class FlashSize *SectMap XferSize */
	{ "1102",    0x2500102B,  2,  0x8000,  Map5,   0x800, 0x2FC },
	{ "1110",    0x0A07102B,  2,  0x1000,  Map5,   0x100, 0x2FC },
	{ "1110",    0x1A07102B,  2,  0x1000,  Map5,   0x100, 0x2FC },
	{ "1111",    0x041E502B,  2,  0x2000,  Map5,   0x200, 0x2FC },
	{ "1111",    0x2516D02B,  2,  0x2000,  Map5,   0x200, 0x2FC },
	{ "1111",    0x0416502B,  2,  0x2000,  Map5,   0x200, 0x2FC },
	{ "1111",    0x2516902B,  2,  0x2000,  Map5,   0x200, 0x2FC },
	{ "1111",    0x00010013,  2,  0x2000,  Map5,   0x200, 0x2FC },
	{ "1111",    0x00010012,  2,  0x2000,  Map5,   0x200, 0x2FC },
	{ "1112",    0x0A24902B,  2,  0x4000,  Map5,   0x200, 0x2FC },
	{ "1112",    0x1A24902B,  2,  0x4000,  Map5,   0x200, 0x2FC },
	{ "1112",    0x042D502B,  2,  0x4000,  Map5,   0x200, 0x2FC },
	{ "1112",    0x2524D02B,  2,  0x4000,  Map5,   0x200, 0x2FC },
	{ "1112",    0x0425502B,  2,  0x4000,  Map5,   0x200, 0x2FC },
	{ "1112",    0x2524902B,  2,  0x4000,  Map5,   0x200, 0x2FC },
	{ "1112",    0x00020023,  2,  0x4000,  Map5,   0x200, 0x2FC },
	{ "1112",    0x00020022,  2,  0x4000,  Map5,   0x200, 0x2FC },
	{ "1113",    0x0434502B,  2,  0x6000,  Map5,   0x400, 0x2FC },
	{ "1113",    0x2532902B,  2,  0x6000,  Map5,   0x400, 0x2FC },
	{ "1113",    0x4034102B,  2,  0x6000,  Map5,  0x1000, 0x2FC },
	{ "1113",    0x2532102B,  2,  0x6000,  Map5,  0x1000, 0x2FC },
	{ "1113",    0x0434102B,  2,  0x6000,  Map5,  0x1000, 0x2FC },
	{ "1113",    0x00030030,  2,  0x6000,  Map5,  0x1000, 0x2FC },
	{ "1113",    0x00030032,  2,  0x6000,  Map5,  0x1000, 0x2FC },
	{ "1114",    0x0A40902B,  2,  0x8000,  Map5,  0x1000, 0x2FC },
	{ "1114",    0x1A40902B,  2,  0x8000,  Map5,   0x400, 0x2FC },
	{ "1114",    0x0444502B,  2,  0x8000,  Map5,   0x400, 0x2FC },
	{ "1114",    0x2540902B,  2,  0x8000,  Map5,  0x1000, 0x2FC },
	{ "1114",    0x0444102B,  2,  0x8000,  Map5,  0x1000, 0x2FC },
	{ "1114",    0x2540102B,  2,  0x8000,  Map5,  0x1000, 0x2FC },
	{ "1114",    0x00040040,  2,  0x8000,  Map5,  0x1000, 0x2FC },
	{ "1114",    0x00040042,  2,  0x8000,  Map5,  0x1000, 0x2FC },
	{ "1114",    0x00040060,  2,  0xC000,  Map5,  0x1000, 0x2FC },
	{ "1114",    0x00040070,  2,  0xE000,  Map5,  0x1000, 0x2FC },
	{ "1115",    0x00050080,  2, 0x10000,  Map5,  0x1000, 0x2FC },

	{ "11C12",   0x1421102B,  2,  0x4000,  Map5,  0x1000, 0x2FC },
	{ "11C14",   0x1440102B,  2,  0x8000,  Map5,  0x1000, 0x2FC },
	{ "11C22",   0x1431102B,  2,  0x4000,  Map5,  0x1000, 0x2FC },
	{ "11C24",   0x1430102B,  2,  0x8000,  Map5,  0x1000, 0x2FC },

	{ "1224",    0x3640C02B,  2,   0x8000, Map5,   0x400, 0x2FC },
	{ "1224",    0x3642C02B,  2,   0xC000, Map5,   0x400, 0x2FC },
	{ "1225",    0x3650002B,  2,  0x10000, Map5,  0x1000, 0x2FC },
	{ "1225",    0x3652002B,  2,  0x14000, Map5,  0x1000, 0x2FC },
	{ "1226",    0x3660002B,  2,  0x18000, Map5,  0x1000, 0x2FC },
	{ "1227",    0x3670002B,  2,  0x20000, Map5,  0x1000, 0x2FC },

	{ "1311",    0x2C42502B,  2,   0x2000, Map5,   0x200, 0x2FC },
	{ "1313",    0x2C40102B,  2,   0x8000, Map5,   0x400, 0x2FC },
	{ "1342",    0x3D01402B,  2,   0x4000, Map5,   0x200, 0x2FC },
	{ "1343",    0x3D00002B,  2,   0x8000, Map5,   0x400, 0x2FC },

	{ "1751",    0x25001110,  1,   0x8000, Map4,   0x400, 0x2FC },
	{ "1751",    0x25001118,  1,   0x8000, Map4,   0x400, 0x2FC },
	{ "1752",    0x25001121,  1,  0x10000, Map4,  0x1000, 0x2FC },
	{ "1754",    0x25011722,  1,  0x20000, Map4,  0x1000, 0x2FC },
	{ "1756",    0x25011723,  1,  0x40000, Map4,  0x1000, 0x2FC },
	{ "1758",    0x25013F37,  1,  0x80000, Map4,  0x1000, 0x2FC },
	{ "1759",    0x25113737,  1,  0x80000, Map4,  0x1000, 0x2FC },
	{ "1764",    0x26011922,  1,  0x20000, Map4,  0x1000, 0x2FC },
	{ "1765",    0x26013733,  1,  0x40000, Map4,  0x1000, 0x2FC },
	{ "1766",    0x26013F33,  1,  0x40000, Map4,  0x1000, 0x2FC },
	{ "1767",    0x26012837,  1,  0x80000, Map4,  0x1000, 0x2FC },
	{ "1768",    0x26013F37,  1,  0x80000, Map4,  0x1000, 0x2FC },
	{ "1769",    0x26113F37,  1,  0x80000, Map4,  0x1000, 0x2FC },
	{ "1774",    0x27011132,  1,  0x20000, Map4,  0x1000, 0x2FC },
	{ "1776",    0x27191F43,  1,  0x40000, Map4,  0x1000, 0x2FC },
	{ "1777",    0x27193747,  1,  0x80000, Map4,  0x1000, 0x2FC },
	{ "1778",    0x27193F47,  1,  0x80000, Map4,  0x1000, 0x2FC },
	{ "1785",    0x281D1743,  1,  0x40000, Map4,  0x1000, 0x2FC },
	{ "1786",    0x281D1F43,  1,  0x40000, Map4,  0x1000, 0x2FC },
	{ "1787",    0x281D3747,  1,  0x80000, Map4,  0x1000, 0x2FC },
	{ "1788",    0x281D3F47,  1,  0x80000, Map4,  0x1000, 0x2FC },

	{ "2103",        327441,  0,   0x8000, Map1,   0x400, 0x1FC },
	{ "2131/2141",   196353,  0,   0x8000, Map1,  0x1000, 0x1FC },
	{ "2132/2142",   196369,  0,  0x10000, Map1,  0x1000, 0x1FC },
	{ "2134/2144",   196370,  0,  0x20000, Map1,  0x1000, 0x1FC },
	{ "2136/2146",   196387,  0,  0x40000, Map1,  0x1000, 0x1FC },
	{ "2138/2148",   196389,  0,  0x7D000, Map1,  0x1000, 0x1FC },
	{ "2109",      33685249,  0,   0xE000, Map2,  0x1000, 0x1FC },
	{ "2119",      33685266,  0,  0x1E000, Map2,  0x1000, 0x1FC },
	{ "2129",      33685267,  0,  0x3E000, Map3,  0x1000, 0x1FC },
	{ "2114",      16908050,  0,  0x1E000, Map2,  0x1000, 0x1FC },
	{ "2124",      16908051,  0,  0x3E000, Map3,  0x1000, 0x1FC },
	{ "2194",      50462483,  0,  0x3E000, Map3,  0x1000, 0x1FC },
	{ "2292",      67239699,  0,  0x3E000, Map3,  0x1000, 0x1FC },
	{ "2294",      84016915,  0,  0x3E000, Map3,  0x1000, 0x1FC },

	{ "2364",     369162498,  0,  0x20000, Map1,  0x1000, 0x1FC },
	{ "2365",     369158179,  0,  0x40000, Map1,  0x1000, 0x1FC },
	{ "2366",     369162531,  0,  0x40000, Map1,  0x1000, 0x1FC },
	{ "2367",     369158181,  0,  0x7E000, Map1,  0x1000, 0x1FC },
	{ "2368",     369162533,  0,  0x7E000, Map1,  0x1000, 0x1FC },
	{ "2377",     385935397,  0,  0x7E000, Map1,  0x1000, 0x1FC },
	{ "2378",     385940773,  0,  0x7E000, Map1,  0x1000, 0x1FC },
	{ "2387",     402716981,  0,  0x7E000, Map1,  0x1000, 0x1FC },
	{ "2388",     402718517,  0,  0x7E000, Map1,  0x1000, 0x1FC },
	{ "2458",     352386869,  0,  0x7E000, Map1,  0x1000, 0x1FC },

	{ "2468",     369164085,  0,  0x7E000, Map1,  0x1000, 0x1FC },
	{ "2478",     386006837,  0,  0x7E000, Map1,  0x1000, 0x1FC },

	{      0,             0,  0,        0,    0,       0, 0 }
};


const DEVICE *Device;	/* Pointer to the detected device property */

DWORD AddrRange[2];		/* Loaded address range {lowest, highest} */
BYTE Buffer[0x80000];	/* Flash data buffer (512K) */

int Freq = 14748;		/* -f<freq> Oscillator frequency [kHz] */
int Port = 1;			/* -p<port> Port numnber */
int Baud = 115200;		/* -p<port>:<bps> Bit rate */
int Pause;				/* -w<mode> Pause before exit program */
int Pol;				/* -c<flag> Invert signal polarity (b0:ER, b1:RS) */
int Read;				/* -r Read operation */




/*-----------------------------------------------------------------------
  Search and Open configuration file
-----------------------------------------------------------------------*/

static
FILE *open_cfgfile(char *filename)
{
	FILE *fp;
	char filepath[256], *dmy;


	if ((fp = fopen(filename, "rt")) != NULL) 
		return fp;
	if (SearchPath(NULL, filename, NULL, sizeof(filepath), filepath, &dmy)) {
		if ((fp = fopen(filepath, "rt")) != NULL) 
			return fp;
	}
	return NULL;
}




/*-----------------------------------------------------------------------
  Hex format manupilations
-----------------------------------------------------------------------*/


/* Pick a hexdecimal value from hex record */

DWORD get_valh (
	char **lp,	/* pointer to line read pointer */
	int count, 	/* number of digits to get (2,4,6,8) */
	BYTE *sum	/* byte check sum */
) {
	DWORD val = 0;
	BYTE n;


	do {
		n = *(*lp)++;
		if ((n -= '0') >= 10) {
			if ((n -= 7) < 10) return 0xFFFFFFFF;
			if (n > 0xF) return 0xFFFFFFFF;
		}
		val = (val << 4) + n;
		if (count & 1) *sum += (BYTE)val;
	} while (--count);
	return val;
}




/* Load Intel Hex and Motorola S format file into data buffer */ 

long input_hexfile (
	FILE *fp,			/* input stream */
	BYTE *buffer,		/* data input buffer */
	DWORD buffsize,		/* size of data buffer */
	DWORD *range		/* effective data size in the input buffer */
) {
	char line[600];			/* line input buffer */
	char *lp;				/* line read pointer */
	long lnum = 0;			/* input line number */
	WORD seg = 0, hadr = 0;	/* address expantion values for intel hex */
	DWORD addr, count, n;
	BYTE sum;


	while (fgets(line, sizeof(line), fp) != NULL) {
		lnum++;
		lp = &line[1]; sum = 0;

		if (line[0] == ':') {	/* Intel Hex format */
			if ((count = get_valh(&lp, 2, &sum)) > 0xFF) return lnum;	/* byte count */
			if ((addr = get_valh(&lp, 4, &sum)) > 0xFFFF) return lnum;	/* offset */

			switch (get_valh(&lp, 2, &sum)) {	/* block type? */
				case 0x00 :	/* data */
					addr += (seg << 4) + (hadr << 16);
					while (count--) {
						n = get_valh(&lp, 2, &sum);		/* pick a byte */
						if (n > 0xFF) return lnum;
						if (addr >= buffsize) continue;	/* clip by buffer size */
						if (addr > range[1])			/* update data size information */
							range[1] = addr;
						if (addr < range[0])
							range[0] = addr;
						buffer[addr++] = (BYTE)n;		/* store the data */
					}
					break;

				case 0x01 :	/* end */
					if (count != 0) return lnum;
					break;

				case 0x02 :	/* segment base [19:4] */
					if (count != 2) return lnum;
					seg = (WORD)get_valh(&lp, 4, &sum);
					if (seg == 0xFFFF) return lnum;
					break;

				case 0x03 :	/* program start address (segment:offset) */
					if (count != 4) return lnum;
					get_valh(&lp, 8, &sum);
					break;

				case 0x04 :	/* high address base [31:16] */
					if (count != 2) return lnum;
					hadr = (WORD)get_valh(&lp, 4, &sum);
					if (hadr == 0xFFFF) return lnum;
					break;

				case 0x05 :	/* program start address (linear) */
					if (count != 4) return lnum;
					get_valh(&lp, 8, &sum);
					break;

				default:	/* invalid block */
					return lnum;
			} /* switch */
			if (get_valh(&lp, 2, &sum) > 0xFF) return lnum;	/* get check sum */
			if (sum) return lnum;							/* test check sum */
			continue;
		} /* if */

		if (line[0] == 'S') {	/* Motorola S format */
			if ((*lp >= '1')&&(*lp <= '3')) {

				switch (*lp++) {	/* record type? (S1/S2/S3) */
					case '1' :
						count = get_valh(&lp, 2, &sum) - 3;
						if (count > 0xFF) return lnum;
						addr = get_valh(&lp, 4, &sum);
						if (addr > 0xFFFF) return lnum;
						break;
					case '2' :
						count = get_valh(&lp, 2, &sum) - 4;
						if (count > 0xFF) return lnum;
						addr = get_valh(&lp, 6, &sum);
						if (addr > 0xFFFFFF) return lnum;
						break;
					default :
						count = get_valh(&lp, 2, &sum) - 5;
						if (count > 0xFF) return lnum;
						addr = get_valh(&lp, 8, &sum);
						if (addr == 0xFFFFFFFF) return lnum;
				}
				while (count--) {
					n = get_valh(&lp, 2, &sum);
					if (n > 0xFF) return lnum;
					if (addr >= buffsize) continue;	/* clip by buffer size */
					if (addr > range[1])			/* update data size information */
						range[1] = addr;
					if (addr < range[0])
						range[0] = addr;
					buffer[addr++] = (BYTE)n;		/* store the data */
				}
				if (get_valh(&lp, 2, &sum) > 0xFF) return lnum;	/* get check sum */
				if (sum != 0xFF) return lnum;					/* test check sum */
			} /* switch */
			continue;
		} /* if */

		if (line[0] >= ' ') return lnum;
	} /* while */

	return feof(fp) ? 0 : -1;
}



/* Put an Intel Hex data block */

void put_hexline (
	FILE *fp,			/* output stream */
	const BYTE *buffer,	/* pointer to data buffer */
	WORD ofs,			/* block offset address */
	BYTE count,			/* data byte count */
	BYTE type			/* block type */
) {
	BYTE sum;

	/* Byte count, Offset address and Record type */
	fprintf(fp, ":%02X%04X%02X", count, ofs, type);
	sum = count + (ofs >> 8) + ofs + type;

	/* Data bytes */
	while (count--) {
		fprintf(fp, "%02X", *buffer);
		sum += *buffer++;
	}

	/* Check sum */
	fprintf(fp, "%02X\n", (BYTE)-sum);
}



/* Output data in Intel Hex format */

void output_ihex (
	FILE *fp,			/* output stream */
	const BYTE *buffer,	/* pointer to data buffer */
	DWORD datasize,		/* number of bytes to be output */
	BYTE blocksize		/* HEX block size (1,2,4,..,128) */
) {
	WORD ofs = 0;
	BYTE hadr[2] = {0,0}, d, n;
	DWORD bc = datasize;


	while (bc) {
		if ((ofs == 0) && (datasize > 0x10000)) {	/* A16 changed? */
			if (datasize > 0x100000) {
				put_hexline(fp, hadr, 0, 2, 4);
				hadr[1]++;
			} else {
				put_hexline(fp, hadr, 0, 2, 2);
				hadr[0] += 0x10;
			}
		}
		if (bc >= blocksize) {	/* full data block */
			for (d = 0xFF, n = 0; n < blocksize; n++) d &= *(buffer+n);
			if (d != 0xFF) put_hexline(fp, buffer, ofs, blocksize, 0);
			buffer += blocksize;
			bc -= blocksize;
			ofs += blocksize;
		} else {				/* fractional data block */
			for (d = 0xFF, n = 0; n < bc; n++) d &= *(buffer+n);
			if (d != 0xFF) put_hexline(fp, buffer, ofs, (BYTE)bc, 0);
			bc = 0;
		}
	}

	put_hexline(fp, NULL, 0, 0, 1);	/* End block */
}



/*-----------------------------------------------------------------------
  Command line analysis
-----------------------------------------------------------------------*/


static
int load_commands (int argc, char **argv)
{
	char *cp, *cmdlst[10], cmdbuff[256];
	int cmd;
	FILE *fp;
	long n;


	/* Clear flash data buffer */
	memset(Buffer, 0xFF, sizeof(Buffer));	/* Default value (if not loaded) */
	AddrRange[0] = sizeof(Buffer);	/* Lowest address */
	AddrRange[1] = 0;				/* Highest address */

	cmd = 0; cp = cmdbuff;

	/* Import ini file as command line parameters */
	fp = open_cfgfile(INIFILE);
	if (fp != NULL) {
		while (fgets(cp, cmdbuff + sizeof(cmdbuff) - cp, fp) != NULL) {
			if (cmd >= (sizeof(cmdlst) / sizeof(cmdlst[0]) - 1)) break;
			if (*cp <= ' ') break;
			while (cp[strlen(cp) - 1] <= ' ') cp[strlen(cp) - 1] = 0;
			cmdlst[cmd++] = cp; cp += strlen(cp) + 1;
		}
		fclose(fp);
	}

	/* Get command line parameters */
	while (--argc && (cmd < (sizeof(cmdlst) / sizeof(cmdlst[0]) - 1)))
		cmdlst[cmd++] = *++argv;
	cmdlst[cmd] = NULL;

	/* Analyze command line parameters... */
	for (cmd = 0; cmdlst[cmd] != NULL; cmd++) {
		cp = cmdlst[cmd];

		if (*cp == '-') {	/* Command switches... */
			cp++;
			switch (tolower(*cp++)) {
				case 'f' :	/* -f<num> (oscillator frequency) */
					Freq = strtoul(cp, &cp, 10);
					break;

				case 'p' :	/* -p<num>[:<bps>] (control port and bit rate) */
					Port = strtoul(cp, &cp, 10);
					if(*cp == ':')
						Baud = strtoul(cp+1, &cp, 10);
					break;

				case 'r' :	/* -r (read command) */
					Read = 1;
					break;

				case 'w' :	/* -w<num> (pause before exit 1:on error or 2:always) */
					Pause = strtoul(cp, &cp, 10);
					break;

				case 'c' :	/* -c<flag> (polarity of control signals) */
					Pol = strtoul(cp, &cp, 10);
					break;

				default :	/* invalid command */
					return 1;
			} /* switch */
			if(*cp >= ' ') return 1;	/* option trails garbage */
		} /* if */

		else {	/* HEX Files */
			fprintf(stderr, "Loading \"%s\"...", cp);
			if ((fp = fopen(cp, "rt")) == NULL) {
				fprintf(stderr, "Unable to open.\n");
				return 2;
			}
			n = input_hexfile(fp, Buffer, sizeof(Buffer), AddrRange);
			fclose(fp);
			if (n) {
				if (n < 0) {
					fprintf(stderr, "file access failure.\n");
				} else {
					fprintf(stderr, "hex format error at line %ld.\n", n);
				}
				return 2;
			}
			fprintf(stderr, "passed.\n");
		} /* else */

	} /* for */

	return 0;
}



/*-----------------------------------------------------------------------
  LPC 2000/1000 programming controls
-----------------------------------------------------------------------*/

/* Get top address of the sector contains given address */
static
DWORD adr2sect (
	DWORD addr
)
{
	DWORD n;

	for (n = 1; addr >= Device->SectMap[n]; n++);
	return n - 1;
}



/* Create an uuencoded asciz string from a byte array */
static
void uuencode (
	const void* src,	/* Pointer to the input data */
	int srcsize,		/* Size of input data (0 to 45) */
	char* dst			/* Pointer to the output buffer */
)
{
	const unsigned char *bin = (const unsigned char*)src;
	unsigned char c1, c2, c3;
	char c;
	int cc;


	if (srcsize >= 0 || srcsize <= 45) {
		c = srcsize + 0x20;
		*dst++ = (c == ' ') ? '`' : c;

		for (cc = 1; srcsize > 0; srcsize -= 3, cc += 4) {
			c1 = *bin++;
			c2 = c3 = 0;
			if (srcsize >= 2) {
				c2 = *bin++;
				if (srcsize >= 3) {
					c3 = *bin++;
				}
			}
			c = (c1 >> 2) + 0x20;
			*dst++ = (c == ' ') ? '`' : c;
			c = ((c1 & 3) << 4) + (c2 >> 4) + 0x20;
			*dst++ = (c == ' ') ? '`' : c;
			c = ((c2 & 15) << 2) + (c3 >> 6) + 0x20;
			*dst++ = (c == ' ') ? '`' : c;
			c = (c3 & 63) + 0x20;
			*dst++ = (c == ' ') ? '`' : c;
		}
	}
	*dst = 0;
}



/* Create a byte array from an uuencoded string */
static
int uudecode (				/* Returns number of bytes decoded (-1:error) */
	const char* src,		/* Pointer to the input string */
	unsigned char* dst		/* Pointer to the output buffer (must be 45 byte at least) */
)
{
	unsigned char b1, b2, b3, b4;
	int bc, cc;


	b1 = *(src++) - 0x20;
	if (b1 > 64) return -1;
	bc = b1 & 63;

	for (cc = bc; cc > 0; cc -= 3) {
		b1 = *(src++) - 0x20;
		if (b1 > 64) return -1;
		b2 = *(src++) - 0x20;
		if (b2 > 64) return -1;
		*(dst++) = (b1 << 2) + ((b2 >> 4) & 3);
		if (cc >= 2) {
			b3 = *(src++) - 0x20;
			if (b3 > 64) return -1;
			*(dst++) = (b2 << 4) + ((b3 >> 2) & 15);
			if (cc >= 3) {
				b4 = *(src++) - 0x20;
				if (b4 > 64) return -1;
				*(dst++) = (b3 << 6) + (b4 & 63);
			}
		}
	}

	return bc;
}



/* Get a line from the device */
static
int rcvr_line (
	HANDLE com,
	char *buff,
	int bufsize
)
{
	int rc, i = 0;


	for (;;) {
		ReadFile(com, &buff[i], 1, &rc, NULL);	/* Get a character */
		if (rc == 0) {	/* I/O error? */
			i = 0; break;
		}
		if (buff[i] == '\n') break;			/* EOL? */
		if ((BYTE)buff[i] < ' ') continue;	/* Ignore invisible chars */
		i++;
		if (i >= bufsize - 1) {				/* Buffer overflow? */
			i = 0; break;
		}
	}
	buff[i] = 0;	/* Remove \n */
	return i;
}




static
int enter_ispmode (
	HANDLE *comm
)
{
	DCB dcb = { sizeof(DCB),
				Baud, TRUE, FALSE, FALSE, FALSE,
				DTR_CONTROL_DISABLE, FALSE,
				TRUE, FALSE, FALSE, FALSE, FALSE,
				RTS_CONTROL_DISABLE, FALSE, 0, 0,
				10, 10,
				8, NOPARITY, ONESTOPBIT, '\x11', '\x13', '\xFF', '\xFF', 0 };
	COMMTIMEOUTS ct1 = { 0, 1, 250, 1, 250};
	char str[20];
	HANDLE h;
	DWORD wc, n;
	int rc = 0;


	/* Open communication port */
	sprintf(str, "\\\\.\\COM%u", Port);
	*comm = h = CreateFile(str, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "%s could not be opened.\n", str);
		return 5;
	}
	SetCommState(h, &dcb);
	SetCommTimeouts(h, &ct1);
	EscapeCommFunction(h, (Pol & 2) ? SETRTS : CLRRTS);	/* Set BOOT pin low if RTS controls it */

	MESS("Entering ISP mode.");
	for (n = 3; n; n--) {
		/* Reset the device if DTR signal controls RESET pin */
		EscapeCommFunction(h, (Pol & 1) ? SETDTR : CLRDTR);
		Sleep(50);
		EscapeCommFunction(h, (Pol & 1) ? CLRDTR : SETDTR);
		Sleep(100);
		PurgeComm(h, PURGE_RXABORT|PURGE_RXCLEAR);

		WriteFile(h, "?", 1, &wc, NULL);
		if (rcvr_line(h, str, sizeof(str)) && !strcmp(str, "Synchronized")) {
			WriteFile(h, "Synchronized\n", 13, &wc, NULL);
			rcvr_line(h, str, sizeof(str));
			if (rcvr_line(h, str, sizeof(str)) && !strcmp(str, "OK")) {
				sprintf(str, "%u\n", Freq);
				WriteFile(h, str, strlen(str), &wc, NULL);
				rcvr_line(h, str, sizeof(str));
				if (rcvr_line(h, str, sizeof(str)) && !strcmp(str, "OK")) break;
			}
		}
	}
	if (!n) {
		MESS("failed to synchronize.\n");
		rc = 6;
	}
	if (!rc) {
		MESS(".");
		WriteFile(h, "A 0\n", 4, &wc, NULL);	/* Echo Off */
		rcvr_line(h, str, sizeof(str));
		if (!rcvr_line(h, str, sizeof(str)) || strcmp(str, "0")) {
			MESS("failed(A).\n");
			rc = 6;
		}
	}
	if (!rc) {
		MESS(".");
		WriteFile(h, "J\n", 2, &wc, NULL);	/* Get device ID */
		if (!rcvr_line(h, str, sizeof(str)) || strcmp(str, "0")) {
			MESS("failed(J).\n");
			rc = 6;
		} else {
			/* Find target device type with device ID */
			rcvr_line(h, str, sizeof(str));
			wc = atol(str);
			for (Device = DevLst; Device->Sign != wc && Device->Sign; Device++) ;
			if (!Device->Sign) {
				fprintf(stderr, " unknown device(%u).", wc);
				rc = 6;
			}
		}
	}
	if (!rc) {
		MESS(".");
		WriteFile(h, "U 23130\n", 8, &wc, NULL);
		if (!rcvr_line(h, str, sizeof(str)) || strcmp(str, "0")) {
			MESS("failed(U).\n");
			rc = 6;
		}
	}
	if (!rc) {
		fprintf(stderr, "passed.\nDetected device is LPC%s.\n", Device->DeviceName);
	}

	return rc;
}



static
void exit_ispmode (
	HANDLE com
)
{
	/* Apply a reset to the device if DTR/RTS controls RESET/BOOT pin */
	EscapeCommFunction(com, (Pol & 2) ? CLRRTS : SETRTS);	/* Set BOOT pin high */
	EscapeCommFunction(com, (Pol & 1) ? SETDTR : CLRDTR);	/* Set RESET pin low */
	Sleep(50);
	EscapeCommFunction(com, (Pol & 1) ? CLRDTR : SETDTR);	/* Set RESET pin high */
}




static
int read_flash (
	HANDLE com
)
{
	DWORD addr, cc, xc, ms, sum, d;
	int n;
	char buf[80];


	MESS("Reading.");

	/* Start to read entire flash memory of the device */
	sprintf(buf, "R 0 %u\n", Device->FlashSize);
	WriteFile(com, buf, strlen(buf), &n, NULL);
	if (!rcvr_line(com, buf, sizeof(buf)) || strcmp(buf, "0")) {
		fprintf(stderr, "Failed(R,%s).\n", buf);
		return 10;
	}

	/* Receive flash memory data and store it to the buffer */
	addr = sum = ms = 0;
	for (;;) {
		if (addr >= ms) {
			ms += 0x2000; MESS(".");
		}
		n = rcvr_line(com, buf, sizeof(buf));
		if (!n) {
			MESS("timeout.\n");
			return 11;
		}
		if (n <= 6 && buf[0] >= '0' && buf[0] <= '9') {
			d = atol(buf);
			strcpy(buf, d == sum ? "OK\n" : "RESEND\n");
			WriteFile(com, buf, strlen(buf), &d, NULL);
			if (addr >= Device->FlashSize) break;
			sum = 0;
		} else {
			n = uudecode(buf, &Buffer[addr]);
			if (n < 0) {
				MESS("uudecode error.\n");
				return 10;
			}
			while (n > 0) {
				sum += Buffer[addr];
				addr++; n--;
			}
		}
	}
	/* Download user code to read vector area needed to disable remapping */
	sprintf(buf, "W %u %u\n", RamAddr[Device->Class], sizeof(RdCode[0]));
	WriteFile(com, buf, strlen(buf), &n, NULL);
	if (!rcvr_line(com, buf, sizeof(buf)) || strcmp(buf, "0")) {
		fprintf(stderr, "failed(W,%s).\n", buf);
		return 10;
	}
	for (xc = sum = 0; xc < sizeof(RdCode[0]); xc += cc) {
		cc = (xc + 45 <= sizeof(RdCode[0])) ? 45 : sizeof(RdCode[0]) - xc;
		uuencode(&RdCode[Device->Class][xc], cc, buf);
		strcat(buf, "\n");
		WriteFile(com, buf, strlen(buf), &d, NULL);
		for (d = 0; d < cc; d++) sum += RdCode[Device->Class][xc+d];
	}
	sprintf(buf, "%u\n", sum);
	WriteFile(com, buf, strlen(buf), &d, NULL);
	if (!rcvr_line(com, buf, sizeof(buf)) || strcmp(buf, "OK")) {
		fprintf(stderr, "failed(%s).\n", buf);
		return 10;
	}

	/* Execute the code */
	sprintf(buf, "G %u T\n", RamAddr[Device->Class]);
	WriteFile(com, buf, strlen(buf), &d, NULL);
	if (!rcvr_line(com, buf, sizeof(buf)) || strcmp(buf, "0")) {
		fprintf(stderr, "failed(G,%s).\n", buf);
		return 10;
	}

	/* Receive vector words and a byte of sum */
	ReadFile(com, Buffer, VectSize[Device->Class], &d, NULL);
	ReadFile(com, buf, 1, &d, NULL);
	if (d == 0) {
		MESS("failed(timeout).\n");
		return 10;
	}
	for (d = sum = 0; d < VectSize[Device->Class]; d++)
		sum += Buffer[d];
	if ((BYTE)sum != (BYTE)buf[0]) {
		MESS("failed(sum).\n");
		return 10;
	}

	MESS("passed.\n");

	/* Check if application code is exist (sum of eight vector data) */
	for (d = sum = 0; d < 32; d += 4)
		sum += LD_DWORD(&Buffer[d]);
	if (sum) {
		MESS("There is no valid program code.\n");
	} else {
		for (addr = d = 0; addr < Device->FlashSize; addr++) {
			if (Buffer[addr] != 0xFF) d = addr + 1;
		}
		fprintf(stderr, "%u%% used by program code.\n", d * 100 / Device->FlashSize);
	}

	return 0;
}




static
int erase_flash (
	HANDLE com
)
{
	DWORD es, n;
	char buf[80];
	COMMTIMEOUTS ct1 = { 0, 1, 2000, 1, 250},
				 ct2 = { 0, 1, 250, 1, 250};


	MESS("Erasing.");

	/* End sector is last sector of the device */
	es = adr2sect(Device->FlashSize - 1);

	/* Prepare to write/erase sectors */
	sprintf(buf, "P 0 %u\n", es);
	WriteFile(com, buf, strlen(buf), &n, NULL);
	MESS(".");
	if (!rcvr_line(com, buf, sizeof(buf)) || strcmp(buf, "0")) {
		fprintf(stderr, "failed(P,%s).\n", buf);
		return 12;
	}

	SetCommTimeouts(com, &ct1);	/* Set processing timeout of 2 sec */
	/* Erase sectors */
	sprintf(buf, "E 0 %u\n", es);
	WriteFile(com, buf, strlen(buf), &n, NULL);
	MESS(".");
	if (!rcvr_line(com, buf, sizeof(buf)) || strcmp(buf, "0")) {
		fprintf(stderr, "failed(E,%s).\n", buf);
		return 12;
	}
	SetCommTimeouts(com, &ct2);	/* Restore processing timeout */

	MESS("passed.\n");
	return 0;
}




static
int write_flash (
	HANDLE com
)
{
	DWORD wa, pc, n, lc, cc, xc, sum;
	char buf[80];


	MESS("Writing.");

	wa = (AddrRange[1] + Device->XferSize) & ~(Device->XferSize - 1);	/* Bottom block */
	pc = 0;

	while (wa > 0) {
		wa -= Device->XferSize;
		sprintf(buf, "W %u %u\n", RamAddr[Device->Class], Device->XferSize);
		WriteFile(com, buf, strlen(buf), &n, NULL);
		if (!rcvr_line(com, buf, sizeof(buf)) || strcmp(buf, "0")) {
			fprintf(stderr, "failed(W,%s).\n", buf);
			return 13;
		}
		for (xc = lc = sum = 0; xc < Device->XferSize; xc += cc) {
			if (xc + 45 <= Device->XferSize) {
				cc = 45;
				lc++;
			} else {
				cc = Device->XferSize - xc;
				lc = 20;
			}
			uuencode(&Buffer[wa+xc], cc, buf);
			strcat(buf, "\n");
			WriteFile(com, buf, strlen(buf), &n, NULL);
			for (n = 0; n < cc; n++) sum += Buffer[wa+xc+n];
			if (lc == 20) {
				sprintf(buf, "%u\n", sum);
				WriteFile(com, buf, strlen(buf), &n, NULL);
				if (!rcvr_line(com, buf, sizeof(buf)) || strcmp(buf, "OK")) {
					fprintf(stderr, "failed(%s).\n", buf);
					return 13;
				}
				lc = sum = 0;
			}
		}
		sprintf(buf, "P %u %u\n", adr2sect(wa), adr2sect(wa));
		WriteFile(com, buf, strlen(buf), &n, NULL);
		if (!rcvr_line(com, buf, sizeof(buf)) || strcmp(buf, "0")) {
			fprintf(stderr, "failed(P,%s).\n", buf);
			return 13;
		}
		sprintf(buf, "C %u %u %u\n", wa, RamAddr[Device->Class], Device->XferSize);
		WriteFile(com, buf, strlen(buf), &n, NULL);
		if (!rcvr_line(com, buf, sizeof(buf)) || strcmp(buf, "0")) {
			fprintf(stderr, "failed(C,%s).\n", buf);
			return 13;
		}

		pc += Device->XferSize;
		if (pc >= 0x2000) {	/* Put a progress indicator every 8K byte */
			pc -= 0x2000;
			MESS(".");
		}
	}

	MESS("passed.\n");
	return 0;
}



void pause (
	int rc
)
{
	if ((Pause == 2) || ((Pause == 1) && rc)) {
		MESS("\nType Enter to exit...");
		getchar();
	}
}



int main (int argc, char** argv)
{
	int rc;
	DWORD s, n, i;
	HANDLE hcom;


	rc = load_commands(argc, argv);
	if (rc) {
		if (rc == 1) MESS(Usage);
		pause(rc);
		return rc;
	}
	if (Read) {
		rc = enter_ispmode(&hcom);
		if (!rc) {
			rc = read_flash(hcom);
			if (!rc)
				output_ihex(stdout, Buffer, Device->FlashSize, 32);
			exit_ispmode(hcom);
		}
	} else {
		if (AddrRange[1] == 0) {	/* Check if any data is loaded */
			MESS(Usage);
			pause(1);
			return 1;
		}
		fprintf(stderr, "Loaded address range is %05X-%05X.\n", AddrRange[0], AddrRange[1]);
		if (AddrRange[0] != 0) {	/* Check if vector area is loaded */
			MESS("No vector table is loaded.\n");
			pause(1);
			return 1;
		}
		rc = enter_ispmode(&hcom);	/* Open port, enter ispmode and detect device type */
		if (!rc) {
			if (AddrRange[1] >= Device->FlashSize) {
				MESS("Too large data for this device.\n");
				pause(1);
				return 1;
			}
			s = LD_DWORD(&Buffer[Device->CRP]);
			if (s == 0x43218765 || s == 0x4E697370) {
				MESS("CAUTION: CRP3 or NO_ISP is selected. This will disable future ISP operations.\nAre you sure? <Y/n>=");
				if (getchar() != 'Y') {
					pause(1);
					return 1;
				}
			}
			/* Validate application code (create check sum) */
			for (i = s = 0; i < 32; i += 4)
				s += LD_DWORD(&Buffer[i]);
			i = AppSum[Device->Class];
			n = LD_DWORD(&Buffer[i]) - s;
			ST_DWORD(&Buffer[i], n);
			/* Erase entire flash memory and write application code */
			rc = erase_flash(hcom);
			if (!rc)
				rc = write_flash(hcom);
			exit_ispmode(hcom);
		}
	}

	pause(rc);
	return rc;
}

