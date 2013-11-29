
/*
	TARGA Vision file header for TARGA 2 file format
*/

#ifndef TARGA_H
#define TARGA_H

#pragma pack(push, 1)
typedef struct _tagTGAHDR
{
    BYTE	tgaIdentSize;          // size of ID field that follows 18 byte header (0 usually)
    BYTE	tgaColorMapType;      // type of colour map 0=none, 1=has palette
    BYTE	tgaImageType;          // type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

    WORD	tgaColorMapOffset;     // first colour map entry in palette
    WORD	tgaColorMapLength;    // number of colours in palette
    BYTE	tgaColorMapBits;      // number of bits per palette entry 15,16,24,32

    WORD	tgaXStart;             // image x origin
    WORD	tgaYStart;             // image y origin
    WORD	tgaWidth;              // image width in pixels
    WORD	tgaHeight;             // image height in pixels
    BYTE	tgaBits;               // image bits per pixel 8,16,24,32
    BYTE	tgaDescriptor;         // image descriptor bits (vh flip bits)
} TARGAINFOHEADER;
#pragma pack(pop)


#endif