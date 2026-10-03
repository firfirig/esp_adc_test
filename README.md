# ESP-IDF 6.1 - ESP32 / ESP32-S3 ADC Calibration Test

Amaç:
- Potansiyometre ile ADC girişini değiştirmek
- ESP-IDF'nin çip üzerindeki eFuse kalibrasyon bilgilerini kullanmak
- RAW ADC ve kalibre edilmiş mV değerini UART üzerinden görmek
- Multimetrede ölçülen gerçek voltaj ile ADC sonucunu karşılaştırmak

## Donanım - ESP32 DevKit C / GPIO34

10 kOhm potansiyometre:
- Bir uç -> 3.3V
- Diğer uç -> GND
- Orta uç -> GPIO34

İsteğe bağlı:
- GPIO34 ile GND arasına 100 nF kondansatör

ESP32 GPIO34 = ADC1_CH6.

## Donanım - ESP32-S3

ESP32-S3 GPIO34 ADC pini değildir. Bu projede başlangıç pini GPIO4:
- Pot orta uç -> GPIO4
- Pot uçları -> 3.3V ve GND

ESP32-S3 GPIO4 = ADC1_CH3.

## Derleme

ESP32:
    idf.py set-target esp32
    idf.py build
    idf.py -p COMx flash monitor

ESP32-S3:
    idf.py set-target esp32s3
    idf.py build
    idf.py -p COMx flash monitor

Aynı kaynak kodu iki hedefte de kullanılır.

## Kalibrasyon

ESP32:
- Line Fitting
- eFuse içindeki üretim kalibrasyon verileri kullanılır.

ESP32-S3:
- Curve Fitting
- eFuse içindeki üretim kalibrasyon verileri kullanılır.

Bu proje "multimetreye bakarak yazılımsal olarak kendi kalibrasyon katsayısını çıkarmaz".
Önce Espressif'in fabrika/eFuse kalibrasyonunu ölçüp değerlendireceğiz. Daha sonra istersen ayrıca
laboratuvar tipi 2/3 noktalı kullanıcı kalibrasyonu ekleyebiliriz.

## Ölçüm tablosu

Potu farklı konumlara getirip multimetrede GPIO ile GND arasını ölç:

| Multimetre | RAW | ADC mV | Hata mV | Hata % |
|---:|---:|---:|---:|---:|
| 0.000 V | | | | |
| 0.500 V | | | | |
| 1.000 V | | | | |
| 1.500 V | | | | |
| 2.000 V | | | | |
| 2.500 V | | | | |
| 3.000 V | | | | |
| 3.300 V | | | | |

Hata:
    Hata(mV) = ADC_mV - Multimetre_mV

Mutlak yüzde hata:
    |ADC_mV - Multimetre_mV| / Multimetre_mV * 100

0 V civarında yüzde hata kullanmak anlamsızlaşır; orada mutlak mV hatasını değerlendirmek daha doğru.

## Önemli

Potu doğrudan 3.3V ile GND arasına bağla. ADC pinine 5V verme.
