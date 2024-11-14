// =====================Подключение необходимых библиотек=========================
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <time.h>
#include <EEPROM.h>
#include <WebServer.h>

// =====================Задаём размер для энергонезависимой памяти=========================
#define EEPROM_SIZE 128

// =====================Задаём флаг для памяти=========================
#define EEPROM_DATA_FLAG 166

// =====================Задаём адреса памяти в который будут записываться данные=========================
#define ssid_addr 10
#define pass_addr 50

// =====================Количество попыток для переаодключения=========================
#define attemps_conn_count 5

const char* server_ssid = "LCD_clock"; // Название сети WiFi модуля для подключению к нему
const char* server_pass = "123456789"; // Пароль от WiFi сети 


String real_ssid; 
String real_pass;


IPAddress local_ip(192,168,4,1); // некоторые настройки для веб - сервера
IPAddress gateway(192,168,4,1);
IPAddress subnet(255,255,255,0);
WebServer server(80);


struct tm timeinfo;


int lcds_time = 0; // переменные для удобного вычесления изменений времени
int real_time = 0;


bool is_server = false;


LiquidCrystal_I2C lcd(0x27, 16,2);

/*
=============== Создание пользовательских символов ==================
*/
byte LT[] = {B00111,  B01111,  B11111,  B11111,  B11111,  B11111,  B11111,  B11111};
byte UB[] = {B11111,  B11111,  B11111,  B00000,  B00000,  B00000,  B00000,  B00000};
byte RT[] = {B11100,  B11110,  B11111,  B11111,  B11111,  B11111,  B11111,  B11111};
byte LL[] = {B11111,  B11111,  B11111,  B11111,  B11111,  B11111,  B01111,  B00111};
byte LB[] = {B00000,  B00000,  B00000,  B00000,  B00000,  B11111,  B11111,  B11111};
byte LR[] = {B11111,  B11111,  B11111,  B11111,  B11111,  B11111,  B11110,  B11100};
byte UMB[] = {B11111,  B11111,  B11111,  B00000,  B00000,  B00000,  B11111,  B11111};
byte LMB[] = {B11111,  B00000,  B00000,  B00000,  B00000,  B11111,  B11111,  B11111};


/*
=============== Приготовление перед запуском ==================
*/
void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  EEPROM.begin(EEPROM_SIZE);
  init_digits(); // инициализация пользовательских символов
  
  if ( EEPROM.read(0) == EEPROM_DATA_FLAG){ // если флаговая переменная совпадает с данными в памяти, то делать ...

/*
=============== Считывание ssid и pass из памяти ==================
*/
    int len = EEPROM.read(ssid_addr);
  
    for (int i = 0; i < len; i++) {
      real_ssid += (char)EEPROM.read(ssid_addr + 1 + i);
    }

    len = EEPROM.read(pass_addr);
  
    for (int i = 0; i < len; i++) {
      real_pass += (char)EEPROM.read(pass_addr + 1 + i);
    }

/*
=============== Попытка подключения к WiFi сети ==================
*/
    WiFi.begin(real_ssid, real_pass);
    int count = 0;
    while(WiFi.status() != WL_CONNECTED) {
      Serial.println("Подключение к Wi-Fi...");
      lcd.setCursor(0,0);
      lcd.print("Connecting...");
      lcd.setCursor(0,1);
      lcd.print("Attempt: ");
      lcd.setCursor(9,1);
      lcd.print(count++);
      delay(1000);
      if(count >= attemps_conn_count)
      break;
    }


    if(WiFi.status() == WL_CONNECTED){ // если получилось подключиться то делать ...
      lcd.clear();
      lcd.print("Connected");
      lcd.setCursor(0,1);
      lcd.print("successfully");
      Serial.println("Подключено к Wi-Fi");
  
      configTime(3 * 3600, 0, "time.google.com", "time.nist.gov"); // получение времени
      delay(1000);
  
      while(!getLocalTime(&timeinfo)) { // передаем время по адресу в структуру timeinfo
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("ERROR");
        lcd.setCursor(0,1);
        lcd.print("FAIL GET TIME");
        Serial.println("Не удалось получить время (failed to get time)");
        delay(1000);
      }

      lcd.setCursor(6,0); // рисуем ':' между чч и мм 
      lcd.write(165);
      lcd.setCursor(6,1);
      lcd.write(165);

      WiFi.disconnect();
    }
    else // если подключиться не получилось, то открываем сервер для конфигурационная
    create_conf_server();
    }
  else // если флаговая переменная не совпадает с данными в памяти, открываем сервер для конфигурационная
    create_conf_server();
}

void loop() {
  if (is_server){// если активен сервер, то слушаем изменения на сервере
    server.handleClient();
  }
  else{ // иначе работаем с временем
  if (!getLocalTime(&timeinfo)) { // если не получилось получить время, то делаем ...
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("ERROR");
    lcd.setCursor(0,1);
    lcd.print("FAIL GET TIME");
    Serial.println("Не удалось получить время (failed to get time)");
    return;
  } // иначе ...
  else{

  String time_p = extractTime(asctime(&timeinfo)); // из всей строки получем только чч:мм
  time_p.remove(2, 1); // убераем ":"

  real_time = ((time_p.charAt(0) - '0')*1000) + ((time_p.charAt(1) - '0')*100) + ((time_p.charAt(2) - '0')*10) + (time_p.charAt(3) - '0'); // получаем время как число в виде 1234


  if(lcds_time == 0) // если время на дисплее 0, то инициалицируем его значением
  lcds_time = (real_time/1000 - 1) * 1000 + (real_time/100%10 - 1) * 100 + (real_time/10%10 - 1) * 10 + real_time%10 - 1;

  
  if(real_time != lcds_time){ // если реальное время не совпадает с временем на часах, то сверяемся и изменяем его 
    
/*
=============== Пояснение какая часть будет проверяеться, будет выделена   ==================
*/ 
    
    if(real_time/1000 != lcds_time/1000) // проверка на Чч:мм
      draw_digit(real_time/1000, 0);
      
    if(real_time/100%10 != lcds_time/100%10)// проверка на чЧ:мм
      draw_digit(real_time/100%10, 3);
      
    if(real_time/10%10 != lcds_time/10%10) // проверка на чч:Мм
      draw_digit(real_time/10%10, 7);
      
    if(real_time%10 != lcds_time%10){ // проверка на чч:мМ
      draw_digit(real_time%10, 10);
      lcds_time = real_time;
      }
  }
  }
  }
}


/*
=============== Что будет происходить при подключении ==================
*/
void handle_OnConnect(){
    lcd.clear();
    lcd.print("New connection"); // вывод на дисплей информации о новом подключении
   String ptr = "<!DOCTYPE html><html lang=\"ru\">"; // формирование HTML страницы
ptr += "<head><meta charset=\"UTF-8\"><title>LCD Clock</title></head>";
ptr += "<body>";
ptr += "<h1><p align=\"center\">Конфигурационная страница</p></h1>";
ptr += "<h2><p align=\"center\">Режим точки доступа WiFi (AP)</p></h2>";
ptr += "<form action=\"/send_data\" method=\"POST\">";
ptr += "<p align=\"center\">Введите имя вашей WiFi сети</p>";
ptr += "<p align=\"center\"><input type=\"text\" id=\"input1\" name=\"input1\"></p>";
ptr += "<p align=\"center\">Введите пароль</p>";
ptr += "<p align=\"center\"><input type=\"password\" id=\"input2\" name=\"input2\"></p>";
ptr += "<p align=\"center\"><button type=\"submit\">Сохранить</button></p>";
ptr += "</form>";
ptr += "</body>";
ptr += "</html>";

    
    server.send(200, "text/html", ptr); // отпрака HTML страницы пользователю
  }

/*
=============== Что будет происходить при отправке данных ==================
*/
void handle_send_data() {
  if (real_ssid.length() == 0 || real_pass.length() == 0) {
    Serial.println("Value(s) are empty");

    }
      else{
              
        if (server.hasArg("input1")) {
          real_ssid = server.arg("input1"); // сохраняем значение первого поля

        if (server.hasArg("input2")) {
          real_pass = server.arg("input2"); // сохраняем значение второго поля
      
        EEPROM.write(0, EEPROM_DATA_FLAG); // запись флаговой переменной

        save_data(); // сохраняем значения

        EEPROM.commit(); 
      }
    }
    else
    {
      Serial.println("Не удалось получить данные");
    }
  }

  String message = "Data has saved:<br>WiFi name: " + real_ssid + "<br>Password: " + real_pass + "<br>"; // отправляем ответ пользователю
  server.send(200, "text/html", message);
}


/*
=============== Что будет происходить при заход на отсутствующую страницу сервера ==================
*/
void handle_NotFound(){
  server.send(404, "text/html", "404 ERROR /n Please go to на \'192.168.4.1\'");
}

/*
=============== Сохранение данных в память ==================
*/
void save_data(){ 
    int len_ssid = real_ssid.length(); // сохраняем длину ssid 
    EEPROM.write(ssid_addr, len_ssid);
    EEPROM.commit();

    for (int i = 0; i < len_ssid; i++) 
      EEPROM.write(ssid_addr + 1 + i, real_ssid[i]);

    EEPROM.commit();
  
    int len_pass = real_pass.length();  // сохраняем длину pass
    EEPROM.write(pass_addr, len_pass); 
    EEPROM.commit();

    for (int i = 0; i < len_pass; i++)
      EEPROM.write(pass_addr + 1 + i, real_pass[i]);
      
    EEPROM.commit();

 lcd.clear();
 lcd.setCursor(0,0);
 lcd.print("Data is saved");  
  }

/*
=============== Отрисовка цифр ==================
*/
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
  

/*
=============== Инициализация символов ==================
*/
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


/*
=============== Создание конфигурационного сервера ==================
*/

void create_conf_server(){ 
    is_server = true;
    WiFi.disconnect(); 

    WiFi.softAP(server_ssid, server_pass); // настройка параметров сервера
    WiFi.softAPConfig(local_ip, gateway, subnet);
    delay(100);

    server.on("/", HTTP_GET, handle_OnConnect); // настрока ссылок на сервере
    server.on("/send_data", HTTP_POST, handle_send_data);
    server.onNotFound(handle_NotFound);
    server.begin(); // запуск сервера

    lcd.clear(); // вывод имени точки доступа и ip адрес на который нужно зайти
    lcd.print(server_ssid);
    lcd.setCursor(0,1);
    lcd.print(local_ip);
  }


  String extractTime(String dateTime) {
    int firstColonIndex = dateTime.indexOf(':'); // Находим позицию первого двоеточия
    if (firstColonIndex != -1) {
        return dateTime.substring(firstColonIndex - 2, firstColonIndex + 3); // Извлекаем чч:мм
    }
    return ""; // Если не найдено, возвращаем пустую строку
}
