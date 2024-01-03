#include <mbed.h>
#define _USE_MATH_DEFINES
#include <math.h>
//this file has all the functions for interacting
//with the screen
#include "drivers\\LCD_DISCO_F429ZI.h" // or "drivers/LCD_DISCO_F429ZI.h"
#define BACKGROUND 1
#define FOREGROUND 0
#define GRAPH_PADDING 5
#define PI 3.14159265358979323846

LCD_DISCO_F429ZI lcd;
//buffer for holding displayed text strings
char display_buf[2][60];
uint32_t graph_width=lcd.GetXSize()-2*GRAPH_PADDING;
uint32_t graph_height=graph_width;

volatile int flag = 0;

SPI spi(PF_9, PF_8, PF_7); // MOSI, MISO, SCLK
DigitalOut cs(PC_1);

void Flagset();
void Modeset();
int16_t read_data(int code);
void calculatedistance(int16_t temp_data);
double trav_dist = 0;

void Flagset() {          
  flag = 1;
}

void Modeset() {          
  cs=0;
  spi.write(0x20);
  spi.write(0xCF);
  cs=1;
}


int16_t read_data(int code){
  cs = 0;
  spi.write(code);//X_L
  uint8_t xl =spi.write(0x00);
  cs = 1;
  cs = 0;
  spi.write(code+1);
  int8_t xh =spi.write(0x00);
  cs = 1;
  int16_t data= xh*256+xl;
  return data;
}

void calculatedistance(int16_t temp_data){
  double real_data;
  if(abs(temp_data) > 3000) {
    // Transform to angle velocity. 
    real_data = (0.00875 * abs(temp_data + 60)); // here 60 is the offset value from calibration
    trav_dist+=(double)(0.05 * real_data / 360 * 2 * PI * 1); // leg length of 1 foot is considered
    printf("Distance: %d meters\n",(int)trav_dist);
  }

}

//sets the background layer 
//to be visible, transparent, and
//resets its colors to all black
void setup_background_layer(){
  lcd.SelectLayer(BACKGROUND);
  lcd.Clear(LCD_COLOR_BLACK);
  lcd.SetBackColor(LCD_COLOR_BLACK);
  lcd.SetTextColor(LCD_COLOR_GREEN);
  lcd.SetLayerVisible(BACKGROUND,ENABLE);
  lcd.SetTransparency(BACKGROUND,0x7Fu);
}

//resets the foreground layer to
//all black
void setup_foreground_layer(){
    lcd.SelectLayer(FOREGROUND);
    lcd.Clear(LCD_COLOR_BLACK);
    lcd.SetBackColor(LCD_COLOR_BLACK);
    lcd.SetTextColor(LCD_COLOR_LIGHTGREEN);
}

// Main-code
int main() {

      cs=1;
  Modeset();
  spi.format(8,3);
  spi.frequency(100000);
	Ticker t;
  t.attach(&Flagset,0.05);
  uint16_t iter = 0;


  while(1) {
    if(flag){
      int16_t dataZ = read_data(0xAC);
      // Real Time distance calculation, shown in Demo video, outputs the total distance while moving
      calculatedistance(dataZ);

      setup_background_layer();

      setup_foreground_layer();

        //creates c-strings in the display buffers, in preparation
        //for displaying them on the screen
        snprintf(display_buf[0],60,"DISTANCE: %d METERS",(int)trav_dist);

        lcd.SelectLayer(FOREGROUND);
        //display the buffered string on the screen

        lcd.DisplayStringAt(0, LINE(17), (uint8_t *)display_buf[0], LEFT_MODE);
        
        lcd.SelectLayer(FOREGROUND); 

    flag = 0;
    }
    
  }

}