#include <Arduino.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include "DHT.h"

#define DHTPIN 32     
#define DHTTYPE DHT22 

float sicaklikOffset = 0.0; 

DHT dht(DHTPIN, DHTTYPE);

class Pins{

private:
    const uint8_t red_pin;
    const uint8_t green_pin;
    const uint8_t blue_pin;
    const uint8_t temp_sensor;
    const uint8_t servo_pin;
    const uint8_t SDA;
    const uint8_t SCL;
    const uint8_t btn_ony;
    const uint8_t btn_artr;
    const uint8_t btn_azalt;

  


public:
    Pins(const uint8_t red_pin, const uint8_t green_pin, const uint8_t blue_pin, const uint8_t temp_sensor,const uint8_t servo_pin, const uint8_t SDA , const uint8_t SCL, const uint8_t btn_ony,const uint8_t btn_artr,const uint8_t btn_azalt):red_pin(red_pin), green_pin(green_pin), blue_pin(blue_pin),temp_sensor(temp_sensor), servo_pin(servo_pin), SDA(SDA), SCL(SCL), btn_ony(btn_ony),btn_artr(btn_artr),btn_azalt(btn_azalt){
    }

    uint8_t getRed_pin() { return red_pin;}
    uint8_t getGreen_pin() { return green_pin;}
    uint8_t getBlue_pin() { return blue_pin;}
    uint8_t getTemp_sensor() { return temp_sensor;}
    uint8_t getOny() { return btn_ony;}
    uint8_t getArtr() { return btn_artr;}
    uint8_t getAzalt() { return btn_azalt;}
    
    void begin(Servo *motor);

};



void Pins::begin(Servo *motor){

  pinMode(red_pin,OUTPUT);

  pinMode(green_pin,OUTPUT);

  pinMode(blue_pin,OUTPUT);

  pinMode(temp_sensor,INPUT);

  pinMode(btn_ony,INPUT_PULLUP);

  pinMode(btn_artr,INPUT_PULLUP);

  pinMode(btn_azalt,INPUT_PULLUP);

  motor->attach(servo_pin); 

}

typedef struct{

  uint8_t sensorId;

  float sensorValue;

}Sensor;

typedef enum{

  ACIK,

  KAPALI

}State_Machine;

void renkayar(uint8_t r,uint8_t g,uint8_t b,Pins *p);

void servoAcik(Servo *motor);

void servoKapali(Servo *motor);

float tempayar(Pins *p);

void saveSettings(uint8_t durum);

uint8_t loadSettings();

void saveCase(uint8_t vana_durum);

uint8_t loadCase();

void saveGoal(float genelhedef);

float loadGoal();

////////////////////

Preferences mypreferences;

Pins pinler(14,26,27,32,25,21,22,33,13,12);

State_Machine case1=KAPALI;

State_Machine case2=KAPALI;

TaskHandle_t task1;

TaskHandle_t task2;

QueueHandle_t sicaklikqueue;

QueueHandle_t casequeue;

QueueHandle_t hedefsicaklikqueue;

uint8_t durum = 0; // 0000 0000

#define OTOMATIK (1<<0) // 0000 0001

#define MANUEL (1<<1) // 0000 0010

#define ANAEKRAN (1<<2) // 0000 0100

#define SICAKLIKAYARLAMA (1<<3) //0000 1000

Servo myservo;

LiquidCrystal_I2C lcd(0x27,16,2);

// HTML Kodunu Hafızada (Flash) saklıyoruz, RAM'i şişirmesin diye PROGMEM kullanıyoruz
const char index_html[] PROGMEM = 
  R"rawliteral(
      <!DOCTYPE HTML><html>
        <head>
          <title>Akilli Vana</title>
          <meta name="viewport" content="width=device-width, initial-scale=1">
          <style> 
              body { font-family: Arial; text-align: center; background-color: #f2f2f2; margin: 20px; }
    
              .btn { display: block; width: 80%; max-width: 300px; margin: 10px auto; padding: 15px; font-size: 20px; color: white; border: 
              none; border-radius: 10px; cursor: pointer; }
    
              .mod-btn { background-color: #008CBA; }
    
              .ac-btn { background-color: #4CAF50; }
    
              .kapa-btn { background-color: #f44336; }
    
    
              /* YENİ: Sıcaklık Göstergesi Tasarımı */
    
              .sicaklik-kutu {
      
              background-color: #fff;
      
              color: #333;
      
              padding: 20px;
      
              margin: 20px auto;
      
              border-radius: 50%;
      
              width: 150px;
      
              height: 150px;
      
              display: flex;
      
              align-items: center;
      
              justify-content: center;
      
              font-size: 30px;
      
              font-weight: bold;
      
              border: 5px solid #ff9800;
      
              box-shadow: 0 4px 8px rgba(0,0,0,0.2);
    
              }
    
              #bildirim { padding: 10px; margin-top: 20px; background: #fff; border: 1px solid #ddd; }
  
              </style>

              </head>

              <body>

  
              <h1>Vana Kontrol Paneli</h1>

  
              <div class="sicaklik-kutu">
    
              <span id="temp_val">--</span>&deg;C
  
              </div>

  
              <h2>Mod Secimi</h2>
  
              <button class="btn mod-btn" onclick="komutGonder('/mod_oto')">OTOMATIK</button>
  
              <button class="btn mod-btn" onclick="komutGonder('/mod_manuel')">MANUEL</button>

  
              <h2>Manuel Kontrol</h2>
  
              <button class="btn ac-btn" onclick="komutGonder('/vana_ac')">AC</button>
  
              <button class="btn kapa-btn" onclick="komutGonder('/vana_kapat')">KAPAT</button>

  
              <div id="bildirim">Sistem Hazir...</div>

              <hr style="width:50%; opacity:0.3; margin-top:20px;">
  
  
              <h2>Hedef Sicaklik</h2>
  
              <input type="number" id="hedef_temp" placeholder="Orn: 25" style="padding:10px; font-size:18px; width:100px; border-radius:5px; 
              border:1px solid #ccc;">
  
              <button class="btn mod-btn" style="display:inline-block; width:auto; margin-left:10px;" onclick="hedefGonder()">AYARLA</button>

  
              <script>
  
              // 1. Butonlara basınca çalışan fonksiyon
  
              function komutGonder(adres) {
  
              document.getElementById("bildirim").innerHTML = "Islem yapiliyor...";
  
              var xhr = new XMLHttpRequest();
  
              xhr.open("GET", adres, true);
  
              xhr.onreadystatechange = function() {
  
              if (this.readyState == 4 && this.status == 200) {
  
              document.getElementById("bildirim").innerHTML = this.responseText;
  
              }
  
              };
  
              xhr.send();
  
              }

    
              // 2. YENİ: Her 2 saniyede bir sıcaklığı güncelleme fonksiyonu
    
              setInterval(function() {
    
              var xhr = new XMLHttpRequest();
    
              xhr.open("GET", "/sicaklik_oku", true); // ESP'ye gizli istek at
    
              xhr.onreadystatechange = function() {
    
              if (this.readyState == 4 && this.status == 200) {
    
              // Gelen cevabı yuvarlak kutunun içine yaz
    
              document.getElementById("temp_val").innerHTML = this.responseText;
    
              }
    
              };
    
              xhr.send();
    
              }, 2000);
               // 2000 milisaniye = 2 saniye
               function hedefGonder() {
  
               var val = document.getElementById("hedef_temp").value;
  
               if(val !== "") {
    
               komutGonder("/set_hedef?deger=" + val);
  
               } else {
    
               alert("Lütfen bir sıcaklık değeri girin!");
  
               }

               }
  
              </script>

              </body>

              </html>

              )rawliteral";

WebServer sunucu(80);

void TaskVana(void *pvParameters);

void TaskWiFi(void *pvParameters);


float genelhedef;

float gecici;

Sensor SicaklikSensor;

float gecici1 = 0;

byte degree[8]={0b01100,
  0b10010,
  0b10010,
  0b01100,
  0b00000,
  0b00000,
  0b00000,
  0b00000};//derece 

  unsigned long onceki_zaman=0;
  unsigned long butonsuresi=300;
  unsigned long sonsicaklikokuma=0;
void setup() {

  Serial.begin(115200);

  sicaklikqueue=xQueueCreate(1,sizeof(Sensor));

  casequeue=xQueueCreate(1,sizeof(State_Machine));

  hedefsicaklikqueue=xQueueCreate(1,sizeof(float));

  SicaklikSensor.sensorId=1;
  
  pinler.begin(&myservo);

  durum=loadSettings();

  case1=(State_Machine)loadCase();//tür değiştirme

  genelhedef=loadGoal();

  lcd.init();

  dht.begin();

  lcd.backlight();

  lcd.createChar(0,degree);

  lcd.setCursor(0,0);

  lcd.print("Basliyor");

  lcd.setCursor(0,1);

  lcd.print("YEHH Sunar..");
  
  delay(5000);

  lcd.clear();

  gecici = genelhedef;

  xTaskCreatePinnedToCore(
        TaskVana,
        "TaskVana",
        15000,
        NULL,
        1,
        &task1,
        1
    );

    xTaskCreatePinnedToCore(
        TaskWiFi,
        "TaskWiFi",
        15000,
        NULL,
        1,
        &task2,
        0
    );

}

void loop() {
  
  vTaskDelay(1000);

}

void TaskVana(void *pvParameters){


  for(;;){

    unsigned long suanki_zaman=millis();

    if(suanki_zaman - sonsicaklikokuma > 2000){

      SicaklikSensor.sensorValue=tempayar(&pinler); //18 derece 

      xQueueOverwrite(sicaklikqueue,&SicaklikSensor);

      sonsicaklikokuma=suanki_zaman;

    }

    

    State_Machine gelenvana;

    if(xQueueReceive(casequeue,&gelenvana,0)==pdTRUE){
      
      case1=gelenvana;

    }
    float webhedefi=0.0;

    if(xQueueReceive(hedefsicaklikqueue,&webhedefi,0)==pdTRUE){
      
        genelhedef=webhedefi;

        saveGoal(genelhedef);

        gecici=genelhedef;

    }

    if(suanki_zaman - onceki_zaman >butonsuresi){

      if(digitalRead(pinler.getOny())==LOW){

          lcd.clear();

          if(durum & ANAEKRAN){
            durum&=~ANAEKRAN;
            durum|=SICAKLIKAYARLAMA;
            gecici = genelhedef;
        
          }
          else{
            durum&=~SICAKLIKAYARLAMA;
            durum|=ANAEKRAN;
            genelhedef = gecici;
            saveGoal(genelhedef);

          }
          onceki_zaman=suanki_zaman;
          saveSettings(durum);
    
      }
      if(digitalRead(pinler.getArtr())==LOW && (durum & SICAKLIKAYARLAMA)){

        gecici++;

        onceki_zaman=suanki_zaman;
        
      }

      else if(digitalRead(pinler.getAzalt())==LOW && (durum & SICAKLIKAYARLAMA)){
        gecici--;

        onceki_zaman=suanki_zaman;

      }
    }
    

    if(durum & SICAKLIKAYARLAMA){

      lcd.setCursor(0, 1);
      lcd.print("Set: ");
      lcd.print(gecici, 1); // Ondalık gösterim
      lcd.write(byte(0));
      lcd.print("C   ");
    }
    else{
        lcd.setCursor(0,1);
        lcd.print("Vana Durumu:");
        lcd.print( (case1 == ACIK) ? "ACIK " : "KAPALI  " );
        
    }

    if(durum & OTOMATIK){ //0000 0010 & 0000 0010 // 0000 0001 & 0000 0010 

      if(SicaklikSensor.sensorValue>=genelhedef){
        case1=KAPALI;
      }
      else if(SicaklikSensor.sensorValue<(genelhedef-1)){
        case1=ACIK;
      }

    }

    if(SicaklikSensor.sensorValue>=45){
      case1=KAPALI;
    }

    if(case1 != case2){
      saveCase(case1);
      case2=case1;

    switch (case1){
    case ACIK:
      renkayar(0,255,0,&pinler);
      servoAcik(&myservo);
      
      break;
      
    case KAPALI:
      renkayar(255,0,0,&pinler);
      servoKapali(&myservo);
      break;
    
    default:
      break;
    }
      
    }
    
    lcd.setCursor(0,0);
    lcd.print("Sicaklik: ");
    lcd.print(SicaklikSensor.sensorValue);
    lcd.write(byte(0));
    lcd.print("C");
    
    vTaskDelay(150);

  }
}
void TaskWiFi(void *pvParameters){

  WiFi.mode(WIFI_AP);
  
  WiFi.softAP("YEHH","yehh1234",1,0,4);

  Serial.print(WiFi.softAPIP());



  sunucu.on("/",HTTP_GET,[](){ //index.html/
    sunucu.send(200,"text/html",index_html);
  });
  sunucu.on("/sicaklik_oku",HTTP_GET,[](){
    Sensor wifisicaklik;

    if(xQueueReceive(sicaklikqueue,&wifisicaklik,0)==pdTRUE && wifisicaklik.sensorId==1){ 

        sunucu.send(200,"text/plain",String(wifisicaklik.sensorValue));
      
    }
    else{
      sunucu.send(200,"text/plain","SICAKLIK VERISI YOK");

    }

  });

  sunucu.on("/set_hedef",HTTP_GET,[](){

      if(sunucu.hasArg("deger")){

        String sicaklikverisi= sunucu.arg("deger");

        float hedefsicaklik = sicaklikverisi.toFloat();

        if(hedefsicaklik>15 && hedefsicaklik<40){

          xQueueOverwrite(hedefsicaklikqueue,&hedefsicaklik);

          sunucu.send(200, "text/plain", "AYARLANDI: " + String(hedefsicaklik));
        } 

        else{
          sunucu.send(200, "text/plain", "GECERSIZ DEGER (15-40 Arasi Girin)");
        }
        
      }
      else{
        sunucu.send(200,"text/plain","HEDEF SICAKLIK YOK");
      }

  });

  


  sunucu.on("/mod_oto",HTTP_GET,[](){

    if(durum&MANUEL){//eğer manueldeyse ve otoya basıldıysa mod manuel olsun

      durum&=~MANUEL; //0000 0001 & 1111 1110 = 0000 0000 
      durum|=OTOMATIK; //0000 0000 | 0000 0010 = 0000 0010 = otomatik 
      sunucu.send(200,"text/plain","OTOMATIGE GECILDI");
      saveSettings(durum);
    }
    else{
      sunucu.send(200,"text/html","ZATEN OTODA");
    }

  });
  //güvenlik bariyeri 40 derece kendini kapatsın

  sunucu.on("/mod_manuel",HTTP_GET,[](){

    if(durum&OTOMATIK){

      durum&=~OTOMATIK;
      durum|=MANUEL;
      sunucu.send(200,"text/plain","MANUELE GECILDI");
      saveSettings(durum);
    }
    else{
      sunucu.send(200,"text/html","ZATEN MANUELDE");
    }

  });


  sunucu.on("/vana_ac",HTTP_GET,[](){

    if(durum&MANUEL){
      State_Machine vana=ACIK;
      xQueueOverwrite(casequeue,&vana);
      sunucu.send(200,"text/plain","VANA ACIK");

    }
    else{
      sunucu.send(200,"text/plain","MANUEL OLMALI!");
    }

  });

  sunucu.on("/vana_kapat",HTTP_GET,[](){

    if(durum&MANUEL){
      State_Machine vana=KAPALI;
      xQueueOverwrite(casequeue,&vana);
      sunucu.send(200,"text/plain","VANA KAPALI");

    }
    else{
      sunucu.send(200,"text/plain","MANUEL OLMALI!");
    }

  });

  sunucu.begin();
  
  for(;;){

    sunucu.handleClient();


    vTaskDelay(100);
  }

}


void renkayar(uint8_t r,uint8_t g,uint8_t b,Pins *p){

  analogWrite(p->getRed_pin(),r); 

  analogWrite(p->getGreen_pin(),g);

  analogWrite(p->getBlue_pin(),b);

}



float tempayar(Pins *p){
  
  float nem = dht.readHumidity();

  float sicaklik = dht.readTemperature();

  

  if (isnan(nem) || isnan(sicaklik)) {

    Serial.println("Sensor Hatasi");

    return gecici1;

  }

  float sonSicaklik = sicaklik - sicaklikOffset;

  gecici1=sonSicaklik;

  return sonSicaklik;

}



void servoAcik(Servo *motor){

  motor->write(90);

}

void servoKapali(Servo *motor){

  motor->write(0); 

}

void saveSettings(uint8_t durum){

  mypreferences.begin("vana",false);

  mypreferences.putUChar("durum",durum); 

  mypreferences.end();

}

uint8_t loadSettings(){

  mypreferences.begin("vana",true);

  uint8_t setting =mypreferences.getUChar("durum",5);

  mypreferences.end();

  return setting;

}

void saveCase(uint8_t vana_durum){

  mypreferences.begin("vana",false);

  mypreferences.putUChar("vana_durum",vana_durum);

  mypreferences.end();

}

uint8_t loadCase(){

  mypreferences.begin("vana",true);

  uint8_t valve = mypreferences.getUChar("vana_durum",1);//kapalı demek

  mypreferences.end();

  return valve;

}
void saveGoal(float genelhedef){

  mypreferences.begin("vana",false);

  mypreferences.putFloat("sicaklik_hedefi",genelhedef);

  mypreferences.end();

}
float loadGoal(){

  mypreferences.begin("vana",true);

  float sicaklikgoal = mypreferences.getFloat("sicaklik_hedefi",24);//kapalı demek

  mypreferences.end();

  return sicaklikgoal;

}



