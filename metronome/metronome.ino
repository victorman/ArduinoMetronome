
#include <LiquidCrystal.h>
#include "pitches.h"


//PINS
const int SPEAKER_PIN = 8;
const int LED_PIN = 13;
//const int BUTTON_PIN = 7;
const int knockSensor = A0;
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const int threshold = 200;
const long debounceDelay = 50;
const boolean ACCENT = true;

int BPM = 0;

unsigned long last;
unsigned long tdelay;
int signiture = 4;

long lastDebounceTime = 0;
int buttonState; 
int lastButtonState = LOW;

const int taps_len = 24;
unsigned long taps[taps_len];
int next_tap = 0;
int total_taps = 0;

int noteDuration;
int beat;

void setup() {
  Serial.begin(9600);
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  //pinMode(BUTTON_PIN, INPUT);
  
  lcd.begin(16, 2);
  
  beat = 0;
  //calculate seconds per beat
  tdelay = 60000/BPM;
  last = millis();
  
  // to calculate the note duration, take one second
  // divided by the note type.
  //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
  noteDuration = 1000 / 16;
}

void loop() {
  int elapsed = millis() - last;
  
  int reading = analogRead(knockSensor);
  

  if ((millis() - lastDebounceTime) > debounceDelay) {
   
      // only toggle the LED if the new button state is HIGH
    if (reading >= threshold) {
      if((millis() - lastDebounceTime) > 2000) {
        reset_taps();
      }
        lcd.setCursor(15, 0);
        lcd.write(-1);
        taps[next_tap] = millis();
        Serial.println(String(next_tap) + ": " + String(taps[next_tap]));
        //lcd.setCursor(0, 1);
        //lcd.print(taps[next_tap]);
        
        BPM = bpms();
        Serial.println("BPM: " + String(BPM));
        
        next_tap = ++next_tap % taps_len;
        total_taps++;
        
      lastDebounceTime = millis();
    } else {
        lcd.setCursor(15, 0);
        lcd.print(" ");
    }
  }
  
  if(BPM == 0) { 
    return;
  }
  
  if(elapsed > noteDuration) { 
    digitalWrite(LED_PIN, LOW);
  }
  
  tdelay = 60000/BPM;
  
  if(elapsed < tdelay) { 
    return;
  }
  
  
  lcd.clear();
  lcd.print("BPM: " + String(BPM));
  
  int play_note = NOTE_C4;
 
  
  beat = beat % signiture;
  lcd.setCursor(0, 1);
  lcd.print(String(beat+1));
  
  if(ACCENT && beat == 0) {
    play_note = NOTE_C6;
  }

  tone(SPEAKER_PIN, play_note, noteDuration);
  digitalWrite(LED_PIN, HIGH);

  
  last = millis();
  beat++;
}

int bpms() {
  if(total_taps < signiture) {
    return 0;
  }
  
  unsigned long total = 0;
  for(int i=1; i<=signiture; i++) {
    int tap = next_tap - i;
    
    if(tap < 0) {
      tap = taps_len + tap;
    }
    
    if(i>1) {
      Serial.print(String(tap+1%taps_len) + ": " + String(taps[tap+1%taps_len]) + " - ");
      Serial.print(String(tap) + ": " + String(taps[tap]));
      
      total += (taps[tap+1%taps_len] - taps[tap]);
    }
    Serial.println(total);
  }
  
  return 60000/(total / (signiture - 1));
}

void reset_taps() {
  beat = 0;
  for(int i=0;i<taps_len;i++) {
    taps[i] = 0;
  }
  next_tap = 0;
  total_taps = 0;
}
