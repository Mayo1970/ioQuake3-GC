/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "tr_local.h"

typedef struct
{
	char id[2];
	unsigned fileSize;
	unsigned reserved0;
	unsigned bitmapDataOffset;
	unsigned bitmapHeaderSize;
	unsigned width;
	unsigned height;
	unsigned short planes;
	unsigned short bitsPerPixel;
	unsigned compression;
	unsigned bitmapDataSize;
	unsigned hRes;
	unsigned vRes;
	unsigned colors;
	unsigned importantColors;
	unsigned char palette[256][4];
} BMPHeader_t;

void R_LoadBMP( const char *name, byte **pic, int *width, int *height )
{
	int		columns, rows;
	unsigned	numPixels;
	byte	*pixbuf;
	#ifndef HW_DOL
	int		row, column;
	#endif
	byte	*buf_p;
	byte	*end;
	union {
		byte *b;
		void *v;
	} buffer;
	int		length;
	BMPHeader_t bmpHeader;
	byte		*bmpRGBA;

	*pic = NULL;

	if(width)
		*width = 0;

	if(height)
		*height = 0;

	//
	// load the file
	//
	length = ri.FS_ReadFile( ( char * ) name, &buffer.v);
	if (!buffer.b || length < 0) {
		return;
	}

	if (length < 54)
	{
		ri.Error( ERR_DROP, "LoadBMP: header too short (%s)", name );
	}

	buf_p = buffer.b;
	end = buffer.b + length;

	bmpHeader.id[0] = *buf_p++;
	bmpHeader.id[1] = *buf_p++;
	bmpHeader.fileSize = LittleLong( * ( int * ) buf_p );
	buf_p += 4;
	bmpHeader.reserved0 = LittleLong( * ( int * ) buf_p );
	buf_p += 4;
	bmpHeader.bitmapDataOffset = LittleLong( * ( int * ) buf_p );
	buf_p += 4;
	bmpHeader.bitmapHeaderSize = LittleLong( * ( int * ) buf_p );
	buf_p += 4;
	bmpHeader.width = LittleLong( * ( int * ) buf_p );
	buf_p += 4;
	bmpHeader.height = LittleLong( * ( int * ) buf_p );
	buf_p += 4;
	bmpHeader.planes = LittleShort( * ( short * ) buf_p );
	buf_p += 2;
	bmpHeader.bitsPerPixel = LittleShort( * ( short * ) buf_p );
	buf_p += 2;
	bmpHeader.compression = LittleLong( * ( int * ) buf_p );
	buf_p += 4;
	bmpHeader.bitmapDataSize = LittleLong( * ( int * ) buf_p );
	buf_p += 4;
	bmpHeader.hRes = LittleLong( * ( int * ) buf_p );
	buf_p += 4;
	bmpHeader.vRes = LittleLong( * ( int * ) buf_p );
	buf_p += 4;
	bmpHeader.colors = LittleLong( * ( int * ) buf_p );
	buf_p += 4;
	bmpHeader.importantColors = LittleLong( * ( int * ) buf_p );
	buf_p += 4;

	if ( bmpHeader.bitsPerPixel == 8 )
	{
		if (buf_p + sizeof(bmpHeader.palette) > end)
			ri.Error( ERR_DROP, "LoadBMP: header too short (%s)", name );

		Com_Memcpy( bmpHeader.palette, buf_p, sizeof( bmpHeader.palette ) );
		buf_p += sizeof(bmpHeader.palette);
	}

	if (buffer.b + bmpHeader.bitmapDataOffset > end)
	{
		ri.Error( ERR_DROP, "LoadBMP: invalid offset value in header (%s)", name );
	}

	buf_p = buffer.b + bmpHeader.bitmapDataOffset;

	if ( bmpHeader.id[0] != 'B' && bmpHeader.id[1] != 'M' ) 
	{
		ri.Error( ERR_DROP, "LoadBMP: only Windows-style BMP files supported (%s)", name );
	}
	if ( bmpHeader.fileSize != length )
	{
		ri.Error( ERR_DROP, "LoadBMP: header size does not match file size (%u vs. %u) (%s)", bmpHeader.fileSize, length, name );
	}
	if ( bmpHeader.compression != 0 )
	{
		ri.Error( ERR_DROP, "LoadBMP: only uncompressed BMP files supported (%s)", name );
	}
	if ( bmpHeader.bitsPerPixel < 8 )
	{
		ri.Error( ERR_DROP, "LoadBMP: monochrome and 4-bit BMP files not supported (%s)", name );
	}

	switch ( bmpHeader.bitsPerPixel )
	{
		case 8:
		case 16:
		case 24:
		case 32:
			break;
		default:
			ri.Error( ERR_DROP, "LoadBMP: illegal pixel_size '%hu' in file '%s'", bmpHeader.bitsPerPixel, name );
			break;
	}

	columns = bmpHeader.width;
	rows = bmpHeader.height;
	if ( rows < 0 )
		rows = -rows;
	numPixels = columns * rows;

	if(columns <= 0 || !rows || numPixels > 0x1FFFFFFF // 4*1FFFFFFF == 0x7FFFFFFC < 0x7FFFFFFF
	    || ((numPixels * 4) / columns) / 4 != rows)
	{
	  ri.Error (ERR_DROP, "LoadBMP: %s has an invalid image size", name);
	}
	if(buf_p + numPixels*bmpHeader.bitsPerPixel/8 > end)
	{
	  ri.Error (ERR_DROP, "LoadBMP: file truncated (%s)", name);
	}

#ifdef HW_DOL
	{
		int shift = R_GCImageShift( columns, rows ), block = 1 << shift;
		int outWidth = columns >> shift, outHeight = rows >> shift;
		int sourceRow, x, bytes = bmpHeader.bitsPerPixel / 8;
		unsigned int stride = ( (unsigned int)columns * bytes + 3 ) & ~3U;
		byte *band;
		if ( stride > (unsigned int)( end - buf_p ) / rows ) {
			ri.FS_FreeFile( buffer.v );
			ri.Error( ERR_DROP, "LoadBMP: file truncated (%s)", name );
			return;
		}
		bmpRGBA = R_IMAGE_MALLOC( outWidth * outHeight * 4 );
		band = R_IMAGE_MALLOC( columns * 4 * block );
		for ( sourceRow = 0; sourceRow < rows; sourceRow++ ) {
			byte *src = buf_p + sourceRow * stride;
			pixbuf = band + ( sourceRow % block ) * columns * 4;
			for ( x = 0; x < columns; x++, src += bytes, pixbuf += 4 ) {
				if ( bytes == 1 ) {
					pixbuf[0] = bmpHeader.palette[*src][2]; pixbuf[1] = bmpHeader.palette[*src][1];
					pixbuf[2] = bmpHeader.palette[*src][0]; pixbuf[3] = 255;
				} else if ( bytes == 2 ) {
					unsigned int pixel = src[0] | ( src[1] << 8 );
					pixbuf[0] = ( pixel & 0x7c00 ) >> 7; pixbuf[1] = ( pixel & 0x03e0 ) >> 2;
					pixbuf[2] = ( pixel & 0x001f ) << 3; pixbuf[3] = 255;
				} else {
					pixbuf[0] = src[2]; pixbuf[1] = src[1]; pixbuf[2] = src[0];
					pixbuf[3] = bytes == 4 ? src[3] : 255;
				}
			}
			if ( sourceRow % block == block - 1 ) {
				int destRow = (int)bmpHeader.height < 0 ? sourceRow / block : outHeight - 1 - sourceRow / block;
				R_GCReduceRows( band, bmpRGBA + destRow * outWidth * 4, columns, shift );
			}
		}
		R_IMAGE_FREE( band ); ri.FS_FreeFile( buffer.v );
		if ( width ) *width = outWidth;
		if ( height ) *height = outHeight;
		*pic = bmpRGBA;
		return;
	}
#else
	if ( width ) 
		*width = columns;
	if ( height )
		*height = rows;

	bmpRGBA = R_IMAGE_MALLOC( numPixels * 4 );
	*pic = bmpRGBA;


	for ( row = rows-1; row >= 0; row-- )
	{
		pixbuf = bmpRGBA + row*columns*4;

		for ( column = 0; column < columns; column++ )
		{
			unsigned char red, green, blue, alpha;
			int palIndex;
			unsigned short shortPixel;

			switch ( bmpHeader.bitsPerPixel )
			{
			case 8:
				palIndex = *buf_p++;
				*pixbuf++ = bmpHeader.palette[palIndex][2];
				*pixbuf++ = bmpHeader.palette[palIndex][1];
				*pixbuf++ = bmpHeader.palette[palIndex][0];
				*pixbuf++ = 0xff;
				break;
			case 16:
				shortPixel = * ( unsigned short * ) pixbuf;
				pixbuf += 2;
				*pixbuf++ = ( shortPixel & ( 31 << 10 ) ) >> 7;
				*pixbuf++ = ( shortPixel & ( 31 << 5 ) ) >> 2;
				*pixbuf++ = ( shortPixel & ( 31 ) ) << 3;
				*pixbuf++ = 0xff;
				break;

			case 24:
				blue = *buf_p++;
				green = *buf_p++;
				red = *buf_p++;
				*pixbuf++ = red;
				*pixbuf++ = green;
				*pixbuf++ = blue;
				*pixbuf++ = 255;
				break;
			case 32:
				blue = *buf_p++;
				green = *buf_p++;
				red = *buf_p++;
				alpha = *buf_p++;
				*pixbuf++ = red;
				*pixbuf++ = green;
				*pixbuf++ = blue;
				*pixbuf++ = alpha;
				break;
			}
		}
	}

	ri.FS_FreeFile( buffer.v );

#endif
}
