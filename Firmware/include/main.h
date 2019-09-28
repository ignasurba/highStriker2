#include <stdint.h>

#define DEBUG_BLINK ((global_ctr % 32) < 16)

#define SETTINGS_COLOR strip.Color(255, 105, 180)
#define HIT_COLOR strip.Color(251, 232, 68)
#define IDLE_COLOR1 strip.Color(233, 31, 131)
#define IDLE_COLOR2 strip.Color(10, 55, 146)
#define BLACK_COLOR strip.Color(0, 0, 0)
#define WHITE_COLOR strip.Color(255, 255, 255)

#define SAMPLE_RATE 10000 // in microseconds
#define CHECK_RATE 50000 // in microseconds

#define NUMBER_OF_RANGES 8

#define ADC_MAX 4095
#define LOAD_CELL_MAX 2000
#define SEGMENT_MAX 999

#define HIT_THRESHOLD 50
#define HIT_CLEAR_DIFF 100 // clearing attempts time
#define HIT_CLEAR_TIME 5000
#define HIT_ANIMATION_DELAY 1 // * LOAD_CELL_MAX = 2s

#define HIGH_SCORE_ANIMATION_DELAY 300
#define HIGH_SCORE_ANIMATION_COUNT 10

#define PEAK_CLEAR_US 250

#define NUM_LEDS 120
#define NUM_LEDS_PAIRED 6

#define LED1 PA0
#define LED2 PA1
#define LED3 PB14
#define MVPKG PA5
#define PEAKKG PA6
#define REFRESH PA7

#define S1 PB0
#define S2 PB1
#define S3 PB12
#define S4 PB4

#define OUT1 PB5
#define OUT2 PB3
#define OUT3 PB15
#define OUT4 PB13
#define OUT5 PA8
#define OUT6 PA9
#define OUT7 PA10
#define OUT8 PA15

#define WS_DATA_PIN 28

#define RS485_DIR_1 PC13
#define RS485_DIR_2 PA4
#define RS485_DIR_3 PB2

#define EEPROM_CHECK 0xAB

const uint16_t sensitivity_ranges[NUMBER_OF_RANGES] = {4095, 3583, 3071, 2559, 2047, 1535, 1023, 511};
uint8_t ADC_pins = PEAKKG; // Channel to be acquired

uint16_t last_leds = 0;
uint32_t last_leds_color = 0;

volatile uint8_t global_ctr = 0;
volatile uint8_t dbg_bool = 0;
volatile uint16_t ADC_reading;

void clearPeak();
void fullColor(uint32_t c);
void sendToLEDs(uint16_t score, uint32_t c);
void sendToLEDs_rainbow(uint16_t score);

void EEPROM_save();
