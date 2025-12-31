# 🚰 ESP32 FreeRTOS Akıllı Vana Kontrol Sistemi

Bu proje, endüstriyel veya ev otomasyonu için geliştirilmiş; sıcaklık takibi yapan, uzaktan kontrol edilebilen ve enerji kesintilerine karşı durumunu koruyan bir akıllı vana sistemidir.

## 🚀 Proje Özellikleri (Technical Highlights)

Bu projede **Gömülü Sistem Mühendisliği** prensipleri uygulanmıştır:

* **FreeRTOS Multitasking:** Sistem tek bir döngüde değil, **Çekirdek 0 (WiFi)** ve **Çekirdek 1 (Kontrol)** üzerinde paralel görevler halinde çalışır.
* **Dual Core Mimari:** Sensör okuma ve motor kontrolü, ağ trafiğinden etkilenmemesi için ayrıştırılmıştır.
* **NVS (Non-Volatile Storage):** `Preferences` kütüphanesi ile elektrik kesintilerinde son durum (Mod, Hedef Sıcaklık) Flash hafızada saklanır.
* **Web Sunucusu (Async WebServer):** HTML/JS arayüzü `PROGMEM` üzerinde tutularak RAM optimizasyonu sağlanmıştır.
* **Donanım Koruması:** Sensör hataları (`isnan`) ve aşırı sıcaklık (>45°C) durumlarında otomatik güvenlik protokolleri devreye girer.
* **Hafıza Yönetimi:** Stack Overflow'u önlemek için görev başına optimize edilmiş bellek yönetimi (15KB Stack Size).

 ## 🛠️ Teknik Uygulama Detayları

### Sonlu Durum Makinesi (FSM)
Sistemin ana kontrol mantığı bir Mealy Makinesi modeline dayanır.
Sistem, sensör verilerine ve buton girişlerine göre **BEKLEME**, **ÇALIŞMA** ve **RESET** durumları arasında geçiş yapar.
Bu yapı, asenkron olayların (race condition) önüne geçer ve resetleme lojiğinin doğru zamanlamada çalışmasını garanti eder.

### Düşük Seviye Bit Manipülasyonu
Maksimum verimlilik için register'lara doğrudan erişim sağlanmıştır:
- **Bitwise AND (`&`):** Giriş sinyallerindeki gürültüyü filtrelemek ve maskeleme yapmak için.
- **Bitwise OR (`|`):** Motor sürücü veya röle kontrol bayraklarını (flags) aktif etmek için.
- **Bitwise NOT (`~`):** Active-low çalışan bileşenler için lojik seviyeleri tersine çevirmek için kullanılmıştır.

## 🛠️ Donanım Listesi

* **Mikrodenetleyici:** ESP32 DevKit V1
* **Sensör:** DHT22 (Sıcaklık ve Nem)
* **Aktüatör:** Servo Motor (SG90/MG995)
* **Ekran:** 16x2 LCD (I2C Modüllü)
* **Giriş:** 3x Push Buton (Mod, Artır, Azalt) - *Dahili Pull-up ve Debounce korumalı*

## 🔌 Pin Bağlantı Şeması

| Bileşen | ESP32 Pini | Notlar |
| :--- | :--- | :--- |
| **Servo Motor** | GPIO 25 | PWM Çıkışı |
| **DHT22 Sensör** | GPIO 32 | 10k Pull-up Direnci ile |
| **LCD (SDA)** | GPIO 21 | I2C Hattı |
| **LCD (SCL)** | GPIO 22 | I2C Hattı |
| **Buton (Onay/Mod)** | GPIO 33 | INPUT_PULLUP |
| **Buton (Artır)** | GPIO 13 | INPUT_PULLUP |
| **Buton (Azalt)** | GPIO 12 | INPUT_PULLUP |

## 💻 Yazılım Mimarisi

Sistem iki ana görev (Task) üzerine kuruludur:

1.  **TaskVana (Core 1):** Sensör verilerini okur, LCD ekranı günceller, buton girişlerini `millis()` tabanlı debounce ile işler ve servo motoru kontrol eder.
2.  **TaskWiFi (Core 0):** SoftAP modunda bir WiFi ağı yayar ve Web Sunucusunu ayakta tutar. Kullanıcıdan gelen AJAX isteklerini kuyruk (Queue) yapısı ile diğer çekirdeğe iletir.


---
**Geliştirici:** Efe Kazan
**Okul:** Adnan Menderes Üniversitesi - Bilgisayar Mühendisliği
Proje Tanıtım Videosu=https://youtu.be/ytQfuZpMm0c
