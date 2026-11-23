# F++ Programlama Dili

C ile sıfırdan yazılmış, kendi yorumlayıcısına (`fpp`), sıfırdan **x86-64 makine kodu (.exe)** üreten derleyicisine (`fppc`) ve terminal editörüne (`fppedit`) sahip eğlenceli ve Türkçe odaklı programlama dili.

---

## Kurulum (Her Bilgisayarda)

**Gereksinim:** Sadece Windows (hiçbir C derleyicisi veya ek araç gerekmez)

```bat
git clone https://github.com/atlas-toby/fpp.git
cd fpp
setup.bat
```

`setup.bat` şunları yapar:
1. `fpp.exe` (yorumlayıcı), `fppc.exe` (x86-64 native derleyici) ve `fppedit.exe` (editör) dosyalarını derler.
2. `%LOCALAPPDATA%\fpp` klasörüne kurar ve kullanıcı `PATH`'ine ekler.
3. Artık herhangi bir klasörde `fpp dosya.fpp` veya `fppc dosya.fpp cikti.exe` yazarak çalıştırabilirsin.

---

## F++ Tam Söz Dizimi (Syntax)

### 1. Yorum Satırları
```fpp
# Bu bir tek satırlık yorumdur
// Bu da bir yorum satırıdır
```

### 2. Değişkenler ve Veri Tipleri
```fpp
NUMARA sayi = 42;                                   # Tam sayı veya ondalıklı sayı (double)
NUMARA ondalik = 1.1;                               # Ondalıklı sayı
NUMARA dev = 999999999999999999999999999999;        # 15 haneden büyük tam sayılar (BIG_INTEGER)

doğruMu aktifMi = aynen;                            # aynen = true
doğruMu bittiMi = yok;                              # yok = false

culuk mesaj = "Merhaba F++";                        # String (metin)
cul harf = 'F';                                     # Char (tek karakter)
```

### 3. Ekrana Yazdırma ve Girdi Alma
```fpp
yaz("Merhaba Dunya!");                              # Ekrana metin yazar
yaz(sayi);                                          # Değişkenin değerini yazar

NUMARA yas = aradur();                              # Klavyeden sayı okur (Scanner gibi)
culuk isim = aradur();                              # Klavyeden metin okur
cul secim = aradur();                               # Klavyeden tek karakter okur
```

### 4. Koşullar (`eğer`, `yada_böyleyse`, `değilse`)
Karşılaştırma operatörleri: `==`, `!=`, `<`, `>`, `<=`, `>=`

```fpp
NUMARA puan = 75;

eğer (puan >= 80) {                                 # if
    yaz("Harika!");
} yada_böyleyse (puan >= 50) {                      # else if
    yaz("Idare eder!");
} değilse() {                                       # else (parantezsiz de yazılabilir: değilse {)
    yaz("Kaldin!");
}
```

### 5. Döngü (`while`)
```fpp
NUMARA i = 1;
döngü_icerisine_sok_bilader_ne_yapalım (i <= 5) {   # Koşul doğru olduğu sürece döner
    yaz(i);
    i = i + 1;
}
```

### 6. Fonksiyon Tanımlama ve Çağırma
```fpp
bak_fonksiyon selamla() {                           # Fonksiyon tanımlar
    yaz("Selamlar!");
}

selamla();                                          # Fonksiyonu çağırır
```

### 7. F++ Özel Komutları
```fpp
NUMARA r = sal(1, 100);                             # 1 ile 100 arasında rastgele sayı üretir
dur_kardeşim(1000);                                 # Programı 1000 ms (1 saniye) bekletir
temizle();                                          # Konsol ekranını temizler

yrenk(Renk.kirmizi);                                # Yazı rengini değiştirir
yaz("Kirmizi yazi!");
yrenk(Renk.sifirla);                                # Rengi varsayılana döndürür
# Mevcut renkler: Renk.kirmizi, Renk.yesil, Renk.mavi, Renk.sari, Renk.mor, Renk.turkuaz, Renk.beyaz, Renk.sifirla

uyari("Dikkat", "Bu bir uyari penceresidir!");      # Ekrana Windows uyarı penceresi çıkarır
foto("resim.png");                                  # Verilen fotoğrafı ekranın ortasında açar
dosyaGezginiRiskAltında();                          # Masaüstüne dosya.txt açar ve "portal çıkmış mı" yazar
hak_team();                                         # Konsola sahte hack mesajları yazar + aynı klasördeki sarki.mp3'ü çalar
```

---

## Çalıştırma ve Derleme

- **Yorumlayıcı ile çalıştırma:**
  ```bat
  fpp test.fpp
  ```
- **Sıfırdan Native x86-64 `.exe` derleme (C derleyicisi olmadan):**
  ```bat
  fppc merhaba.fpp merhaba.exe
  merhaba.exe
  ```
- **Dahili Terminal Editörü:**
  ```bat
  fppedit test.fpp
  ```
