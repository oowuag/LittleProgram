/* 
 * QR Code generator demo (C++)
 * 
 * Run this command-line program with no arguments. The program computes a bunch of demonstration
 * QR Codes and prints them to the console. Also, the SVG code for one QR Code is printed as a sample.
 * 
 * Copyright (c) Project Nayuki
 * https://www.nayuki.io/page/qr-code-generator-library
 * 
 * (MIT License)
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * - The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 * - The Software is provided "as is", without warranty of any kind, express or
 *   implied, including but not limited to the warranties of merchantability,
 *   fitness for a particular purpose and noninfringement. In no event shall the
 *   authors or copyright holders be liable for any claim, damages or other
 *   liability, whether in an action of contract, tort or otherwise, arising from,
 *   out of or in connection with the Software or the use or other dealings in the
 *   Software.
 */

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include "QrCode.hpp"


//	-------------------------------------------------------
//	DEFines
//	-------------------------------------------------------

#define QRCODE_TEXT					"ABCDEF0123456789ABCDEF0123456789";		// Text to encode into QRCode
#define OUT_FILE					"test.bmp"								// Output file name
#define OUT_FILE_PIXEL_PRESCALER	8											// Prescaler (number of pixels in bmp file for each QRCode pixel, on each dimension)

#define PIXEL_COLOR_R				0x19											// Color of bmp pixels
#define PIXEL_COLOR_G				0x19
#define PIXEL_COLOR_B				0x70

// BMP defines

typedef unsigned short	WORD;
typedef unsigned long	DWORD;
typedef signed long		LONG;

#define BI_RGB			0L

#pragma pack(push, 2)

typedef struct
{
    WORD    bfType;
    DWORD   bfSize;
    WORD    bfReserved1;
    WORD    bfReserved2;
    DWORD   bfOffBits;
} BITMAPFILEHEADER;

typedef struct
{
    DWORD      biSize;
    LONG       biWidth;
    LONG       biHeight;
    WORD       biPlanes;
    WORD       biBitCount;
    DWORD      biCompression;
    DWORD      biSizeImage;
    LONG       biXPelsPerMeter;
    LONG       biYPelsPerMeter;
    DWORD      biClrUsed;
    DWORD      biClrImportant;
} BITMAPINFOHEADER;

#pragma pack(pop)
//	-------------------------------------------------------


// Prints the given QR Code to the console.
void printQr(const qrcodegen::QrCode &qr)
{
	int border = 4;
	for (int y = -border; y < qr.size + border; y++) {
		for (int x = -border; x < qr.size + border; x++) {
			std::cout << (qr.getModule(x, y) == 1 ? "#" : " ");
		}
		std::cout << std::endl;
	}
}

bool writeQrBMP(const qrcodegen::QrCode &qr, int boarder)
{
    unsigned int unWidth = qr.size + boarder * 2;
    unsigned int unWidthAdjusted = unWidth * OUT_FILE_PIXEL_PRESCALER * 3;
    if (unWidthAdjusted % 4) {
        unWidthAdjusted = (unWidthAdjusted / 4 + 1) * 4;
    }
    unsigned int unDataBytes = unWidthAdjusted * unWidth * OUT_FILE_PIXEL_PRESCALER;

    // Allocate pixels buffer
    unsigned char* pRGBData =  new unsigned char[unDataBytes];
    if (NULL == pRGBData) {
        printf("Out of memory");
        return false;
    }
    // Preset to white
    memset(pRGBData, 0xff, unDataBytes);

    // Prepare bmp headers
    BITMAPFILEHEADER kFileHeader;
    kFileHeader.bfType = 0x4d42;  // "BM"
    kFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)+ unDataBytes;
    kFileHeader.bfReserved1 = 0;
    kFileHeader.bfReserved2 = 0;
    kFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    BITMAPINFOHEADER kInfoHeader;
    kInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
    kInfoHeader.biWidth = unWidth * OUT_FILE_PIXEL_PRESCALER;
    kInfoHeader.biHeight = -((int)unWidth * OUT_FILE_PIXEL_PRESCALER);
    kInfoHeader.biPlanes = 1;
    kInfoHeader.biBitCount = 24;
    kInfoHeader.biCompression = BI_RGB;
    kInfoHeader.biSizeImage = 0;
    kInfoHeader.biXPelsPerMeter = 0;
    kInfoHeader.biYPelsPerMeter = 0;
    kInfoHeader.biClrUsed = 0;
    kInfoHeader.biClrImportant = 0;


    // Convert QrCode bits to bmp pixels
    for (size_t y = 0; y < unWidth; y++)
    {
        unsigned char* pDestData = pRGBData + unWidthAdjusted * y * OUT_FILE_PIXEL_PRESCALER;
        for (size_t x = 0; x < unWidth; x++)
        {
            if (qr.getModule(x - boarder, y - boarder) & 1)
            {
                for (size_t l = 0; l < OUT_FILE_PIXEL_PRESCALER; l++)
                {
                    for (size_t n = 0; n < OUT_FILE_PIXEL_PRESCALER; n++)
                    {
                        *(pDestData + n * 3 + unWidthAdjusted * l) = PIXEL_COLOR_B;
                        *(pDestData + 1 + n * 3 + unWidthAdjusted * l) = PIXEL_COLOR_G;
                        *(pDestData + 2 + n * 3 + unWidthAdjusted * l) = PIXEL_COLOR_R;
                    }
                }
            }
            pDestData += 3 * OUT_FILE_PIXEL_PRESCALER;
        }
    }

    // Output the bmp file
    FILE* f = NULL;
    if (!(fopen_s(&f, OUT_FILE, "wb"))) {
        fwrite(&kFileHeader, sizeof(BITMAPFILEHEADER), 1, f);
        fwrite(&kInfoHeader, sizeof(BITMAPINFOHEADER), 1, f);
        fwrite(pRGBData, sizeof(unsigned char), unDataBytes, f);
        fclose(f);
    }
    else {
        printf("Unable to open file");
    }

    // Free data
    delete[] pRGBData;

    return true;
}

bool writeQrSVG(const qrcodegen::QrCode &qr, int boarder)
{
    std::string svgStr = qr.toSvgString(boarder);
    // Output the bmp file
    FILE* f = NULL;
    if (!(fopen_s(&f, "test.svg", "wb"))) {
        fwrite(svgStr.c_str(), sizeof(unsigned char), svgStr.size(), f);
        fclose(f);
    }
    else {
        printf("Unable to open file");
        return false;
    }

    return true;
}



// Creates a single QR Code, then prints it to the console.
void doBasicDemo()
{
    const char *text = QRCODE_TEXT;  // User-supplied text
    const qrcodegen::QrCode::Ecc &errCorLvl = qrcodegen::QrCode::Ecc::LOW;  // Error correction level

    // Make and print the QR Code symbol
    const qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(text, errCorLvl);

    int border = 4;
    writeQrBMP(qr, border);
    writeQrSVG(qr, border);
}


// The main application program.
int main()
{
    doBasicDemo();
    return EXIT_SUCCESS;
}