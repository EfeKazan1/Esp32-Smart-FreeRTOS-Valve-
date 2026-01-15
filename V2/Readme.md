## 🌟 Sürüm 2.0 (v2) Güncellemesi: Robustness & Verification

Projenin ikinci sürümünde, sistemin endüstriyel standartlarda **kararlılığını (stability)** ve **güvenilirliğini (reliability)** artırmak amacıyla kritik yazılım mimarisi güncellemeleri yapılmıştır:

### 🛡️ 1. Watchdog Timer (WDT) Entegrasyonu - (System Safety)
Sistemin olası kilitlenme (deadlock) veya sonsuz döngüye girme durumlarına karşı donanımsal koruma mekanizması eklenmiştir.
* **Mekanizma:** `esp_task_wdt` kütüphanesi kullanılarak her görev (Task) için bir zaman aşımı süresi tanımlanmıştır.
* **İşleyiş:** Eğer bir görev (örneğin sensör okuma veya WiFi bağlantısı) belirlenen sürede yanıt vermezse, Watchdog Timer devreye girer ve sistemi otomatik olarak **RESET**ler.
* **Sonuç:** İnsan müdahalesine gerek kalmadan kendi kendini kurtarabilen (Fault-Tolerant) bir yapı sağlanmıştır.

### ✅ 2. Unit Test (Birim Testleri) - (Verification)
Kritik karar mekanizmalarının (Vana Aç/Kapa, Mod Değişimi) doğruluğu, donanımdan soyutlanmış test senaryoları ile kanıtlanmıştır.
* **Yöntem:** Karar algoritmaları "Pure C" fonksiyonlarına dönüştürülerek donanım bağımlılıkları (Mocking) izole edilmiştir.
* **Kapsam:**
    * *Sınır Değer Analizi:* Histeresis (Hedef sıcaklık ±1°C) durumlarının doğrulanması.
    * *Mod Güvenliği:* Manuel moddayken otomatik sistemin devre dışı kaldığının doğrulanması.
    * *Bitwise Kontrolü:* Durum bayraklarının (Flags) doğru set/reset edildiğinin testi.
* **Kanıt:** Tüm senaryolar bilgisayar ortamında simüle edilmiş ve **%100 Başarı (Test Passed)** oranı yakalanmıştır.


