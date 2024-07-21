#include "RA8875definitions.h"
// Display Initialization
  void PLLinit(void);
  void initialize(void);
  void fullInitLCD();
  
 // Lower level access
  void writeReg(uint8_t address, uint8_t data);
  uint8_t readReg(uint8_t address);
  void writeData(uint8_t data);
  uint8_t readData(void);
  void writeCommand(uint8_t data);
  uint8_t readStatus(void);  
  bool waitPoll(uint8_t regname, uint8_t waitflag);
  
  // Brightness Controll
  void displayOn(bool on);
  void GPIOX(bool on);
  void PWM1config(bool on, uint8_t clock);
  void PWM1out(uint8_t p);
  
  //Text functions
  void textMode(void);
  void textSetCursor(uint16_t x, uint16_t y);
  void textColor(uint16_t foreColor, uint16_t bgColor);
  void textTransparent(uint16_t foreColor);
  void textEnlarge(uint8_t scale);
  void textWrite(const char *buffer, uint16_t len);
  void cursorBlink(uint8_t rate);

  //Graphics functions
  void graphicsMode(void);
  void setXY(uint16_t x, uint16_t y);
  void fillScreen(uint16_t color);
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
  void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
  void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2,
                    int16_t y2, uint16_t color);
  void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2,
                    int16_t y2, uint16_t color);
  void drawEllipse(int16_t xCenter, int16_t yCenter, int16_t longAxis,
                   int16_t shortAxis, uint16_t color);
  void fillEllipse(int16_t xCenter, int16_t yCenter, int16_t longAxis,
                   int16_t shortAxis, uint16_t color);
  void drawCurve(int16_t xCenter, int16_t yCenter, int16_t longAxis,
                 int16_t shortAxis, uint8_t curvePart, uint16_t color);
  void fillCurve(int16_t xCenter, int16_t yCenter, int16_t longAxis,
                 int16_t shortAxis, uint8_t curvePart, uint16_t color);
  void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r,
                     uint16_t color);
  void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r,
                     uint16_t color);
  
  //Helper Functions
  void circleHelper(int16_t x, int16_t y, int16_t r, uint16_t color,
                    bool filled);
  void rectHelper(int16_t x, int16_t y, int16_t w, int16_t h,
                                 uint16_t color, bool filled);
  void triangleHelper(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                      int16_t x2, int16_t y2, uint16_t color, bool filled);
  void ellipseHelper(int16_t xCenter, int16_t yCenter, int16_t longAxis,
                     int16_t shortAxis, uint16_t color, bool filled);
  void curveHelper(int16_t xCenter, int16_t yCenter, int16_t longAxis,
                   int16_t shortAxis, uint8_t curvePart, uint16_t color,
                   bool filled);
  void roundRectHelper(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r,
                       uint16_t color, bool filled);
  
  
  
   