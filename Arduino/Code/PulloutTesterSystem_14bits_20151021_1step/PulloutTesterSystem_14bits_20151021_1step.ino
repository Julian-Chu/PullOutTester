#include <eRCaGuy_analogReadXXbit.h>

/////////////////////////////////////////////////
//Project Name: Pull out Tester System
//Author: Julian.Chu@ASG
//Version: 1.0
//lastModified: 20150511
//
//20150511: integrated R measurement with 14 bits
//20150529: calibration xxx
//20150610: add Startswitch
//20150610: calibration coefficient:*9/22
//20150813: calibration coefficient: *6/20
//20150824: *25.47/88
//20151014: manual command: 3 steps--> 1step
/////////////////////////////////////////////////

//Oversampling head file
#include<eRCaGuy_analogReadXXbit.h>
eRCaGuy_analogReadXXbit adc;

///Resistance measurement function
//5V voltage source 
//220 ohm resistance
//2 voltage values 

#define V1_PIN A1
#define V2_PIN A2
#define SWITCH_PIN 7

double V1;
double V2;
double R_SMA;
double R_arr[100];
const double R_constant=220;
double current;
const byte V1_input=V1_PIN;
const byte V2_input=V2_PIN;

const float MAX_READING_12_bits=4092.0;
const float MAX_READING_14_bits=16368.0;

int bits_of_precision=14;
int num_samples=4;



///parameter of Stepper motor 
#define START_PIN 13
#define ALMRST_PIN 12
#define STOP_PIN 11
#define M0_PIN 10   //M0: 1 step forward
#define M1_PIN 9   //M1: 1 step backward
boolean Operated=true;
char MotorDirection;  //g: Forward,  b: Backward

///parameter of Load Cell with LCD
#include<LiquidCrystal_I2C.h>
#include<Wire.h>
//#define SWITCH_PIN 7
int InitialValue=0;
int val;
int vals[100];
const byte analogInput=A0;
double force;
LiquidCrystal_I2C lcd(0x27,16,2);
double maxforce=0;


///transfer data to PC
byte inputByte_0;
byte inputByte_1;

///Switch function
const byte StartSwitch=4;

/*---------------------------------------------*/
void setup()
{
//initialize serial port: BaudRate:9600    ex:300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, or 115200
Serial.begin(9600);

//initialize switch pin of Resistance measurement
pinMode(SWITCH_PIN,OUTPUT);
  
//initialize pin and port for stepper motor
pinMode(START_PIN,OUTPUT);
pinMode(STOP_PIN,OUTPUT);
pinMode(ALMRST_PIN, OUTPUT);
pinMode(M0_PIN, OUTPUT);
pinMode(M1_PIN, OUTPUT);
Operated=true;

//initialize pin and LCD for load cell
lcd.init();
lcd.backlight();
maxforce=0;

//initialize switch
pinMode(StartSwitch,INPUT);
}


/*----------------------------------------*/
void loop()
{
  boolean SwitchVal=digitalRead(StartSwitch);
  
  if(Serial.available()>=2)  // SendData2PC
  {
    
    //lcd.println("On");
   // delay(1000);
    inputByte_0=Serial.read();
    delay(100);
    inputByte_1=Serial.read();
    delay(100);
   // Serial.println(inputByte_0);
   // Serial.println(inputByte_1);
    while(Serial.available()>0)
      byte t=Serial.read();
    
      SendData2PC(inputByte_0,inputByte_1);    
      lcd.println("inputByte_0="+inputByte_0);
      inputByte_0=0;
      inputByte_1=0;  
  }
  else if(Serial.available()>0 || SwitchVal==true)        //test turn on
  {
    if(SwitchVal==true)
      MotorDirection='s';
    else
      MotorDirection=Serial.read();
    Operated=false;                   
    Serial.print("MotorDirection is ");
    Serial.print( MotorDirection);
    Serial.println("!!!");
    lcd.setCursor(0,0);
    lcd.print("On");
    lcd.setCursor(0,1);
    lcd.print("Clear max Force");
    maxforce=0;
    delay(500);
  }
    
  else{
    lcd.clear();
    lcd.print("OFF");
    lcd.setCursor(0,1);
    lcd.print("max Force:");
    lcd.print(maxforce);
    delay(500);
    InitialValue=ReadLoadcell();
    lcd.setCursor(0,0);
    lcd.print(InitialValue);
    lcd.print(" , ");
    R_SMA=R_measurement();
    lcd.print("R=");
    lcd.println(R_SMA);
    delay(500);
    
  }
  
  
  /// Moving motor ,reading load cell and measuring  resistance
  
  if(Operated==false)
  {
    
    for(int i=0;i<100;i++)
    {
      vals[i]=0;
      R_arr[i]=0;
    }
    
    InitialValue=ReadLoadcell();
    R_SMA=R_measurement();
    lcd.setCursor(0,0);
    lcd.println(InitialValue);
    lcd.setCursor(0,1);
    lcd.print("R=");
    lcd.println(R_SMA);
  ///Motor moving
  if(MotorDirection=='s')
  {
    
    /*
    for(int i=0;i<10;i++)
    {
      AStepForward();
      delay(100);
      InitialValue=ReadLoadcell();      
    }
    
    for(int i=0;i<10;i++)
    {
      AStepBackward();
      delay(100);
      InitialValue=ReadLoadcell();      
    }
    */
    
    
    for(int i=0;i<100;i++)
    {
      AStepBackward();
      val=ReadLoadcell()-InitialValue;  //calibration -27
      //if(val<0) val=0;
      vals[i]=val;
      R_arr[i]=R_measurement();
      /// show the value on the LCD
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("V=");
      lcd.print(vals[i]);
      lcd.print(" , ");
      lcd.print("R=");
      lcd.print(R_arr[i]);
      
  

//      force=(double)val*9/22;  20150610
//      force=(double)val*6/20;  20150813
      force=(double)val*25.47/88;  //20150824
      if(force>maxforce)
      maxforce=force;
      lcd.setCursor(0,1);
      lcd.print("F=");
      lcd.print(force);   
      Serial.print(i);
      Serial.println(" step");
       
      Serial.print("Value=");
      Serial.println(val);
      Serial.print("Force=");
      Serial.println(force);
      delay(500);
    }
    for(int i=0;i<100;i++)
    {
      AStepForward();
      lcd.setCursor(0,0);
      lcd.print("Returning.....");
      lcd.setCursor(0,1);
      lcd.println();
    }
  }
  else if(MotorDirection=='p')  //for press testing
  {
     // for(int i=0;i<60;i++)
      {
      AStepForward();
      byte p=112;
       //Serial.println(p);      
      }
        
  }
  
  else{
    for(int i=0;i<1;i++)   //1 step
    {
      if(MotorDirection=='b')
        AStepBackward();
      else if(MotorDirection=='g')
        AStepForward();
      /// Read load cell
      val=ReadLoadcell()-InitialValue;
      vals[i]=val;
      /// show the value on the LCD
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("V=");
      lcd.print(vals[i]);
  
      //force=(double)val/870*2;
      force=(double)val*9/22;
      if(force>maxforce)
      maxforce=force;
      lcd.setCursor(0,1);
      lcd.print("F=");
      lcd.print(force);   
      Serial.print(i);
      Serial.println(" step");
       
      Serial.print("Value=");
      Serial.println(val);
      Serial.print("Force=");
      Serial.println(force);
      delay(500);
    }
  }
   // AStepBackward();
  /// Motor is operated
  Operated=true;
  }
}


/*-------------------------------------*/
///Stepping motor function
void AStepForward()
{  
  digitalWrite(STOP_PIN,HIGH);
  digitalWrite(M0_PIN,HIGH);
  digitalWrite(START_PIN,HIGH);
  delay(500);  
  digitalWrite(STOP_PIN,LOW);
  digitalWrite(M0_PIN,LOW);
  digitalWrite(START_PIN,LOW);
  delay(500);
}

void AStepBackward()
{
   digitalWrite(STOP_PIN,HIGH);
  digitalWrite(M1_PIN,HIGH);
  digitalWrite(START_PIN,HIGH);
  delay(500);  
  digitalWrite(STOP_PIN,LOW);
  digitalWrite(M1_PIN,LOW);
  digitalWrite(START_PIN,LOW);
  delay(500); 
}

///Load cell with LCD
double ReadLoadcell()
{
  double value;
  value=analogRead(analogInput);
  return value;
}

void LCDtest()
{
lcd.setCursor(0,0);
lcd.println("LCD test");
lcd.setCursor(0,1);
lcd.println("is done");
}

///SendData2PC
void SendData2PC(byte inputByte_0, byte inputByte_1)
{
  /* Command byte[2]={16,127}  ==>  check the connecton to Arduino;
     Commabd byte[2]={16,128}  ==>  send the load data to PC
  
  */
  if(inputByte_0==16)
  {
    switch(inputByte_1)
    {
      case 126:
      lcd.setCursor(0,0);
      lcd.println("Received 16 126");
      lcd.println("Upload Rdata to PC");
      for(int i=0;i<100;i++)
      {
        Serial.print(R_arr[i]);
        if(i!=99)
          Serial.print(",");        
      }
      break;
            
      ///check communicatuon 
      case 127:
      lcd.setCursor(0,0);
      lcd.println("Received 16 127");
      lcd.clear();
      Serial.println("Connected");  // return the message to PC
      break;
      
      ///send force data to pc
      case 128:
      //lcd.clear();
      lcd.setCursor(0,0);
      lcd.println("Received 16 128");
      //lcd.setCursor(0,1);
      lcd.println("Upload Fdata to PC");
      for(int i=0;i<100;i++)
      {
         Serial.print(vals[i]);
         if(i!=99)
           Serial.print(",");
      }
      break;
        
      default:
      lcd.println("Command error");
    
    }
  }
  else{ lcd.println("Command error or not connect to Arduino"); }

}

//Resistance measurement
double R_measurement()
{
  double R;
  digitalWrite(SWITCH_PIN,HIGH);
  delay(100);
  V1=adc.analogReadXXbit(V1_input,bits_of_precision,num_samples);
 // V1=analogRead(V1_input)/1023*5;
  delay(100);
  V2=adc.analogReadXXbit(V2_input,bits_of_precision,num_samples);
  current=(((V1-V2)/MAX_READING_14_bits)*5)/R_constant;  
  R=((V2/MAX_READING_14_bits)*5)/current;
  /*
      Serial.print("V1=");Serial.print(V1/MAX_READING_14_bits*5,5);Serial.print(" V\n");
    Serial.print("V2=");Serial.print(V2/MAX_READING_14_bits*5,5);Serial.print(" V\n");
    Serial.print("current=");Serial.print(current*1000,5);Serial.print(" mA\n");
    Serial.print("R_SMA=");Serial.print(R_SMA,5);Serial.println(" ohm");
  */
  digitalWrite(SWITCH_PIN,LOW);
  delay(20);
  if(R>100) return 0;
  return R;
  

}
