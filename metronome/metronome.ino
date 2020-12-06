
#include <LiquidCrystal.h>
#include "pitches.h"

#define LCD_WIDTH 16
#define LCD_HEIGHT 2
#define TAP_CTR 14 //position of tap count indicator
//PINS
#define SPEAKER_PIN 8
#define LED_PIN 13
#define RS_PIN 12
#define ENABLE_PIN 11
#define D4_PIN 5
#define D5_PIN 4
#define D6_PIN 3
#define D7_PIN 2
#define PIEZO_PIN A0

#define SIXTY_S 60000
#define TWO_S 2000
LiquidCrystal lcd(RS_PIN, ENABLE_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN);

/*
 * A piezo sensor will always provide a non-zero value so, but tapping it creates a spike in voltage.
 * We need a value that is going to eliminate false taps but be sensitive enough to detect taps.
 * This value may need to change based on a number of factors for each individual implmentation.
 * The sensor used, the surface it is on, etc.
 */
const int min_tap_intensity = 150;

/*
 * a tap isn't just a single number. the arduino is taking readings 16000 times a second so a tap on a piezo may be registered 
 * a few thousand times in the span a tap is registered. We need a time frame after a spike is registered to let the sensor return
 * back to a baseline reading before we expect another tap. 50ms seems to be enough for me, or 1/20th of a second. If we actually tapped
 * at that rate we would get 1200 bpms, so I think we can safely assume no one is going to want that.
 */
const long debounceDelay = 50;

const boolean ACCENT = true;

int bpm  = 0;

unsigned long last_millis;
unsigned long tdelay;
int time_signiture = 4;

unsigned long last_debounce_millis = 0;

const int taps_len = 24;
unsigned long taps_millis[taps_len];
int next_tap = 0;
int total_taps = 0;

int note_duration ;
int beat;

void setup() {
  Serial.begin(9600);
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  
  lcd.begin(LCD_WIDTH, LCD_HEIGHT);
  
  last_millis = millis();

  // play a note for 1/16 of a second
  note_duration  = 1000 / 16;
  beat = 0;
}

void loop() {
  int elapsed_millis = millis() - last_millis;
  
  int sensor_value = analogRead(PIEZO_PIN);
  

  if ((millis() - last_debounce_millis) > debounceDelay) {
   
      // only toggle the LED if the new button state is HIGH
    if (sensor_value >= min_tap_intensity) {
      if((millis() - last_debounce_millis) > TWO_S) {
        reset_taps();
      }
        lcd.setCursor(TAP_CTR, 0);
        lcd.print(next_tap);
        taps_millis[next_tap] = millis();
        Serial.println(String(next_tap) + ": " + String(taps_millis[next_tap]));
        //lcd.setCursor(0, 1);
        //lcd.print(taps_millis[next_tap]);
        
        bpm  = calculate_bpms();
        Serial.println("BPM: " + String(bpm));
        
        next_tap = ++next_tap % taps_len;
        total_taps++;
        
      last_debounce_millis = millis();
    } else {
        lcd.setCursor(TAP_CTR, 0);
        lcd.print(" ");
    }
  }
  
  if(bpm == 0) { 
    return;
  }
  
  if(elapsed_millis > note_duration ) { 
    digitalWrite(LED_PIN, LOW);
  }
  
  tdelay = SIXTY_S/bpm;
  
  if(elapsed_millis < tdelay) { 
    return;
  }
  
  
  lcd.clear();
  lcd.print("BPM: " + String(bpm));
  
  int play_note = NOTE_C4;
 
  
  beat = beat % time_signiture;
  lcd.setCursor(0, 1);
  lcd.print(String(beat+1));
  
  if(ACCENT && beat == 0) {
    play_note = NOTE_C6;
  }

  tone(SPEAKER_PIN, play_note, note_duration);
  digitalWrite(LED_PIN, HIGH);

  
  last_millis = millis();
  beat++;
}

/*
 * 
 */
int calculate_bpms() {
  if(total_taps < time_signiture) {
    return 0;
  }
  
  unsigned long total = 0;
  for(int i=1; i<=time_signiture; i++) {
    int tap = next_tap - i;
    
    if(tap < 0) {
      tap = taps_len + tap;
    }
    
    if(i>1) {
      Serial.print(String(tap + 1 % taps_len) + ": " + String(taps_millis[tap+1%taps_len]) + " - ");
      Serial.print(String(tap) + ": " + String(taps_millis[tap]));
      
      total += (taps_millis[tap + 1 % taps_len] - taps_millis[tap]);
    }
    Serial.println(total);
  }
  
  return SIXTY_S/(total / (time_signiture - 1));
}

void reset_taps() {
  beat = 0;
  for(int i=0;i<taps_len;i++) {
    taps_millis[i] = 0;
  }
  next_tap = 0;
  total_taps = 0;
}
