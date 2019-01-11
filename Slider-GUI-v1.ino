// Slider Control V1
// Mega-Testberichte.de - Marco Kleine-Albers
// Special thanks to Frank for C pointer help :)

//hersteller docs, saint smart
//http://wiki.sainsmart.com/index.php/3.2"_TFT_Touch_LCD

//docs
//https://www.pjrc.com/teensy/td_libs_AccelStepper.html
//https://www.airspayce.com/mikem/arduino/AccelStepper/
//http://domoticx.com/arduino-library-accelstepper/
//good example https://www.brainy-bits.com/setting-stepper-motors-home-position-using-accelstepper/
//example: http://www.schmalzhaus.com/EasyDriver/Examples/EasyDriverExamples.html

//1,8°, 360/1,8 = 200
//200 steps is a full turn in fullstep mode
//use microstepping for smoother motion. see later in code

#include <UTFT.h>
#include <UTouch.h>
#include <AccelStepper.h>

//motor pins
int ENABLE_PIN = 11;
int STEP_PIN = 10;
int DIR_PIN = 8;

//microstepping Pins M1 =A0, M2=A1, M3=A3
//TABELLE, Abschnitt Software: https://www.mega-testberichte.de/testbericht/einen-kameraslider-motorisieren-arduino-a4988-steppermotor-touchdisplay-do-it-yourself
//pin mappings 
int M1 = A0;
int M2 = A1;
int M3 = A2;

//when we do 1/2. 1/4, 1/8 odr 1/16 we multiply the base fullsteps with 2,4,5 or 16
//fullstep set to 1
int step_multiplier = 16;

//1 is the DRIVER interface as i use a driver board
AccelStepper stepper(1, STEP_PIN, DIR_PIN); 

//@todo use more SYMBOLS instead of text!
// Declare which fonts we will be using
// http://www.rinkydinkelectronics.com/r_fonts.php
extern uint8_t BigFont[]; 
extern uint8_t SmallFont[]; 
extern uint8_t Sinclair_M[]; 
extern uint8_t Various_Symbols_16x32_v2[]; 


// Remember to change the model parameter to suit your display module!
//see examples
//taken from: http://wiki.sainsmart.com/index.php/3.2"_TFT_Touch_LCD
UTFT        LCD(ITDB32S, 38, 39, 40, 41); 
UTouch      Touch(6, 5, 4, 3, 2);

//debug/msg
int x, y;
String msg="";
char text_buffer[80];

//--button for speed
long int speed_set = 10*step_multiplier;
long int raise_speed_by = 10*step_multiplier;
long int speed_max = 400*step_multiplier;

//--needed for timelapse, buttons
int time_set = 60; //later multiplied with 1000, because the delay funct want millisec
int raise_time_by = 10;
int time_max = 400;

//--steps for timelapse
long int steps_set = 10*step_multiplier; //200 steps is a full turn in fullstep mode, in 1/16 a full turn is 16*200=3200
long int steps_max = 200*step_multiplier;
long int raise_steps_by = 10*step_multiplier; //raised by steps

//--slider length in steps
//slider length is 90 cm = 4000 fullsteps, with 1/16 it is 4000*16=64000
long int slider_length = 64000;  
//1/16 31000
//fullstep 4000

/*************************
**   Draw UI  **
*************************/
void drawButtons() {

  //-------------------Draw a Start/NORMAL button for NORMAL
  //solid
  LCD.setColor(77, 188, 37);
  LCD.fillRoundRect (10, 10, 210, 60);
  //border
  LCD.setColor(255, 255, 255);
  LCD.drawRoundRect (10, 10, 210, 60);

  //the text
  LCD.setColor(255, 255, 255);
  LCD.setBackColor(77, 188, 37);
  LCD.print("NORMAL", 20, 26);

  //---------------------Draw a Start button for ACCELERATED
  //solid
  LCD.setColor(255, 153, 51);
  LCD.fillRoundRect (10, 70, 210, 120);
  //border
  LCD.setColor(255, 255, 255);
  LCD.drawRoundRect (10, 70, 210, 120);

  //the text
  LCD.setColor(255, 255, 255);
  LCD.setBackColor(255, 153, 51);
  LCD.print("ACCELERATED", 20, 86);

  //---------------------Draw SPEED Button (number)
  //solid
  LCD.setColor(0, 0, 255);
  LCD.fillRoundRect (220, 10, 310, 60);
  //border
  LCD.setColor(255, 255, 255);
  LCD.drawRoundRect (220, 10, 310, 60);

  //the text
  LCD.setColor(255, 255, 255);
  LCD.setBackColor(0, 0, 255);    
  int_to_string(speed_set, text_buffer); 
  LCD.print(text_buffer, 237, 26);

  //-------------------Draw a START
  //solid
  LCD.setColor(77, 188, 37);
  LCD.fillRoundRect (10, 180, 100, 220);
  //border
  LCD.setColor(255, 255, 255);
  LCD.drawRoundRect (10, 180, 100, 220);  

  //the text
  LCD.setColor(255, 255, 255);
  LCD.setBackColor(77, 188, 37);
  LCD.print("START", 15, 193);  

  //-------------------Draw a BACKW
   //solid
  LCD.setColor(255, 153, 51);
  LCD.fillRoundRect (110, 180, 200, 220);
  //border
  LCD.setColor(255, 255, 255);
  LCD.drawRoundRect (110, 180, 200, 220);  

  //the text
  LCD.setColor(255, 255, 255);
  LCD.setBackColor(255, 153, 51);
  LCD.print("BACKW", 115, 193);

  //-------------------Draw a HOME
  //solid
  LCD.setColor(255, 0, 0);
  LCD.fillRoundRect (210, 180, 310, 220);
  //border
  LCD.setColor(255, 255, 255);
  LCD.drawRoundRect (210, 180, 310, 220);  

  //the text
  LCD.setColor(255, 255, 255);
  LCD.setBackColor(255, 0, 0);
  LCD.print("HOME", 230, 193);

  //-------------------Draw a TIMELAPSE START
  //solid    
  LCD.setColor(82, 7, 80);
  LCD.fillRoundRect (10, 130, 150, 170);
  //border
  LCD.setColor(255, 255, 255);
  LCD.drawRoundRect (10, 130, 150, 170);  

  //the text  
  LCD.setColor(255, 255, 255);
  LCD.setBackColor(82, 7, 80);
  
  //change font
  LCD.setFont(Various_Symbols_16x32_v2);
  LCD.print("z", 20, 135); //sanduhr

  //back to std font
  LCD.setFont(Sinclair_M);
  LCD.print("START", 45, 143);  

  //---------------------Draw TIME Button (set number)
  //solid
  LCD.setColor(0, 0, 255);
  LCD.fillRoundRect (160, 130, 220, 170);
  //border
  LCD.setColor(255, 255, 255);
  LCD.drawRoundRect (160, 130, 220, 170);

  //the text
  LCD.setColor(255, 255, 255);
  LCD.setBackColor(0, 0, 255);    
  int_to_string(time_set, text_buffer); 
  LCD.print(text_buffer, 170, 143);

  //---------------------Draw A STEPS Button (set number)
  //solid
  LCD.setColor(68, 0, 31);
  LCD.fillRoundRect (230, 130, 310, 170);
  //border
  LCD.setColor(255, 255, 255);
  LCD.drawRoundRect (230, 130, 310, 170);

  //the text
  LCD.setColor(255, 255, 255);
  LCD.setBackColor(68, 0, 31);    
  int_to_string(steps_set, text_buffer); 
  LCD.print(text_buffer, 240, 143);  
}

/***************************************************************************
 * update string so we see what has been touched - debug help
 ****************************************************************************/
void updateStr(String msg){ 
    //clear the area first or other chars will stay if not overriden
    //is there a better way to do this?
    LCD.setColor(0, 0, 0);
    LCD.fillRect(0, 224, 319, 239);     
    
    LCD.setColor(255, 255, 255);
    LCD.print(msg, LEFT, 224);    
}

/****************************************************************************
 * Draw a border while a button is touched  
 ****************************************************************************/
void touchBorder(int x1, int y1, int x2, int y2) {
  LCD.setColor(0, 0, 255);
  LCD.drawRoundRect (x1, y1, x2, y2);
  while (Touch.dataAvailable())
    Touch.read();
  LCD.setColor(255, 255, 255);
  LCD.drawRoundRect (x1, y1, x2, y2);
}


/*****************************************************************************
 * convert X to String
 ****************************************************************************/
void int_to_string(int val, char* string) {
  if(string) 
     sprintf(string, "%d", val);  
  return;  
}

/*************************
**  Required functions  **
*************************/
void setup() {  

  //motor
  pinMode(ENABLE_PIN, INPUT);
  digitalWrite(ENABLE_PIN, HIGH); 

  //mircosteppin
  pinMode(M1, OUTPUT);
  pinMode(M2, OUTPUT);
  pinMode(M3, OUTPUT);

  //1/16 had the lowest vibration
  //see table for step values: https://forum.arduino.cc/index.php?topic=415724.0
  digitalWrite(M1, HIGH);
  digitalWrite(M2, HIGH);
  digitalWrite(M3, HIGH);

  // Initial setup for LCD
  //@todo is it possible to turn the lcd off with this model? Seems no.
  LCD.InitLCD();
  LCD.clrScr();

  //touch precision
  Touch.InitTouch();
  Touch.setPrecision(PREC_HI);  

  //std font for button drawings
  LCD.setFont(Sinclair_M);
  
  //call function to draw our gui  
  drawButtons();
  
  LCD.setBackColor(0, 0, 255);  

  Serial.begin(9600);
  Serial.println("Starting Stepper Test\r\n");  
  
  //Setup for motor
  //disable the power of the motor
  stepper.disableOutputs();
  
  //Set Max Speed and Acceleration of each Steppers at startup
  stepper.setMaxSpeed(1000); //Set Max Speed of Stepper (Slower to get better accuracy). Steps per second
  stepper.setAcceleration(100); // Set Acceleration of Stepper
  stepper.setSpeed(400);    
  
  //debug test
  //  stepper.runToNewPosition(0);
  //  stepper.runToNewPosition(500);
  //  stepper.runToNewPosition(100);
  //  stepper.runToNewPosition(120); 
  //  stepper.runToNewPosition(0); 

}

void loop() {

    //touchscreen
    if (Touch.dataAvailable()){
      Touch.read();
      x = Touch.getX();
      y = Touch.getY();  

      //debug
      Serial.print("Touch X is:");
      Serial.print(x);
      Serial.print("\r\n");

      Serial.print("Touch Y is:");
      Serial.print(y);
      Serial.print("\r\n");

      //check the area where we drew the buton for touch

      //--------------------------NORMAL button 
      if ((y >= 10) && (y <= 60) && (x >= 10) && (x <= 210) ) { 
        touchBorder(10, 10, 210, 60);
        updateStr("Start Motor");          
        
        //enable disable power manuelly
        digitalWrite(ENABLE_PIN, LOW);           
        
        stepper.setCurrentPosition(0); //must be set to 0 because manual moving START/BACKW/HOME may have changed the position and normal must always begin at start/0
        stepper.moveTo(slider_length);

        /****************************************************************************************************/
        /* IMPROTANT: DO NOT USE Serial.print in hihly repeated loops. Slows down everything. See code above
        /****************************************************************************************************/
        
        while (stepper.currentPosition() != slider_length) {
          //Serial.print("START");                    //DO NOT ACTIVATE THIS IT WILL SLOW DOWN THE LOOP MASSIVELY
          //Serial.print(stepper.currentPosition());  //DO NOT ACTIVATE THIS IT WILL SLOW DOWN THE LOOP MASSIVELY
          //Serial.print("\r\n");                    //DO NOT ACTIVATE THIS IT WILL SLOW DOWN THE LOOP MASSIVELY          
          stepper.setSpeed(speed_set);  
          stepper.run();          
        }               

        //after the run move back to position 0. that happens with acceleration
        //Serial.print("END");        
        stepper.runToNewPosition(0);        
        digitalWrite(ENABLE_PIN, HIGH);             
      }

      //---------------------ACCELERATE button 10, 70, 210, 120
      if ((y >= 70) && (y <= 120) && (x >= 10) && (x <= 210) ) { 
        touchBorder(10, 70, 210, 120);
        updateStr("Accelerate Motor");    
        
        digitalWrite(ENABLE_PIN, LOW);    
        stepper.setCurrentPosition(0); //must be set to 0 because manual moving START/BACKW/HOME may have changed the position and normal must always begin at start/0        
        
        stepper.setSpeed(speed_set);  
        stepper.runToNewPosition(slider_length);  

        //after the run move back to position 0
        stepper.runToNewPosition(0);        
        digitalWrite(ENABLE_PIN, HIGH);               
      }

      //---------------------START button 10, 180, 100, 220   
      if ((y >= 180) && (y <= 220) && (x >= 10) && (x <= 100) ) {         
        //touchBorder(10, 170, 100, 220); //can not use that here because it will block because we press and hold       
        updateStr("START");   
  
        digitalWrite(ENABLE_PIN, LOW); 
        Serial.print("start run start at pos \r\n"); 
        Serial.print(stepper.currentPosition());
        Serial.print("\r\n");    
        
        stepper.setAcceleration(100);
        stepper.setSpeed(speed_set);   

        while (Touch.dataAvailable() == true) {      
          stepper.setSpeed(speed_set);  
          stepper.run();  
          //Serial.print("running by button pressed normal \r\n");  //DO NOT ACTIVATE THIS IT WILL SLOW DOWN THE LOOP MASSIVELY       
        }        

        //stop completely after, otherwise movement is still there
        stepper.stop();
        Serial.print("normal run stopped \r\n"); 
        Serial.print(stepper.currentPosition());
        Serial.print("\r\n"); 
               
        digitalWrite(ENABLE_PIN, HIGH);                            
      }

     //---------------------BACKW button 110, 170, 200, 220
     if ((y >= 180) && (y <= 220) && (x >= 110) && (x <= 200) ) { 
        //touchBorder(110, 170, 200, 220); //can not use that here because it will block      
        updateStr("BACKW");         
              
        digitalWrite(ENABLE_PIN, LOW); 
        Serial.print("backward run start at pos \r\n"); 
        Serial.print(stepper.currentPosition());
        Serial.print("\r\n");    
        
        stepper.setAcceleration(100);
        stepper.setSpeed(speed_set);  

        while (Touch.dataAvailable() == true) {
          stepper.setSpeed(-speed_set);  
          stepper.run();          
          //Serial.print("running by button pressed backwards"); //DO NOT ACTIVATE THIS IT WILL SLOW DOWN THE LOOP MASSIVELY
        }    

        Serial.print("backward run stopped \r\n"); 
        Serial.print(stepper.currentPosition());
        Serial.print("\r\n");      

        //stop completely after, otherwise movement is still there
        stepper.stop();
        //Serial.print("backward run stopped"); 
        
        digitalWrite(ENABLE_PIN, HIGH);                           
      }     

      //--------------------->HOME button 210, 170, 310, 220
      if ((y >= 180) && (y <= 220) && (x >= 210) && (x <= 310) ) { 
        touchBorder(210, 180, 310, 220);
        updateStr("HOME"); 

        digitalWrite(ENABLE_PIN, LOW);

        Serial.print("Home start, current POS is:");          
        Serial.print(stepper.currentPosition());
        Serial.print("\r\n");         
    
        stepper.setSpeed(speed_set);         
        stepper.setCurrentPosition(stepper.currentPosition()); //set correct position because its not known by default

        //after the run move back to position 0
        stepper.runToNewPosition(0);             
        
        digitalWrite(ENABLE_PIN, HIGH);                     
      }      

      //------------------------SPEED button with UPDATE of number 220, 10, 310, 60
      if ((y >= 10) && (y <= 60) && (x >= 220) && (x <= 310) ) { 

        //one touch raises speed by X up to "speed_max"
        if(speed_set < speed_max) {
          speed_set = speed_set + raise_speed_by;  
        }
        else {
          speed_set = 10;
        }        

        //if i dont set that here the manual START/REWIND will not work after i change the speed and press them again. why....
        stepper.setSpeed(speed_set);  

        // Draw Button
        //solid
        LCD.setColor(0, 0, 255);
        LCD.fillRoundRect (220, 10, 310, 60);
        //border
        LCD.setColor(255, 255, 255);
        LCD.drawRoundRect (220, 10, 310, 60);
      
        //the text
        LCD.setColor(255, 255, 255);
        LCD.setBackColor(0, 0, 255);
        
        int_to_string(speed_set, text_buffer); 
        LCD.print(text_buffer, 237, 26);        
        
        touchBorder(220, 10, 310, 60);
        updateStr("Speed");
      }
      
      //------------------------TIMELAPSE START button START 10, 130, 150, 170
      if ((y >= 130) && (y <= 170) && (x >= 10) && (x <= 150) ) { 
    
        int_to_string(speed_set, text_buffer); 
        LCD.print(text_buffer, 237, 26);  

        Serial.print("Starting Timelapse\r\n");  

        //@good blog, method for using a cable to the camera for timalapse https://photoscs.wordpress.com/2014/03/25/arduino-stepper-code-time-lapse/

        //enable disable power manually
        digitalWrite(ENABLE_PIN, LOW); //motor an             

        Serial.print("Timelapse start, current POS is:");          
        Serial.print(stepper.currentPosition());
        Serial.print("\r\n"); 

        //see: http://forum.arduino.cc/index.php?topic=206796.0
        //multiplied with 1000, we need millicseconds //use millis() instead later, non blocking
        long ms_time = time_set*1000L;       
        Serial.print("Delay time set in MS:\r\n"); 
        Serial.print(ms_time);
        Serial.print("\r\n");
        
        while (stepper.currentPosition() != slider_length) {
          
          stepper.runToNewPosition(steps_set);
          steps_set = steps_set+ raise_steps_by;         
          stepper.run();  
          digitalWrite(ENABLE_PIN, HIGH); //motor aus           
          delay(ms_time); //wait, delay     
          digitalWrite(ENABLE_PIN, LOW);  //motor an   
          
          //Serial.print("Timelapse running, current POS is:");  //DO NOT ACTIVATE THIS IT WILL SLOW DOWN THE LOOP MASSIVELY        
          //Serial.print(stepper.currentPosition());             //DO NOT ACTIVATE THIS IT WILL SLOW DOWN THE LOOP MASSIVELY
          //Serial.print("\r\n");                                //DO NOT ACTIVATE THIS IT WILL SLOW DOWN THE LOOP MASSIVELY
        }
            
        digitalWrite(ENABLE_PIN, HIGH); //finish. motor off            

        //we start the timelapse. Move Motor by x              
        touchBorder(10, 130, 150, 170);
        updateStr("TIMELAPSE");
      }     

      //------------------------TIME button with UPDATE of number 160, 130, 220, 170
      if ((y >= 130) && (y <= 170) && (x >= 160) && (x <= 220) ) { 

        //one touch raises time by X up to time_max
        if(time_set < time_max) {
          time_set = time_set + raise_time_by;  
        }
        else {
          time_set = 10;
        }             

        // Draw Button
        //solid
        LCD.setColor(0, 0, 255);
        LCD.fillRoundRect (160, 130, 220, 170);
        //border
        LCD.setColor(255, 255, 255);
        LCD.drawRoundRect (160, 130, 220, 170);      
        //the text
        LCD.setColor(255, 255, 255);
        LCD.setBackColor(0, 0, 255);
        
        int_to_string(time_set, text_buffer); 
        LCD.print(text_buffer, 170, 143);                
        
        touchBorder(160, 130, 220, 170);
        updateStr("Time");
      }      

       //------------------------STEPS button with UPDATE of number 230, 130, 310, 170
      if ((y >= 130) && (y <= 170) && (x >= 230) && (x <= 310) ) {
        //one touch raises steps by 10 up to steps_max
        if(steps_set < steps_max) {
          steps_set = steps_set + raise_steps_by;  
        }
        else {
          steps_set = 10;
        }             

        // Draw Button
        //solid
        LCD.setColor(68, 0, 31);
        LCD.fillRoundRect (230, 130, 310, 170);
        //border
        LCD.setColor(68, 0, 31);
        LCD.drawRoundRect (230, 130, 310, 170);
      
        //the text
        LCD.setColor(255, 255, 255);
        LCD.setBackColor(68, 0, 31);
        
        int_to_string(steps_set, text_buffer); 
        LCD.print(text_buffer, 240, 143);                
        
        touchBorder(230, 130, 310, 170);
        updateStr("Steps");
      }    
      
    }

      
    
  
}
