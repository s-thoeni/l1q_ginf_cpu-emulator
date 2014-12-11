/*

  
  gb.c
  bho1 - init 9.2.2013
  
  Gameboy graphic device
  
  you may have to install libsdl-dev
  sudo apt-get install libsdl1.2-dev

  Compile with  `sdl-config --cflags --libs`
  


  --------> x
  |
  |
  |
  |
  |
  |
  \/
  
  y

  2BPP SNES/Gameboy/GBC
  Colors Per Tile - 0-3
  Space Used - 2 bits per pixel.  16 bytes per 8x8 tile.

  Note: This is a tiled, planar bitmap format.
  Each pair represents one byte
  Format:

  [r0, bp1], [r0, bp2], [r1, bp1], [r1, bp2], [r2, bp1], [r2, bp2], [r3, bp1], [r3, bp2]
  [r4, bp1], [r4, bp2], [r5, bp1], [r5, bp2], [r6, bp1], [r6, bp2], [r7, bp1], [r7, bp2]

  Short Description:

  Bitplanes 1 and 2 are intertwined row by row.

Example:
 Tile:                                     Image:

  .33333..                     .33333.. -> 01111100 -> 7Ch
  22...22.                                 01111100 -> 7Ch
  11...11.                     22...22. -> 00000000 -> 00h
  2222222. <-- digits                      11000110 -> C6h
  33...33.     represent       11...11. -> 11000110 -> C6h
  22...22.     color                       00000000 -> 00h
  11...11.     numbers         2222222. -> 00000000 -> 00h
  ........                                 11111110 -> FEh
                               33...33. -> 11000110 -> C6h
                                           11000110 -> C6h
                               22...22. -> 00000000 -> 00h
                                           11000110 -> C6h
                               11...11. -> 11000110 -> C6h
                                           00000000 -> 00h
                               ........ -> 00000000 -> 00h
                                           00000000 -> 00h
*/

struct tile {
  char* pixelbyte[16];
};

struct gbgraphic {
  char* SCROLLX;
  char* SCROLLY;
  char* LCDC;
  char* WNDPOSX;
  char* WNDPOSY;
  char* WX;
};

// OAM - Object Attribute Memory
$FE00-FE9F
