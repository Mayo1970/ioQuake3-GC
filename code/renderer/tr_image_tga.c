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

/*
========================================================================

TGA files are used for 24/32 bit images

========================================================================
*/

typedef struct _TargaHeader {
	unsigned char 	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} TargaHeader;

void R_LoadTGA ( const char *name, byte **pic, int *width, int *height)
{
	unsigned	columns, rows, numPixels;
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
	TargaHeader	targa_header;
	byte		*targa_rgba;
	int length;

	*pic = NULL;

	if(width)
		*width = 0;
	if(height)
		*height = 0;

	//
	// load the file
	//
	length = ri.FS_ReadFile ( ( char * ) name, &buffer.v);
	if (!buffer.b || length < 0) {
		return;
	}

	if(length < 18)
	{
		ri.Error( ERR_DROP, "LoadTGA: header too short (%s)", name );
	}

	buf_p = buffer.b;
	end = buffer.b + length;

	targa_header.id_length = buf_p[0];
	targa_header.colormap_type = buf_p[1];
	targa_header.image_type = buf_p[2];
	
	memcpy(&targa_header.colormap_index, &buf_p[3], 2);
	memcpy(&targa_header.colormap_length, &buf_p[5], 2);
	targa_header.colormap_size = buf_p[7];
	memcpy(&targa_header.x_origin, &buf_p[8], 2);
	memcpy(&targa_header.y_origin, &buf_p[10], 2);
	memcpy(&targa_header.width, &buf_p[12], 2);
	memcpy(&targa_header.height, &buf_p[14], 2);
	targa_header.pixel_size = buf_p[16];
	targa_header.attributes = buf_p[17];

	targa_header.colormap_index = LittleShort(targa_header.colormap_index);
	targa_header.colormap_length = LittleShort(targa_header.colormap_length);
	targa_header.x_origin = LittleShort(targa_header.x_origin);
	targa_header.y_origin = LittleShort(targa_header.y_origin);
	targa_header.width = LittleShort(targa_header.width);
	targa_header.height = LittleShort(targa_header.height);

	buf_p += 18;

	if (targa_header.image_type!=2 
		&& targa_header.image_type!=10
		&& targa_header.image_type != 3 ) 
	{
		ri.Error (ERR_DROP, "LoadTGA: Only type 2 (RGB), 3 (gray), and 10 (RGB) TGA images supported");
	}

	if ( targa_header.colormap_type != 0 )
	{
		ri.Error( ERR_DROP, "LoadTGA: colormaps not supported" );
	}

	if ( ( targa_header.pixel_size != 32 && targa_header.pixel_size != 24 ) && targa_header.image_type != 3 )
	{
		ri.Error (ERR_DROP, "LoadTGA: Only 32 or 24 bit images supported (no colormaps)");
	}

	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows * 4;

	if(!columns || !rows || numPixels > 0x7FFFFFFF || numPixels / columns / 4 != rows)
	{
		ri.Error (ERR_DROP, "LoadTGA: %s has an invalid image size", name);
	}


#ifdef HW_DOL
	{
		int shift = R_GCImageShift( columns, rows ), block = 1 << shift;
		int outWidth = columns >> shift, outHeight = rows >> shift;
		int sourceRow, x, remaining = 0, repeat = 0, bytes = targa_header.pixel_size / 8;
		byte color[4] = { 0, 0, 0, 255 }, *band;
		if ( ( targa_header.image_type == 3 && targa_header.pixel_size != 8 && targa_header.pixel_size != 24 && targa_header.pixel_size != 32 ) ||
			buf_p + targa_header.id_length > end ) {
			ri.FS_FreeFile( buffer.v );
			ri.Error( ERR_DROP, "LoadTGA: invalid header (%s)", name );
			return;
		}
		buf_p += targa_header.id_length;
		targa_rgba = R_IMAGE_MALLOC( outWidth * outHeight * 4 );
		band = R_IMAGE_MALLOC( columns * 4 * block );
		for ( sourceRow = 0; sourceRow < (int)rows; sourceRow++ ) {
			pixbuf = band + ( sourceRow % block ) * columns * 4;
			for ( x = 0; x < (int)columns; x++ ) {
				if ( !remaining ) {
					if ( targa_header.image_type == 10 ) {
						if ( buf_p >= end ) goto gc_tga_truncated;
						remaining = 1 + ( *buf_p & 127 );
						repeat = *buf_p++ & 128;
					} else { remaining = 1; repeat = 0; }
					if ( repeat ) {
						if ( end - buf_p < bytes ) goto gc_tga_truncated;
						color[2] = *buf_p++; color[1] = *buf_p++; color[0] = *buf_p++;
						color[3] = bytes == 4 ? *buf_p++ : 255;
					}
				}
				if ( !repeat ) {
					if ( end - buf_p < bytes ) goto gc_tga_truncated;
					if ( bytes == 1 ) { color[0] = color[1] = color[2] = *buf_p++; color[3] = 255; }
					else { color[2] = *buf_p++; color[1] = *buf_p++; color[0] = *buf_p++; color[3] = bytes == 4 ? *buf_p++ : 255; }
				}
				memcpy( pixbuf + x * 4, color, 4 ); remaining--;
			}
			if ( sourceRow % block == block - 1 ) {
				int destRow = sourceRow / block;
				if ( !( targa_header.attributes & 0x20 ) ) destRow = outHeight - 1 - destRow;
				R_GCReduceRows( band, targa_rgba + destRow * outWidth * 4, columns, shift );
			}
		}
		R_IMAGE_FREE( band );
		ri.FS_FreeFile( buffer.v );
		if ( width ) *width = outWidth;
		if ( height ) *height = outHeight;
		*pic = targa_rgba;
		return;
	gc_tga_truncated:
		R_IMAGE_FREE( band ); R_IMAGE_FREE( targa_rgba ); ri.FS_FreeFile( buffer.v );
		ri.Error( ERR_DROP, "LoadTGA: file truncated (%s)", name );
		return;
	}
#else
	targa_rgba = R_IMAGE_MALLOC (numPixels);

	if (targa_header.id_length != 0)
	{
		if (buf_p + targa_header.id_length > end)
			ri.Error( ERR_DROP, "LoadTGA: header too short (%s)", name );

		buf_p += targa_header.id_length;  // skip TARGA image comment
	}
	
	if ( targa_header.image_type==2 || targa_header.image_type == 3 )
	{ 
		if(buf_p + columns*rows*targa_header.pixel_size/8 > end)
		{
			ri.Error (ERR_DROP, "LoadTGA: file truncated (%s)", name);
		}

		// Uncompressed RGB or gray scale image
		for(row=rows-1; row>=0; row--) 
		{
			pixbuf = targa_rgba + row*columns*4;
			for(column=0; column<columns; column++) 
			{
				unsigned char red,green,blue,alphabyte;
				switch (targa_header.pixel_size) 
				{
					
				case 8:
					blue = *buf_p++;
					green = blue;
					red = blue;
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = 255;
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
					alphabyte = *buf_p++;
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = alphabyte;
					break;
				default:
					ri.Error( ERR_DROP, "LoadTGA: illegal pixel_size '%d' in file '%s'", targa_header.pixel_size, name );
					break;
				}
			}
		}
	}
	else if (targa_header.image_type==10) {   // Runlength encoded RGB images
		unsigned char red,green,blue,alphabyte,packetHeader,packetSize,j;

		red = 0;
		green = 0;
		blue = 0;
		alphabyte = 0xff;

		for(row=rows-1; row>=0; row--) {
			pixbuf = targa_rgba + row*columns*4;
			for(column=0; column<columns; ) {
				if(buf_p + 1 > end)
					ri.Error (ERR_DROP, "LoadTGA: file truncated (%s)", name);
				packetHeader= *buf_p++;
				packetSize = 1 + (packetHeader & 0x7f);
				if (packetHeader & 0x80) {        // run-length packet
					if(buf_p + targa_header.pixel_size/8 > end)
						ri.Error (ERR_DROP, "LoadTGA: file truncated (%s)", name);
					switch (targa_header.pixel_size) {
						case 24:
								blue = *buf_p++;
								green = *buf_p++;
								red = *buf_p++;
								alphabyte = 255;
								break;
						case 32:
								blue = *buf_p++;
								green = *buf_p++;
								red = *buf_p++;
								alphabyte = *buf_p++;
								break;
						default:
							ri.Error( ERR_DROP, "LoadTGA: illegal pixel_size '%d' in file '%s'", targa_header.pixel_size, name );
							break;
					}
	
					for(j=0;j<packetSize;j++) {
						*pixbuf++=red;
						*pixbuf++=green;
						*pixbuf++=blue;
						*pixbuf++=alphabyte;
						column++;
						if (column==columns) { // run spans across rows
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							pixbuf = targa_rgba + row*columns*4;
						}
					}
				}
				else {                            // non run-length packet

					if(buf_p + targa_header.pixel_size/8*packetSize > end)
						ri.Error (ERR_DROP, "LoadTGA: file truncated (%s)", name);
					for(j=0;j<packetSize;j++) {
						switch (targa_header.pixel_size) {
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
									alphabyte = *buf_p++;
									*pixbuf++ = red;
									*pixbuf++ = green;
									*pixbuf++ = blue;
									*pixbuf++ = alphabyte;
									break;
							default:
								ri.Error( ERR_DROP, "LoadTGA: illegal pixel_size '%d' in file '%s'", targa_header.pixel_size, name );
								break;
						}
						column++;
						if (column==columns) { // pixel packet run spans across rows
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							pixbuf = targa_rgba + row*columns*4;
						}						
					}
				}
			}
			breakOut:;
		}
	}

#if 0 
  // TTimo: this is the chunk of code to ensure a behavior that meets TGA specs 
  // bit 5 set => top-down
  if (targa_header.attributes & 0x20) {
    unsigned char *flip = (unsigned char*)malloc (columns*4);
    unsigned char *src, *dst;

    for (row = 0; row < rows/2; row++) {
      src = targa_rgba + row * 4 * columns;
      dst = targa_rgba + (rows - row - 1) * 4 * columns;

      memcpy (flip, src, columns*4);
      memcpy (src, dst, columns*4);
      memcpy (dst, flip, columns*4);
    }
    free (flip);
  }
#endif
  // instead we just print a warning
  if (targa_header.attributes & 0x20) {
    ri.Printf( PRINT_WARNING, "WARNING: '%s' TGA file header declares top-down image, ignoring\n", name);
  }

  if (width)
	  *width = columns;
  if (height)
	  *height = rows;

  *pic = targa_rgba;

  ri.FS_FreeFile (buffer.v);
#endif
}
