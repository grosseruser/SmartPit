/***************************************************
  This is our library for the Adafruit ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#include "Adafruit_ILI9341.h"
#include <pgmspace.h>
#include <limits.h>
#include "pins_arduino.h"
#include "wiring_private.h"
#include <SPI.h>


// If the SPI library has transaction support, these functions
// establish settings and protect from interference from other
// libraries.  Otherwise, they simply do nothing.
static inline void spi_begin(void) __attribute__((always_inline));
static inline void spi_begin(void) {
	// max speed!
	SPI.beginTransaction(SPISettings(80000000, MSBFIRST, SPI_MODE0));
	//SPI.setClockDivider(0x00000001);
	yield();
}
static inline void spi_end(void) __attribute__((always_inline));
static inline void spi_end(void) {
	SPI.endTransaction();
}


// Constructor when using hardware SPI.  Faster, but must use SPI pins
// specific to each board type (e.g. 11,13 for Uno, 51,52 for Mega, etc.)
Adafruit_ILI9341::Adafruit_ILI9341(int8_t cs, int8_t dc, int8_t rst) : Adafruit_GFX(ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT) {
  _cs   = cs;
  _dc   = dc;
  _rst  = rst;
  hwSPI = true;
  _mosi  = _sclk = 0;
}

void Adafruit_ILI9341::setDataBits(uint16_t bits) {
    const uint32_t mask = ~((SPIMMOSI << SPILMOSI) | (SPIMMISO << SPILMISO));
    bits--;
    SPI1U1 = ((SPI1U1 & mask) | ((bits << SPILMOSI) | (bits << SPILMISO)));
}

void Adafruit_ILI9341::writecommand(uint8_t c) {
	digitalWrite(_dc, LOW);
	digitalWrite(_sclk, LOW);
	digitalWrite(_cs, LOW);
	SPI.write(c);
	digitalWrite(_cs, HIGH);
}

void Adafruit_ILI9341::writedata(uint8_t c) {
	digitalWrite(_dc, HIGH);
	digitalWrite(_sclk, LOW);
	digitalWrite(_cs, LOW);
	SPI.write(c);
	digitalWrite(_cs, HIGH);
} 

void Adafruit_ILI9341::writedata16(uint16_t c) {
	digitalWrite(_dc, HIGH);
	digitalWrite(_sclk, LOW);
	digitalWrite(_cs, LOW);
	SPI.write16(c);
	digitalWrite(_cs, HIGH);
} 

void Adafruit_ILI9341::writedata32(uint32_t c) {
	digitalWrite(_dc, HIGH);
	digitalWrite(_sclk, LOW);
	digitalWrite(_cs, LOW);
	SPI.write32(c);
	digitalWrite(_cs, HIGH);
} 




void Adafruit_ILI9341::begin(void) {

	pinMode(_dc, OUTPUT);
	pinMode(_cs, OUTPUT);
	
    SPI.begin();
  
	spi_begin();
	writecommand(0xEF);
	writedata16((0x03<<8)|(0x80));
	writedata(0x02);

	writecommand(0xCF);  
	writedata16((0x00<<8)|(0XC1)); 
	writedata(0X30); 

	writecommand(0xED);  
	writedata32((0x64<<24)|(0x03<<16)|(0X12<<8)|(0X81)); 
 
	writecommand(0xE8);  
	writedata16((0x85<<8)|(0x00)); 
	writedata(0x78); 

	writecommand(0xCB);  
	writedata32((0x39<<24)|(0x2C<<16)|(0x00<<8)|(0x34)); 
	writedata(0x02); 
 
	writecommand(0xF7);  
	writedata(0x20); 

	writecommand(0xEA);  
	writedata16((0x00<<8)|(0x00)); 
 
	writecommand(ILI9341_PWCTR1);    //Power control 
	writedata(0x23);   //VRH[5:0] 
 
	writecommand(ILI9341_PWCTR2);    //Power control 
	writedata(0x10);   //SAP[2:0];BT[3:0] 
 
	writecommand(ILI9341_VMCTR1);    //VCM control 
	writedata16((0x3e<<8)|(0x28)); //¶Ô±È¶Èµ÷½Ú
  
	writecommand(ILI9341_VMCTR2);    //VCM control2 
	writedata(0x86);  //--
 
	writecommand(ILI9341_MADCTL);    // Memory Access Control 
	writedata(0x48);

	writecommand(ILI9341_PIXFMT);    
	writedata(0x55); 
  
	writecommand(ILI9341_FRMCTR1);    
	writedata16((0x00<<8)|(0x18));  
	
	writecommand(ILI9341_DFUNCTR);    // Display Function Control 
	writedata16((0x08<<8)|(0x82)); 
	writedata(0x27);  
	
	writecommand(0xF2);    // 3Gamma Function Disable 
	writedata(0x00); 
	
	writecommand(ILI9341_GAMMASET);    //Gamma curve selected 
	writedata(0x01); 
	
	writecommand(ILI9341_GMCTRP1);    //Set Gamma 
	writedata32((0x0F<<24)|(0x31<<16)|(0x2B<<8)|(0x0C)); 
	writedata32((0x0E<<24)|(0x08<<16)|(0x4E<<8)|(0xF1)); 
	writedata32((0x37<<24)|(0x07<<16)|(0x10<<8)|(0x03)); 
	writedata16((0x0E<<8)|(0x09)); 
	writedata(0x00); 
	
	writecommand(ILI9341_GMCTRN1);    //Set Gamma 
	writedata32((0x00<<24)|(0x0E<<16)|(0x14<<8)|(0x03)); 
	writedata32((0x11<<24)|(0x07<<16)|(0x31<<8)|(0xC1)); 
	writedata32((0x48<<24)|(0x08<<16)|(0x0F<<8)|(0x0C)); 
	writedata16((0x31<<8)|(0x36)); 
	writedata(0x0F); 

	writecommand(ILI9341_SLPOUT);    //Exit Sleep 
	spi_end();
	delay(120); 		
	spi_begin();
	writecommand(ILI9341_DISPON);    //Display on 
	spi_end();

}


void Adafruit_ILI9341::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
	writecommand(ILI9341_CASET); // Column addr set
	writedata32((x0<<16)|x1);
	writecommand(ILI9341_PASET); // Row addr set
	writedata32((y0<<16)|y1);
	writecommand(ILI9341_RAMWR); // write to RAM
}

void Adafruit_ILI9341::pushColor(uint16_t color) {
	spi_begin();
	digitalWrite(_dc, HIGH);
	digitalWrite(_cs, LOW);
	SPI.write16(color);
	digitalWrite(_cs, HIGH);
	spi_end();
}

void Adafruit_ILI9341::drawPixel(int16_t x, int16_t y, uint16_t color) {

	if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) return;
	
	spi_begin();
	setAddrWindow(x,y,x+1,y+1);
	digitalWrite(_dc, HIGH);
	digitalWrite(_cs, LOW);
	SPI.write16(color);
	digitalWrite(_cs, HIGH);
	spi_end();
}


void Adafruit_ILI9341::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {

	// Rudimentary clipping
	if((x >= _width) || (y >= _height)) return;
	if((y+h-1) >= _height)  h = _height-y;
	
	spi_begin();
	setAddrWindow(x, y, x, y+h-1);
	digitalWrite(_dc, HIGH);
	digitalWrite(_cs, LOW);
	
	while(SPI1CMD & SPIBUSY) {}
	uint32_t c32 = (color<<16)|color;
	
	if(h%2){
		setDataBits(16);
		SPI1W0 = color;
		SPI1CMD |= SPIBUSY;
		while(SPI1CMD & SPIBUSY) {}
		h--;
	}
	setDataBits(32);
	while(h){
		SPI1W0 = c32;
		SPI1CMD |= SPIBUSY;
		while(SPI1CMD & SPIBUSY) {}
		h-=2;
	}

	
	digitalWrite(_cs, HIGH);
	spi_end();
}


void Adafruit_ILI9341::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {

	// Rudimentary clipping
	if((x >= _width) || (y >= _height)) return;
	if((x+w-1) >= _width)  w = _width-x;
		
	spi_begin();
	setAddrWindow(x, y, x+w-1, y);
	digitalWrite(_dc, HIGH);
	digitalWrite(_cs, LOW);
	
	while(SPI1CMD & SPIBUSY) {}
	uint32_t c32 = (color<<16)|color;
	
	if(w%2){
		setDataBits(16);
		SPI1W0 = color;
		SPI1CMD |= SPIBUSY;
		while(SPI1CMD & SPIBUSY) {}
		w--;
	}
	setDataBits(32);
	while(w){
		SPI1W0 = c32;
		SPI1CMD |= SPIBUSY;
		while(SPI1CMD & SPIBUSY) {}
		w-=2;
	}

	digitalWrite(_cs, HIGH);
	spi_end();
}

void Adafruit_ILI9341::fillScreen(uint16_t color) {
	fillRect(0, 0,  _width, _height, color);
}

// fill a rectangle
void Adafruit_ILI9341::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {

	// rudimentary clipping (drawChar w/big text requires this)
	if((x >= _width) || (y >= _height)) return;
	if((x + w - 1) >= _width)  w = _width  - x;
	if((y + h - 1) >= _height) h = _height - y;
	
	spi_begin();
	setAddrWindow(x, y, x+w-1, y+h-1);
	digitalWrite(_dc, HIGH);
	digitalWrite(_cs, LOW);
	
	while(SPI1CMD & SPIBUSY) {}
	uint32_t cn = h * w;
	uint32_t c32 = (color<<16)|color;
	
	if(cn%2){
		setDataBits(16);
		SPI1W0 = color;
		SPI1CMD |= SPIBUSY;
		while(SPI1CMD & SPIBUSY) {}
		cn--;
	}
	setDataBits(32);
	while(cn){
		SPI1W0 = c32;
		SPI1CMD |= SPIBUSY;
		while(SPI1CMD & SPIBUSY) {}
		cn-=2;
	}
	
	digitalWrite(_cs, HIGH);
	spi_end();
}


// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t Adafruit_ILI9341::color565(uint8_t r, uint8_t g, uint8_t b) {
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}


#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

void Adafruit_ILI9341::setRotation(uint8_t m) {

	spi_begin();
	writecommand(ILI9341_MADCTL);
	rotation = m % 4; // can't be higher than 3
	switch (rotation) {
		case 0:
			writedata(MADCTL_MX | MADCTL_BGR);
			_width  = ILI9341_TFTWIDTH;
			_height = ILI9341_TFTHEIGHT;
		break;
		case 1:
			writedata(MADCTL_MV | MADCTL_BGR);
			_width  = ILI9341_TFTHEIGHT;
			_height = ILI9341_TFTWIDTH;
		break;
		case 2:
			writedata(MADCTL_MY | MADCTL_BGR);
			_width  = ILI9341_TFTWIDTH;
			_height = ILI9341_TFTHEIGHT;
		break;
		case 3:
			writedata(MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
			_width  = ILI9341_TFTHEIGHT;
			_height = ILI9341_TFTWIDTH;
		break;
	}
	spi_end();
}


void Adafruit_ILI9341::invertDisplay(boolean i) {
	spi_begin();
	writecommand(i ? ILI9341_INVON : ILI9341_INVOFF);
	spi_end();
}


////////// stuff not actively being used, but kept for posterity


uint8_t Adafruit_ILI9341::spiread(void) {
  uint8_t r = 0;

  if (hwSPI) {
#if defined (__AVR__)
  #ifndef SPI_HAS_TRANSACTION
    uint8_t backupSPCR = SPCR;
    SPCR = mySPCR;
  #endif
    SPDR = 0x00;
    while(!(SPSR & _BV(SPIF)));
    r = SPDR;

  #ifndef SPI_HAS_TRANSACTION
    SPCR = backupSPCR;
  #endif
#else
    r = SPI.transfer(0x00);
#endif

  } else {

    for (uint8_t i=0; i<8; i++) {
      digitalWrite(_sclk, LOW);
      digitalWrite(_sclk, HIGH);
      r <<= 1;
      if (digitalRead(_miso))
	r |= 0x1;
    }
  }
  //Serial.print("read: 0x"); Serial.print(r, HEX);
  
  return r;
}

 uint8_t Adafruit_ILI9341::readdata(void) {
   digitalWrite(_dc, HIGH);
   digitalWrite(_cs, LOW);
   uint8_t r = spiread();
   digitalWrite(_cs, HIGH);
   
   return r;
}
 

uint8_t Adafruit_ILI9341::readcommand8(uint8_t c, uint8_t index) {
   if (hwSPI) spi_begin();
   digitalWrite(_dc, LOW); // command
   digitalWrite(_cs, LOW);
   SPI.write(0xD9);  // woo sekret command?
   digitalWrite(_dc, HIGH); // data
   SPI.write(0x10 + index);
   digitalWrite(_cs, HIGH);

   digitalWrite(_dc, LOW);
   digitalWrite(_sclk, LOW);
   digitalWrite(_cs, LOW);
   SPI.write(c);
 
   digitalWrite(_dc, HIGH);
   uint8_t r = spiread();
   digitalWrite(_cs, HIGH);
   if (hwSPI) spi_end();
   return r;
}


 
/*

 uint16_t Adafruit_ILI9341::readcommand16(uint8_t c) {
 digitalWrite(_dc, LOW);
 if (_cs)
 digitalWrite(_cs, LOW);
 
 spiwrite(c);
 pinMode(_sid, INPUT); // input!
 uint16_t r = spiread();
 r <<= 8;
 r |= spiread();
 if (_cs)
 digitalWrite(_cs, HIGH);
 
 pinMode(_sid, OUTPUT); // back to output
 return r;
 }
 
 uint32_t Adafruit_ILI9341::readcommand32(uint8_t c) {
 digitalWrite(_dc, LOW);
 if (_cs)
 digitalWrite(_cs, LOW);
 spiwrite(c);
 pinMode(_sid, INPUT); // input!
 
 dummyclock();
 dummyclock();
 
 uint32_t r = spiread();
 r <<= 8;
 r |= spiread();
 r <<= 8;
 r |= spiread();
 r <<= 8;
 r |= spiread();
 if (_cs)
 digitalWrite(_cs, HIGH);
 
 pinMode(_sid, OUTPUT); // back to output
 return r;
 }
 
 */
