# F++ v0.4 Kontrol Akisi ve Dongu Testi
NUMARA puan = sal(1, 100);
yaz("Puan:");
yaz(puan);

eğer (puan >= 80) {
    yrenk(Renk.yesil);
    yaz("Harika puan!");
} yada_böyleyse (puan >= 50) {
    yrenk(Renk.sari);
    yaz("Idare eder!");
} değilse() {
    yrenk(Renk.kirmizi);
    yaz("Kaldin bilader!");
}
yrenk(Renk.sifirla);

NUMARA sayac = 1;
döngü_icerisine_sok_bilader_ne_yapalım (sayac <= 5) {
    yaz(sayac);
    sayac = sayac + 1;
    dur_kardeşim(150);
}

yaz("Dongu bitti!");
