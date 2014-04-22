#include <Arduino.h>
#include <stdint.h>
#include <Linduino.h>
#include <LT_SPI.h>
#include <UserInterface.h>
#include <LTC68041.h>
#include <SPI.h>

int error;
int input;
uint16_t currbat; //stores the battery value within the for statment to keep from accessing the 1894
uint16_t lowestbat; //stores the value of the lowest battery of the packs
const int lowbat = 31200;  //triggers LED 1 if above this voltage*.0001
const int medbat = 34800;  //triggers LED 2 if above this voltage*.0001
const int highbat = 39000; //triggers LED 3 if above this voltage*.0001
const int fullbat = 40800; //triggers LED 4 if above this voltage*.0001
const int CELL_NUM = 4; //max cell number on each IC
const int TOTAL_IC = 1; //number of IC's
uint16_t cell_codes[TOTAL_IC][12];
uint16_t aux_codes[TOTAL_IC][6];
uint8_t tx_cfg[TOTAL_IC][6];
uint8_t rx_cfg[TOTAL_IC][8];

void setup(){
  Serial.begin(115200);
  LTC6804_initialize();  //Initialize LTC6804 hardware
  init_cfg();            //Initialize the 6804 configuration array to be written
}

void loop(){
  Serial.println("Starting voltage loop. Transmit 'm' to quit.");
  wakeup_sleep();
  LTC6804_wrcfg(TOTAL_IC,tx_cfg);
  while (input != 'm'){
    if (Serial.available() > 0){
      input = Serial.available();
    }
    wakeup_idle();
    LTC6804_adcv();
    delay(10);
    wakeup_idle();
    error = LTC6804_rdcv(0, TOTAL_IC,cell_codes);
    if (error == 255){
      Serial.println("A PEC error was detected in the received data");
      while (error == 255){
        error = LTC6804_rdcv(0, TOTAL_IC,cell_codes);
        digitalWrite (2, HIGH);
        digitalWrite (3, HIGH);
        digitalWrite (4, HIGH);
        digitalWrite (5, HIGH);
        delay(100);
        digitalWrite (2, LOW);
        digitalWrite (3, LOW);
        digitalWrite (4, LOW);
        digitalWrite (5, LOW);
        delay(100);
      }
    }else{
      lowestbat = cell_codes[0][0];
      for (int current_ic = 0 ; current_ic < TOTAL_IC; current_ic++){
        for(int i=0; i < CELL_NUM; i++){
          currbat = cell_codes[current_ic][i];
          if(lowestbat > currbat && currbat >= 10000){
            lowestbat = currbat;
          }
        }
      }
      if (lowestbat >= lowbat){
        digitalWrite(2, HIGH);
      }else{
        digitalWrite(2, LOW);
      }
      if (lowestbat >= medbat){
        digitalWrite(3, HIGH);
      }else{
        digitalWrite(3, LOW);
      }
      if (lowestbat >= highbat){
        digitalWrite(4, HIGH);
      }else{
        digitalWrite(4, LOW);
      }
      if (lowestbat >= fullbat){
        digitalWrite(5, HIGH);
      }else{
        digitalWrite(5, LOW);
      }
      print_cells();
      Serial.print("Lowest Battery: ");Serial.print(lowestbat*.0001, 4);Serial.print("V ");
      Serial.println();
      Serial.println();
    }
    delay(500);
  }
}

void init_cfg(){
  for(int i = 0; i<TOTAL_IC;i++){
    tx_cfg[i][0] = 0xFE;
    tx_cfg[i][1] = 0x00; 
    tx_cfg[i][2] = 0x00;
    tx_cfg[i][3] = 0x00; 
    tx_cfg[i][4] = 0x00;
    tx_cfg[i][5] = 0x00;
  }
}

void print_cells(){
  for (int current_ic = 0 ; current_ic < TOTAL_IC; current_ic++){
    Serial.print(" IC ");
    Serial.print(current_ic+1,DEC);
    for(int i=0; i<CELL_NUM; i++){
      Serial.print(" C");
      Serial.print(i+1,DEC);
      Serial.print(":");
      Serial.print(cell_codes[current_ic][i]*.0001, 4);
      Serial.print(",");
    }
     Serial.println(); 
  }
}
