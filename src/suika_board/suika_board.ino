/*
    Nefrock Party 2019
    SUIKA board

ATmega 328P Pin assign------------------------------------------------------------------
PIN-No.     Name    function                PWM     PRG         SUIKA       Arduino-pin 
DIP TQFP                                                                                
 1  29      PC6     PCINT14/XRESET                  PRG_XRST                22          
 2  30      PD0     PCINT16/RXD                                 EXPIO_0     0           
 3  31      PD1     PCINT17/TXD                                 EXPIO_1     1           
 4  32      PD2     PCINT18/INT0                                EXPIO_2     2           
 5  1       PD3     PCINT19/OC2B/INT1       PWM                 LED_Y5      3           
 6  2       PD4     PCINT20/XCK/T0                              SP_EN       4           
 7  4,6     VCC                                                                         
 8  3,5     GND                                                                         
 9  7       PB6     PCINT6/XTAL1/TOSC1                                      20          
10  8       PB7     PCINT7/XTAL2/TOSC2                                      21          
11  9       PD5     PICNT21/OC0A/T1         PWM                 LED_Y4      5           
12  10      PD6     PCINT22/OC0A/AIN0       PWM                 LED_Y3      6           
13  11      PD7     PCINT23/AIN1                                PSW1        7           
14  12      PB0     PCINT0/CLKO/ICP1                            BZR         8           
15  13      PB1     OC1A/PCINT1             PWM                 LED_Y1      9           
16  14      PB2     XSS/OC1B/PCINT2         PWM                 LED_Y2      10          
17  15      PB3     MOSI/OC2A/PCINT3        PWM     PRG_MOSI                11          
18  16      PB4     MISO/PCINT4                     PRG_MISO                12          
19  17      PB5     SCK/PCINT5                      PRG_SCK                 13          
20  18      AVCC                                                                        
21  20      AREF                                                                        
22  21      GND                                                                         
23  23      PC0     ADC0/PCINT8                                 LED_R1      14/A1       
24  24      PC1     ADC1/PCINT9                                 LED_R2      15/A2       
25  25      PC2     ADC2/PCINT10                                LED_R3      16/A2       
26  26      PC3     ADC3/PCINT11                                PSW2        17/A3       
27  27      PC4     ADC4/SDA/PCINT12                            I2C_SDA     18/A4       
28  28      PC5     ADC5/SCL/PCINT13                            I2C_SCL     19/A5       
 -  19              AD6                                                     A6          
 -  22              AD7                                         EXPIO_3     A7          
----------------------------------------------------------------------------------------

*/

#include <Wire.h>                 // Must include Wire library for I2C
#include "SparkFun_MMA8452Q.h"    // Click here to get the library: http://librarymanager/All#SparkFun_MMA8452Q
#include "suika_board.h"
#include "pitches.h"
MMA8452Q accel;         // create instance of the MMA8452 class
suika_state state = TONE;
suika_mode mode = DRINK;
float deg;
// 最小周波数
int MIN_HZ= NOTE_B2;
// 最小周波数
int MAX_HZ = NOTE_GS5;
// バンド幅
int BAND_WIDTH = MAX_HZ - MIN_HZ;
// フィーバー突入角度
float FEVER_THRESHOLD = 0.70;
// TONE突入角度
float TONE_POSEDGE_THRESHOLD = -0.80;
float TONE_NEGEDGE_THRESHOLD = -0.90;

// BELL突入DEG
float BELL_POSEDGE_THRESHOLD = 0.10;
// BELL脱出DEG
float BELL_NEGEDGE_THRESHOLD = 0.00;

// LED点灯パターン
int LED_SEQUENCE[] = {LED_R1, LED_R2, LED_R3, LED_Y2, LED_Y1, LED_Y3, LED_Y4, LED_Y5, LED_R3, LED_R2, LED_R1, LED_Y5, LED_Y4,LED_Y3, LED_Y1, LED_Y2};
// LED数
int NUM_LED = sizeof(LED_SEQUENCE) / sizeof(LED_SEQUENCE[0]);

int START_TONE_INDEX = 13;
int TONE_SIZE = 63;

int current_led = 0;
int current_freq = 0;
int target_freq = 0;
int current_music = 0;
int current_music_first_tone_index = 0;
int current_tone_index = START_TONE_INDEX;

// the setup function runs once when you press reset or power the board
void setup(){
  // I/O 設定
  pinMode(BZR,OUTPUT);
  pinMode(SP_EN,OUTPUT);
  pinMode(LED_R1,OUTPUT);
  pinMode(LED_R2,OUTPUT);
  pinMode(LED_R3,OUTPUT);
  pinMode(LED_Y1,OUTPUT);
  pinMode(LED_Y2,OUTPUT);
  pinMode(LED_Y3,OUTPUT);
  pinMode(LED_Y4,OUTPUT);
  pinMode(LED_Y5,OUTPUT);
  pinMode(PSW1,INPUT_PULLUP);
  pinMode(PSW2,INPUT);
  pinMode(EXPIO_3, INPUT);
  digitalWrite(SP_EN, HIGH);
  randomSeed(analogRead(EXPIO_3));
  setNextMusic();
  // I2C Init
  Wire.begin();
  // Check MMA8452Q
  while(!accel.begin());
}

void setNextMusic() {
  int num_music = sizeof(MUSICS) / sizeof(MUSICS[0]);    
  current_music = random(num_music);
  int current_music_first_tone = MUSICS[current_music][1];
  int i = 0;
  while(1){
    if(current_music_first_tone <= NOTES[i]){
      current_music_first_tone_index = i;
      return;
    }
    i++;
  }
}

void loop() {
  // stateの反映
  applyState();
  switch(state){
    case TONE:
      play();
      break;
    case FEVER:
      playFever();
      break;
    case BELL:
      playBell();
      break;
    case SILENT:
      noTone(BZR);
      break;
  }
}


void applyState(){
  deg = deg + (readAccel() - deg) / 10.0;
  mode = readMode();
  if(mode == DRINK){
    switch(state){
      case FEVER:
        if(deg< TONE_NEGEDGE_THRESHOLD){
          state = SILENT;
        }else if(deg < FEVER_THRESHOLD){
          state = TONE;
        }
        break;
      case TONE:
        if(FEVER_THRESHOLD < deg){
          setNextMusic();
          state = FEVER;
        }else if(deg< TONE_NEGEDGE_THRESHOLD){
          state = SILENT;
        }else{
          state = TONE;
        }
        break;
      case SILENT:
        if(TONE_POSEDGE_THRESHOLD < deg){
          state = TONE;
        }else{
          state = SILENT;
        }
        break;
      default:
        state = SILENT;
        break;
    }
    return;
  }else{ // state == PLAY
    current_tone_index = readTone();
    setToneIndexLed();
    switch(state){
    case SILENT:
      if(BELL_POSEDGE_THRESHOLD < deg){
        state = BELL;
      }
      break;
    case BELL:      
      if(deg < BELL_NEGEDGE_THRESHOLD){
        state = SILENT;
      }
      break;
    default:
      state = SILENT;
      break;
    }
    return;
  }
}

suika_mode readMode(){
  static int lowCount = 0;
  int threshold = 5;
  int val = digitalRead(PSW1);
  if(val == LOW){
    lowCount++;
  }else{
    lowCount = 0;
    return mode;
  }
  if(lowCount < threshold){
    return mode;
  }else if(threshold < lowCount){
    lowCount = threshold + 1;
    return mode;
  }
  stopAllLed();
  noTone(BZR);
  if(mode == DRINK){
    current_tone_index = START_TONE_INDEX;
    return PLAY;
  }else{
    return DRINK;
  }
}

// freq計算系
int calcFreq(){
  return MIN_HZ + (int)(BAND_WIDTH * ((deg + 1.0) / 2.0));
}
int degToNoteIndex(int max_index){
  float val = deg + 1.0;
  val = val < 0.0 ? 0.0 : (2.0 < val ? 2.0 : val);
  return (int)(max_index * val / 2.0);
}
int calcFreqByTone(){
  int tone_size = sizeof(NOTES) / sizeof(NOTES[0]);  
//  int index = degToNoteIndex(tone_size-1);
  int index = degToNoteIndex(current_music_first_tone_index-1);
  return NOTES[index];
}

void play(){
//  int freq = calcFreq();
  target_freq = calcFreqByTone();
  current_freq = current_freq + (target_freq - current_freq) / 10;
  tone(BZR, current_freq);
}


void playBell(){
  tone(BZR, NOTES[current_tone_index]);
}


void playFever(){
  playMusic(current_music);
}

void playMusic(int music_index){
  int* music = MUSICS[music_index];
  int length = music[0];    
  for(int i = 0; i < length; i++){
    rotLed();
    int freq = music[i*2+1];
    int duration = music[i*2+2];
    noTone(BZR);
    tone(BZR, freq, duration);
    delay(duration);
  }
  noTone(BZR);
  stopAllLed();
}

float readAccel(){
  Wire.requestFrom(0x1D, 1);
  // Accel sensor
  if (!accel.available()){
    return deg;
  }
  float val = accel.getCalculatedZ();
  if(val < -1.0){
    val = -1.0;
  }
  if(1.0 < val){
    val = 1.0;
  }
  return val;
}

int readTone(){
  static int lowCount = 0;
  int threshold = 10;
  int val = digitalRead(PSW2);
  int ret = current_tone_index;
  if(val == LOW){
    lowCount++;
  }else{
    lowCount = 0;
    return current_tone_index;
  }
  if(threshold < lowCount){
    lowCount = threshold + 1;
  }else if(lowCount == threshold){
    ret++;
  }
  return START_TONE_INDEX + ((ret - START_TONE_INDEX) % TONE_SIZE);
}

int readPlayButton(){
  static int lowCount = 0;
  int threshold = 10;
  int val = digitalRead(PSW2);
  if(val == LOW){
    lowCount++;
  }else{
    lowCount = 0;
    return HIGH;
  }
  if(lowCount < threshold){
    return HIGH;
  }else{
    lowCount = threshold;
    return LOW;
  }
}

void rotLed(){
  int old_led_pos = LED_SEQUENCE[current_led++];
  current_led %= NUM_LED;
  int new_led_pos = LED_SEQUENCE[current_led];
  digitalWrite(old_led_pos, LOW);
  digitalWrite(new_led_pos, HIGH);  
}

void setToneIndexLed(){
  int led_index = current_tone_index - START_TONE_INDEX + 1;
  //LED_Y2, LED_Y1, LED_Y3, LED_Y4, LED_Y5, R1
  digitalWrite(LED_Y5, led_index & 0b0000000000000001);   
  digitalWrite(LED_Y4, led_index & 0b0000000000000010);   
  digitalWrite(LED_Y3, led_index & 0b0000000000000100);   
  digitalWrite(LED_Y1, led_index & 0b0000000000001000);   
  digitalWrite(LED_Y2, led_index & 0b0000000000010000);   
  digitalWrite(LED_R1, led_index & 0b0000000000100000);   
}

void stopAllLed(){
  for(int i = 0; i < NUM_LED; i++){
    digitalWrite(LED_SEQUENCE[i], LOW);    
  }
}

void sample(){
// Buzzer
  tone(BZR, 784, 200);
  delay(600);
  tone(BZR, 784, 200);
  delay(600);
  tone(BZR, 784, 200);
  delay(600);
  tone(BZR, 1046, 1200);
  delay(1200);
  noTone(BZR);

// Red-LED

  digitalWrite(LED_R1, HIGH);
  delay(1000);
  digitalWrite(LED_R1, LOW);
  digitalWrite(LED_R2, HIGH);
  delay(1000);
  digitalWrite(LED_R2, LOW);
  digitalWrite(LED_R3, HIGH);
  delay(1000);
  digitalWrite(LED_R3, LOW);

// Yellow-LED
  digitalWrite(LED_Y1, HIGH);
  delay(1000);
  digitalWrite(LED_Y1, LOW);
  digitalWrite(LED_Y2, HIGH);
  delay(1000);
  digitalWrite(LED_Y2, LOW);
  digitalWrite(LED_Y3, HIGH);
  delay(1000);
  digitalWrite(LED_Y3, LOW);
  digitalWrite(LED_Y4, HIGH);
  delay(1000);
  digitalWrite(LED_Y4, LOW);
  digitalWrite(LED_Y5, HIGH);
  delay(1000);
  digitalWrite(LED_Y5, LOW);

}
