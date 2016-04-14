#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

SoftwareSerial gsmSerial(7, 6); // RX, TX
LiquidCrystal lcd(13, 12, A0, A1, A2, A3);

int atENPin = 8;
int led1Pin = 9;
int mailCount = 0;
unsigned long previousMillis = 0; 
const long interval = 1000; 

bool sendCmd(String cmd, String errorText)
{
  String dataInput = "";
  Serial.println(cmd);
  dataInput = Serial.readString(); 
 
  if(dataInput.indexOf("OK")== -1)
  {  
    lcd.clear(); 
    lcd.print(errorText);
    lcd.setCursor(0,1); 
    lcd.print("Press Reset");
    while(1);
    return false;
  }
  return true;
}
void setup() 
{
  pinMode(atENPin, OUTPUT); 
  pinMode(led1Pin, OUTPUT); 
  digitalWrite(atENPin, HIGH); 
  gsmSerial.begin(9600);
  
  Serial.begin(38400);
  Serial.setTimeout(500);
  lcd.begin(16, 2);
  lcd.print("BT MAILBOX MASTER");
  delay(5000);
  /*digitalWrite(atENPin, HIGH);
  delay(1000);
  //digitalWrite(atENPin, LOW);
  Serial.begin(38400);
  Serial.setTimeout(500);

  lcd.setCursor(0,1);
  sendCmd("AT", "BT AT Error"); 
  lcd.print(".");
  /*sendCmd("AT+PSWD=5521", "BT PSWD Error"); 
  lcd.print(".");
  sendCmd("AT+ROLE=1", "BT ROLE Error");
  lcd.print(".");
  sendCmd("AT+RMAAD", "BT RMAAD Error"); 
  lcd.print(".");
  sendCmd("AT+CMODE=1", "BT CMODE Error");  
  lcd.print(".");  
  Serial.println("AT+INIT");
  delay(200);
  lcd.print(".");  
  Serial.println("AT+RNAME?98D3,31,209DF3");
  input=Serial.readString(); 
  lcd.clear(); 
  
  lcd.print(input.substring(7));
  if(input.indexOf("BTMAILBOXREMOTE")== -1)
  { 
   lcd.clear(); 
   lcd.print("BT Error REMOTE");
   lcd.setCursor(0,1); 
   lcd.print("Press Reset");
   while(1);
  }
  delay(2000);
  lcd.print(".");  
  //Serial.println("AT+BIND=98D3,31,209DF3");

  Serial.println("AT+LINK=98D3,31,209DF3");
  //98D3,31,209DF3
  lcd.setCursor(0,1); 
  lcd.print("REMOTE CONNECTED!");
  delay(2000);
  //GSM Setting
  
  lcd.clear();
  gsmSerial.println("AT");
  input=gsmSerial.readString(); 
  lcd.setCursor(0,1);
  if(input.indexOf("OK")== -1)
  {  
    lcd.clear(); 
    lcd.print("GSM Error AT");
    lcd.setCursor(0,1); 
    lcd.print("Press Reset");
    while(1);
  }

  gsmSerial.println("AT+CSCS=\"GSM\"");
  input=gsmSerial.readString(); 
  lcd.setCursor(0,1);
  if(input.indexOf("OK")== -1)
  {  
    lcd.clear(); 
    lcd.print("GSM Error CSCS");
    lcd.setCursor(0,1); 
    lcd.print("Press Reset");
    while(1);
  }

  /*gsmSerial.println("AT+CMGF=1");
  input=gsmSerial.readString(); 
  lcd.setCursor(0,1);
  if(input.indexOf("OK")== -1)
  {  
    lcd.clear(); 
    lcd.print("GSM Error CMGF");
    lcd.setCursor(0,1); 
    lcd.print("Press Reset");
    while(1);
  }*/

  lcd.clear();
}

void loop()
{
  char inChar;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    digitalWrite(led1Pin, !digitalRead(led1Pin));
    Serial.print("A");
    previousMillis = currentMillis;
  }   
   
  if(Serial.available()) 
  {
    inChar = (char)Serial.read();
    switch(inChar)
    {
      case 'A':
        mailCount=Serial.parseInt();
        lcd.setCursor(0,0); 
        lcd.print("MAIL:");
        lcd.print(mailCount);   
        lcd.print("   ");         
      break;
      default:
      break;
    }    
  }
}
void sms(String text, String phone)
{
  gsmSerial.println("AT+CMGS=\"" + phone + "\"");
  delay(500);
  gsmSerial.print(text);
  delay(500);
  gsmSerial.print((char)26);
  delay(2000);
}

