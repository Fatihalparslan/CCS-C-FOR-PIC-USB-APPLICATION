#include <18F4550.h>//Kullan�lan mikrodenetleyiciye ait k�t�phane eklenmi�tir.

/*
HSPLL-Y�ksek h�zl� kristal osilat�r PLL ile y�kseltilip clock source olarak 
kullan�r�z.
Kristal osilat�r=20MHz
PLL5->20MHz osilat�r frekans� 5'e b�l�n�r.PIC18F4550'nin PLL giri�ine her zaman 4MHz uygulan�r.
PLL giri�indeki 4MHz 24 ile �arp�l�r.PLL ��k��� 96MHz'dir.
CPUDIV2 System clock elde etmek i�in PLL ��k��� frekans� 2'ye b�l�n�r. 96MHz/2=48MHz'dir.
PIC18F4550'nin usb haberle�mesi USB Serial Interface Engine(SIE) taraf�ndan ger�ekle�tirilir. 
SIE'nin clock gir�i frekans� 48MHz(Full-speed mod) veya 6MHz(low-speed mod) de�erlerinden biri olmak zorundad�r.
USBDIV SIE'nin clock giri�ine PLL/2 de�erini yani 48MHz de�erini atar.
VREGEN USB D+ Pininin pull-up direnci �zerinden 3.3V ile beslenmesi i�in 18.pinden 3.3V �retilmesini sa�lar.
*/
#fuses HSPLL, NOWDT, NOPROTECT, NOLVP, NODEBUG, USBDIV, PLL5, CPUDIV2, VREGEN
#use delay(clock = 48 M) //clock frekans� 48MHzolarak kaydedildi.
//16x2 LCD displayin kontrol edilmesini sa�layan pinler tan�mland�.
#define LCD_RS_PIN PIN_D2
#define LCD_RW_PIN PIN_D3
#define LCD_ENABLE_PIN PIN_D4
#define LCD_DATA4 PIN_D5
#define LCD_DATA5 PIN_D6
#define LCD_DATA6 PIN_D7
#define LCD_DATA7 PIN_B0
//LCD displayin kontrol edilmesini sa�layan k�t�phane eklendi.
#include <lcd.c>
 //isalpha ve isdigit fonksiyonlar�n� i�eren k�t�phane eklendi.
#include <ctype.h>
 //15 byte hosta g�nderildi�inde ba�ar�l� bir �ekilde ekrana yazd�r�l�r.
// Hostan 10 byte alabilir
#define USB_CONFIG_HID_TX_SIZE 15 //15 byte hosta g�nderildi�inde ba�ar�l� bir �ekilde ekrana yazd�r�l�r.
#define USB_CONFIG_HID_RX_SIZE 10 // Hostan 10 byte alabilir
//Bu pin tan�mlanmazsa usb_attach ve usb task fonksiyonlar� s�rekli olarak usb ba�l� olarak alg�lar.
//Bu pine 5V uygulan�rsa usb ba�l� 0V uygulan�rsa usb ba�l� de�il olarak alg�lan�r.
//Bu uygulamada bu pin dorudan 5V a ba�l�d�r.
#define USB_CON_SENSE_PIN PIN_B4
#include <pic18_usb.h>   //USB.c i�in Microchip PIC18FFXX5X donan�m katman� tan�mlamalar�n� i�eren k�t�phane
 //usb_desc_hid2.h USB konfig�rasyonlar�n� ve usb descriptor de�erlerini i�erir.
//usb_desc_hid.h dosyas� C:\Program Files (x86)\PICC\Drivers yolunda bulunmaktad�r. usb_desc_hid2.h dosyas� ayn� dosya olup sadece
// 364. sat�rda char const USB_STRING_DESC[] dizisinde de�i�iklilik yap�lm��t�r. char USB_STRING_DESC_OFFSET[]={0,4,18} de�eri de�i�tirilmi�tir.

#include <usb_desc_hid2.h>

//#include <usb.h>        //usb fonksiyonlar�n� i�eren dosyay� ekledik.

#include <usb.c>        //usb fonksiyonlar�n� i�eren dosyay� ekledik.

#define LED_RED PIN_B7 //Yeni enumerate i�lemi oldu�unu g�steren led tan�mland�
#define LED_GREEN PIN_B6 //yeni enumerate i�lemi olmad���n� g�steren led.

//LED Yakma fonksiyonudur.
#ifndef LED_ON
#define LED_ON(x) output_low(x)
#endif
//Led s�nd�rme fonksiyonudur.
#ifndef LED_OFF
#define LED_OFF(x) output_high(x)
#endif
//Buttona bas�ld���nda 1 bas�lmad���nda ise 0 de�erini veren fonksiyon.
#define BUTTON_PRESSED()(!input(PIN_B5))
//usb ba�lant�s�n�n durumunun s�rekli olarak takip edilmesini sa�layan fonksiyondur.  

void usb_debug_task(void) {
  static int8 last_connected;
  static int8 last_enumerated;
  int8 new_connected;
  int8 new_enumerated;

  new_connected = usb_attached(); //usb_attached() fonksiyonu PIC usb ba�lant�s� alg�lad���nda 1 aksi durumda 0 verir.
  new_enumerated = usb_enumerated(); //usb_enumerated() fonksiyonu PIC host taraf�ndan listelendi�inde(enumeration) 1 aksi halde 0 verir.

  /*

  Yeni listeleme oldu�unda k�rm�z� led yanar.  
  Ard�ndan tekrar s�ner ve ye�il led yanar.
  */
  if (new_enumerated) {
    LED_ON(LED_RED);
    LED_OFF(LED_GREEN);
  } else {
    LED_OFF(LED_RED);
    LED_ON(LED_GREEN);
  }
  //E�er yeni ba�lant� alg�land�ysa ve daha �nce alg�lanmad�ysa 

  if (new_connected && !last_connected) {
    // printf(lcd_putc,"\fUSB Ba�lant�s� kuruldu.");

  }
  if (!new_connected && last_connected) {
    // printf(lcd_putc,"\fUSB Ba�lant�s� Kesildi");

  }
  if (new_enumerated && !last_enumerated) {
    // printf(lcd_putc,"\fUSB aygit y�neticisinde eklendi");

  }
  if (!new_enumerated && last_enumerated) {
    // printf(lcd_putc,"\fUSB ba�lant�s� alg�land�\nAyg�t y�neticisinde listelenmesi bekleniyor.");

  }
  last_connected = new_connected;
  last_enumerated = new_enumerated;
}

void main(void) {
  //PC'den gelecek ve PC'ye g�nderilecek verileri tutacak diziler tan�mland�
  char out_data[USB_CONFIG_HID_TX_SIZE] = "Buttona Basildi";
  char in_data[USB_CONFIG_HID_RX_SIZE];
  //lcd fonksiyonlar� ba�lat�ld�.
  //lcd ekran� temizlendi.
  //USB HID Basladi mesaj� yazd�r�ld�.
  lcd_init();
  lcd_putc('\f');
  lcd_putc("USB HID Basladi");

  //usb �evresel birimi ve usb stack'� ba�lat�r .
  //Bu fonksiyon usb interruptlar�n� aktif etmez ve do�rudan veri yolunu usb yoluna ba�lamaz.
  //Bu fonksiyona laternatif olarak usb_init fonksiyonu kullan�labilinir. 
  //Bu fonksiyon usb �evresel birimi ve usb stack'� ba�lat�r.interruptlar� aktif edip otomatik olarak usb veri yolu ba�lant�s�n� ger�ekle�tirir.
  //Bu fonksiyonu kullanmamam�z�n sebebi bu fonksiyonun usb ba�lant�s� ger�ekle�ene kadar sonsuz d�ng�de beklemesidir. 
  //Oysa biz usbnin ba�l� olmad��� durumuda alg�lamak istiyoruz.
  //USB ba�l� olmad��� durumda da periyodik olarak usb task fonksiyonunu �a��rark usb ba�lant�s� oldu�unda interruptlar� aktif edip,usb veri veri yolu ba�lant�s�n� kurabiliriz.
  usb_init_cs();
  //in_data dizisinin t�m elemanlar�na 0x00 de�eri atand�
  memset(in_data, 0x00, USB_CONFIG_HID_RX_SIZE);

  while (TRUE) {
    /*
    usb_task fonksiyonu �a��r�ld���nda usb alg�lama pinini kontrol eder. 
    E�er usb ba�lant�s� alg�lan�r ve usb �evresel birimi PC'ye ba�l� de�ilse
    bu fonksiyon USB �evresel birimini PC'ye ba�lar ve USB interruptlar�n� aktif eder. Bundan sonra 
    PC enumeration i�lemini ba�lat�r.E�er usb alg�lama pini tan�mlanmam��sa usb_task fonksiyonu s�rekli olarak usb ba�l� olarak alg�lar.
    Bu durumda usb_enumerated() fonksiyonu 0 de�eri d�nd�r�r. 
    Bu fonksiyon USB �evresel birimleri �al���rken USB ba�lant�s� olmad���n� tespit ederse USB �evresel birimini resetler. 
    */

    usb_task();

    usb_debug_task();

    //E�er PIC PC taraf�ndan konfig�re edilip, ayg�t listesine eklenmi�se       
    if (usb_enumerated()) {
      //E�er pin-b5'e ba�l� buttona bas�ld� ise 
      if (BUTTON_PRESSED()) {

        while (BUTTON_PRESSED());
        //usb hid ile out_data dizisinin USB_CONFIG_HID_TX_SIZE bytel�k elemanlar� PCye g�nderilir.
        usb_put_packet(1, out_data, USB_CONFIG_HID_TX_SIZE, USB_DTS_TOGGLE);
        delay_ms(200);
      }
      //usb_kbhit(1) fonksiyonu RX buffer�nda yeni veri varsa true yoksa false de�erini d�nd�r�r.
      if (usb_kbhit(1)) {
        //Rx bufferdan al�nan veri in_data dizisine atan�r.
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
