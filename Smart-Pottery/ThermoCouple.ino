#include <Adafruit_MAX31856.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
int POWER = 9;
int TEMP_UP = 8;
int TEMP_DOWN = 7;
int ON_OFF_PIN = 6;
float Temperature = 0.0;
float MaxTemperature = 45;
char is_program_on=0;
long temp_hold_started_at = 0;
long temp_hold_end_time = 0;
int is_heating=0;


#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 
static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000 };

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif


// Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31856 max = Adafruit_MAX31856(10, 11, 12, 13);
// use hardware SPI, just pass in the CS pin
//Adafruit_MAX31856 max = Adafruit_MAX31856(10);

void setup() {
  pinMode(POWER, OUTPUT);
  digitalWrite(POWER,LOW);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(1.5);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Welcome :)");
  display.display();
  
  max.begin();
  max.setThermocoupleType(MAX31856_TCTYPE_K);
}

void loop() {
  readOnOff();
  readTempControlButtons();
  prepareScreen();
  
  if(is_program_on){

    //Initiate Hold Temp.
    if(Temperature >= MaxTemperature && temp_hold_started_at == 0) {
      temp_hold_started_at = millis();//Initiate Start Time;
      temp_hold_end_time = temp_hold_started_at + (60000);
    }

    if(Temperature < MaxTemperature)       {
      heat_on();
    }
    else if(temp_hold_started_at > 0 && millis() < temp_hold_end_time){
      hold_heat();
    }
    else if(temp_hold_started_at > 0 && millis() > temp_hold_end_time){
      heat_off();
      is_program_on = 0;
      reset_hold();
    }
    else{ heat_off(); }
    
  }
  else
  {
    heat_off();
    reset_hold();
  }

  if(is_program_on != 1){
    display.println(" OFF");
  }
  else
    if(is_heating==1) 
      display.println("   Heating"); 
    else 
      display.println("   Cooling"); 
    
  display.print("Hold Tem.: "); display.println(MaxTemperature); 

  
  if(millis() < temp_hold_end_time)
    {
      display.print("Holding time: ");
      display.print((temp_hold_end_time-millis())/1000);
      display.println(" Sec.");
    }
    
  display.display();

  check_thermocouple();
 }


void hold_heat(){
  if(Temperature < MaxTemperature)       
    heat_on();
  else
    heat_off();
}

void heat_off(){
  digitalWrite(POWER, LOW);
  is_heating = 0;
}

void heat_on(){
  digitalWrite(POWER, HIGH);  
  is_heating = 1;
}

void readOnOff(){
  if(digitalRead(ON_OFF_PIN)==HIGH)
    if(is_program_on) 
      is_program_on = 0;
    else 
      is_program_on = 1;
}

void reset_hold(){
   temp_hold_started_at = 0;
   temp_hold_end_time = 0;
}

void readTempControlButtons(){
  if(digitalRead(TEMP_UP)==HIGH)
    MaxTemperature++;

  if(digitalRead(TEMP_DOWN)==HIGH)
    MaxTemperature--;
}

void prepareScreen(){
  display.clearDisplay();
  display.setCursor(0,0);
  Temperature=max.readThermocoupleTemperature();
  display.print("T: "); display.print(Temperature);
}
void check_thermocouple(){
  uint8_t fault = max.readFault();
  if (fault) {
    display.clearDisplay();
    display.setCursor(0,0);
    if (fault & MAX31856_FAULT_CJRANGE) display.println("CJ Range Fault");
    if (fault & MAX31856_FAULT_TCRANGE) display.println("TC Range Fault");
    if (fault & MAX31856_FAULT_CJHIGH)  display.println("CJ High Fault");
    if (fault & MAX31856_FAULT_CJLOW)   display.println("CJ Low Fault");
    if (fault & MAX31856_FAULT_TCHIGH)  display.println("TC High Fault");
    if (fault & MAX31856_FAULT_TCLOW)   display.println("TC Low Fault");
    if (fault & MAX31856_FAULT_OVUV)    display.println("Over/Under Voltage Fault");
    if (fault & MAX31856_FAULT_OPEN)    display.println("TC Open Fault");

    display.display();
  }
}


