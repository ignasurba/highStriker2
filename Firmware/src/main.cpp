#include <Arduino.h>
#include <I2C_EEPROM.h>
#include <STM32ADC.h>
#include <WS2812B.h>
#include <HSV_RGB.h>
#include <crc16.h>
#include <Wire.h>
#include <main.h>

WS2812B strip = WS2812B(NUM_LEDS); //Connect the WS2812B to MOSI (OUT3) on your board
STM32ADC ADC(ADC1);

struct {
  uint16_t load_cell_offset;
  uint8_t sensitivity_current;
  uint16_t highscore;
  uint16_t eeprom_crc;
} EEPROM_store;

#define CRC_SIZE (sizeof(EEPROM_store) - sizeof(EEPROM_store.eeprom_crc))

void adcIRQ(void) {
  ADC_reading = ADC.getData();
}

void checkIRQ(void) {
  global_ctr++;
  dbg_bool = 1;
}

void setup() {
  afio_cfg_debug_ports(AFIO_DEBUG_SW_ONLY); //Release PB3 and PB5
  afio_remap(AFIO_REMAP_USART1);
  
  //Measurements
  Timer2.setPeriod(CHECK_RATE);
  Timer2.setChannel1Mode(TIMER_OUTPUT_COMPARE);
  Timer2.setCompare(TIMER_CH1, 1);
  Timer2.attachCompare1Interrupt(checkIRQ);
 
  Timer3.setPeriod(SAMPLE_RATE);
  Timer3.setMasterModeTrGo(TIMER_CR2_MMS_UPDATE);
  
  pinMode(ADC_pins, INPUT_ANALOG);
  
  ADC.calibrate();
  ADC.setSampleRate(ADC_SMPR_7_5);
  ADC.setPins(&ADC_pins, 1);
  ADC.setTrigger(ADC_EXT_EV_TIM3_TRGO);
  ADC.attachInterrupt(adcIRQ, ADC_EOC);

  //Configuration
  pinMode(S1, INPUT_PULLUP);
  pinMode(S2, INPUT_PULLUP);
  pinMode(S3, INPUT_PULLUP);
  pinMode(S4, INPUT_PULLUP);

  pinMode(RS485_DIR_1, OUTPUT);
  pinMode(RS485_DIR_2, OUTPUT);
  pinMode(RS485_DIR_3, OUTPUT);

  digitalWrite(RS485_DIR_1, HIGH);
  digitalWrite(RS485_DIR_2, HIGH);
  digitalWrite(RS485_DIR_3, HIGH);
  
  Serial.begin(); //USB
  Serial1.begin(115200);
  Serial2.begin(115200); 
  Serial3.begin(115200); 

  strip.begin(); //SPIClass SPI(2); //SPI_CLOCK_DIV16
  strip.show();
  //strip.setBrightness(16);

  Wire.begin(); //TwoWire Wire(1, I2C_FAST_MODE|I2C_REMAP);
 
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  pinMode(REFRESH, OUTPUT);
  delay(1000);

  //Init
  fullColor(strip.Color(255, 0, 0));
  digitalWrite(LED1, HIGH);
  delay(1000);
  fullColor(strip.Color(0, 255, 0));
  digitalWrite(LED2, HIGH);
  delay(1000);
  fullColor(strip.Color(0, 0, 255));
  digitalWrite(LED3, HIGH);
  delay(1000);

  fullColor(WHITE_COLOR);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  delay(1000);

  //Version
  Serial.println("highStriker v2");
  Serial.print("compiled: ");
  Serial.print(__DATE__);
  Serial.print(" ");
  Serial.println(__TIME__);
  delay(100);

  //Get EEPROM
  EEPROM_get(0, EEPROM_store);

  //Test
  Serial.print("I2C EEPROM: ");
  if(crc_ccitt(0xFFFF, (uint8_t *)&EEPROM_store, CRC_SIZE) == EEPROM_store.eeprom_crc){
    Serial.println("OK");
  } else {
    Serial.println("FAIL");
    memset(&EEPROM_store, 0, sizeof(EEPROM_store));
    EEPROM_store.highscore = HIT_THRESHOLD + 1;
    EEPROM_save();
  }
  delay(100);
  
  //Go
  Serial.print("Highscore: ");
  Serial.println(EEPROM_store.highscore);
  Serial.print("Sensitivity: ");
  Serial.println(sensitivity_ranges[EEPROM_store.sensitivity_current]);
  Serial.print("Offset: ");
  Serial.println(EEPROM_store.load_cell_offset);
}

void loop() {
  uint16_t peak = map(ADC_reading, 0, sensitivity_ranges[EEPROM_store.sensitivity_current], 0, LOAD_CELL_MAX);
  static uint16_t hitScore = 0, ledAnimate = 0;
  static unsigned long hitTime = 0;
  static bool hitHighScore = false;

  //Buttons
  if(digitalRead(S1) == LOW){
    EEPROM_store.load_cell_offset = peak;
    sendToLEDs(2000, SETTINGS_COLOR);
    EEPROM_save();
    delay(1000);
  }

  //Debug
  if(dbg_bool){
    Serial.print("Peak: ");
    Serial.print(peak);
    Serial.print(", ");
  }

  //Offset
  if(peak > EEPROM_store.load_cell_offset){
    peak -= EEPROM_store.load_cell_offset;
  } else {
    peak = 0;
  }

  //Debug
  if(dbg_bool){
    Serial.println(peak);
    dbg_bool = 0;
  }

  digitalWrite(LED1, DEBUG_BLINK);
  
  //Loop
  if(EEPROM_store.highscore < peak){
    EEPROM_store.highscore = peak;
    hitHighScore = true;
  }

  if(peak > HIT_THRESHOLD || ((millis() - hitTime) < HIT_CLEAR_TIME)){
    digitalWrite(LED3, HIGH);
    if(peak > hitScore) hitScore = peak;
    if(hitTime == 0) hitTime = millis();
    if(hitScore > ledAnimate) ledAnimate++;
    sendToLEDs(ledAnimate, HIT_COLOR);
    if((millis() - hitTime) > (HIT_CLEAR_TIME - HIT_CLEAR_DIFF)) clearPeak();
    delay(HIT_ANIMATION_DELAY);
    digitalWrite(LED3, LOW);
  } else if(hitHighScore){
    for(uint8_t i = 0; i < HIGH_SCORE_ANIMATION_COUNT; i++){
      fullColor(BLACK_COLOR);
      delay(HIGH_SCORE_ANIMATION_DELAY);
      sendToLEDs(EEPROM_store.highscore, HIT_COLOR);
      delay(HIGH_SCORE_ANIMATION_DELAY);
    }
    EEPROM_save();
    hitHighScore = false;
  } else {
    hitTime = 0;
    hitScore = 0;
    ledAnimate = 0;
    sendToLEDs_rainbow(EEPROM_store.highscore);
    digitalWrite(LED2, DEBUG_BLINK);
  }

  if(digitalRead(S2) == LOW && digitalRead(S3) == LOW){
    EEPROM_store.highscore = HIT_THRESHOLD + 1;
    sendToLEDs(EEPROM_store.highscore, SETTINGS_COLOR);
    EEPROM_save();
    delay(100);
  }

  if(digitalRead(S4) == LOW){
    EEPROM_store.sensitivity_current = (EEPROM_store.sensitivity_current + 1) % NUMBER_OF_RANGES;
    sendToLEDs(map(sensitivity_ranges[EEPROM_store.sensitivity_current], 0, ADC_MAX, 0, LOAD_CELL_MAX), SETTINGS_COLOR);
    EEPROM_save();
    delay(1000);
  }
}

void EEPROM_save(){
  EEPROM_store.eeprom_crc = crc_ccitt(0xFFFF, (uint8_t *)&EEPROM_store, CRC_SIZE);
  EEPROM_put(0, EEPROM_store);
}

void clearPeak() {
  digitalWrite(REFRESH, HIGH);
  delayMicroseconds(PEAK_CLEAR_US);
  digitalWrite(REFRESH, LOW);
}

void setColor(float *rgb, int led) {
  strip.setPixelColor(led, strip.Color((int)((1.0 - rgb[0]) * 255), (int)((1.0 - rgb[1]) * 255), (int)((1.0 - rgb[2]) * 255)));
}

void fullColor(uint32_t c) {
  for(uint16_t i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
  
  last_leds = NUM_LEDS;
  last_leds_color = c;
}

void sendToLEDs(uint16_t score, uint32_t c){
  score = map(score, 0, LOAD_CELL_MAX, 0, NUM_LEDS);

  if(last_leds != score || last_leds_color != c){
    for(uint16_t i = 0; i < NUM_LEDS; i++) {
      if(i < score){
        strip.setPixelColor(i, c);
      } else {
        strip.setPixelColor(i, strip.Color(0, 0, 0));
      }
    }
    strip.show();
    
    last_leds = score;
    last_leds_color = c;

    delay(10);
  }
}

void sendToLEDs_rainbow(uint16_t score){
  static float hue = 0.0;
  score = map(score, 0, LOAD_CELL_MAX, 0, NUM_LEDS);

  for(uint16_t i = 0; i < NUM_LEDS; i++) {
    if(i < score){
      setColor(hsv2rgb(hue, 1.0, 1.0), i);
    } else {
      strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
    hue += 0.00001;
    if (hue >= 1.0) hue = 0.0;
  }
  strip.show();

  last_leds = 0;
  last_leds_color = 0;

  delay(10);
}
