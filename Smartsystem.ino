#include "DHT.h"
#include <LiquidCrystal.h>

#define FERTILISER_OFF_TIME 300000 // 300 seconds = 5 minutes
#define FERTILISER_ON_TIME  60000 // 60 seconds = 1 minute

#define  TempSensor      A2
#define  LDR             A0
#define  DHT11_PIN       2
#define  MoistureSensor  A3

#define FAN              12
#define LED              6
#define SPRINKLER        7
#define SPRAYER          3

#define  TempSensor2      A9
#define  LDR2             A8
#define  DHT11_PIN2       22
#define  MoistureSensor2  A10

#define FAN2              24
#define LED2              28
#define SPRINKLER2        30
#define SPRAYER2          26

#define FERTILISER        13

#define MIN_MOISTURE_ADC  70.0
#define MAX_MOISTURE_ADC  680.0

#define NUM_PROFILES      3  
#define NUM_BOARDS        2

#define SPRAY_OFF_TIME    5000

DHT dht ( DHT11_PIN, DHT11 );
DHT dht2 ( DHT11_PIN2, DHT11 );

LiquidCrystal lcd ( 4, 5, 8, 9, 10, 11 ); // LCD (RS, EN, D4, D5, D6, D7);

static const float tempProfileMin [ NUM_PROFILES ] = { 20, 13, 20};
static const float tempProfileMax [ NUM_PROFILES ] = { 29, 18, 30};
static const float brightnessProfile [ NUM_PROFILES ] = { 200, 200, 200};
static const float moistProfileMin [ NUM_PROFILES ] = { 43, 50, 55};
static const float moistProfileMax [ NUM_PROFILES ] = { 75, 65, 65 };
static const float humidityProfile [ NUM_PROFILES ] = { 57, 45, 45};

unsigned char profileIndex;
unsigned long fertiliserTime; // to store time for turning on/off the fertiliser
boolean isFertilising = false; // to store the state of the fertiliser (false = now fertiliser off, true = now fertiliser on)
boolean isSpraying [ 2 ] = { false, false };
unsigned long sprayOffTime [ 2 ];
int currBoard = 0; // which layer's measurement to show on LCD (0 = layer 1, 1 = layer 2)

void setup ( void )
{
  char layer;

  pinMode ( FAN, OUTPUT );
  pinMode ( LED, OUTPUT );
  pinMode ( SPRINKLER, OUTPUT );
  pinMode ( SPRAYER, OUTPUT );
  pinMode ( FAN2, OUTPUT );
  pinMode ( LED2, OUTPUT );
  pinMode ( SPRINKLER2, OUTPUT );
  pinMode ( SPRAYER2, OUTPUT );
  pinMode ( FERTILISER, OUTPUT );

  digitalWrite ( FAN, LOW );
  digitalWrite ( LED, LOW );
  digitalWrite ( SPRINKLER, LOW );
  digitalWrite ( SPRAYER, LOW );
  digitalWrite ( FAN2, LOW );
  digitalWrite ( LED2, LOW );
  digitalWrite ( SPRINKLER2, LOW );
  digitalWrite ( SPRAYER2, LOW );
  digitalWrite ( FERTILISER, LOW );

  lcd.begin ( 16, 2 );

  lcd.setCursor ( 0, 0 );
  lcd.print ( "Please Select" );

  lcd.setCursor ( 0, 1 );
  lcd.print ( "Profile..." );

  dht.begin(); dht2.begin();

  Serial.begin ( 57600 );

  Serial.println ( "Available Profiles" );
  Serial.println ( "1) Spinach" );
  Serial.println ( "2) Broccoli" );
  Serial.println ( "3) Parsley" );

  while ( Serial.available() )
    Serial.read();

  do
  {
    Serial.print ( "\nEnter profile: " );

    while ( Serial.available() == 0 ) // wait user press something
      continue;

    profileIndex = Serial.read(); // read from user selected profile
    Serial.write ( profileIndex ); // print out the selection
  } while ( profileIndex <= '0' || profileIndex > ( '0' + NUM_PROFILES ) ); // keep looking for 0 till 3.

  profileIndex -= '1';

  fertiliserTime = millis() / 1000 + FERTILISER_OFF_TIME;

  Serial.print ( "\nProfile selected: " );

  lcd.clear();
  lcd.setCursor ( 0, 0 );
  lcd.print ( "Profile Selected" );

  lcd.setCursor ( 0, 1 );

  switch ( profileIndex )
  {
    case 0: Serial.println ( "Spinach" ); lcd.print ( "Spinach" ); break;
    case 1: Serial.println ( "Broccoli" ); lcd.print ( "Broccoli" ); break;
    case 2: Serial.println ( "Parsley" ); lcd.print ( "Parsley" ); break;
  }

  delay ( 1000 );

  while ( Serial.available() )
    Serial.read();

  lcd.clear();
  lcd.setCursor ( 0, 0 );
  lcd.print ( "Please Select" );

  lcd.setCursor ( 0, 1 );
  lcd.print ( "Layer..." );

  do
  {
    Serial.println ( "Please press 1 to view layer 1 readings, or 2 to view layer 2 readings." );

    while ( Serial.available() == 0 ) // wait user press something
      continue;

    layer = Serial.read(); // read from user selected profile
    Serial.write ( layer ); // print out the selection
  } while ( layer != '1' && layer != '2' ); // keep looking for 1 or 2

  currBoard = layer - '1';

  lcd.clear();
  lcd.setCursor ( 0, 0 );
  lcd.print ( "Layer Selected" );

  lcd.setCursor ( 0, 1 );

  switch ( currBoard )
  {
    case 0: Serial.println ( "Layer 1" ); break;
    case 1: Serial.println ( "Layer 2" ); break;
  }

  delay ( 1000 );

  lcd.clear();

  fertiliserTime = millis() + 100;
}

void loop ( void )
{
  int temp [ NUM_BOARDS ], bright [ NUM_BOARDS ], moisture [ NUM_BOARDS ], boardIndex;
  float temp_celcius [ NUM_BOARDS ], bright_voltage [ NUM_BOARDS ], moisture_voltage [ NUM_BOARDS ], moisture_percent [ NUM_BOARDS ], humidity [ NUM_BOARDS ];
  double brightness [ NUM_BOARDS ];
  char command;

  while ( Serial.available() > 0 ) // continue reading command from Serial Monitor, so long as there is data on serial port
  {
    command = Serial.read();

    switch ( command )
    {
      case '1':
        currBoard = 0;
        Serial.println ( "Layer 1 readings being displayed on LCD!" );
        break;

      case '2':
        currBoard = 1;
        Serial.println ( "Layer 2 readings being displayed on LCD!" );
        break;
    }
  }

  if ( millis() >= fertiliserTime )
  {
    if ( !isFertilising )
    {
      fertiliserTime = millis() + FERTILISER_ON_TIME;
      digitalWrite ( FERTILISER, HIGH );
    }
    else
    {
      fertiliserTime = millis() + FERTILISER_OFF_TIME;
      digitalWrite ( FERTILISER, LOW );
    }

    isFertilising = !isFertilising;
  }

  temp [ 0 ] = analogRead ( TempSensor );
  bright [ 0 ] = analogRead ( LDR );
  moisture [ 0 ] = analogRead ( MoistureSensor );
  temp [ 1 ] = analogRead ( TempSensor2 );
  bright [ 1 ] = analogRead ( LDR2 );
  moisture [ 1 ] = analogRead ( MoistureSensor2 );

  humidity [ 0 ] = dht.readHumidity() * 94.0 / 54;
  humidity [ 1 ] = dht2.readHumidity();

  for ( boardIndex = 0; boardIndex < NUM_BOARDS; ++boardIndex )
  {
    temp_celcius [ boardIndex ] = temp [ boardIndex ] / 10.23 * 5.0;
    bright_voltage [ boardIndex ] = bright [ boardIndex ] / 1023.0 * 5.0;
    moisture_voltage [ boardIndex ] = moisture [ boardIndex ] / 1023.0 * 5.0;

    if ( moisture [ boardIndex ] <= MIN_MOISTURE_ADC )
      moisture_percent [ boardIndex ] = 0.0;
    else if ( moisture [ boardIndex ] >= MAX_MOISTURE_ADC )
      moisture_percent [ boardIndex ] = 100.0;
    else
      moisture_percent [ boardIndex ] = ( moisture [ boardIndex ] - MIN_MOISTURE_ADC ) / ( MAX_MOISTURE_ADC - MIN_MOISTURE_ADC ) * 100;

    brightness [ boardIndex ] = 500.0 / ( 10.0 * bright_voltage [ boardIndex ] / ( 5 - bright_voltage [ boardIndex ] ) ) * 5;
  }

  brightness [ 0 ] *= 160.0 / 680.0;
  brightness [ 1 ] = ( brightness [ 1 ] + 20 ) * ( 200.0 / 112.0 );


  lcd.setCursor ( 0, 0 );
  lcd.print ( "T:" ); lcd.print ( temp_celcius [ currBoard ], 1 ); lcd.write ( 0xdf );
  lcd.print ( "C  " );

  lcd.setCursor ( 9, 0 );
  lcd.print ( "B:" ); lcd.print ( brightness [ currBoard ], 0 );
  lcd.print ( "Lux  " );

  if ( brightness [ 0 ] <= brightnessProfile [ profileIndex ] )
    digitalWrite ( LED, HIGH );
  else
    digitalWrite ( LED, LOW );

  if ( brightness [ 1 ] <= brightnessProfile [ profileIndex ] )
    digitalWrite ( LED2, HIGH );
  else
    digitalWrite ( LED2, LOW );

  lcd.setCursor ( 0, 1 );
  lcd.print ( "M:" ); lcd.print ( moisture_percent [ currBoard ] );
  lcd.print ( "%  " );
  //lcd.print ( "   " );

  if ( !isnan ( humidity [ currBoard ] ) )
  {
    lcd.setCursor ( 9, 1 );
    lcd.print ( "H:" ); lcd.print ( humidity [ currBoard ], 1 ); lcd.print ( "%   " );
  }

  if ( temp_celcius [ 0 ] > tempProfileMax [ profileIndex ] )
    digitalWrite ( FAN, HIGH );
  else if ( temp_celcius [ 0 ] < tempProfileMin [ profileIndex ] )
    digitalWrite ( FAN, LOW );

  if ( temp_celcius [ 1 ] > tempProfileMax [ profileIndex ] )
    digitalWrite ( FAN2, HIGH );
  else if ( temp_celcius [ 1 ] < tempProfileMin [ profileIndex ] )
    digitalWrite ( FAN2, LOW );

  if ( moisture_percent [ 0 ] < moistProfileMin [ profileIndex ] )
    digitalWrite ( SPRINKLER, HIGH );
  else
    digitalWrite ( SPRINKLER, LOW );

  if ( moisture_percent [ 1 ] < moistProfileMin [ profileIndex ] )
    digitalWrite ( SPRINKLER2, HIGH );
  else
    digitalWrite ( SPRINKLER2, LOW );

  if ( isSpraying [ 0 ] && millis() >= sprayOffTime [ 0 ] )
      isSpraying [ 0 ] = false;

  if ( isSpraying [ 1 ] && millis() >= sprayOffTime [ 1 ] )
      isSpraying [ 1 ] = false;

  if ( !isSpraying [ 0 ] && humidity [ 0 ] < humidityProfile [ profileIndex ] )
  {
    sprayOffTime [ 0 ] = millis() + SPRAY_OFF_TIME;
    isSpraying [ 0 ] = true;
    digitalWrite ( SPRAYER, HIGH );
  }
  else
    digitalWrite ( SPRAYER, LOW );

  if ( !isSpraying [ 1 ] && humidity [ 1 ] < humidityProfile [ profileIndex ] )
  {
    sprayOffTime [ 1 ] = millis() + SPRAY_OFF_TIME;
    isSpraying [ 1 ] = true;
    digitalWrite ( SPRAYER2, HIGH );
  }
  else
    digitalWrite ( SPRAYER2, LOW );

  Serial.print ( "Temperature Sensor Values (Layer 1, Layer 2): " );
  Serial.print ( temp_celcius [ 0 ] ); Serial.print ( ", " ); Serial.println ( temp_celcius [ 1 ] );

  Serial.print ( "Moisture Sensor Values (Layer 1, Layer 2): " );
  Serial.print ( moisture_percent [ 0 ] ); Serial.print ( ", " ); Serial.println ( moisture_percent [ 1 ] );

  Serial.print ( "Humidity Sensor Values (Layer 1, Layer 2): " );
  Serial.print ( humidity [ 0 ] ); Serial.print ( ", " ); Serial.println ( humidity [ 1 ] );

  Serial.print ( "Brightness Sensor Values (Layer 1, Layer 2): " );
  Serial.print ( brightness [ 0 ] ); Serial.print ( ", " ); Serial.println ( brightness [ 1 ] );
  //Serial.print ( "Relay Layer 1:" );Serial.print(FAN, BIN);Serial.print ( ", " );Serial.print(LED, BIN);Serial.print ( ", " );Serial.print(SPRINKLER, BIN);Serial.print ( ", " );Serial.println(SPRAYER, BIN);
  //Serial.print ( "Relay Layer 1:" );Serial.print(FAN2, BIN);Serial.print ( ", " );Serial.print(LED2, BIN);Serial.print ( ", " );Serial.print(SPRINKLER2, BIN);Serial.print ( ", " );Serial.println(SPRAYER2, BIN);


  delay ( 1000 );
}

