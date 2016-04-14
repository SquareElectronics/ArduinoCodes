int out1Pin = 8;
int out2Pin = 9;
int ledPin = 13;

void setup() 
{
  pinMode(out1Pin, OUTPUT);
  pinMode(out2Pin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
  //Set password
  Serial.print("AT+PIN4545");
}

void loop() 
{
  char inChar;
  if(Serial.available()) 
  {
        inChar = (char)Serial.read();
        if(inChar == 'A')
        {
          Serial.print(" OPEN GATE");
          digitalWrite(out1Pin, HIGH);
          digitalWrite(ledPin, HIGH);
          delay(3000);    
          digitalWrite(out1Pin, LOW);
          digitalWrite(ledPin, LOW); 
        }
        if(inChar == 'B')
        {
          Serial.print(" CLOSE GATE");
          digitalWrite(out2Pin, HIGH);
          digitalWrite(ledPin, HIGH);
          delay(3000);    
          digitalWrite(out2Pin, LOW); 
          digitalWrite(ledPin, LOW); 
        }  
   } 
}

