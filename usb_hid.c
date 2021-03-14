#include <18F4550.h>//Kullanýlan mikrodenetleyiciye ait kütüphane eklenmiþtir.

/*
HSPLL-Yüksek hýzlý kristal osilatör PLL ile yükseltilip clock source olarak 
kullanýrýz.
Kristal osilatör=20MHz
PLL5->20MHz osilatör frekansý 5'e bölünür.PIC18F4550'nin PLL giriþine her zaman 4MHz uygulanýr.
PLL giriþindeki 4MHz 24 ile çarpýlýr.PLL çýkýþý 96MHz'dir.
CPUDIV2 System clock elde etmek için PLL çýkýþý frekansý 2'ye bölünür. 96MHz/2=48MHz'dir.
PIC18F4550'nin usb haberleþmesi USB Serial Interface Engine(SIE) tarafýndan gerçekleþtirilir. 
SIE'nin clock girþi frekansý 48MHz(Full-speed mod) veya 6MHz(low-speed mod) deðerlerinden biri olmak zorundadýr.
USBDIV SIE'nin clock giriþine PLL/2 deðerini yani 48MHz deðerini atar.
VREGEN USB D+ Pininin pull-up direnci üzerinden 3.3V ile beslenmesi için 18.pinden 3.3V üretilmesini saðlar.
*/
#fuses HSPLL, NOWDT, NOPROTECT, NOLVP, NODEBUG, USBDIV, PLL5, CPUDIV2, VREGEN
#use delay(clock = 48 M) //clock frekansý 48MHzolarak kaydedildi.
//16x2 LCD displayin kontrol edilmesini saðlayan pinler tanýmlandý.
#define LCD_RS_PIN PIN_D2
#define LCD_RW_PIN PIN_D3
#define LCD_ENABLE_PIN PIN_D4
#define LCD_DATA4 PIN_D5
#define LCD_DATA5 PIN_D6
#define LCD_DATA6 PIN_D7
#define LCD_DATA7 PIN_B0
//LCD displayin kontrol edilmesini saðlayan kütüphane eklendi.
#include <lcd.c>
 //isalpha ve isdigit fonksiyonlarýný içeren kütüphane eklendi.
#include <ctype.h>
 //15 byte hosta gönderildiðinde baþarýlý bir þekilde ekrana yazdýrýlýr.
// Hostan 10 byte alabilir
#define USB_CONFIG_HID_TX_SIZE 15 //15 byte hosta gönderildiðinde baþarýlý bir þekilde ekrana yazdýrýlýr.
#define USB_CONFIG_HID_RX_SIZE 10 // Hostan 10 byte alabilir
//Bu pin tanýmlanmazsa usb_attach ve usb task fonksiyonlarý sürekli olarak usb baðlý olarak algýlar.
//Bu pine 5V uygulanýrsa usb baðlý 0V uygulanýrsa usb baðlý deðil olarak algýlanýr.
//Bu uygulamada bu pin dorudan 5V a baðlýdýr.
#define USB_CON_SENSE_PIN PIN_B4
#include <pic18_usb.h>   //USB.c için Microchip PIC18FFXX5X donaným katmaný tanýmlamalarýný içeren kütüphane
 //usb_desc_hid2.h USB konfigürasyonlarýný ve usb descriptor deðerlerini içerir.
//usb_desc_hid.h dosyasý C:\Program Files (x86)\PICC\Drivers yolunda bulunmaktadýr. usb_desc_hid2.h dosyasý ayný dosya olup sadece
// 364. satýrda char const USB_STRING_DESC[] dizisinde deðiþiklilik yapýlmýþtýr. char USB_STRING_DESC_OFFSET[]={0,4,18} deðeri deðiþtirilmiþtir.

#include <usb_desc_hid2.h>

//#include <usb.h>        //usb fonksiyonlarýný içeren dosyayý ekledik.

#include <usb.c>        //usb fonksiyonlarýný içeren dosyayý ekledik.

#define LED_RED PIN_B7 //Yeni enumerate iþlemi olduðunu gösteren led tanýmlandý
#define LED_GREEN PIN_B6 //yeni enumerate iþlemi olmadýðýný gösteren led.

//LED Yakma fonksiyonudur.
#ifndef LED_ON
#define LED_ON(x) output_low(x)
#endif
//Led söndürme fonksiyonudur.
#ifndef LED_OFF
#define LED_OFF(x) output_high(x)
#endif
//Buttona basýldýðýnda 1 basýlmadýðýnda ise 0 deðerini veren fonksiyon.
#define BUTTON_PRESSED()(!input(PIN_B5))
//usb baðlantýsýnýn durumunun sürekli olarak takip edilmesini saðlayan fonksiyondur.  

void usb_debug_task(void) {
  static int8 last_connected;
  static int8 last_enumerated;
  int8 new_connected;
  int8 new_enumerated;

  new_connected = usb_attached(); //usb_attached() fonksiyonu PIC usb baðlantýsý algýladýðýnda 1 aksi durumda 0 verir.
  new_enumerated = usb_enumerated(); //usb_enumerated() fonksiyonu PIC host tarafýndan listelendiðinde(enumeration) 1 aksi halde 0 verir.

  /*

  Yeni listeleme olduðunda kýrmýzý led yanar.  
  Ardýndan tekrar söner ve yeþil led yanar.
  */
  if (new_enumerated) {
    LED_ON(LED_RED);
    LED_OFF(LED_GREEN);
  } else {
    LED_OFF(LED_RED);
    LED_ON(LED_GREEN);
  }
  //Eðer yeni baðlantý algýlandýysa ve daha önce algýlanmadýysa 

  if (new_connected && !last_connected) {
    // printf(lcd_putc,"\fUSB Baðlantýsý kuruldu.");

  }
  if (!new_connected && last_connected) {
    // printf(lcd_putc,"\fUSB Baðlantýsý Kesildi");

  }
  if (new_enumerated && !last_enumerated) {
    // printf(lcd_putc,"\fUSB aygit yöneticisinde eklendi");

  }
  if (!new_enumerated && last_enumerated) {
    // printf(lcd_putc,"\fUSB baðlantýsý algýlandý\nAygýt yöneticisinde listelenmesi bekleniyor.");

  }
  last_connected = new_connected;
  last_enumerated = new_enumerated;
}

void main(void) {
  //PC'den gelecek ve PC'ye gönderilecek verileri tutacak diziler tanýmlandý
  char out_data[USB_CONFIG_HID_TX_SIZE] = "Buttona Basildi";
  char in_data[USB_CONFIG_HID_RX_SIZE];
  //lcd fonksiyonlarý baþlatýldý.
  //lcd ekraný temizlendi.
  //USB HID Basladi mesajý yazdýrýldý.
  lcd_init();
  lcd_putc('\f');
  lcd_putc("USB HID Basladi");

  //usb çevresel birimi ve usb stack'ý baþlatýr .
  //Bu fonksiyon usb interruptlarýný aktif etmez ve doðrudan veri yolunu usb yoluna baðlamaz.
  //Bu fonksiyona laternatif olarak usb_init fonksiyonu kullanýlabilinir. 
  //Bu fonksiyon usb çevresel birimi ve usb stack'ý baþlatýr.interruptlarý aktif edip otomatik olarak usb veri yolu baðlantýsýný gerçekleþtirir.
  //Bu fonksiyonu kullanmamamýzýn sebebi bu fonksiyonun usb baðlantýsý gerçekleþene kadar sonsuz döngüde beklemesidir. 
  //Oysa biz usbnin baðlý olmadýðý durumuda algýlamak istiyoruz.
  //USB baðlý olmadýðý durumda da periyodik olarak usb task fonksiyonunu çaðýrark usb baðlantýsý olduðunda interruptlarý aktif edip,usb veri veri yolu baðlantýsýný kurabiliriz.
  usb_init_cs();
  //in_data dizisinin tüm elemanlarýna 0x00 deðeri atandý
  memset(in_data, 0x00, USB_CONFIG_HID_RX_SIZE);

  while (TRUE) {
    /*
    usb_task fonksiyonu çaðýrýldýðýnda usb algýlama pinini kontrol eder. 
    Eðer usb baðlantýsý algýlanýr ve usb çevresel birimi PC'ye baðlý deðilse
    bu fonksiyon USB çevresel birimini PC'ye baðlar ve USB interruptlarýný aktif eder. Bundan sonra 
    PC enumeration iþlemini baþlatýr.Eðer usb algýlama pini tanýmlanmamýþsa usb_task fonksiyonu sürekli olarak usb baðlý olarak algýlar.
    Bu durumda usb_enumerated() fonksiyonu 0 deðeri döndürür. 
    Bu fonksiyon USB çevresel birimleri çalýþýrken USB baðlantýsý olmadýðýný tespit ederse USB çevresel birimini resetler. 
    */

    usb_task();

    usb_debug_task();

    //Eðer PIC PC tarafýndan konfigüre edilip, aygýt listesine eklenmiþse       
    if (usb_enumerated()) {
      //Eðer pin-b5'e baðlý buttona basýldý ise 
      if (BUTTON_PRESSED()) {

        while (BUTTON_PRESSED());
        //usb hid ile out_data dizisinin USB_CONFIG_HID_TX_SIZE bytelýk elemanlarý PCye gönderilir.
        usb_put_packet(1, out_data, USB_CONFIG_HID_TX_SIZE, USB_DTS_TOGGLE);
        delay_ms(200);
      }
      //usb_kbhit(1) fonksiyonu RX bufferýnda yeni veri varsa true yoksa false deðerini döndürür.
      if (usb_kbhit(1)) {
        //Rx bufferdan alýnan veri in_data dizisine atanýr.
        usb_get_packet(1, in_data, USB_CONFIG_HID_RX_SIZE);

        lcd_gotoxy(1, 1);
        printf(lcd_putc, "\f");
        for (int i = 0; i < USB_CONFIG_HID_RX_SIZE; i++) {
          if ((isalpha(in_data[i]) != 0) || (isdigit(in_data[i]) != 0)) {
            printf(lcd_putc, "%c", in_data[i]);
          }
        }
      }

    }

  }
}
