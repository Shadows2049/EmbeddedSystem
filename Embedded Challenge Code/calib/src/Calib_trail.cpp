#include <mbed.h>


volatile int flag = 0;
SPI spi(PF_9, PF_8, PF_7); // mosi, miso, sclk
DigitalOut cs(PC_6);
int16_t dataSet[4000];
void setFlag();
void setMode();
int16_t readData(int code);
double calibration();

int16_t offset;

void setFlag() {          
  flag = 1;
}

void setMode() {          
  cs=0;
  spi.write(0x20);
  spi.write(0xCF);
  cs=1;
}

// Measure gyro Values continuously
int16_t readData(int code){
  cs = 0;
  spi.write(code);
  uint8_t xl = spi.write(0x00);
  cs = 1;
  cs = 0;
  spi.write(code + 1);
  int8_t xh = spi.write(0x00);
  cs = 1;
  int16_t data = xh*256+xl;
  return data;
}

/*
Calibration: Elimintate the flucturation of the raw data. Calculate the average value of raw data without moving the clip
*/
double calibration(){
  double total = 0;
  for (int i = 0; i < 4000; i++){
    total+=dataSet[i];
    printf("%d ",dataSet[i]);
  }
  return total/4000;
}


int main() {
  cs=1;
  setMode();
  spi.format(8,3);
  spi.frequency(100000);
  Ticker t;
  t.attach(&setFlag,0.01);
  uint16_t iter = 0;

  while(1) {
    if(flag){
      int16_t dataZ = readData(0xAC);
      printf("%d\n", iter);
      dataSet[iter] = dataZ;
      iter = iter + 1;

      if (iter == 4000){
          double cal1 = calibration();
          printf("\nThe first round of calibration gives %d\n", (int)cal1);
          offset=-63;
        break;
      }
      flag = 0;
    }
    
  }
}


