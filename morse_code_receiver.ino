#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define LDR_PIN A0
#define UNIT_TIME 300
#define LIGHT_THRESHOLD 900
#define TOLERANCE 0.10
#define BUTTON_PIN D7  

LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long signalStart = 0;
unsigned long signalEnd = 0;
unsigned long lastSignalTime = 0;

bool isLightOn = false;
bool firstSignalReceived = false;
String morseChar = "";
String decodedMessage = "";

int LCD_cursor_col = 0;
int LCD_cursor_row = 0;

struct MorseMap {
  const char* code;
  char letter;
};

MorseMap morseTable[] = {
  {".-", 'A'},   {"-...", 'B'}, {"-.-.", 'C'}, {"-..", 'D'}, {".", 'E'},
  {"..-.", 'F'}, {"--.", 'G'},  {"....", 'H'}, {"..", 'I'},  {".---", 'J'},
  {"-.-", 'K'},  {".-..", 'L'}, {"--", 'M'},   {"-.", 'N'},  {"---", 'O'},
  {".--.", 'P'}, {"--.-", 'Q'}, {".-.", 'R'},  {"...", 'S'}, {"-", 'T'},
  {"..-", 'U'},  {"...-", 'V'}, {".--", 'W'},  {"-..-", 'X'}, {"-.--", 'Y'},
  {"--..", 'Z'}, {"-----", '0'}, {".----", '1'}, {"..---", '2'},
  {"...--", '3'}, {"....-", '4'}, {".....", '5'}, {"-....", '6'},
  {"--...", '7'}, {"---..", '8'}, {"----.", '9'}
};

bool isDurationClose(unsigned long duration, unsigned long target) {
  unsigned long lowerLimit = target * (1.0 - TOLERANCE);
  unsigned long upperLimit = target * (1.0 + TOLERANCE);
  return (duration >= lowerLimit && duration <= upperLimit);
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  pinMode(LDR_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);  
  lcd.setCursor(0, 0);
  lcd.print("Morse Receiver");
}

void loop() {
  
  static bool buttonPrevState = HIGH;
  bool buttonCurrentState = digitalRead(BUTTON_PIN);

  if (buttonPrevState == HIGH && buttonCurrentState == LOW) {
    int lightVal = analogRead(LDR_PIN);
    Serial.print("Calibrate Mode - LDR Value: ");
    Serial.println(lightVal);
    delay(300);  
    
  }
  buttonPrevState = buttonCurrentState;

  int ldrValue = analogRead(LDR_PIN);
  unsigned long currentTime = millis();

  // Işık algılandı
  if (ldrValue < LIGHT_THRESHOLD) {
    if (!isLightOn) {
      isLightOn = true;
      signalStart = currentTime;

      if (!firstSignalReceived) {
        lcd.clear();
        firstSignalReceived = true;
      }

      if (currentTime - lastSignalTime > UNIT_TIME * 3 && morseChar.length() > 0) {
        decodeMorse(morseChar);
        morseChar = "";
      }

      if (currentTime - lastSignalTime > UNIT_TIME * 7) {
        decodedMessage += " ";
        lcdPrint(' ');
      }
    }
  } 
  
  else {
    if (isLightOn) {
      isLightOn = false;
      signalEnd = currentTime;
      unsigned long duration = signalEnd - signalStart;

      if (isDurationClose(duration, UNIT_TIME)) {
        morseChar += ".";
      } else if (isDurationClose(duration, UNIT_TIME * 3)) {
        morseChar += "-";
      }

      lastSignalTime = currentTime;
    }
  }

  if (!isLightOn && morseChar.length() > 0) {
    if (millis() - lastSignalTime > UNIT_TIME * 3) {
      decodeMorse(morseChar);
      morseChar = "";
    }
  }
}

void decodeMorse(String morse) {
  for (int i = 0; i < sizeof(morseTable) / sizeof(MorseMap); i++) {
    if (morse.equals(morseTable[i].code)) {
      decodedMessage += morseTable[i].letter;
      lcdPrint(morseTable[i].letter);
      break;
    }
  }
}

void lcdPrint(char ch) {
  lcd.setCursor(LCD_cursor_col, LCD_cursor_row);
  lcd.print(ch);
  LCD_cursor_col++;

  if (LCD_cursor_col >= 16) {
    LCD_cursor_col = 0;
    LCD_cursor_row++;

    if (LCD_cursor_row >= 2) {
      lcd.clear();
      LCD_cursor_row = 0;
      LCD_cursor_col = 0;
    } else {
      lcd.setCursor(0, LCD_cursor_row);
      for (int i = 0; i < 16; i++) {
        lcd.print(" ");
      }
      lcd.setCursor(0, LCD_cursor_row);
    }
  }
}
