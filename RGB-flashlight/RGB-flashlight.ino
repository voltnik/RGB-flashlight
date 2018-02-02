// Мощный цветной фонарь
// Креатед бай voltNik (c) в 2018 году нашей эры
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
//****************************

// пины для подключения шлюза мосфета. нужны пины с ШИМ, это 3, 5, 6, 9, 10, 11.
#define R_LED 6
#define G_LED 5
#define B_LED 3

// пины кнопок управления
#define R_BTN 10
#define G_BTN 9
#define B_BTN 8
#define MODE_BTN 11   // кнопка смены режима работы
#define POT_BTN 7   // потенциометр А7! (не 7, а именно А7!)

#define LCD_RENEW 300           // тайминг обновления экрана
#define LED_RENEW 300           // тайминг обновления экрана
#define BTN_PROTECT 100         // защита дребезга кнопки 

byte R_BRIGHT, G_BRIGHT, B_BRIGHT;
byte R_MNL, G_MNL, B_MNL;
byte R_BtnVal, G_BtnVal, B_BtnVal, mode_BtnVal, mode;
int LEDcolor, k, steps = 0;
long now_millis, lcd_millis, led_millis, r_millis, g_millis, b_millis, mode_millis;
String modes[5] = {"MANUALRGB","ALARM    ","STROBE   ","RAINBOW  ","RANDOM   "}; // массив названий режимов  9 символов

boolean red_or_blue = false, lamp_on = false;
int alarm_int[4]={150, 300, 450, 800};  // тайминг мигалки
int strob_int[6]={100, 100, 100, 1000}; // тайминг стробоскопа
//****************************
LiquidCrystal_I2C lcd(0x3f,16,2);  // обычно китайские модули I2C для экрана имеют адрес 0x27 или 0x3F
//****************************
void setup()
{
  // увеличиваем частоту ШИМ до 62.5кГц
  TCCR1A = TCCR1A & 0xe0 | 1;
  TCCR1B = TCCR1B & 0xe0 | 0x09;
  
  Serial.begin(9600);
  Serial.println("RGB FLASHLIGHT");
  randomSeed(analogRead(0));  // включение случайного random

  // выход на транзисторы
  pinMode(R_LED, OUTPUT);
  pinMode(G_LED, OUTPUT);
  pinMode(B_LED, OUTPUT);

  //кнопки управления
  pinMode(R_BTN, INPUT_PULLUP);
  pinMode(G_BTN, INPUT_PULLUP);
  pinMode(B_BTN, INPUT_PULLUP);
  pinMode(MODE_BTN, INPUT_PULLUP);
  pinMode(POT_BTN, INPUT);
  
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("RGB FLASHLIGHT");
  lcd.setCursor(6,1);
  lcd.print("1.0");
  delay(1000);
  lcd.clear();
}
//****************************
void loop()
{
  now_millis = millis();
  // считываем состояние кнопок
  R_BtnVal = digitalRead(R_BTN);
  G_BtnVal = digitalRead(G_BTN);
  B_BtnVal = digitalRead(B_BTN);
  mode_BtnVal = digitalRead(MODE_BTN);
  // обработка нажатия кнопок с защитой от дребезга, цикл от 0 до 5
  if ((R_BtnVal == LOW) & (now_millis - r_millis)> BTN_PROTECT) { 
    R_MNL = (R_MNL + 1) % 6;
    R_BRIGHT = 51 * R_MNL;
    r_millis = now_millis + 300;
  }
  if ((G_BtnVal == LOW) & (now_millis - g_millis)> BTN_PROTECT) { 
    G_MNL = (G_MNL + 1) % 6;
    G_BRIGHT = 51 * G_MNL;
    g_millis = now_millis + 300;
  }
  if ((B_BtnVal == LOW) & (now_millis - b_millis)> BTN_PROTECT) { 
    B_MNL = (B_MNL + 1) % 6;
    B_BRIGHT = 51 * B_MNL;
    b_millis = now_millis + 300;
  }
  if ((mode_BtnVal == LOW) & (now_millis - mode_millis)> BTN_PROTECT) { 
    mode = (mode + 1) % 5;
    awhiteoff();
    lamp_on = false;
    steps = 0;
    mode_millis = now_millis + 300;
  }

  //***************** режимы работы фонаря
  switch (mode) {
  case 0: // ручной режим
    if (now_millis - led_millis > LED_RENEW) {
      analogWrite(R_LED, R_BRIGHT);
      analogWrite(G_LED, G_BRIGHT);
      analogWrite(B_LED, B_BRIGHT);
      led_millis = now_millis;
    }    
    break;
  case 1: // мигалка
    if (now_millis - led_millis > alarm_int[steps-1] + LEDcolor) { // проверяем интервал
    if (steps == 4) {
      red_or_blue = !red_or_blue;
      led_millis = now_millis; 
    }
    digitalWrite(red_or_blue ? R_LED:B_LED, steps & 1);
    if (red_or_blue) {R_BRIGHT=255; G_BRIGHT=0; B_BRIGHT=0;} else {R_BRIGHT=0; G_BRIGHT=0; B_BRIGHT=255;}  
      steps = steps + 1 - 4 * (steps > 3);
    }
  break;
  case 2: // стробоскоп
    if (now_millis - led_millis > strob_int[steps] + LEDcolor) {
      steps = steps + 1 - 4 * (steps > 3);
      lamp_on = !lamp_on;
      led_millis = now_millis; 
      if (!lamp_on) {awhiteon();} else {awhiteoff();}; 
    }    
    break;
  case 3: // радуга
    if (now_millis - led_millis > 300) {
      LEDcolor = analogRead(POT_BTN);
      //Serial.println(LEDcolor);
      if (LEDcolor <= 250) { 
        k = map(LEDcolor, 0, 250, 0, 255); 
        R_BRIGHT = 0; G_BRIGHT = k; B_BRIGHT = 255; 
      } 
      if (LEDcolor > 250 && LEDcolor <= 500) { 
        k = map(LEDcolor, 250, 500, 0, 255); 
        R_BRIGHT = 0; G_BRIGHT = 255; B_BRIGHT = 255 - k; 
      } 
      if (LEDcolor > 500 && LEDcolor <= 750) { 
        k = map(LEDcolor, 500, 750, 0, 255); 
        R_BRIGHT = k; G_BRIGHT = 255; B_BRIGHT = 0; 
      } 
      if (LEDcolor >= 750) { 
        k = map(LEDcolor, 750, 1023, 0, 255); 
        R_BRIGHT = 255; G_BRIGHT = 255 - k; B_BRIGHT = 0; 
      }  
      analogWrite(R_LED, R_BRIGHT);
      analogWrite(G_LED, G_BRIGHT);
      analogWrite(B_LED, B_BRIGHT);
      led_millis = now_millis; 
    }
    break;
  case 4: // случайный цвет
    if (now_millis - led_millis > 200 + LEDcolor) {
      R_BRIGHT = random(6)*51;
      G_BRIGHT = random(6)*51;
      B_BRIGHT = random(6)*51;
      analogWrite(R_LED, R_BRIGHT);
      analogWrite(G_LED, G_BRIGHT);
      analogWrite(B_LED, B_BRIGHT);
      led_millis = now_millis; 
    }
    break;
  }
  
  // обновление экрана
  if (now_millis - lcd_millis > LCD_RENEW) {
   LEDcolor = analogRead(POT_BTN);
   print_lcd();
   lcd_millis = now_millis;
  } 
}
//****************************
void print_lcd(void) {
 lcd.clear();
 lcd.setCursor(0,0);
 lcd.print("R"); lcd.print(R_BRIGHT);
 lcd.setCursor(4,0);
 lcd.print(" G"); lcd.print(G_BRIGHT);
 lcd.setCursor(9,0);
 lcd.print(" B"); lcd.print(B_BRIGHT);
 lcd.setCursor(0,1);
 lcd.print("Mode:"); lcd.print(mode); lcd.print("-"+modes[mode]);
}
//****************************
void awhiteoff() {
  R_BRIGHT = 0;
  G_BRIGHT = 0;
  B_BRIGHT = 0;
  analogWrite(R_LED, R_BRIGHT);
  analogWrite(G_LED, G_BRIGHT);
  analogWrite(B_LED, B_BRIGHT);
}
//****************************
void awhiteon() {
  R_BRIGHT = 255;
  G_BRIGHT = 255;
  B_BRIGHT = 255;
  analogWrite(R_LED, R_BRIGHT);
  analogWrite(G_LED, G_BRIGHT);
  analogWrite(B_LED, B_BRIGHT);
}
