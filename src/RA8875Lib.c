#include "RA8875definitions.h"
#include "definitions.h"
#include <stddef.h>               
#include <stdbool.h>            
#include <stdlib.h>                        
#include <samd21j18a.h>
#include <stdint.h>
#include "RA8875Lib.h"

/**************************************************************************/
/*!
      Initialise the PLL
*/
/**************************************************************************/
void PLLinit(){
    writeReg(RA8875_PLLC1, RA8875_PLLC1_PLLDIV1 + 11);
    SYSTICK_DelayMs(1);
    writeReg(RA8875_PLLC2, RA8875_PLLC2_DIV4);
    SYSTICK_DelayMs(1);
}

/**************************************************************************/
/*!
      Initialises the driver IC (clock setup, etc.)
*/
/**************************************************************************/
void initalize(){
  PLLinit();
  writeReg(RA8875_SYSR, RA8875_SYSR_16BPP | RA8875_SYSR_MCU8);
  writeReg(RA8875_PCSR, RA8875_PCSR_PDATL | RA8875_PCSR_2CLK);
  SYSTICK_DelayMs(1);

  /* Horizontal settings registers */
  writeReg(RA8875_HDWR, (800 / 8) - 1); // H width: (HDWR + 1) * 8 = 480
  writeReg(RA8875_HNDFTR, RA8875_HNDFTR_DE_HIGH + HSYNC_FINETUNE);
  writeReg(RA8875_HNDR, (HSYNC_NONDISP - HSYNC_FINETUNE - 2) /
                            8); // H non-display: HNDR * 8 + HNDFTR + 2 = 10
  writeReg(RA8875_HSTR, HSYNC_START / 8 - 1); // Hsync start: (HSTR + 1)*8
  writeReg(RA8875_HPWR,
           RA8875_HPWR_LOW +
               (HSYNC_PW / 8 - 1)); // HSync pulse width = (HPWR+1) * 8

  /* Vertical settings registers */
  writeReg(RA8875_VDHR0, (uint16_t)(480 - 1 ) & 0xFF);
  writeReg(RA8875_VDHR1, (uint16_t)(480 - 1 ) >> 8);
  writeReg(RA8875_VNDR0, VSYNC_NONDISP - 1); // V non-display period = VNDR + 1
  writeReg(RA8875_VNDR1, VSYNC_NONDISP >> 8);
  writeReg(RA8875_VSTR0, VSYNC_START - 1); // Vsync start position = VSTR + 1
  writeReg(RA8875_VSTR1, VSYNC_START >> 8);
  writeReg(RA8875_VPWR,
           RA8875_VPWR_LOW + VSYNC_PW - 1); // Vsync pulse width = VPWR + 1

  /* Set active window X */
  writeReg(RA8875_HSAW0, 0); // horizontal start point
  writeReg(RA8875_HSAW1, 0);
  writeReg(RA8875_HEAW0, (uint16_t)(800 - 1) & 0xFF); // horizontal end point
  writeReg(RA8875_HEAW1, (uint16_t)(800 - 1) >> 8);

  /* Set active window Y */
  writeReg(RA8875_VSAW0, 0); // vertical start point
  writeReg(RA8875_VSAW1, 0);
  writeReg(RA8875_VEAW0,
           (uint16_t)(480 - 1 ) & 0xFF); // vertical end point
  writeReg(RA8875_VEAW1, (uint16_t)(480 - 1 ) >> 8);

  /* Clear the entire window */
  writeReg(RA8875_MCLR, RA8875_MCLR_START | RA8875_MCLR_FULL);
  SYSTICK_DelayMs(500);
}
/**************************************************************************/
/*!
      Complete initialization 
*/
/**************************************************************************/
void fullInitLCD(){
    RST_DISPLAY_Clear();
    SYSTICK_DelayMs(100);
    RST_DISPLAY_Set();
    SYSTICK_DelayMs(100);
    SPI_TRANSFER_SETUP  setup;
    setup.clockFrequency=125000;
    setup.clockPhase=SERCOM_SPIM_CTRLA_CPHA_LEADING_EDGE;
    setup.clockPolarity=SERCOM_SPIM_CTRLA_CPOL_IDLE_LOW;
    setup.dataBits=SERCOM_SPIM_CTRLB_CHSIZE_8_BIT;

    SERCOM1_SPI_TransferSetup(&setup,48000000);
    initalize();
    setup.clockFrequency=2000000;
    SERCOM1_SPI_TransferSetup(&setup,48000000);
}
/**************************************************************************/
/*!
    Low level access/writeCommand
*/
/**************************************************************************/
void writeCommand(uint8_t address){
    CS_DISPLAY_Clear();
    uint8_t tempbuffer[]={RA8875_CMDWRITE,address};
    SERCOM1_SPI_Write(&tempbuffer[0],sizeof(tempbuffer));
    CS_DISPLAY_Set();
}
/**************************************************************************/
/*!
    Low level access/writeData
*/
/**************************************************************************/
void writeData(uint8_t data){
    CS_DISPLAY_Clear();
    uint8_t tempbuffer[]={RA8875_DATAWRITE,data};
    SERCOM1_SPI_Write(&tempbuffer[0],sizeof(tempbuffer));
    CS_DISPLAY_Set();
}
/**************************************************************************/
/*!
    Low level access/readData
*/
/**************************************************************************/
uint8_t readData(){
     uint8_t tempbuffer[]={RA8875_DATAREAD,0x00};
    uint8_t readbuffer[sizeof(tempbuffer)];
    CS_DISPLAY_Clear();
    SERCOM1_SPI_WriteRead(&tempbuffer[0],sizeof(tempbuffer),&readbuffer[0],sizeof(tempbuffer));   
    CS_DISPLAY_Set();
    return readbuffer[1];
}
/**************************************************************************/
/*!
    Low level access/writeReg
*/
/**************************************************************************/
void writeReg(uint8_t reg,uint8_t data){
    writeCommand(reg);
    writeData(data);
}
/**************************************************************************/
/*!
    Low level access/readReg
*/
/**************************************************************************/
uint8_t readReg(uint8_t reg){
    writeCommand(reg);
    uint8_t temp= readData();
    return temp;
}
/**************************************************************************/
/*!
    Low level access/readstatus
*/
/**************************************************************************/
uint8_t readstatus(){
    uint8_t tempbuffer[]={RA8875_CMDREAD,0x00};
    uint8_t readbuffer[sizeof(tempbuffer)];
    CS_DISPLAY_Clear();
    SERCOM1_SPI_WriteRead(&tempbuffer[0],sizeof(tempbuffer),&readbuffer[0],sizeof(tempbuffer));   
    CS_DISPLAY_Set();
    return readbuffer[1];
}
/**************************************************************************/
/*!
    Low level access/waitPoll
*/
/**************************************************************************/
bool waitPoll(uint8_t regname, uint8_t waitflag) {
  /* Wait for the command to finish */
  while (1) {
    uint8_t temp = readReg(regname);
    if (!(temp & waitflag))
      return true;
  }
  return false; // MEMEFIX: yeah i know, unreached! - add timeout?
}
/**************************************************************************/
/*!
    Brightness Controll/displayON
*/
/**************************************************************************/
void displayOn(bool on) {
  if (on)
    writeReg(RA8875_PWRR, RA8875_PWRR_NORMAL | RA8875_PWRR_DISPON);
  else
    writeReg(RA8875_PWRR, RA8875_PWRR_NORMAL | RA8875_PWRR_DISPOFF);
}
/**************************************************************************/
/*!
    Brightness Controll/GPIOX
*/
/**************************************************************************/
void GPIOX(bool on) {
  if (on)
    writeReg(RA8875_GPIOX, 1);
  else
    writeReg(RA8875_GPIOX, 0);
}
/**************************************************************************/
/*!
    Brightness Controll/PWM!out
*/
/**************************************************************************/
void PWM1out(uint8_t p) { writeReg(RA8875_P1DCR, p); }
/**************************************************************************/
/*!
    Brightness Controll/PWM!config
*/
/**************************************************************************/
void PWM1config(bool on, uint8_t clock) {
  if (on) {
    writeReg(RA8875_P1CR, RA8875_P1CR_ENABLE | (clock & 0xF));
  } else {
    writeReg(RA8875_P1CR, RA8875_P1CR_DISABLE | (clock & 0xF));
  }
}
/************************* Text Mode ***********************************/

/**************************************************************************/
/*!
      Sets the display in text mode (as opposed to graphics mode)
*/
/**************************************************************************/
void textMode(void) {
  /* Set text mode */
  writeCommand(RA8875_MWCR0);
  uint8_t temp = readData();
  temp |= RA8875_MWCR0_TXTMODE; // Set bit 7
  writeData(temp);

  /* Select the internal (ROM) font */
  writeCommand(0x21);
  temp = readData();
  temp &= ~((1 << 7) | (1 << 5)); // Clear bits 7 and 5
  writeData(temp);
}

/**************************************************************************/
/*!
      Sets the display in text mode (as opposed to graphics mode)

      @param x The x position of the cursor (in pixels, 0..1023)
      @param y The y position of the cursor (in pixels, 0..511)
*/
/**************************************************************************/
void textSetCursor(uint16_t x, uint16_t y) {
 
  /* Set cursor location */
  writeCommand(0x2A);
  writeData(x & 0xFF);
  writeCommand(0x2B);
  writeData(x >> 8);
  writeCommand(0x2C);
  writeData(y & 0xFF);
  writeCommand(0x2D);
  writeData(y >> 8);
}

/**************************************************************************/
/*!
      Sets the fore and background color when rendering text

      @param foreColor The RGB565 color to use when rendering the text
      @param bgColor   The RGB565 colot to use for the background
*/
/**************************************************************************/
void textColor(uint16_t foreColor, uint16_t bgColor) {
  /* Set Fore Color */
  writeCommand(0x63);
  writeData((foreColor & 0xf800) >> 11);
  writeCommand(0x64);
  writeData((foreColor & 0x07e0) >> 5);
  writeCommand(0x65);
  writeData((foreColor & 0x001f));

  /* Set Background Color */
  writeCommand(0x60);
  writeData((bgColor & 0xf800) >> 11);
  writeCommand(0x61);
  writeData((bgColor & 0x07e0) >> 5);
  writeCommand(0x62);
  writeData((bgColor & 0x001f));

  /* Clear transparency flag */
  writeCommand(0x22);
  uint8_t temp = readData();
  temp &= ~(1 << 6); // Clear bit 6
  writeData(temp);
}

/**************************************************************************/
/*!
      Sets the fore color when rendering text with a transparent bg

      @param foreColor The RGB565 color to use when rendering the text
*/
/**************************************************************************/
void textTransparent(uint16_t foreColor) {
  /* Set Fore Color */
  writeCommand(0x63);
  writeData((foreColor & 0xf800) >> 11);
  writeCommand(0x64);
  writeData((foreColor & 0x07e0) >> 5);
  writeCommand(0x65);
  writeData((foreColor & 0x001f));

  /* Set transparency flag */
  writeCommand(0x22);
  uint8_t temp = readData();
  temp |= (1 << 6); // Set bit 6
  writeData(temp);
}

/**************************************************************************/
/*!
      Sets the text enlarge settings, using one of the following values:

      0 = 1x zoom
      1 = 2x zoom
      2 = 3x zoom
      3 = 4x zoom

      @param scale   The zoom factor (0..3 for 1-4x zoom)
*/
/**************************************************************************/
void textEnlarge(uint8_t scale) {
  if (scale > 3)
    scale = 3; // highest setting is 3

  /* Set font size flags */
  writeCommand(0x22);
  uint8_t temp = readData();
  temp &= ~(0xF); // Clears bits 0..3
  temp |= scale << 2;
  temp |= scale;

  writeData(temp);

}

/**************************************************************************/
/*!
     Enable Cursor Visibility and Blink
     Here we set bits 6 and 5 in 40h
     As well as the set the blink rate in 44h
     The rate is 0 through max 255
     the lower the number the faster it blinks (00h is 1 frame time,
     FFh is 256 Frames time.
     Blink Time (sec) = BTCR[44h]x(1/Frame_rate)

     @param rate The frame rate to blink
 */
/**************************************************************************/

void cursorBlink(uint8_t rate) {

  writeCommand(RA8875_MWCR0);
  uint8_t temp = readData();
  temp |= RA8875_MWCR0_CURSOR;
  writeData(temp);

  writeCommand(RA8875_MWCR0);
  temp = readData();
  temp |= RA8875_MWCR0_BLINK;
  writeData(temp);

  if (rate > 255)
    rate = 255;
  writeCommand(RA8875_BTCR);
  writeData(rate);
}

/**************************************************************************/
/*!
      Renders some text on the screen when in text mode

      @param buffer    The buffer containing the characters to render
      @param len       The size of the buffer in bytes
*/
/**************************************************************************/
void textWrite(const char *buffer, uint16_t len) {
  //if (len == 0)
//    len = strlen(buffer);
  writeCommand(RA8875_MRWC);
  for (uint16_t i = 0; i < len; i++) {
    writeData(buffer[i]);
      SYSTICK_DelayMs(1);
  }
}
/**************************************************************************/
/*!
    Graphics Functions/graphicsMode:
        Sets the display in graphics mode (as opposed to text mode)
*/
/**************************************************************************/
void graphicsMode(void){
    writeCommand(RA8875_MWCR0);
    uint8_t temp = readData();
    temp &= ~RA8875_MWCR0_TXTMODE; // bit #7
  writeData(temp);
}

/**************************************************************************/
/*!
    Graphics Functions/setXY
      Sets the current X/Y position on the display before drawing

      @param x The 0-based x location
      @param y The 0-base y location
*/
/**************************************************************************/
void setXY(uint16_t x, uint16_t y) {
  writeReg(RA8875_CURH0, x);
  writeReg(RA8875_CURH1, x >> 8);
  writeReg(RA8875_CURV0, y);
  writeReg(RA8875_CURV1, y >> 8);
}
/**************************************************************************/
/*!
 * Graphics Functions/fillScreen
      Fills the screen with the spefied RGB565 color

      @param color The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void fillScreen(uint16_t color) {
  rectHelper(0, 0, 800 - 1, 480 - 1, color, true);
}
/**************************************************************************/
/*!Graphics Functions/drawLine
      Draws a HW accelerated line on the display

      @param x0    The 0-based starting x location
      @param y0    The 0-base starting y location
      @param x1    The 0-based ending x location
      @param y1    The 0-base ending y location
      @param color The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                               uint16_t color) {
 
  /* Set X */
  writeCommand(0x91);
  writeData(x0);
  writeCommand(0x92);
  writeData(x0 >> 8);

  /* Set Y */
  writeCommand(0x93);
  writeData(y0);
  writeCommand(0x94);
  writeData(y0 >> 8);

  /* Set X1 */
  writeCommand(0x95);
  writeData(x1);
  writeCommand(0x96);
  writeData((x1) >> 8);

  /* Set Y1 */
  writeCommand(0x97);
  writeData(y1);
  writeCommand(0x98);
  writeData((y1) >> 8);

  /* Set Color */
  writeCommand(0x63);
  writeData((color & 0xf800) >> 11);
  writeCommand(0x64);
  writeData((color & 0x07e0) >> 5);
  writeCommand(0x65);
  writeData((color & 0x001f));

  /* Draw! */
  writeCommand(RA8875_DCR);
  writeData(0x80);

  /* Wait for the command to finish */
  waitPoll(RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
}
/**************************************************************************/
/*!Graphics Functions/drawRect
      Draws a HW accelerated rectangle on the display

      @param x     The 0-based x location of the top-right corner
      @param y     The 0-based y location of the top-right corner
      @param w     The rectangle width
      @param h     The rectangle height
      @param color The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void drawRect(int16_t x, int16_t y, int16_t w, int16_t h,
                               uint16_t color) {
  rectHelper(x, y, x + w - 1, y + h - 1, color, false);
}
/**************************************************************************/
/*!
    Graphics Functions/fillRect
      Draws a HW accelerated filled rectangle on the display

      @param x     The 0-based x location of the top-right corner
      @param y     The 0-based y location of the top-right corner
      @param w     The rectangle width
      @param h     The rectangle height
      @param color The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/

void fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                               uint16_t color) {
  rectHelper(x, y, x + w - 1, y + h - 1, color, true);
}

/**************************************************************************/
/*!
    Graphics Functions/drawCircle
      Draws a HW accelerated circle on the display

      @param x     The 0-based x location of the center of the circle
      @param y     The 0-based y location of the center of the circle
      @param r     The circle's radius
      @param color The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void drawCircle(int16_t x, int16_t y, int16_t r,
                                 uint16_t color) {
  circleHelper(x, y, r, color, false);
}

/**************************************************************************/
/*!Graphics Functions/fillCircle
      Draws a HW accelerated filled circle on the display

      @param x     The 0-based x location of the center of the circle
      @param y     The 0-based y location of the center of the circle
      @param r     The circle's radius
      @param color The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void fillCircle(int16_t x, int16_t y, int16_t r,
                                 uint16_t color) {
  circleHelper(x, y, r, color, true);
}
/**************************************************************************/
/*!Graphics Functions/drawTriangle
      Draws a HW accelerated triangle on the display

      @param x0    The 0-based x location of point 0 on the triangle
      @param y0    The 0-based y location of point 0 on the triangle
      @param x1    The 0-based x location of point 1 on the triangle
      @param y1    The 0-based y location of point 1 on the triangle
      @param x2    The 0-based x location of point 2 on the triangle
      @param y2    The 0-based y location of point 2 on the triangle
      @param color The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void drawTriangle(int16_t x0, int16_t y0, int16_t x1,
                                   int16_t y1, int16_t x2, int16_t y2,
                                   uint16_t color) {
  triangleHelper(x0, y0, x1, y1, x2, y2, color, false);
}

/**************************************************************************/
/*!Graphics Functions/fillTriangle
      Draws a HW accelerated filled triangle on the display

      @param x0    The 0-based x location of point 0 on the triangle
      @param y0    The 0-based y location of point 0 on the triangle
      @param x1    The 0-based x location of point 1 on the triangle
      @param y1    The 0-based y location of point 1 on the triangle
      @param x2    The 0-based x location of point 2 on the triangle
      @param y2    The 0-based y location of point 2 on the triangle
      @param color The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void fillTriangle(int16_t x0, int16_t y0, int16_t x1,
                                   int16_t y1, int16_t x2, int16_t y2,
                                   uint16_t color) {
  triangleHelper(x0, y0, x1, y1, x2, y2, color, true);
}

/**************************************************************************/
/*!Graphics Functions/drawEllipse
      Draws a HW accelerated ellipse on the display

      @param xCenter   The 0-based x location of the ellipse's center
      @param yCenter   The 0-based y location of the ellipse's center
      @param longAxis  The size in pixels of the ellipse's long axis
      @param shortAxis The size in pixels of the ellipse's short axis
      @param color     The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void drawEllipse(int16_t xCenter, int16_t yCenter,
                                  int16_t longAxis, int16_t shortAxis,
                                  uint16_t color) {
  ellipseHelper(xCenter, yCenter, longAxis, shortAxis, color, false);
}

/**************************************************************************/
/*!Graphics Functions/fillEllipse
      Draws a HW accelerated filled ellipse on the display

      @param xCenter   The 0-based x location of the ellipse's center
      @param yCenter   The 0-based y location of the ellipse's center
      @param longAxis  The size in pixels of the ellipse's long axis
      @param shortAxis The size in pixels of the ellipse's short axis
      @param color     The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void fillEllipse(int16_t xCenter, int16_t yCenter,
                                  int16_t longAxis, int16_t shortAxis,
                                  uint16_t color) {
  ellipseHelper(xCenter, yCenter, longAxis, shortAxis, color, true);
}

/**************************************************************************/
/*!Graphics Functions/drawCurve
      Draws a HW accelerated curve on the display

      @param xCenter   The 0-based x location of the ellipse's center
      @param yCenter   The 0-based y location of the ellipse's center
      @param longAxis  The size in pixels of the ellipse's long axis
      @param shortAxis The size in pixels of the ellipse's short axis
      @param curvePart The corner to draw, where in clock-wise motion:
                            0 = 180-270°
                            1 = 270-0°
                            2 = 0-90°
                            3 = 90-180°
      @param color     The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void drawCurve(int16_t xCenter, int16_t yCenter,
                                int16_t longAxis, int16_t shortAxis,
                                uint8_t curvePart, uint16_t color) {
  curveHelper(xCenter, yCenter, longAxis, shortAxis, curvePart, color, false);
}

/**************************************************************************/
/*!Graphics Functions/fillCurve
      Draws a HW accelerated filled curve on the display

      @param xCenter   The 0-based x location of the ellipse's center
      @param yCenter   The 0-based y location of the ellipse's center
      @param longAxis  The size in pixels of the ellipse's long axis
      @param shortAxis The size in pixels of the ellipse's short axis
      @param curvePart The corner to draw, where in clock-wise motion:
                            0 = 180-270°
                            1 = 270-0°
                            2 = 0-90°
                            3 = 90-180°
      @param color     The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void fillCurve(int16_t xCenter, int16_t yCenter,
                                int16_t longAxis, int16_t shortAxis,
                                uint8_t curvePart, uint16_t color) {
  curveHelper(xCenter, yCenter, longAxis, shortAxis, curvePart, color, true);
}

/**************************************************************************/
/*!Graphics Functions/drawRoundRect
      Draws a HW accelerated rounded rectangle on the display

      @param x   The 0-based x location of the rectangle's upper left corner
      @param y   The 0-based y location of the rectangle's upper left corner
      @param w   The size in pixels of the rectangle's width
      @param h   The size in pixels of the rectangle's height
      @param r   The radius of the curves in the corners of the rectangle
      @param color  The RGB565 color to use when drawing the pixel
 */
/**************************************************************************/
void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h,
                                    int16_t r, uint16_t color) {
  roundRectHelper(x, y, x + w, y + h, r, color, false);
}

/**************************************************************************/
/*!Graphics Functions/fillRoundRect
      Draws a HW accelerated filled rounded rectangle on the display

      @param x   The 0-based x location of the rectangle's upper left corner
      @param y   The 0-based y location of the rectangle's upper left corner
      @param w   The size in pixels of the rectangle's width
      @param h   The size in pixels of the rectangle's height
      @param r   The radius of the curves in the corners of the rectangle
      @param color  The RGB565 color to use when drawing the pixel
 */
/**************************************************************************/
void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h,
                                    int16_t r, uint16_t color) {
  roundRectHelper(x, y, x + w, y + h, r, color, true);
}


/**************************************************************************/
/*!
    HelperFunction/circleHelper
      Helper function for higher level circle drawing code
*/
/**************************************************************************/
void circleHelper(int16_t x, int16_t y, int16_t r,
                                   uint16_t color, bool filled) {
 
  /* Set X */
  writeCommand(0x99);
  writeData(x);
  writeCommand(0x9a);
  writeData(x >> 8);

  /* Set Y */
  writeCommand(0x9b);
  writeData(y);
  writeCommand(0x9c);
  writeData(y >> 8);

  /* Set Radius */
  writeCommand(0x9d);
  writeData(r);

  /* Set Color */
  writeCommand(0x63);
  writeData((color & 0xf800) >> 11);
  writeCommand(0x64);
  writeData((color & 0x07e0) >> 5);
  writeCommand(0x65);
  writeData((color & 0x001f));

  /* Draw! */
  writeCommand(RA8875_DCR);
  if (filled) {
    writeData(RA8875_DCR_CIRCLE_START | RA8875_DCR_FILL);
  } else {
    writeData(RA8875_DCR_CIRCLE_START | RA8875_DCR_NOFILL);
  }

  /* Wait for the command to finish */
  waitPoll(RA8875_DCR, RA8875_DCR_CIRCLE_STATUS);
}

/**************************************************************************/
/*!
    HelperFunction/rectHelper
      Helper function for higher level rectangle drawing code
*/
/**************************************************************************/
void rectHelper(int16_t x, int16_t y, int16_t w, int16_t h,
                                 uint16_t color, bool filled) {

  /* Set X */
  writeCommand(0x91);
  writeData(x);
  writeCommand(0x92);
  writeData(x >> 8);

  /* Set Y */
  writeCommand(0x93);
  writeData(y);
  writeCommand(0x94);
  writeData(y >> 8);

  /* Set X1 */
  writeCommand(0x95);
  writeData(w);
  writeCommand(0x96);
  writeData((w) >> 8);

  /* Set Y1 */
  writeCommand(0x97);
  writeData(h);
  writeCommand(0x98);
  writeData((h) >> 8);

  /* Set Color */
  writeCommand(0x63);
  writeData((color & 0xf800) >> 11);
  writeCommand(0x64);
  writeData((color & 0x07e0) >> 5);
  writeCommand(0x65);
  writeData((color & 0x001f));

  /* Draw! */
  writeCommand(RA8875_DCR);
  if (filled) {
    writeData(0xB0);
  } else {
    writeData(0x90);
  }

  /* Wait for the command to finish */
  waitPoll(RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
}

/**************************************************************************/
/*!HelperFunction/triangleHelper
      Helper function for higher level triangle drawing code
*/
/**************************************************************************/
void triangleHelper(int16_t x0, int16_t y0, int16_t x1,
                                     int16_t y1, int16_t x2, int16_t y2,
                                     uint16_t color, bool filled) {
  
  /* Set Point 0 */
  writeCommand(0x91);
  writeData(x0);
  writeCommand(0x92);
  writeData(x0 >> 8);
  writeCommand(0x93);
  writeData(y0);
  writeCommand(0x94);
  writeData(y0 >> 8);

  /* Set Point 1 */
  writeCommand(0x95);
  writeData(x1);
  writeCommand(0x96);
  writeData(x1 >> 8);
  writeCommand(0x97);
  writeData(y1);
  writeCommand(0x98);
  writeData(y1 >> 8);

  /* Set Point 2 */
  writeCommand(0xA9);
  writeData(x2);
  writeCommand(0xAA);
  writeData(x2 >> 8);
  writeCommand(0xAB);
  writeData(y2);
  writeCommand(0xAC);
  writeData(y2 >> 8);

  /* Set Color */
  writeCommand(0x63);
  writeData((color & 0xf800) >> 11);
  writeCommand(0x64);
  writeData((color & 0x07e0) >> 5);
  writeCommand(0x65);
  writeData((color & 0x001f));

  /* Draw! */
  writeCommand(RA8875_DCR);
  if (filled) {
    writeData(0xA1);
  } else {
    writeData(0x81);
  }

  /* Wait for the command to finish */
  waitPoll(RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
}

/**************************************************************************/
/*!HelperFunction/ellipseHelper
      Helper function for higher level ellipse drawing code
*/
/**************************************************************************/
void ellipseHelper(int16_t xCenter, int16_t yCenter,
                                    int16_t longAxis, int16_t shortAxis,
                                    uint16_t color, bool filled) {
  
  /* Set Center Point */
  writeCommand(0xA5);
  writeData(xCenter);
  writeCommand(0xA6);
  writeData(xCenter >> 8);
  writeCommand(0xA7);
  writeData(yCenter);
  writeCommand(0xA8);
  writeData(yCenter >> 8);

  /* Set Long and Short Axis */
  writeCommand(0xA1);
  writeData(longAxis);
  writeCommand(0xA2);
  writeData(longAxis >> 8);
  writeCommand(0xA3);
  writeData(shortAxis);
  writeCommand(0xA4);
  writeData(shortAxis >> 8);

  /* Set Color */
  writeCommand(0x63);
  writeData((color & 0xf800) >> 11);
  writeCommand(0x64);
  writeData((color & 0x07e0) >> 5);
  writeCommand(0x65);
  writeData((color & 0x001f));

  /* Draw! */
  writeCommand(0xA0);
  if (filled) {
    writeData(0xC0);
  } else {
    writeData(0x80);
  }

  /* Wait for the command to finish */
  waitPoll(RA8875_ELLIPSE, RA8875_ELLIPSE_STATUS);
}

/**************************************************************************/
/*!HelperFunction/curveHelper
      Helper function for higher level curve drawing code
*/
/**************************************************************************/
void curveHelper(int16_t xCenter, int16_t yCenter,
                                  int16_t longAxis, int16_t shortAxis,
                                  uint8_t curvePart, uint16_t color,
                                  bool filled) {
  
  /* Set Center Point */
  writeCommand(0xA5);
  writeData(xCenter);
  writeCommand(0xA6);
  writeData(xCenter >> 8);
  writeCommand(0xA7);
  writeData(yCenter);
  writeCommand(0xA8);
  writeData(yCenter >> 8);

  /* Set Long and Short Axis */
  writeCommand(0xA1);
  writeData(longAxis);
  writeCommand(0xA2);
  writeData(longAxis >> 8);
  writeCommand(0xA3);
  writeData(shortAxis);
  writeCommand(0xA4);
  writeData(shortAxis >> 8);

  /* Set Color */
  writeCommand(0x63);
  writeData((color & 0xf800) >> 11);
  writeCommand(0x64);
  writeData((color & 0x07e0) >> 5);
  writeCommand(0x65);
  writeData((color & 0x001f));

  /* Draw! */
  writeCommand(0xA0);
  if (filled) {
    writeData(0xD0 | (curvePart & 0x03));
  } else {
    writeData(0x90 | (curvePart & 0x03));
  }

  /* Wait for the command to finish */
  waitPoll(RA8875_ELLIPSE, RA8875_ELLIPSE_STATUS);
}

/**************************************************************************/
/*!HelperFunction/roudnRectHelper
      Helper function for higher level rounded rectangle drawing code
 */
/**************************************************************************/
void roundRectHelper(int16_t x, int16_t y, int16_t w,
                                      int16_t h, int16_t r, uint16_t color,
                                      bool filled) {
  
  /* Set X */
  writeCommand(0x91);
  writeData(x);
  writeCommand(0x92);
  writeData(x >> 8);

  /* Set Y */
  writeCommand(0x93);
  writeData(y);
  writeCommand(0x94);
  writeData(y >> 8);

  /* Set X1 */
  writeCommand(0x95);
  writeData(w);
  writeCommand(0x96);
  writeData((w) >> 8);

  /* Set Y1 */
  writeCommand(0x97);
  writeData(h);
  writeCommand(0x98);
  writeData((h) >> 8);

  writeCommand(0xA1);
  writeData(r);
  writeCommand(0xA2);
  writeData((r) >> 8);

  writeCommand(0xA3);
  writeData(r);
  writeCommand(0xA4);
  writeData((r) >> 8);

  /* Set Color */
  writeCommand(0x63);
  writeData((color & 0xf800) >> 11);
  writeCommand(0x64);
  writeData((color & 0x07e0) >> 5);
  writeCommand(0x65);
  writeData((color & 0x001f));

  /* Draw! */
  writeCommand(RA8875_ELLIPSE);
  if (filled) {
    writeData(0xE0);
  } else {
    writeData(0xA0);
  }

  /* Wait for the command to finish */
  waitPoll(RA8875_ELLIPSE, RA8875_DCR_LINESQUTRI_STATUS);
}

