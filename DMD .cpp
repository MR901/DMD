#include "DMD.h"


DMD::DMD(byte panelsWide, byte panelsHigh)
{
    uint16_t ui;
    DisplaysWide=panelsWide;
    DisplaysHigh=panelsHigh;
    DisplaysTotal=DisplaysWide*DisplaysHigh;
    row1 = DisplaysTotal<<4;
    row2 = DisplaysTotal<<5;
    row3 = ((DisplaysTotal<<2)*3)<<2;
    bDMDScreenRAM = (byte *) malloc(DisplaysTotal*DMD_RAM_SIZE_BYTES);

    // initialize the SPI port
    SPI.begin();		// probably don't need this since it inits the port pins only, which we do just below with the appropriate DMD interface setup
    SPI.setBitOrder(MSBFIRST);	//
    SPI.setDataMode(SPI_MODE0);	// CPOL=0, CPHA=0
    SPI.setClockDivider(SPI_CLOCK_DIV2);	// system clock / 4 = 4MHz SPI CLK to shift registers. If using a short cable, can put SPI_CLOCK_DIV2

here for 2x faster updates

    digitalWrite(PIN_DMD_A, LOW);	//
    digitalWrite(PIN_DMD_B, LOW);	//
    digitalWrite(PIN_DMD_CLK, LOW);	//
    digitalWrite(PIN_DMD_SCLK, LOW);	//
    digitalWrite(PIN_DMD_R_DATA, HIGH);	//
    digitalWrite(PIN_DMD_nOE, LOW);	//

    pinMode(PIN_DMD_A, OUTPUT);	//
    pinMode(PIN_DMD_B, OUTPUT);	//
    pinMode(PIN_DMD_CLK, OUTPUT);	//
    pinMode(PIN_DMD_SCLK, OUTPUT);	//
    pinMode(PIN_DMD_R_DATA, OUTPUT);	//
    pinMode(PIN_DMD_nOE, OUTPUT);	//

    clearScreen(true);

    // init the scan line/ram pointer to the required start point
    bDMDByte = 0;
}

//DMD::~DMD()
//{
//   // nothing needed here
//}

/*--------------------------------------------------------------------------------------
 Set or clear a pixel at the x and y location (0,0 is the top left corner)
--------------------------------------------------------------------------------------*/
void
 DMD::writePixel(unsigned int bX, unsigned int bY, byte bGraphicsMode, byte bPixel)
{
    unsigned int uiDMDRAMPointer;

    if (bX >= (DMD_PIXELS_ACROSS*DisplaysWide) || bY >= (DMD_PIXELS_DOWN * DisplaysHigh)) {
	    return;
    }
    byte panel=(bX/DMD_PIXELS_ACROSS) + (DisplaysWide*(bY/DMD_PIXELS_DOWN));
    bX=(bX % DMD_PIXELS_ACROSS) + (panel<<5);
    bY=bY % DMD_PIXELS_DOWN;
    //set pointer to DMD RAM byte to be modified
    uiDMDRAMPointer = bX/8 + bY*(DisplaysTotal<<2);

    byte lookup = bPixelLookupTable[bX & 0x07];

    switch (bGraphicsMode) {
    case GRAPHICS_NORMAL:
	    if (bPixel == true)
		bDMDScreenRAM[uiDMDRAMPointer] &= ~lookup;	// zero bit is pixel on
	    else
		bDMDScreenRAM[uiDMDRAMPointer] |= lookup;	// one bit is pixel off
	    break;
    case GRAPHICS_INVERSE:
	    if (bPixel == false)
		    bDMDScreenRAM[uiDMDRAMPointer] &= ~lookup;	// zero bit is pixel on
	    else
		    bDMDScreenRAM[uiDMDRAMPointer] |= lookup;	// one bit is pixel off
	    break;
    case GRAPHICS_TOGGLE:
	    if (bPixel == true) {
		if ((bDMDScreenRAM[uiDMDRAMPointer] & lookup) == 0)
		    bDMDScreenRAM[uiDMDRAMPointer] |= lookup;	// one bit is pixel off
		else
		    bDMDScreenRAM[uiDMDRAMPointer] &= ~lookup;	// one bit is pixel off
	    }
	    break;
    case GRAPHICS_OR:
	    //only set pixels on
	    if (bPixel == true)
		    bDMDScreenRAM[uiDMDRAMPointer] &= ~lookup;	// zero bit is pixel on
	    break;
    case GRAPHICS_NOR:
	    //only clear on pixels
	    if ((bPixel == true) &&
		    ((bDMDScreenRAM[uiDMDRAMPointer] & lookup) == 0))
		    bDMDScreenRAM[uiDMDRAMPointer] |= lookup;	// one bit is pixel off
	    break;
    }

}

void DMD::drawString(int bX, int bY, const char *bChars, byte length,
		     byte bGraphicsMode)
{
    if (bX >= (DMD_PIXELS_ACROSS*DisplaysWide) || bY >= DMD_PIXELS_DOWN * DisplaysHigh)
	return;
    uint8_t height = pgm_read_byte(this->Font + FONT_HEIGHT);
    if (bY+height<0) return;

    int strWidth = 0;
	//this->drawLine(bX -1 , bY, bX -1 , bY + height, GRAPHICS_INVERSE);

    for (int i = 0; i < length; i++) {
        int charWide = this->drawChar(bX+strWidth, bY, bChars[i], bGraphicsMode);
	    if (charWide > 0) {
	        strWidth += charWide ;
	        //this->drawLine(bX + strWidth , bY, bX + strWidth , bY + height, GRAPHICS_INVERSE);
            strWidth++;
        } else if (charWide < 0) {
            return;
        }
        if ((bX + strWidth) >= DMD_PIXELS_ACROSS * DisplaysWide || bY >= DMD_PIXELS_DOWN * DisplaysHigh) return;
    }
}

void DMD::drawStringH(int bX, int bY, const char *bChars, byte length,
		     byte bGraphicsMode)
{
    if (bX >= (DMD_PIXELS_ACROSS*DisplaysWide) || bY >= DMD_PIXELS_DOWN * DisplaysHigh)
	return;
    uint8_t height = pgm_read_byte(this->Font + FONT_HEIGHT);
    if (bY+height<0) return;

    int strWidth = 0;
	//this->drawLine(bX -1 , bY, bX -1 , bY + height, GRAPHICS_INVERSE);

    for (int i = 0; i < length; i++) {
	           char b1='\0',b2='\0',b3='\0';

			  int fl=0;
              fl= hin(bChars[i],bChars[i+1],bChars[i+2],bChars[i+3]);
	          //char matra[9]={'h','q','w','s','S','z','Z','a','k'};
//
//              char hf[9]={'E','R','T','Y','U','I','O','?',39};
//              char k[1]={'$'};
//			  if(bChars[i]=='f'){
//                           for(int i1=0;i1<9;i1++){
//							 if(bChars[i+1]==hf[i1]){
//                                    fl=9;
//							      if(bChars[i+2]=='k'){
//								      fl=10;
//									  //for(int i2=0;i2<8;i2++){
////									       if(bChars[i+3]==matra[i2]){
////										       fl=11;
////											    //for(int i3=0;i3<8;i3++){
//////												    if(bChars[i+4]==matra[i3]){
//////													    fl=5;
//////                                                      break;
//////													}
//////												}
////                                             break;
////										   }
////									  }
//                                   break;
//								  }
//						     }
//					       }
//			          if(fl==0){
//                          fl=7;
//					   for(int i1=0;i1<8;i1++){
//                         if(bChars[i+2]==matra[i1]){
//                          fl=8;
//                          break;
//                         }
//					   }
//                     }
//			  }
//
//
//			  else{
//					  for(int i1=0;i1<9;i1++){
//							 if(bChars[i]==hf[i1]){
//                                     fl=3;
//							      if(bChars[i+1]=='k'){
//								      fl=4;
//									  for(int i2=0;i2<9;i2++){
//								       if(bChars[i+2]==matra[i2]){
//										       fl=5;
//											    for(int i3=0;i3<8;i3++){
//												    if(bChars[i+3]==matra[i3]){
//													    fl=6;
//                                                      break;
//													}
//												}
//                                            break;
//										   }
//									  }
//                                   break;
//								  }
//						     }
//					  }
//			       if(fl==0){
//
//					  for(int i1=0;i1<9;i1++){
//                        if(bChars[i+1]==matra[i1]){
//                          fl=1;
//						   for(int i2=0;i2<9;i2++){
//							  if(bChars[i+2]==matra[i2]){
//								   fl=2;
//
//							  }
//						   }
//                          break;
//                         }
//					  }
//                     }
//			  }
                char k[1]={'$'};
			  switch(fl)
			  {
			    case 1: b1=bChars[i+1];
            		    b2=bChars[i];
            		    b3=bChars[i];
				       break;
			    case 2:b1=bChars[i+1];
				       b2=bChars[i+2];
					   b3=bChars[i];
				       break;
				case 3:b1=bChars[i];
					   b2=bChars[i];
					   b3=bChars[i];
				       break;
				case 4:b1=k[0];
				       b2=bChars[i];
                       b3=bChars[i];
				       break;
				case 5:b1=bChars[i+2];
				       b2=k[0];
					   b3=bChars[i];
				       break;
                case 6:b1=bChars[i+2];
                       b2=k[0];
                       b3=bChars[i+3];
                       break;
                case 7:b1=bChars[i+1];
                       b2=bChars[i+1];
                       b3=bChars[i+1];
                       break;
                case 8:b1=bChars[i+1];
                       b2=bChars[i+2];
                       b3=bChars[i+2];
                       break;
                case 9:b1=bChars[i+1];
                       b2=bChars[i+1];
                       b3=bChars[i+1];
                       break;
                case 10:b1=bChars[i+1];
                        b2=k[0];
                        b3=bChars[i+1];
                        break;
                case 11:b1=bChars[i+1];
                        b2=k[0];
                        b3=bChars[i+2];
			    default:b1=bChars[i];
				        b2=bChars[i];
						b3=bChars[i];
				        break;
			  }

        int charWide = this->drawCharH(bX+strWidth, bY, bChars[i],b1,b2,b3, bGraphicsMode);

        switch(fl)
		{
		   case 1:i++;
		          break;
		   case 2:i++;i++;
		          break;
		   case 3:i=i;
		          break;
		   case	4:i++;
		          break;
		   case 5:i++;i++;i++;
		          break;
           case 6:i++;i++;i++;
                  break;
           case 7:i++;
		          break;
           case 8:i++;i++;
		          break;
           case 9:i++;
                  break;
           case 10:i++;i++;
                  break;
           case 11:i++;i++;
                  break;

		}


							if (charWide > 0) {
									strWidth += charWide ;
									//this->drawLine(bX + strWidth , bY, bX + strWidth , bY ,

GRAPHICS_INVERSE);
									//strWidth++;
							} else if (charWide < 0) {
									return;
							  }

        if ((bX + strWidth) >= DMD_PIXELS_ACROSS * DisplaysWide || bY >= DMD_PIXELS_DOWN * DisplaysHigh) return;


    }
}

void DMD::drawMarquee(const char *bChars, byte length, int left, int top)
{
    marqueeWidth = 0;
    for (int i = 0; i < length; i++) {
	    marqueeText[i] = bChars[i];
	    marqueeWidth += charWidth(bChars[i]) + 1;
    }
    marqueeHeight=pgm_read_byte(this->Font + FONT_HEIGHT);
    marqueeText[length] = '\0';
    marqueeOffsetY = top;
    marqueeOffsetX = left;
    marqueeLength = length;
    drawString(marqueeOffsetX, marqueeOffsetY, marqueeText, marqueeLength,
	   GRAPHICS_NORMAL);
}

boolean DMD::stepMarquee(int amountX, int amountY)
{
    boolean ret=false;
    marqueeOffsetX += amountX;
    marqueeOffsetY += amountY;
    if (marqueeOffsetX < -marqueeWidth) {
	    marqueeOffsetX = DMD_PIXELS_ACROSS * DisplaysWide;
	    clearScreen(true);
        ret=true;
    } else if (marqueeOffsetX > DMD_PIXELS_ACROSS * DisplaysWide) {
	    marqueeOffsetX = -marqueeWidth;
	    clearScreen(true);
        ret=true;
    }


    if (marqueeOffsetY < -marqueeHeight) {
	    marqueeOffsetY = DMD_PIXELS_DOWN * DisplaysHigh;
	    clearScreen(true);
        ret=true;
    } else if (marqueeOffsetY > DMD_PIXELS_DOWN * DisplaysHigh) {
	    marqueeOffsetY = -marqueeHeight;
	    clearScreen(true);
        ret=true;
    }

    // Special case horizontal scrolling to improve speed
    if (amountY==0 && amountX==-1) {
        // Shift entire screen one bit
        for (int i=0; i<DMD_RAM_SIZE_BYTES*DisplaysTotal;i++) {
            if ((i%(DisplaysWide*4)) == (DisplaysWide*4) -1) {
                bDMDScreenRAM[i]=(bDMDScreenRAM[i]<<1)+1;
            }
				else {
                bDMDScreenRAM[i]=(bDMDScreenRAM[i]<<1) + ((bDMDScreenRAM[i+1] & 0x80) >>7);
            }
        }

        // Redraw last char on screen
        int strWidth=marqueeOffsetX;
        for (byte i=0; i < marqueeLength; i++) {

            int wide = charWidth(marqueeText[i]);
            if (strWidth+wide >= DisplaysWide*DMD_PIXELS_ACROSS) {
                drawChar(strWidth, marqueeOffsetY,marqueeText[i],GRAPHICS_NORMAL);
                return ret;
            }
            strWidth += wide+1;
        }
    } else if (amountY==0 && amountX==1) {
        // Shift entire screen one bit
        for (int i=(DMD_RAM_SIZE_BYTES*DisplaysTotal)-1; i>=0;i--) {
            if ((i%(DisplaysWide*4)) == 0) {
                bDMDScreenRAM[i]=(bDMDScreenRAM[i]>>1)+128;
            } else {
                bDMDScreenRAM[i]=(bDMDScreenRAM[i]>>1) + ((bDMDScreenRAM[i-1] & 1) <<7);
            }
        }

        // Redraw last char on screen
        int strWidth=marqueeOffsetX;
        for (byte i=0; i < marqueeLength; i++) {
            int wide = charWidth(marqueeText[i]);
            if (strWidth+wide >= 0) {
                drawChar(strWidth, marqueeOffsetY,marqueeText[i],GRAPHICS_NORMAL);
                return ret;
            }
            strWidth += wide+1;
        }
    } else {
        drawString(marqueeOffsetX, marqueeOffsetY, marqueeText, marqueeLength,
	       GRAPHICS_NORMAL);
    }

    return ret;
}

/*--------------------------------------------------------------------------------------
 Clear the screen in DMD RAM
--------------------------------------------------------------------------------------*/
void DMD::clearScreen(byte bNormal)
{
    if (bNormal) // clear all pixels
        memset(bDMDScreenRAM,0xFF,DMD_RAM_SIZE_BYTES*DisplaysTotal);
    else // set all pixels
        memset(bDMDScreenRAM,0x00,DMD_RAM_SIZE_BYTES*DisplaysTotal);
}

/*--------------------------------------------------------------------------------------
 Draw or clear a line from x1,y1 to x2,y2
--------------------------------------------------------------------------------------*/
void DMD::drawLine(int x1, int y1, int x2, int y2, byte bGraphicsMode)
{
    int dy = y2 - y1;
    int dx = x2 - x1;
    int stepx, stepy;

    if (dy < 0) {
	    dy = -dy;
	    stepy = -1;
    } else {
	    stepy = 1;
    }
    if (dx < 0) {
	    dx = -dx;
	    stepx = -1;
    } else {
	    stepx = 1;
    }
    dy <<= 1;			// dy is now 2*dy
    dx <<= 1;			// dx is now 2*dx

    writePixel(x1, y1, bGraphicsMode, true);
    if (dx > dy) {
	    int fraction = dy - (dx >> 1);	// same as 2*dy - dx
	    while (x1 != x2) {
	        if (fraction >= 0) {
		        y1 += stepy;
		        fraction -= dx;	// same as fraction -= 2*dx
	        }
	        x1 += stepx;
	        fraction += dy;	// same as fraction -= 2*dy
	        writePixel(x1, y1, bGraphicsMode, true);
	    }
    } else {
	    int fraction = dx - (dy >> 1);
	    while (y1 != y2) {
	        if (fraction >= 0) {
		        x1 += stepx;
		        fraction -= dy;
	        }
	        y1 += stepy;
	        fraction += dx;
	        writePixel(x1, y1, bGraphicsMode, true);
	    }
    }
}

/*--------------------------------------------------------------------------------------
 Draw or clear a circle of radius r at x,y centre
--------------------------------------------------------------------------------------*/
void DMD::drawCircle(int xCenter, int yCenter, int radius,
		     byte bGraphicsMode)
{
    int x = 0;
    int y = radius;
    int p = (5 - radius * 4) / 4;

    drawCircleSub(xCenter, yCenter, x, y, bGraphicsMode);
    while (x < y) {
	    x++;
	    if (p < 0) {
	        p += 2 * x + 1;
	    } else {
	        y--;
	        p += 2 * (x - y) + 1;
	    }
	    drawCircleSub(xCenter, yCenter, x, y, bGraphicsMode);
    }
}

void DMD::drawCircleSub(int cx, int cy, int x, int y, byte bGraphicsMode)
{

    if (x == 0) {
	    writePixel(cx, cy + y, bGraphicsMode, true);
	    writePixel(cx, cy - y, bGraphicsMode, true);
	    writePixel(cx + y, cy, bGraphicsMode, true);
	    writePixel(cx - y, cy, bGraphicsMode, true);
    } else if (x == y) {
	    writePixel(cx + x, cy + y, bGraphicsMode, true);
	    writePixel(cx - x, cy + y, bGraphicsMode, true);
	    writePixel(cx + x, cy - y, bGraphicsMode, true);
	    writePixel(cx - x, cy - y, bGraphicsMode, true);
    } else if (x < y) {
	    writePixel(cx + x, cy + y, bGraphicsMode, true);
	    writePixel(cx - x, cy + y, bGraphicsMode, true);
	    writePixel(cx + x, cy - y, bGraphicsMode, true);
	    writePixel(cx - x, cy - y, bGraphicsMode, true);
	    writePixel(cx + y, cy + x, bGraphicsMode, true);
	    writePixel(cx - y, cy + x, bGraphicsMode, true);
	    writePixel(cx + y, cy - x, bGraphicsMode, true);
	    writePixel(cx - y, cy - x, bGraphicsMode, true);
    }
}

/*--------------------------------------------------------------------------------------
 Draw or clear a box(rectangle) with a single pixel border
--------------------------------------------------------------------------------------*/
void DMD::drawBox(int x1, int y1, int x2, int y2, byte bGraphicsMode)
{
    drawLine(x1, y1, x2, y1, bGraphicsMode);
    drawLine(x2, y1, x2, y2, bGraphicsMode);
    drawLine(x2, y2, x1, y2, bGraphicsMode);
    drawLine(x1, y2, x1, y1, bGraphicsMode);
}

/*--------------------------------------------------------------------------------------
 Draw or clear a filled box(rectangle) with a single pixel border
--------------------------------------------------------------------------------------*/
void DMD::drawFilledBox(int x1, int y1, int x2, int y2,
			byte bGraphicsMode)
{
    for (int b = x1; b <= x2; b++) {
	    drawLine(b, y1, b, y2, bGraphicsMode);
    }
}

/*--------------------------------------------------------------------------------------
 Draw the selected test pattern
--------------------------------------------------------------------------------------*/
void DMD::drawTestPattern(byte bPattern)
{
    unsigned int ui;

    int numPixels=DisplaysTotal * DMD_PIXELS_ACROSS * DMD_PIXELS_DOWN;
    int pixelsWide=DMD_PIXELS_ACROSS*DisplaysWide;
    for (ui = 0; ui < numPixels; ui++) {
	    switch (bPattern) {
	    case PATTERN_ALT_0:	// every alternate pixel, first pixel on
		    if ((ui & pixelsWide) == 0)
		        //even row
		        writePixel((ui & (pixelsWide-1)), ((ui & ~(pixelsWide-1)) / pixelsWide), GRAPHICS_NORMAL, ui & 1);
		    else
		        //odd row
		        writePixel((ui & (pixelsWide-1)), ((ui & ~(pixelsWide-1)) / pixelsWide), GRAPHICS_NORMAL, !(ui & 1));
		    break;
	    case PATTERN_ALT_1:	// every alternate pixel, first pixel off
		    if ((ui & pixelsWide) == 0)
		        //even row
		        writePixel((ui & (pixelsWide-1)), ((ui & ~(pixelsWide-1)) / pixelsWide), GRAPHICS_NORMAL, !(ui & 1));
		    else
		        //odd row
		        writePixel((ui & (pixelsWide-1)), ((ui & ~(pixelsWide-1)) / pixelsWide), GRAPHICS_NORMAL, ui & 1);
		    break;
	    case PATTERN_STRIPE_0:	// vertical stripes, first stripe on
		    writePixel((ui & (pixelsWide-1)), ((ui & ~(pixelsWide-1)) / pixelsWide), GRAPHICS_NORMAL, ui & 1);
		    break;
	    case PATTERN_STRIPE_1:	// vertical stripes, first stripe off
		    writePixel((ui & (pixelsWide-1)), ((ui & ~(pixelsWide-1)) / pixelsWide), GRAPHICS_NORMAL, !(ui & 1));
		    break;
        }
    }
}

/*--------------------------------------------------------------------------------------
 Scan the dot matrix LED panel display, from the RAM mirror out to the display hardware.
 Call 4 times to scan the whole display which is made up of 4 interleaved rows within the 16 total rows.
 Insert the calls to this function into the main loop for the highest call rate, or from a timer interrupt
--------------------------------------------------------------------------------------*/
void DMD::scanDisplayBySPI()
{
    //if PIN_OTHER_SPI_nCS is in use during a DMD scan request then scanDisplayBySPI() will exit without conflict! (and skip that scan)
    if( digitalRead( PIN_OTHER_SPI_nCS ) == HIGH )
    {
        //SPI transfer pixels to the display hardware shift registers
        int rowsize=DisplaysTotal<<2;
        int offset=rowsize * bDMDByte;
        for (int i=0;i<rowsize;i++) {
            SPI.transfer(bDMDScreenRAM[offset+i+row3]);
            SPI.transfer(bDMDScreenRAM[offset+i+row2]);
            SPI.transfer(bDMDScreenRAM[offset+i+row1]);
            SPI.transfer(bDMDScreenRAM[offset+i]);
        }

        OE_DMD_ROWS_OFF();
        LATCH_DMD_SHIFT_REG_TO_OUTPUT();
        switch (bDMDByte) {
        case 0:			// row 1, 5, 9, 13 were clocked out
            LIGHT_DMD_ROW_01_05_09_13();
            bDMDByte=1;
            break;
        case 1:			// row 2, 6, 10, 14 were clocked out
            LIGHT_DMD_ROW_02_06_10_14();
            bDMDByte=2;
            break;
        case 2:			// row 3, 7, 11, 15 were clocked out
            LIGHT_DMD_ROW_03_07_11_15();
            bDMDByte=3;
            break;
        case 3:			// row 4, 8, 12, 16 were clocked out
            LIGHT_DMD_ROW_04_08_12_16();
            bDMDByte=0;
            break;
        }
        OE_DMD_ROWS_ON();
    }
}

void DMD::selectFont(const uint16_t * font)
{
    this->Font = font;
}


int DMD::drawChar(const int bX, const int bY, const unsigned char letter, byte bGraphicsMode)
{
    if (bX > (DMD_PIXELS_ACROSS*DisplaysWide) || bY > (DMD_PIXELS_DOWN*DisplaysHigh) ) return -1;
    unsigned char c = letter;
    uint8_t height = pgm_read_byte(this->Font + FONT_HEIGHT);

    if (c == ' ') {
	    int charWide = charWidth(' ');
	    this->drawFilledBox(bX, bY, bX + charWide, bY + height, GRAPHICS_INVERSE);
	    return charWide;
    }
    uint8_t width = 0;
    uint8_t bytes = (height + 7) / 8;

    uint8_t firstChar = pgm_read_byte(this->Font + FONT_FIRST_CHAR);
    uint8_t charCount
    = pgm_read_byte(this->Font + FONT_CHAR_COUNT);

    uint16_t index = 0;

    if (c < firstChar || c >= (firstChar + charCount)) return 0;
    c -= firstChar;


    if (pgm_read_byte(this->Font + FONT_LENGTH) == 0
	    && pgm_read_byte(this->Font + FONT_LENGTH + 1) == 0) {
	    // zero length is flag indicating fixed width font (array does not contain width data entries)
	    width = pgm_read_byte(this->Font + FONT_FIXED_WIDTH);
	    index = c * bytes * width + FONT_WIDTH_TABLE;
    } else {
	    // variable width font, read width data, to get the index
	    for (uint8_t i = 0; i < c; i++) {
	        index += pgm_read_byte(this->Font + FONT_WIDTH_TABLE + i);
	    }
	    index = index * bytes + charCount + FONT_WIDTH_TABLE;
	    width = pgm_read_byte(this->Font + FONT_WIDTH_TABLE + c);
    }
    if (bX < -width || bY < -height) return width;



    // last but not least, draw the character
    for (uint8_t j = 0; j < width; j++) { // Width
	    for (uint8_t i = bytes - 1; i < 254; i--) { // Vertical Bytes
	        uint8_t data = pgm_read_byte(this->Font + index + j + (i * width));
            uint8_t data1= 0;

		    int offset = (i * 8);
		    if ((i == bytes - 1) && bytes > 1) {
		        offset = height - 8;
            }
	        for (uint8_t k = 0; k < 8; k++) { // Vertical bits
		        if ((offset+k >= i*8) && (offset+k <= height)) {
		            if (data & (1 << k)) {
			            writePixel(bX + j, bY + offset + k, bGraphicsMode, true);
		            } else {
			            writePixel(bX + j, bY + offset + k, bGraphicsMode, false);
		            }
		        }
	        }
	    }
    }
    return width;

}

int DMD::drawCharH(const int bX, const int bY, const unsigned char letter,const unsigned char letter1, const unsigned char letter2, const unsigned char

letter3,  byte bGraphicsMode)
{
    if (bX > (DMD_PIXELS_ACROSS*DisplaysWide) || bY > (DMD_PIXELS_DOWN*DisplaysHigh) ) return -1;
    unsigned char c = letter;
	unsigned char c1 = letter1;
    unsigned char c2 = letter2;
	unsigned char c3 = letter3;
    uint8_t height = pgm_read_byte(this->Font + FONT_HEIGHT);
    if (c == ' ') {
	    int charWide = charWidth(' ');
	    this->drawFilledBox(bX, bY, bX + charWide, bY + height, GRAPHICS_INVERSE);
	    return charWide;
    }
    uint8_t width = 0;uint8_t width1 = 0;uint8_t width2 = 0;uint8_t width3 = 0;
    uint8_t bytes = (height + 7) / 8;

    uint8_t firstChar = pgm_read_byte(this->Font + FONT_FIRST_CHAR);
    uint8_t charCount = pgm_read_byte(this->Font + FONT_CHAR_COUNT);

    uint16_t index = 0;uint16_t index1 = 0;uint16_t index2 = 0;uint16_t index3 = 0;

    if (c < firstChar || c >= (firstChar + charCount)) return 0;
    c -= firstChar;c1 -= firstChar;c2 -= firstChar;c3 -= firstChar;

    if (pgm_read_byte(this->Font + FONT_LENGTH) == 0
	    && pgm_read_byte(this->Font + FONT_LENGTH + 1) == 0) {
	    // zero length is flag indicating fixed width font (array does not contain width data entries)
	    width = pgm_read_byte(this->Font + FONT_FIXED_WIDTH);
	    index = c * bytes * width + FONT_WIDTH_TABLE;

		width1 = pgm_read_byte(this->Font + FONT_FIXED_WIDTH);
	    index1 = c1 * bytes * width1 + FONT_WIDTH_TABLE;

	    width2 = pgm_read_byte(this->Font + FONT_FIXED_WIDTH);
	    index2 = c2 * bytes * width2 + FONT_WIDTH_TABLE;
		width3 = pgm_read_byte(this->Font + FONT_FIXED_WIDTH);
	    index3 = c3 * bytes * width3 + FONT_WIDTH_TABLE;
    } else {
	    // variable width font, read width data, to get the index
	    for (uint8_t i = 0; i < c; i++) {
	        index += pgm_read_byte(this->Font + FONT_WIDTH_TABLE + i);
	    }
	    index = index * bytes + charCount + FONT_WIDTH_TABLE;
	    width = pgm_read_byte(this->Font + FONT_WIDTH_TABLE + c);

		for (uint8_t i1 = 0; i1 < c1; i1++) {
	        index1 += pgm_read_byte(this->Font + FONT_WIDTH_TABLE + i1);
	    }
	    index1 = index1 * bytes + charCount + FONT_WIDTH_TABLE;
	    width1 = pgm_read_byte(this->Font + FONT_WIDTH_TABLE + c1);

		for (uint8_t i2 = 0; i2 < c2; i2++) {
	        index2 += pgm_read_byte(this->Font + FONT_WIDTH_TABLE + i2);
	    }
	    index2 = index2 * bytes + charCount + FONT_WIDTH_TABLE;
	    width2 = pgm_read_byte(this->Font + FONT_WIDTH_TABLE + c2);

	for (uint8_t i3 = 0; i3 < c3; i3++) {
	        index3 += pgm_read_byte(this->Font + FONT_WIDTH_TABLE + i3);
	    }
	    index3 = index3 * bytes + charCount + FONT_WIDTH_TABLE;
	    width3 = pgm_read_byte(this->Font + FONT_WIDTH_TABLE + c3);
    }
    if (bX < -width || bY < -height) return width;


    // last but not least, draw the character

if(letter=='f'){


       for (uint8_t j = 0; j < width1; j++) { // Width
	    for (uint8_t i = bytes - 1; i < 254; i--) { // Vertical Bytes
		    uint8_t data;uint8_t data1;uint8_t data2;uint8_t data3=0;
			 if(j<width){
                data = pgm_read_byte(this->Font + index + j + (i * width));
			 }
			 if(j<width1){
                 data1 = pgm_read_byte(this->Font + index1 + j + (i * width1));
			 }




			 if(j<width2){
			      data2 = pgm_read_byte(this->Font + index2 + j + (i * width2));
			 }
			 if(j<width3){
			      data3 = pgm_read_byte(this->Font + index3 + j + (i * width3));
		     }

		    int offset = (i * 8);
		    if ((i == bytes - 1) && bytes > 1) {
		        offset = height - 8;
            }
	        for (uint8_t k = 0; k < 8; k++) { // Vertical bits
		        if ((offset+k >= i*8) && (offset+k <= height)) {
                     bool y=data &(1<<k) ;


                    if(j<width1){
                         y=y||data1 &(1<<k);
                    }
                    if(j<=width2){
                        y=y||data2 &(1<<k);
                    }
                    if(j<width3){
                        y=y||data3 &(1<<k);
                    }

                    if(y) {
			            writePixel(bX + j, bY + offset + k, bGraphicsMode, true);
		            } else {
			            writePixel(bX + j, bY + offset + k, bGraphicsMode, false);
		            }
		        }
	        }
	    }

    }

  return width;
}

else{


    for (uint8_t j = 0; j < width1; j++) { // Width
	    for (uint8_t i = bytes - 1; i < 254; i--) { // Vertical Bytes
		    uint8_t data;uint8_t data1;uint8_t data2;uint8_t data3=0;
			 if(j<width){
                data = pgm_read_byte(this->Font + index + j + (i * width));
			 }
			 if(j<width1){
                 data1 = pgm_read_byte(this->Font + index1 + j + (i * width1));
			 }

			 if(j<width2){
			      data2 = pgm_read_byte(this->Font + index2 + j + (i * width2));
			 }
			 if(j<width3){
			      data3 = pgm_read_byte(this->Font + index3 + j + (i * width3));
		     }

		    int offset = (i * 8);
		    if ((i == bytes - 1) && bytes > 1) {
		        offset = height - 8;
            }
	        for (uint8_t k = 0; k < 8; k++) { // Vertical bits
		        if ((offset+k >= i*8) && (offset+k <= height)) {
                     bool y=data1 &(1<<k) ;
                    if(j<width){
                         y=y||data &(1<<k);
                    }
                    if(j<width2){
                        y=y||data2 &(1<<k);
                    }
                    if(j<width3){
                        y=y||data3 &(1<<k);
                    }

                    if(y) {
			            writePixel(bX + j, bY + offset + k, bGraphicsMode, true);
		            } else {
			            writePixel(bX + j, bY + offset + k, bGraphicsMode, false);
		            }
		        }
	        }
	    }
    }


      return width1;
   }
}

int DMD::charWidth(const unsigned char letter)
{
    unsigned char c = letter;
    // Space is often not included in font so use width of 'n'
    if (c == ' ') c = ']';
    uint8_t width = 0;

    uint8_t firstChar = pgm_read_byte(this->Font + FONT_FIRST_CHAR);
    uint8_t charCount = pgm_read_byte(this->Font + FONT_CHAR_COUNT);

    uint16_t index = 0;

    if (c < firstChar || c >= (firstChar + charCount)) {
	    return 0;
    }
    c -= firstChar;

    if (pgm_read_byte(this->Font + FONT_LENGTH) == 0
	&& pgm_read_byte(this->Font + FONT_LENGTH + 1) == 0) {
	    // zero length is flag indicating fixed width font (array does not contain width data entries)
	    width = pgm_read_byte(this->Font + FONT_FIXED_WIDTH);
    } else {
	    // variable width font, read width data
	    width = pgm_read_byte(this->Font + FONT_WIDTH_TABLE + c);
    }
    return width;
}

int DMD::charWidthH(const unsigned char letter,const unsigned char letter1,const unsigned char letter2,const unsigned char letter3)
{
    unsigned char c = letter;
    unsigned char c1 = letter1;
    unsigned char c2 = letter2;
    unsigned char c3 = letter3;
    // Space is often not included in font so use width of 'n'
    if (c == ' ') c = ']';
    uint8_t width = 0;uint8_t width1 = 0;uint8_t width2 = 0;uint8_t width3 = 0;uint8_t w = 0;

    uint8_t firstChar = pgm_read_byte(this->Font + FONT_FIRST_CHAR);
    uint8_t charCount = pgm_read_byte(this->Font + FONT_CHAR_COUNT);

    uint16_t index = 0;uint16_t index1 = 0;uint16_t index2 = 0;uint16_t index3 = 0;

    if (c < firstChar || c >= (firstChar + charCount)) {
	    return 0;
    }
    c -= firstChar;c1 -= firstChar;c2 -= firstChar;c3 -= firstChar;

    if (pgm_read_byte(this->Font + FONT_LENGTH) == 0
	&& pgm_read_byte(this->Font + FONT_LENGTH + 1) == 0) {
	    // zero length is flag indicating fixed width font (array does not contain width data entries)
	    width = pgm_read_byte(this->Font + FONT_FIXED_WIDTH);
    } else {
	    // variable width font, read width data
	    width = pgm_read_byte(this->Font + FONT_WIDTH_TABLE + c);
        width1 = pgm_read_byte(this->Font + FONT_WIDTH_TABLE + c1);
        width2 = pgm_read_byte(this->Font + FONT_WIDTH_TABLE + c2);
        width3 = pgm_read_byte(this->Font + FONT_WIDTH_TABLE + c3);

    }

      if(width<=width1){
         w=width1;
      } else{
        w=width;
      }

    if(w<=width2){
         w=width2;
    }
    if(w<=width3){
       w=width3;
    }
    return w;
}


boolean DMD::stepMarqueeH(int amountX, int amountY)
{
    boolean ret=false;
    marqueeOffsetX += amountX;
    marqueeOffsetY += amountY;
    if (marqueeOffsetX < -marqueeWidth) {
	    marqueeOffsetX = DMD_PIXELS_ACROSS * DisplaysWide;
	    clearScreen(true);
        ret=true;
    } else if (marqueeOffsetX > DMD_PIXELS_ACROSS * DisplaysWide) {
	    marqueeOffsetX = -marqueeWidth;
	    clearScreen(true);
        ret=true;
    }


    if (marqueeOffsetY < -marqueeHeight) {
	    marqueeOffsetY = DMD_PIXELS_DOWN * DisplaysHigh;
	    clearScreen(true);
        ret=true;
    } else if (marqueeOffsetY > DMD_PIXELS_DOWN * DisplaysHigh) {
	    marqueeOffsetY = -marqueeHeight;
	    clearScreen(true);
        ret=true;
    }

    // Special case horizontal scrolling to improve speed
    if (amountY==0 && amountX==-1) {
        // Shift entire screen one bit
        for (int i=0; i<DMD_RAM_SIZE_BYTES*DisplaysTotal;i++) {
            if ((i%(DisplaysWide*4)) == (DisplaysWide*4) -1) {
                bDMDScreenRAM[i]=(bDMDScreenRAM[i]<<1)+1;
            } else {
                bDMDScreenRAM[i]=(bDMDScreenRAM[i]<<1) + ((bDMDScreenRAM[i+1] & 0x80) >>7);
            }
        }

        // Redraw last char on screen
        int strWidth=marqueeOffsetX;
        for (byte i=0; i < marqueeLength; i++) {
              char b1='\0',b2='\0',b3='\0';

			  int fl=0;
              fl= hin(marqueeText[i],marqueeText[i+1],marqueeText[i+2],marqueeText[i+3]);
                 char k[1]={'$'};
			  switch(fl)
			  {
			    case 1: b1=marqueeText[i+1];
            		    b2=marqueeText[i];
            		    b3=marqueeText[i];
				       break;
			    case 2:b1=marqueeText[i+1];
				       b2=marqueeText[i+2];
					   b3=marqueeText[i];
				       break;
				case 3:b1=marqueeText[i];
					   b2=marqueeText[i];
					   b3=marqueeText[i];
				       break;
				case 4:b1=k[0];
				       b2=marqueeText[i];
                       b3=marqueeText[i];
				       break;
				case 5:b1=marqueeText[i+2];
				       b2=k[0];
					   b3=marqueeText[i];
				       break;
                case 6:b1=marqueeText[i+2];
                       b2=k[0];
                       b3=marqueeText[i+3];
                       break;
                case 7:b1=marqueeText[i+1];
                       b2=marqueeText[i+1];
                       b3=marqueeText[i+1];
                       break;
                case 8:b1=marqueeText[i+1];
                       b2=marqueeText[i+2];
                       b3=marqueeText[i+2];
                       break;
                case 9:b1=marqueeText[i+1];
                       b2=marqueeText[i+1];
                       b3=marqueeText[i+1];
                       break;
                case 10:b1=marqueeText[i+1];
                        b2=k[0];
                        b3=marqueeText[i+1];
                        break;
                case 11:b1=marqueeText[i+1];
                        b2=k[0];
                        b3=marqueeText[i+2];
			    default:b1=marqueeText[i];
				        b2=marqueeText[i];
						b3=marqueeText[i];
				        break;
			  }


            int wide = charWidthH(marqueeText[i],b1,b2,b3);
            if (strWidth+wide >= DisplaysWide*DMD_PIXELS_ACROSS) {
                drawCharH(strWidth, marqueeOffsetY,marqueeText[i],b1,b2,b3,GRAPHICS_NORMAL);
                return ret;
            }
            strWidth += wide;

            switch(fl)
		  {
		   case 1:i++;
		          break;
		   case 2:i++;i++;
		          break;
		   case 3:i=i;
		          break;
		   case	4:i++;
		          break;
		   case 5:i++;i++;i++;
		          break;
           case 6:i++;i++;i++;
                  break;
           case 7:i++;
		          break;
           case 8:i++;i++;
		          break;
           case 9:i++;
                  break;
           case 10:i++;i++;
                  break;
           case 11:i++;i++;
                  break;

		  }

        }
    }

    return ret;
}
int DMD::hin(const unsigned char letter,const unsigned char letter1, const unsigned char letter2, const unsigned char letter3)
{
    int fl=0;
         char matra[9]={'h','q','w','s','S','z','Z','a','k'};

              char hf[10]={'E','R','T','Y','U','I','O','?',39,'/'};

			  if(letter=='f'){
                           for(int i1=0;i1<10;i1++){
							 if(letter1==hf[i1]){
                                    fl=9;
							      if(letter2=='k'){
								      fl=10;

                                   break;
								  }
						     }
					       }
			          if(fl==0){
                          fl=7;
					   for(int i1=0;i1<8;i1++){
                         if(letter2==matra[i1]){
                          fl=8;
1                          break;
                         }
					   }
                     }
			  }


			  else{
					  for(int i1=0;i1<10;i1++){
							 if(letter==hf[i1]){
                                     fl=3;
							      if(letter1=='k'){
								      fl=4;
									  for(int i2=0;i2<9;i2++){
								       if(letter2==matra[i2]){
										       fl=5;
											    for(int i3=0;i3<8;i3++){
												    if(letter3==matra[i3]){
													    fl=6;
                                                      break;
													}
												}
                                            break;
										   }
									  }
                                   break;
								  }
						     }
					  }
			       if(fl==0){

					  for(int i1=0;i1<9;i1++){
                        if(letter1==matra[i1]){
                          fl=1;
						   for(int i2=0;i2<9;i2++){
							  if(letter2==matra[i2]){
								   fl=2;

							  }
						   }
                          break;
                         }
					  }
                     }
			  }
    return fl;
}
