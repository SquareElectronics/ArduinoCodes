//#include <stdint.h>
#include <TFTv2.h>
#include <SPI.h>
#include <SeeedTouchScreen.h>
#include <SD.h>
#include <Streaming.h>
#include <SoftwareSerial.h>

//SoftwareSerial wifiSerial(3, 2); // RX, TX
#define YP A2   // must be an analog pin, use "An" notation!
#define XM A1   // must be an analog pin, use "An" notation!
#define YM 54   // can be a digital pin, this is A0
#define XP 57   // can be a digital pin, this is A3 

#define TS_MINX 116*2
#define TS_MAXX 890*2
#define TS_MINY 83*2
#define TS_MAXY 913*2
#define TSDEBUG

#define KEYWIDTH 24
#define KEYHEIGHT 35
#define KEYSTRINGHEIGHT 3

const int PIN_SD_CS = 4;                        // pin of sd card
const int LED = 3;
const int BarcodeTrigger = 8;
const int Buzzer = 2;

uint8_t bmpDepth, bmpImageoffset;
int bmpWidth, bmpHeight;

String barcodeString = "", stringKeyPress="";         // a string to hold incoming data
boolean barcodeReceived = false;
long startTime = 0;

TouchScreen ts = TouchScreen(XP, YP, XM, YM);
char* keyT1[] = {"Q","W","E","R","T","Y","U","I","O","P"};
char* keyT2[] = {"A","S","D","F","G","H","J","K","L", " "};
char* keyT3[] = {"Z","X","C","V","B","N","M"," ", " ", " "};
char* keyNumber[] = {"1","2","3","4","5","6","7","8","9","0"};
char tmpKeyString1[] = "Del";
char tmpKeyString2[] = "Space";
char tmpKeyString3[] = "Enter";

String replyMap[30];
String replyQR[30];
String tcpRecvString[30];

#define MAIN_PAGE 0
#define MAIN_PAGE_WAIT 1
#define FINDER_PAGE 2
#define FINDER_PAGE_WAIT 3
#define SEARCH_PAGE 4
#define SEARCH_PAGE_WAIT 5
#define MAP_PAGE 6
#define MAP_PAGE_WAIT 7
#define QR_PAGE 8
#define QR_PAGE_WAIT 9
#define PRICE_CHECK 10
#define STATE_WAIT 255

byte state = MAIN_PAGE;
unsigned long endTime = 0; 
int itemSelect = 0;
int totalSearchItem = 0;
int pageDisplayItem = 0;
int remainDisplayItem = 0;
int tcpSearchIndex = 0;
int currentPage = 0;
void setup()
{
    pinMode(BarcodeTrigger, OUTPUT);
    pinMode(LED, OUTPUT);   
    pinMode(Buzzer, OUTPUT);  
    Serial.begin(9600);
    
    Serial1.begin(9600);
    Serial2.begin(9600);
    pinMode(PIN_SD_CS,OUTPUT);
    digitalWrite(PIN_SD_CS,HIGH);  
    
    Tft.TFTinit();      // init TFT library
    
    Sd2Card card;
    card.init(SPI_FULL_SPEED, PIN_SD_CS); 
    
    if(!SD.begin(PIN_SD_CS))              
    { 
        while(1);                               // init fail, die here
    }
    stringKeyPress = "";
    barcodeString.reserve(50);
}

void loop()
{ 
  displayStateMachine();
  getTouchScreen();
}

void getTouchScreen()
{
  Point p = ts.getPoint();  
  p.x = map(p.x,TS_MINX,TS_MAXX,0,240);
  p.y = map(p.y,TS_MINY,TS_MAXY,0,320);  

  if (p.z > __PRESURE && p.z < 1100)
  {
    digitalWrite(Buzzer, HIGH);
    if (state == MAIN_PAGE_WAIT)
    {
      if (p.y > 110 && p.y < 180)
      {         
        state = FINDER_PAGE;
        stringKeyPress = "";
        delay(150);
      }  
    }
    else if (state == FINDER_PAGE_WAIT)
    {        
      readFinderKey(p.x,p.y);
      delay(150);
    }
    else if (state == SEARCH_PAGE_WAIT)
    {
      if (p.x > 180 && p.x < 240)
      {
        if (p.y > 0 && p.y < 40)
        {
          state = SEARCH_PAGE;
        }
      }
      if (p.x > 20 && p.x < 200)
      {
        if (p.y > 40)
        {
          state = MAP_PAGE;
          itemSelect = (p.y-40)/50;
        }
      }         
       delay(150);
    }
    else if (state == MAP_PAGE_WAIT)
    {  
      if (p.x > 70 && p.x < 240)
      {
        if (p.y > 270 && p.y < 310)
        {
          state = QR_PAGE;
        }
      }
      delay(150);
    }
    else if (state == STATE_WAIT)
    {
      if (p.x > 20 && p.x < 220)
      {
        if (p.y > 150 && p.y < 310)
        {
          state = MAIN_PAGE;
        }
      }
    }
    digitalWrite(Buzzer, LOW); 
  }
}

void displayStateMachine()
{
  unsigned long currentTime = 0;  
  if (state == MAIN_PAGE)
  {
    digitalWrite(LED, HIGH);
    drawMainMenu();
    state = MAIN_PAGE_WAIT;
  }
  else if (state == MAIN_PAGE_WAIT)
  {
    currentTime = millis();
    if (currentTime - startTime > 700)
    {  
      digitalWrite(BarcodeTrigger, LOW);
      startTime = currentTime;
    } 
    else
      digitalWrite(BarcodeTrigger, HIGH);
  }
  else if (state == FINDER_PAGE)
  {
    digitalWrite(LED, LOW);
    stringKeyPress = "";
    clearScreen();
    drawKeyboard();
    state = FINDER_PAGE_WAIT;
    endTime = millis() + 30000;
  }
  else if (state == FINDER_PAGE_WAIT)
  {
    if (millis()>= endTime)
        state = MAIN_PAGE;
  }
  else if (state == SEARCH_PAGE)
  {
    
    boolean toggleDisplay = false;
    int item = 0;
    if (pageDisplayItem == 0)
    {
      if (remainDisplayItem != 0)
      {
        item = remainDisplayItem;
        remainDisplayItem = 0;
        clearScreen();
      }
      else
      {
        item = 0;
        drawStringCustom("NO ITEM", 10, 120, 2, RED); 
      }
      Tft.fillScreen(170,240,0,40,WHITE); 
    }
    else
    {
      drawStringCustom("NEXT", 180, 15, 2, RED); 
      item = pageDisplayItem*5;
      pageDisplayItem--;
    }
    drawStringCustom("RESULT", 50, 15, 2, BLACK);  
    for(int m = 0; m < item ; m++) 
    {
      drawSearchReply((m*50)+40, tcpRecvString[tcpSearchIndex], toggleDisplay, tcpSearchIndex);
      toggleDisplay = !toggleDisplay;
      tcpSearchIndex++;
    }  
    currentPage++;    
    state = SEARCH_PAGE_WAIT;
  }
  else if (state == MAP_PAGE)
  {
    clearScreen();  
    Serial.println(currentPage-1); 
    Serial.println(itemSelect); 
    Serial.println(((currentPage-1)*5)+itemSelect);  
    if(drawPicture(replyMap[((currentPage-1)*5)+itemSelect], 0, 0))
    {
      Tft.fillScreen(70, 250, 275, 305, WHITE);
      drawStringCustom("DOWNLOAD MAP", 80, 280, 2, BLACK);
      Tft.drawRectangle(70, 275, 180, 30, RED);
      state = MAP_PAGE_WAIT;
      endTime = millis() + 50000;
    }
    else
    {
      state = STATE_WAIT;
      endTime = millis() + 20000;
    }
  }
  else if (state == MAP_PAGE_WAIT)
  {
    if (millis()>= endTime)
      state = MAIN_PAGE;
  }
  else if (state == QR_PAGE)
  {
    clearScreen();    
    drawPicture(replyQR[((currentPage-1)*5)+itemSelect], 0, 0);
    state = QR_PAGE_WAIT;
    endTime = millis() + 50000;
  }
  else if (state == QR_PAGE_WAIT)
  {
   if (millis()>= endTime)
      state = MAIN_PAGE;
  }
  else if (state == PRICE_CHECK)
  {
    drawFinderReply(tcpRecvString[0],tcpRecvString[1],tcpRecvString[2],tcpRecvString[3]);
    state = STATE_WAIT; 
    endTime = millis() + 10000;
  }
  else if (state == STATE_WAIT)
  {
    drawStringCustom("TOUCH TO CONTINUE...", 10, 310, 1, YELLOW);
   if (millis()>= endTime)
      state = MAIN_PAGE;
  }
}
void drawMainMenu()
{
  clearScreen();
  drawPicture("logo", 50, 50);  
  drawStringCustom("FINDER", 30, 120, 5, 0xFBE0);
  Tft.drawRectangle(20, 110, 200, 50, 0x79E0);
  drawStringCustom("SCAN BELOW", 30, 220, 3, BLACK);
  Tft.drawRectangle(25, 215, 200, 30, RED);
  Tft.drawTraingle(119, 319,180, 260, 60, 260, BLUE);
}

void drawKeyBoardRow(char* displayString[10], int length, int keyX, int keyY, boolean toggleKey )
{
  long color = 0;
  for (int i = 0; i < length ; i++)
  {
    if(toggleKey)
      color = CYAN;
    else
      color = YELLOW;
      
    Tft.fillScreen(keyX-1,keyX-1+KEYWIDTH,keyY-5,keyY-5+KEYHEIGHT,color);  
    drawStringCustom(displayString[i], keyX, keyY, KEYSTRINGHEIGHT, BLACK); 
    Tft.drawRectangle(keyX-1, keyY-5, KEYWIDTH, KEYHEIGHT, BLUE);
    keyX += KEYWIDTH;
    toggleKey = !toggleKey;
  }
}
void drawKeyBoardSpecial(char* displayString, int keyX, int keyY, int keyWidth)
{
    drawStringCustom(displayString, keyX, keyY, KEYSTRINGHEIGHT, BLACK); 
    Tft.drawRectangle(keyX-1, keyY-5, keyWidth, KEYHEIGHT,BLUE);
}
void drawKeyboard()
{
  drawStringCustom("SEARCH KEYWORDS", 30, 20, 2, RED);
  drawKeyBoardRow(keyNumber, 10, 1, 120, false);
  drawKeyBoardRow(keyT1, 10, 1, 160, true);
  drawKeyBoardRow(keyT2, 9, 10, 200, false);
  drawKeyBoardRow(keyT3, 7, 1, 240, true);
  drawKeyBoardSpecial(tmpKeyString1, 175, 240, 55);
  drawKeyBoardSpecial(tmpKeyString2, 30, 280, 100);
  drawKeyBoardSpecial(tmpKeyString3, 130, 280, 100);
  Tft.drawRectangle(0, 65, 239, 45, RED);
}

char readKeyBoard(int touchX, int touchY)
{
  if (touchY  >120 && touchY <160)
     return *keyNumber[touchX/24];
  else if (touchY>160 && touchY<200)
     return *keyT1[touchX/24];
  else if (touchY>200 && touchY<240)
  {
     if (touchX < 220)
       return *keyT2[(touchX-10)/24];
     else
       return ' ';
  }
  else if (touchY>240 && touchY<280)
  {
     if (touchX > 175)
        return 8;
     else
       return *keyT3[touchX/24];
  }
  else if (touchY>280 && touchY<320)
  {
     if (touchX > 30 && touchX < 130)
        return 32;
     else if (touchX > 130 && touchX < 230)
       return 13; 
     else
       return ' ';
  }
  else
    return ' '; 
}  
void readFinderKey(int x, int y)
{
  char buffer[50];
  char keyData = readKeyBoard(x, y) ;
  if (keyData == 8)
  {
    if (stringKeyPress.length() > 0)
    {
      stringKeyPress = stringKeyPress.substring(0,stringKeyPress.length()-1); 
    }
  }  
  else if ( keyData == 13 )
  {
    delay(150);
    digitalWrite(Buzzer, LOW); 
    clearScreen();
    sendTCPData(stringKeyPress, 1);
    stringKeyPress = "";    
    return;
  }
  else
  {
    if (stringKeyPress.length() < 13)
    {
      if(keyData != ' ')
        stringKeyPress += keyData;
    }
  }

  Tft.fillScreen(3,237,70,105,WHITE);
  drawStringCustom(stringKeyPress, 0, 80, 3, BLACK);
  
}
#define BUFFPIXEL 10
#define MULTIPLY 3

void bmpdraw(File f, int x, int y) 
{
  f.seek(bmpImageoffset);  
  
  uint16_t red;
  uint8_t green, blue;

  uint8_t sdbuffer[MULTIPLY * BUFFPIXEL];  // 3 * pixels to buffer
  uint8_t buffidx = MULTIPLY * BUFFPIXEL;

  for (int i=bmpHeight ; i>0 ; i--)
  {
    for (int j=0; j<bmpWidth; j++) 
    {
      // read more pixels
      if (buffidx >= MULTIPLY * BUFFPIXEL) 
      {
        f.read(sdbuffer, MULTIPLY * BUFFPIXEL);
        buffidx = 0;
      }

      // convert pixel from 888 to 565
      blue = sdbuffer[buffidx++];     // blue
      green = sdbuffer[buffidx++];     // green
      red = sdbuffer[buffidx++];     // red

      red >>= 3;
      red <<= 6;

      green >>= 2;
      red |= green;
      red <<= 5;

      blue >>= 3;
      red |= blue;

      // write out the 16 bits of color
      Tft.setPixel(j,i,red);
    }
  }
}

boolean bmpReadHeader(File f) 
{

  if (read16(f) != 0x4D42) 
    return false;

  // read file size
  read32(f);  
  // read and ignore creator bytes
  read32(f);

  bmpImageoffset = read32(f);  

  // read DIB header
  read32(f);

  bmpWidth = read32(f);
  bmpHeight = read32(f);


  if (read16(f) != 1)
    return false;

  read16(f);

  if (read32(f) != 0)
    return false;

  return true;
}

/*********************************************/
// These read data from the SD card file and convert them to big endian
// (the data is stored in little endian format!)

// LITTLE ENDIAN!
uint16_t read16(File f)
{
    uint16_t d;
    uint8_t b;
    b = f.read();
    d = f.read();
    d <<= 8;
    d |= b;
    return d;
}

// LITTLE ENDIAN!
uint32_t read32(File f)
{
    uint32_t d;
    uint16_t b;

    b = read16(f);
    d = read16(f);
    d <<= 16;
    d |= b;
    return d;
}
void serialEvent2() 
{
  while (Serial2.available()) 
  {
    char inChar = (char)Serial2.read(); 
    if (inChar == '\r')// || inChar == '\n' )
    {
      barcodeReceived = true;   
      break;
    }
    else
      barcodeString += inChar;   
  }
  
  if (barcodeReceived) 
  {
    Tft.fillScreen(20,240,180,200,WHITE);
    drawStringCustom(barcodeString,20,180,2,BLUE);
    delay(500);
    sendTCPData(barcodeString, 0);
    barcodeString = "";
    barcodeReceived = false;
  }
}

boolean sendTCPData(String input, int mode)
{
  byte index = 0;
  boolean tcpDataReceived = false;
  for (int i = 0 ; i < 30 ; i++)
    tcpRecvString[i] = "";
  
  Serial1.println("(");
  Serial1.flush();
  if (waitForString(")",1, 5000))
  {
    if(mode == 0)
      Serial1.print("BAR:");
    else
      Serial1.print("SER:");
      
    Serial1.print(input);
    Serial1.println();
    Serial1.flush();
    unsigned long readTime = millis() + 10000;
    while (readTime >= millis()) 
    {
      while (Serial1.available()) 
      {
        char inChar = (char)Serial1.read(); 
        if (inChar == '\r' )
        {
          tcpDataReceived = true;
          break;
        }   
        else if (inChar == ',' )
          index++;      
        else
          tcpRecvString[index] += inChar;      
      }
      
      if(tcpDataReceived)
      {
        if (tcpRecvString[0] == "NDF")
         {
           clearScreen();
           
           if(mode == 0)
           {
             drawStringCustom("ITEM NO REGISTER,", 10, 120, 2, RED);
             drawStringCustom("PLEASE CONTACT ", 10, 145, 2, RED);
             drawStringCustom("RECEPTION...", 10, 170, 2, RED);
           } 
           else
            drawStringCustom("0 ITEM FOUND!", 10, 120, 2, RED);
            
            state = STATE_WAIT;
            endTime = millis() + 10000;
            return true;
         }  
        if (mode == 0)
        {          
          state = PRICE_CHECK;
          return true;
        }
        else
        {             
          state = SEARCH_PAGE;          
          totalSearchItem = tcpRecvString[0].toInt();
          pageDisplayItem = (totalSearchItem - 1) / 5;
          remainDisplayItem = ((totalSearchItem - 1) % 5 + 1);
          tcpSearchIndex = 1; 
          currentPage = 0;  
          return true;              
        }        
        return true;
      }
    }
  }
  else
  {
    clearScreen();
    drawStringCustom("TIMEOUT", 10, 120, 2, RED); 
    state = STATE_WAIT;
    endTime = millis() + 10000;
  }
}
//Wait for specific input string until timeout runs out
boolean waitForString(char* input, uint8_t length, unsigned int timeout) 
{
  unsigned long end_time = millis() + timeout;
  int current_byte = 0;
  uint8_t index = 0;
  while (end_time >= millis()) 
  {
    if(Serial1.available()) 
    {
    //Read one byte from serial port
      current_byte = Serial1.read();
      if (current_byte != -1) 
      {
        //Search one character at a time
        if (current_byte == input[index]) 
        {
          index++;
          //Found the string
          if (index == length) 
            return true;
          //Restart position of character to look for
        } 
        else 
          index = 0; 
      }
    }
  }
  //Timed out
  return false;
}
#define FONT_SPACE 6

void drawFinderReply(String replyTitle, String replyPrice, String replyLocation, String replyQuantity)
{
  clearScreen();
  drawStringCustom( replyTitle, 20, 40, 2, BLUE); 
  replyPrice = "RM"+replyPrice;
  drawStringCustom( "PRICE:", 20, 80, 2, RED);  
  drawStringCustom( replyPrice, 20, 100, 4, BLUE);
  drawStringCustom( "LOCATION:", 20, 140, 2, RED);  
  drawStringCustom( replyLocation, 20, 160, 4, BLUE); 
  drawStringCustom( "QUANTITY:", 20, 200, 2, RED);  
  drawStringCustom( replyQuantity, 20, 220, 4, BLUE);   
}

void clearScreen()
{
  Tft.fillScreen(0,240,0,320,WHITE);
}

boolean drawPicture(String fileName, int x, int y)
{
  File bmpFile;
  char buffer[20];
  fileName += ".bmp";
  fileName.toCharArray(buffer, sizeof(buffer));
  bmpFile = SD.open(buffer);
  if (!bmpFile)
  {
    drawStringCustom( "FILE ERROR", 20, 240, 2, RED); 
    return false;
  }
  if(! bmpReadHeader(bmpFile)) 
  {
    drawStringCustom( "BMP ERROR", 20, 240, 2, RED); 
    return false;
  }
  bmpdraw(bmpFile, x, y);
  bmpFile.close(); 
  return true;
}

void drawSearchReply(int resultY, String inputString, boolean toggle, int arrayNumber)
{
  String splitString[5];
  int splitIndex = 0;  
  long color = 0;
  splitString[0] = "";
  splitString[1] = "";
  splitString[2] = ""; 
  splitString[3] = "";
  splitString[4] = "";
  
  for (int i = 0 ; i < inputString.length() ; i++)
  {                              
    if (inputString.charAt(i) == '|')
      splitIndex++;
    else
      splitString[splitIndex] += inputString.charAt(i);
  }
  replyMap[arrayNumber-1] = splitString[3];
  replyQR[arrayNumber-1] = splitString[4];  
  
  if (toggle)
    color = YELLOW;
  else
    color = CYAN;
    
  Tft.fillScreen(0,240,resultY,resultY+50,color);    
  drawStringCustom(splitString[0], 20, resultY+5, 2, BLUE);
  splitString[1] = "RM"+splitString[1];
  drawStringCustom( splitString[1], 20, resultY+30, 2, BLUE);  
  drawStringCustom( splitString[2], 150, resultY+30, 2, BLUE); 
  Tft.drawRectangle(0, resultY, 240, 50,BLACK);
}

void drawStringCustom(String input, INT16U poX, INT16U poY, INT16U size,INT16U fgcolor)
{
    for (int i = 0 ; i<input.length() ; i++)
    {
        Tft.drawChar(input.charAt(i), poX, poY, size, fgcolor);
        if(poX < MAX_X)
            poX += FONT_SPACE*size;                                             
    }
}
