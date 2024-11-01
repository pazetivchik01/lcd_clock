#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <time.h>


const char* ssid = "your_wifi";
const char* pass = "your_pass";


struct tm timeinfo;


LiquidCrystal_I2C lcd(0x27, 16,2);

byte LT[] = {B00111,  B01111,  B11111,  B11111,  B11111,  B11111,  B11111,  B11111};
byte UB[] = {B11111,  B11111,  B11111,  B00000,  B00000,  B00000,  B00000,  B00000};
byte RT[] = {B11100,  B11110,  B11111,  B11111,  B11111,  B11111,  B11111,  B11111};
byte LL[] = {B11111,  B11111,  B11111,  B11111,  B11111,  B11111,  B01111,  B00111};
byte LB[] = {B00000,  B00000,  B00000,  B00000,  B00000,  B11111,  B11111,  B11111};
byte LR[] = {B11111,  B11111,  B11111,  B11111,  B11111,  B11111,  B11110,  B11100};
byte UMB[] = {B11111,  B11111,  B11111,  B00000,  B00000,  B00000,  B11111,  B11111};
byte LMB[] = {B11111,  B00000,  B00000,  B00000,  B00000,  B11111,  B11111,  B11111};
byte dot_top[] = {B00000, B00000, B01110,B01110, B01110, B01110, B00000, B00000};


void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();


 WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Подключение к Wi-Fi...");
  }
  Serial.println("Подключено к Wi-Fi");


  configTime(3 * 3600, 0, "time.google.com", "time.nist.gov");

delay(1000);

    
  while(!getLocalTime(&timeinfo)) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("ERROR");
    lcd.setCursor(0,1);
    lcd.print("FAILD TO GET TIME");
    Serial.println("неудалось получить время (failed to get time)");

    delay(1000);
  }

  WiFi.disconnect();
  
      Serial.print("Текущее время: ");
      Serial.print(asctime(&timeinfo));

  
  init_digits();
  draw_digit(0,0);// digit , colounm, default row  - 0
  draw_digit(1,3);
  draw_digit(2,6);
  draw_digit(3,9);
}

void loop() {

  lcd.clear();

  if (!getLocalTime(&timeinfo)) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("ERROR");
    lcd.setCursor(0,1);
    lcd.print("FAILD GET TIME");
    Serial.println("неудалось получить время (failed to get time)");
    return;
  }
  
  String time_p = extractTime(asctime(&timeinfo));
  time_p.remove(2, 1);
  int i_tmp;
  for(int i = 0; i < 4; i++){
    if(i == 0)  i_tmp=0;
    if(i == 1)  i_tmp=3;
    if(i == 2)  i_tmp=7;
    if(i == 3)  i_tmp=10;
  
    draw_digit(time_p.charAt(i) - '0', i_tmp);
    
  }
  lcd.setCursor(6,0);
  lcd.write(165);
  lcd.setCursor(6,1);
  lcd.write(165);

  delay(10000);
}

void draw_digit(int dig, int x){
  switch (dig) {
    case 0:
      lcd.setCursor(x, 0); 
      lcd.write(0);  
      lcd.write(1);  
      lcd.write(2);
      lcd.setCursor(x, 1); 
      lcd.write(3);  
      lcd.write(4);  
      lcd.write(5);
      break;
    case 1:
      lcd.setCursor(x + 1, 0);
      lcd.write(1);
      lcd.write(2);
      lcd.setCursor(x + 2, 1);
      lcd.write(5);
      break;
    case 2:
      lcd.setCursor(x, 0);
      lcd.write(6);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(x, 1);
      lcd.write(3);
      lcd.write(7);
      lcd.write(7);
      break;
    case 3:
      lcd.setCursor(x, 0);
      lcd.write(6);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(x, 1);
      lcd.write(7);
      lcd.write(7);
      lcd.write(5);
      break;
    case 4:
      lcd.setCursor(x, 0);
      lcd.write(3);
      lcd.write(4);
      lcd.write(2);
      lcd.setCursor(x + 2, 1);
      lcd.write(5);
      break;
    case 5:
      lcd.setCursor(x, 0);
      lcd.write(0);
      lcd.write(6);
      lcd.write(6);
      lcd.setCursor(x, 1);
      lcd.write(7);
      lcd.write(7);
      lcd.write(5);
      break;
    case 6:
      lcd.setCursor(x, 0);
      lcd.write(0);
      lcd.write(6);
      lcd.write(6);
      lcd.setCursor(x, 1);
      lcd.write(3);
      lcd.write(7);
      lcd.write(5);
      break;
    case 7:
      lcd.setCursor(x, 0);
      lcd.write(1);
      lcd.write(1);
      lcd.write(2);
      lcd.setCursor(x + 1, 1);
      lcd.write(0);
      break;
    case 8:
      lcd.setCursor(x, 0);
      lcd.write(0);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(x, 1);
      lcd.write(3);
      lcd.write(7);
      lcd.write(5);
      break;
    case 9:
      lcd.setCursor(x, 0);
      lcd.write(0);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(x + 1, 1);
      lcd.write(4);
      lcd.write(5);
      break;
  }
}
  

void init_digits(){
  lcd.createChar(0, LT);
  lcd.createChar(1, UB);
  lcd.createChar(2, RT);
  lcd.createChar(3, LL);
  lcd.createChar(4, LB);
  lcd.createChar(5, LR);
  lcd.createChar(6, UMB);
  lcd.createChar(7, LMB);
    
  }

  String extractTime(String dateTime) {
    int firstColonIndex = dateTime.indexOf(':'); // Находим позицию первого двоеточия
    if (firstColonIndex != -1) {
        return dateTime.substring(firstColonIndex - 2, firstColonIndex + 3); // Извлекаем чч:мм
    }
    return ""; // Если не найдено, возвращаем пустую строку
}
