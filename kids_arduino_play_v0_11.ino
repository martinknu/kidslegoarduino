/*
Created : 2022-12-02
Author : Martin Knudsen
Purpose : To control servo motors for fun and play

Version / Changes: 
0.1 / First version, includes https://playground.arduino.cc/Code/Time/
*/

// ---------------------------------------------------------------------------//
//				 Inclusiions
// ---------------------------------------------------------------------------//

#include <Servo.h> 


// ------------- Time library -------------   
#define TIME_MSG_LEN  11   // time sync to PC is HEADER followed by unix time_t as ten ascii digits
#define TIME_HEADER  'T'   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 

// ------------- Servo library ------------- 
Servo myservo;  // create servo object to control a servo 


// ---------------------------------------------------------------------------//
//				 Declerations I/O
// ---------------------------------------------------------------------------//
// Setup of I/O
// Digital
const int RX = 0;
const int TX = 1;
const int moveOut = 2;
const int PwmA = 3;
const int spare4 = 4;
const int spare5 = 5;
const int spare6 = 6;
const int spare7 = 7;
const int spare8 = 8;
const int spare9 = 9;
const int ServoControl = 10;
const int spare11 = 11;
const int spare12 = 12;
const int LedIndicator = 13;
// Analog
const int CurrentServo = 0;

// Digital I/O
bool diMoveOut;
bool doPwmA;
bool doServoControl;
bool doLedIndicator;

// AnalogIO


// ---------------------------------------------------------------------------//
//				 Declerations other var
// ---------------------------------------------------------------------------//

// UP01
int iUP01Cmd = 1;
int iUP01Seq = 0;
int iUP01SeqLastStep = 0;
bool bUP01SeqStep = false;

unsigned long iUP01_SeqTimer001;
unsigned long iUP01_SeqTimerPre001 = 60000;
bool bUP01_SeqTimerDone001 = false;

unsigned long iUP01_OpenTime = 64000;
unsigned long iUP01_CloseTime = 72000;

float fOpenMaxCurrent = 9.0;
float fCloseMaxCurrent = 11.0;


// EM01
int iEM01Cmd = 0;
int iEM01Seq = 0;
int iEM01SeqLastStep = 0;
bool bEM01SeqStep = false;

unsigned long iEM01_SeqTimer001;
unsigned long iEM01_SeqTimerPre001;
bool bEM01_SeqTimerDone001 = false;



// System variables
bool bFirstScan = true;
bool bFirstScanAux = false;
unsigned long iLastScan;
unsigned long iMilliSec;
unsigned long iMilliSecLast;


// Aux var
int iOpenHour = 9;
int iOpenMin = 00;

int iCloseHour = 17;
int iCloseMin = 0; 

byte bPwmA = 0;
byte bPwmB = 0;

unsigned long iControlLED_on = 0;
unsigned long iControlLED_off = 0;

bool bControlLED_on = false;
bool bControlLED_off = false;

// ---------------------------------------------------------------------------//
//				      Setup
// ---------------------------------------------------------------------------//
void setup() {                

  //pinMode(RX, OUTPUT);
  //pinMode(TX, INPUT);
  pinMode(moveOut, INPUT);
  pinMode(PwmA, OUTPUT);
  pinMode(ServoControl, OUTPUT);
  pinMode(LedIndicator, OUTPUT); 
  
  // Serial begin
  Serial.begin(9600);      // open the serial port at 9600 bps: 

  
  // Servo
  myservo.attach(ServoControl);  // attaches the servo on pin xx to the servo object 

}


// ---------------------------------------------------------------------------//
//				    Subroutines
// ---------------------------------------------------------------------------//

// ---------------------------------------------------------------------------//
//				    Simple timer 
// ---------------------------------------------------------------------------// 

bool SimpleTimer(bool bStart, unsigned long &iTimerVal, unsigned long iTime) {

  bool bTimerDone = false;
    
  // Start timer
  if ( bStart and iTimerVal <= iTime) 
    iTimerVal = iTimerVal + iLastScan; 
  
  if  (!bStart)
    iTimerVal = 0;
    
  // Return result of timer
  if ( bStart and iTimerVal >= iTime)
  {
    bTimerDone = true;
  }
  
  return bTimerDone;
}


// ---------------------------------------------------------------------------//
//				    Scale analog
// ---------------------------------------------------------------------------//
float ScaleAnalog(int iAnalogRead, float fHigh, float fLow) {
  
  float fCalcOut;
  float fAnalogRead; 
  float fNorm;
  float fRange; 
  
  // Read value from integer
  fAnalogRead = (float) iAnalogRead;
  
  // Calculate normalised value 0-100
  fNorm = (float) iAnalogRead/1023;
  
  // Calculate range
  fRange = fHigh - fLow;
  
  // Calculate output
  fCalcOut = fLow + fRange * fNorm; 
  
  return fCalcOut;
}


// ---------------------------------------------------------------------------//
//				    System functions
// ---------------------------------------------------------------------------//
void fncSystem(){
/* Associated system variables
// System variables
bool bFirstScan = true;
bool bFirstScanAux = false;
unsigned long iLastScan;
unsigned long iMilliSec;
unsigned long iMilliSecLast;
*/
 
  
// First scan
  	if ( bFirstScanAux == true )
    {
     	bFirstScan = false; 
    }
  	bFirstScanAux = true;
  

// Timer tags
	// Get the milliseconds  
  	iMilliSec = millis();
  
 	// Find if normal operation or rollover 
  	if (iMilliSec >=  iLastScan) // Normal operation
	{
		iLastScan = iMilliSec - iMilliSecLast; 
	}
	else // Rollover
	{
		iLastScan = iMilliSec  + (4294967295 - iMilliSecLast);
		Serial.print("Rollover\r\n");
    }
	// Save last millisec value
	iMilliSecLast = iMilliSec;   
}





// ---------------------------------------------------------------------------//
//				    Print digits
// ---------------------------------------------------------------------------//
void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}







// ---------------------------------------------------------------------------//
//				   Main Loop
// ---------------------------------------------------------------------------//
void loop() {

//  ------------- Read  DI -------------  
diMoveOut = digitalRead(moveOut);  



//  ------------- Read  AI -------------  

//  ------------- SYSTEM FUNCTIONS -------------
  fncSystem();  



// Open
myservo.write(180);

if(diMoveOut)
{
  myservo.write(30);
  bPwmA = 30;
  Serial.print("Off");
}

// Close
if(not diMoveOut)
{
  myservo.write(180); 
  bPwmA = 180;
  Serial.print("On");  
}


// LED Indicator
bControlLED_on = SimpleTimer(!bControlLED_off, iControlLED_on, 500);
bControlLED_off = SimpleTimer(bControlLED_on, iControlLED_off, 500);      

digitalWrite(LedIndicator, bControlLED_on);
  
  
//  ------------- Write to DO  -------------  
analogWrite(PwmA, bPwmA);


//  ------------ Minimum scan time ------------- 
delay(100);
}
