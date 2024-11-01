#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <time.h>


const char* ssid = "your_wifi";
const char* pass = "your_pass";


struct tm timeinfo;


int lcds_time = 3399;
int real_time = 0;



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
    lcd.setCursor(0,0);
    lcd.print("Connecting...");
  }
  lcd.clear();
  lcd.print("Connected");
  lcd.setCursor(0,1);
  lcd.print("successfully");
  Serial.println("Подключено к Wi-Fi");
  
  configTime(3 * 3600, 0, "time.google.com", "time.nist.gov");
  delay(1000);
  
  while(!getLocalTime(&timeinfo)) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("ERROR");
    lcd.setCursor(0,1);
    lcd.print("FAILD GET TIME");
    Serial.println("Не удалось получить время (failed to get time)");
    delay(1000);
  }

  WiFi.disconnect();
  
  Serial.print("Текущее время: ");
  Serial.print(asctime(&timeinfo));

  init_digits(); // initialization digits
  lcd.clear();
}

void loop() {
  if (!getLocalTime(&timeinfo)) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("ERROR");
    lcd.setCursor(0,1);
    lcd.print("FAILD GET TIME");
    Serial.println("Не удалось получить время (failed to get time)");
    return;
  }
  String time_p = extractTime(asctime(&timeinfo));
  time_p.remove(2, 1);

  real_time = ((time_p.charAt(0) - '0')*1000) + ((time_p.charAt(1) - '0')*100) + ((time_p.charAt(2) - '0')*10) + (time_p.charAt(3) - '0');
  
  if(real_time != lcds_time){
    if(real_time/1000 != lcds_time/1000)
      draw_digit(real_time/1000, 0);
      
    if(real_time/100%10 != lcds_time/100%10)
      draw_digit(real_time/100%10, 3);
      
    if(real_time/10%10 != lcds_time/10%10)
      draw_digit(real_time/10%10, 7);
      
    if(real_time%10 != lcds_time%10){
      draw_digit(real_time%10, 10);
      lcds_time = real_time;
      }
      else{
        draw_digit(real_time%10, 10);
        lcds_time = real_time;
      }
  }
}

void draw_digit(int dig, int x){
  switch (dig) {

    // lcd.write(16);  void symbol
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
      lcd.setCursor(x, 0);
      lcd.write(16);
      lcd.write(1);
      lcd.write(2);
      lcd.setCursor(x, 1);
      lcd.write(16);
      lcd.write(16);
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
      lcd.setCursor(x, 1);
      lcd.write(16);
      lcd.write(16);
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
      lcd.setCursor(x, 1);
      lcd.write(16);
      lcd.write(0);
      lcd.write(16);
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
      lcd.setCursor(x, 1);
      lcd.write(16);
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
