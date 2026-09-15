# C tilida `stdio.h`: Standart Kirish/Chiqish Kutubxonasi — To'liq Darslik

## 1. Kirish: `stdio.h` nima va nega kerak?

`stdio.h` — "Standard Input/Output" — C tilining eng ko'p ishlatiladigan standart kutubxonalaridan biri. U dasturingiz bilan "tashqi dunyo" (klaviatura, ekran, fayllar, boshqa dasturlar) o'rtasida ma'lumot almashishni ta'minlaydi.

Siz allaqachon `printf()` funksiyasini chuqur o'rgangansiz. Endi savol shu: `printf()` aslida *qayerga* yozadi? Va uning "opa-singlisi" bo'lgan `scanf()` *qayerdan* o'qiydi? Javob — **oqimlar (streams)** tushunchasida yashiringan. Shu darslikda biz stdio.h ning butun arxitekturasini — oqimlardan tortib, fayllar bilan ishlashgacha — tushunamiz.

### Nega bu muhim?

- Har qanday real dastur — foydalanuvchidan ma'lumot oladi, natijani chiqaradi, fayllarni o'qiydi/yozadi.
- `stdio.h` ning ichki mexanizmini (bufferlash, oqimlar) tushunmasdan, dasturingizdagi g'alati xatolarni (masalan, `scanf` va `getchar` aralashib ketishi) hech qachon to'liq tushunolmaysiz.
- Fayl operatsiyalari (`fopen`, `fread`, `fwrite`) — ma'lumotlar bazasi, log tizimlari, konfiguratsiya fayllari kabi deyarli har qanday amaliy dasturning asosi.

---

## 2. Standart Oqimlar: `stdin`, `stdout`, `stderr`

### 2.1. Oqim (stream) tushunchasi

C tilida I/O (Input/Output) **oqim** abstraktsiyasi orqali ishlaydi. Oqim — bu baytlar ketma-ketligi bo'lib, uning manbai yoki manzili klaviatura, ekran, fayl yoki hatto tarmoq bo'lishi mumkin — dastur uchun bularning farqi yo'q, chunki hammasi bir xil interfeys (`FILE*`) orqali ishlaydi.

Bu **abstraksiya** — dasturchi sifatida siz "fayldan o'qiyapmanmi yoki klaviaturadanmi" haqida deyarli bir xil kodni yozasiz.

```c
#include <stdio.h>

typedef struct _IO_FILE FILE;  // haqiqiy ta'rif kutubxonaga bog'liq (glibc, musl va h.k.)
```

`FILE` — bu struct bo'lib, ichida:
- fayl deskriptori (operatsion tizim darajasidagi identifikator),
- buffer (vaqtinchalik xotira),
- joriy pozitsiya (fayl ichida qayerda turganimiz),
- xato/fayl-oxiri bayroqlari

kabi ma'lumotlarni saqlaydi. Siz `FILE` strukturasining ichiga bevosita kira olmaysiz (u "qora quti") — faqat `stdio.h` funksiyalari orqali ishlaysiz.

### 2.2. Uchta standart oqim

Har bir C dasturi ishga tushganda, operatsion tizim avtomatik ravishda 3 ta oqimni ochib beradi:

| Oqim | Fayl deskriptori | Ma'nosi | Odatiy manzil |
|---|---|---|---|
| `stdin` | 0 | Standart kirish | Klaviatura |
| `stdout` | 1 | Standart chiqish | Terminal ekrani |
| `stderr` | 2 | Standart xato chiqishi | Terminal ekrani |

```c
#include <stdio.h>

int main(void) {
    fprintf(stdout, "Bu oddiy xabar\n");   // printf("...") bilan bir xil
    fprintf(stderr, "Bu xato xabari\n");   // xatolarni alohida oqimga yozish
    return 0;
}
```

**Muhim savol: nega `stdout` va `stderr` ikkalasi ham ekranga chiqadi-yu, lekin alohida oqim?**

Chunki ular **mustaqil ravishda qayta yo'naltirilishi (redirect)** mumkin. Bu — Unix falsafasining markaziy g'oyasi: "oddiy natija" va "xato xabari"ni ajratish, shunda ular alohida fayllarga yoki quvurlarga (pipe) yo'naltirilishi mumkin.

```bash
./dastur > natija.txt        # faqat stdout faylga yoziladi, xatolar ekranda qoladi
./dastur 2> xatolar.txt      # faqat stderr faylga yoziladi
./dastur > natija.txt 2>&1   # ikkalasi ham bitta faylga
```

**Amaliy qoida**: foydalanuvchiga mo'ljallangan asosiy natijalarni `stdout` ga, diagnostika/xato xabarlarini `stderr` ga yozing. Bu skript va quvurlarda ishlaydigan dasturlar uchun juda muhim.

### 2.3. `FILE*` — hamma narsaning markazi

Diqqat qiling: `stdin`, `stdout`, `stderr` — bularning barchasi `FILE*` turidagi o'zgaruvchilar (aniqrog'i, makrolar sifatida e'lon qilingan bo'lishi mumkin, lekin `FILE*` ga ishora qiladi). Shuning uchun, siz `fopen()` bilan ochgan istalgan faylni ham xuddi shu funksiyalar (`fprintf`, `fscanf`, `fgets`...) bilan boshqarasiz — chunki ular hammasi `FILE*` qabul qiladi.

```c
int fprintf(FILE *stream, const char *format, ...);
int fscanf(FILE *stream, const char *format, ...);
```

`printf(...)` aslida ichki tomondan `fprintf(stdout, ...)` ga teng.

---

## 3. Bufferlash (Buffering): Nega I/O sekin va uni qanday tezlashtiramiz

### 3.1. Nega bufferlash kerak?

Operatsion tizim darajasidagi I/O chaqiruvlari (`write()`, `read()` syscall'lari) **qimmat** — chunki ular dasturingizni "user mode"dan "kernel mode"ga o'tkazadi, bu kontekst almashinuvi (context switch) vaqt oladi.

Agar siz har bir belgi uchun alohida syscall qilsangiz (masalan, million marta `putchar()` chaqirib, har safar to'g'ridan-to'g'ri ekranga yozsangiz), dasturingiz juda sekinlashadi.

**Yechim**: `stdio.h` ma'lumotlarni avval **buffer** (vaqtinchalik xotira massivi) ga yig'adi, va faqat buffer to'lganda (yoki boshqa shartlarda) haqiqiy syscall orqali operatsion tizimga yuboradi.

### 3.2. Uchta bufferlash rejimi

| Rejim | Nomi | Qachon bo'shatiladi | Odatda qaerda ishlatiladi |
|---|---|---|---|
| `_IOFBF` | To'liq bufferlangan (fully buffered) | Buffer to'lganda | Fayllar |
| `_IOLBF` | Qator bo'yicha bufferlangan (line buffered) | `\n` uchraganda | Terminal (interaktiv) |
| `_IONBF` | Bufersiz (unbuffered) | Darhol, har chaqiruvda | `stderr` (odatiy holatda) |

Standart holatda:
- `stdout` — agar terminalga ulangan bo'lsa **line buffered**, agar faylga/quvurga yo'naltirilgan bo'lsa **fully buffered**.
- `stderr` — odatda **unbuffered** (yoki ba'zi tizimlarda line buffered) — chunki xato xabarlari darhol ko'rinishi kerak, hatto dastur keyin qulasa ham.
- Fayllar (`fopen` orqali) — odatda **fully buffered**.

### 3.3. Buferni qo'lda boshqarish

```c
#include <stdio.h>

int setvbuf(FILE *stream, char *buf, int mode, size_t size);
void setbuf(FILE *stream, char *buf);   // eskiroq, soddaroq versiya
```

Misol — buferni o'chirib qo'yish (har bir yozuv darhol chiqadi):

```c
setvbuf(stdout, NULL, _IONBF, 0);
```

O'zingizning buferingizni berish:

```c
char mybuf[1024];
setvbuf(stdout, mybuf, _IOFBF, sizeof(mybuf));
```

**Eslatma**: `setvbuf` chaqiruvi oqim ochilgandan keyin, lekin undan hech narsa o'qilishi/yozilishidan **oldin** qilinishi kerak.

### 3.4. `fflush()` — buferni majburan bo'shatish

```c
int fflush(FILE *stream);
```

Bu funksiya buferda "kutib turgan" ma'lumotlarni darhol haqiqiy manzilga (fayl/terminal) yuboradi.

```c
printf("Yuklanmoqda");
fflush(stdout);   // agar stdout to'liq bufferlangan bo'lsa (masalan, faylga yo'naltirilgan),
                   // "Yuklanmoqda" so'zi buferda qolib ketishi mumkin edi
sleep(2);
printf("...tayyor!\n");
```

**Muhim**: `fflush(stdin)` — bu **aniqlanmagan xatti-harakat (undefined behavior)**! Ko'pchilik boshlang'ich dasturchilar `scanf()` dan keyin qolgan `\n` ni tozalash uchun `fflush(stdin)` ishlatishga urinadi — bu **noto'g'ri** va portativ emas. Buning o'rniga quyidagi kabi usul ishlatiladi:

```c
int c;
while ((c = getchar()) != '\n' && c != EOF) {
    // stdin buferidagi qolgan belgilarni "yeb tashlaymiz"
}
```

### 3.5. Bufferlashning amaliy ta'siri: unumdorlik

Bufferlash amaliyotda katta farq qiladi. Masalan, 1 million qatorni faylga yozishda:

- Har bir `fputc()` chaqiruvida darhol `fflush` qilish: juda sekin (millionlab syscall).
- Standart bufferlash bilan: tezkor (buferga yig'ilib, kamdan-kam syscall).

Bu — "amortizatsiyalangan murakkablik" tushunchasining amaliy misoli: har bir yozuv operatsiyasi o'rtacha $O(1)$ vaqt oladi, chunki qimmat syscall xarajati ko'p yozuvlar orasida "taqsimlanadi" (amortized). Agar buferni o'chirsangiz, har bir yozuv operatsiyasi haqiqiy syscall xarajatini to'liq to'laydi — bu amaliyotda 10-100 barobar sekinlashishga olib kelishi mumkin.

---

## 4. Belgi Asosidagi Kirish/Chiqish (Character I/O)

### 4.1. `getchar()` va `putchar()`

```c
int getchar(void);          // stdin dan bitta belgi o'qiydi
int putchar(int c);         // stdout ga bitta belgi yozadi
```

**Nega qaytish turi `int`, `char` emas?**

Bu — muhim dizayn qarori. `EOF` (fayl oxiri) maxsus qiymat bo'lib, odatda `-1` ga teng. Agar `getchar()` `char` qaytarsa va `char` imzosiz (unsigned) bo'lsa (ba'zi platformalarda shunday), u hech qachon `-1` qiymatini qabul qila olmaydi — shuning uchun `EOF` bilan haqiqiy belgini farqlab bo'lmas edi. `int` ishlatish orqali barcha mumkin bo'lgan `unsigned char` qiymatlari (0-255) + `EOF` (-1) bir-biridan ajratilishi kafolatlanadi.

```c
#include <stdio.h>

int main(void) {
    int c;
    while ((c = getchar()) != EOF) {
        putchar(c);   // kiritilgan har bir belgini qaytarib chiqaradi (echo)
    }
    return 0;
}
```

**Tez-tez uchraydigan xato**:

```c
char c;   // NOTO'G'RI - int bo'lishi kerak!
while ((c = getchar()) != EOF) { ... }
```

Agar `char` imzosiz platformada `getchar()` `EOF` (-1) qaytarsa, u `char` ga o'girilganda kutilmagan qiymatga aylanadi va tsikl hech qachon to'xtamasligi mumkin — bu klassik, ammo tez-tez uchraydigan bag'dir.

### 4.2. `getc()` va `putc()` — fayl versiyalari

```c
int getc(FILE *stream);
int putc(int c, FILE *stream);
```

Bular `getchar()`/`putchar()` bilan deyarli bir xil, faqat oqimni parametr sifatida qabul qiladi:

```c
getchar()   ekvivalenti   getc(stdin)
putchar(c)  ekvivalenti   putc(c, stdout)
```

**Farqi nima?** `getc`/`putc` odatda **makro** sifatida amalga oshirilishi mumkin (tezroq, lekin argumentni bir necha marta baholashi mumkin), `fgetc`/`fputc` esa har doim **haqiqiy funksiya** (bir marta baholaydi, xavfsizroq yon-ta'sirli ifodalar bilan). Amaliyotda ko'pchilik zamonaviy kutubxonalar (glibc) hammasini funksiya sifatida amalga oshiradi, lekin standart shuni kafolatlamaydi — shu sababli, agar argumentingizda yon ta'sir bo'lsa (masalan, `getc(f++)`), `fgetc` ishlatish xavfsizroq.

### 4.3. `ungetc()` — belgini "qaytarib qo'yish"

```c
int ungetc(int c, FILE *stream);
```

Bu funksiya bitta belgini oqim buferiga "qaytarib qo'yadi" — go'yo u hech o'qilmagandek. Bu **lookahead parsing** (oldinga qarab tahlil qilish) uchun juda foydali.

```c
#include <stdio.h>
#include <ctype.h>

int main(void) {
    int c = getchar();
    if (isdigit(c)) {
        ungetc(c, stdin);   // raqamni qaytarib qo'yamiz
        int son;
        scanf("%d", &son);  // endi scanf uni to'liq raqam sifatida o'qiydi
        printf("Son: %d\n", son);
    }
    return 0;
}
```

**Cheklov**: bir vaqtning o'zida faqat **bitta** belgini qaytarib qo'yish kafolatlangan (ba'zi amalga oshirishlar ko'proq qo'llab-quvvatlaydi, lekin standart faqat bittani kafolatlaydi).

---

## 5. Qator Asosidagi Kirish/Chiqish (Line I/O)

### 5.1. `gets()` — ISHLATMANG!

Tarixiy jihatdan `gets()` funksiyasi mavjud edi, lekin u **C11 standartidan olib tashlangan**, chunki u buffer chegarasini tekshirmaydi va **buffer overflow** zaifligining eng mashhur manbalaridan biri edi (masalan, 1988-yildagi mashhur "Morris worm" qurti aynan shu funksiyadan foydalangan).

```c
char buf[10];
gets(buf);   // XATO: kiritilgan matn 10 belgidan uzun bo'lsa - xotira buzilishi!
```

**Hech qachon `gets()` ishlatmang.** Uning o'rnini `fgets()` bosadi.

### 5.2. `fgets()` — xavfsiz qator o'qish

```c
char *fgets(char *str, int n, FILE *stream);
```

- Maksimal `n-1` belgi o'qiydi (oxiriga `\0` qo'yish uchun joy qoldiradi).
- Agar `\n` uchrasa, uni **ham buferga qo'shadi** (bu `gets()` dan farqi).
- Fayl oxiriga yetsa yoki xato bo'lsa `NULL` qaytaradi.

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    char buf[100];
    printf("Ismingizni kiriting: ");
    if (fgets(buf, sizeof(buf), stdin) != NULL) {
        // \n ni olib tashlash (agar mavjud bo'lsa)
        size_t len = strlen(buf);
        if (len > 0 && buf[len - 1] == '\n') {
            buf[len - 1] = '\0';
        }
        printf("Salom, %s!\n", buf);
    }
    return 0;
}
```

**Amaliy qoida**: matn qatorlarini o'qish uchun har doim `fgets()` dan foydalaning, `scanf("%s", ...)` dan emas — chunki `%s` ham buffer chegarasini nazorat qilmaydi (agar kenglik spetsifikatori berilmasa).

### 5.3. `puts()` va `fputs()`

```c
int puts(const char *str);              // qatorni chiqaradi + AVTOMATIK \n qo'shadi
int fputs(const char *str, FILE *stream); // qatorni chiqaradi, \n QO'SHMAYDI
```

```c
puts("Salom");            // "Salom\n" chiqadi
fputs("Salom", stdout);   // "Salom" chiqadi (yangi qatorsiz)
fputs("Salom\n", stdout); // agar \n kerak bo'lsa, o'zingiz qo'shishingiz kerak
```

Bu assimetriya (`gets` avtomatik `\n`ni olib tashlaydi, `puts` esa qo'shadi) ko'p yangi dasturchilarni chalg'itadi — shuning uchun buni alohida eslab qoling.

---

## 6. Formatlangan Kirish: `scanf` Oilasi

`printf()` bo'yicha to'liq darslikni allaqachon ko'rib chiqqansiz, shuning uchun bu yerda `scanf` oilasiga e'tibor qaratamiz.

### 6.1. Asosiy funksiyalar

```c
int scanf(const char *format, ...);                    // stdin dan
int fscanf(FILE *stream, const char *format, ...);      // istalgan oqimdan
int sscanf(const char *str, const char *format, ...);   // satrdan (fayl emas!)
```

Qaytish qiymati — **muvaffaqiyatli o'qilgan va tayinlangan elementlar soni** (yoki xato/EOF holida `EOF`). Bu qiymatni har doim tekshirish kerak:

```c
int yosh;
if (scanf("%d", &yosh) != 1) {
    fprintf(stderr, "Noto'g'ri kiritish!\n");
}
```

### 6.2. Format spetsifikatorlari (qisqacha)

| Spetsifikator | Turi |
|---|---|
| `%d` | `int` |
| `%f` | `float` |
| `%lf` | `double` (scanf da `%f` emas, `%lf` kerak!) |
| `%c` | bitta belgi |
| `%s` | belgilar satri (bo'shliqqa qadar) |
| `%ld`, `%lld` | `long`, `long long` |
| `%x` | o'n oltilik (hex) |

**Muhim farq**: `printf` da `double` uchun `%f` ishlatiladi (chunki variadik funksiyalarda `float` avtomatik `double`ga ko'tariladi), lekin `scanf` da manzil (`&o'zgaruvchi`) beriladi, shu sababli aniq tur mos kelishi shart — `double` uchun `%lf`, `float` uchun `%f`.

### 6.3. `scanf` ning eng katta xavfi: `stdin` buferida qolgan `\n`

```c
int son;
char ism[50];

printf("Son kiriting: ");
scanf("%d", &son);          // foydalanuvchi "5\n" kiritadi, %d faqat "5" ni oladi,
                             // \n buferda QOLIB KETADI

printf("Ism kiriting: ");
fgets(ism, sizeof(ism), stdin);  // bu darhol buferdagi qolgan \n ni o'qib, bo'sh satr qaytaradi!
```

**Yechim** — `scanf` dan keyin qolgan belgilarni tozalash:

```c
int c;
while ((c = getchar()) != '\n' && c != EOF);
```

Yoki, umuman `scanf` o'rniga `fgets` + `sscanf`/`strtol` kombinatsiyasidan foydalanish — bu ko'proq nazorat va xavfsizlik beradi:

```c
char qator[100];
fgets(qator, sizeof(qator), stdin);
int son;
sscanf(qator, "%d", &son);
```

### 6.4. `%s` ning xavfi

```c
char ism[10];
scanf("%s", ism);   // agar foydalanuvchi 20 belgi kiritsa - buffer overflow!
```

Xavfsiz versiya — kenglikni cheklash:

```c
scanf("%9s", ism);   // maksimal 9 belgi + \0
```

---

## 7. Fayllar Bilan Ishlash

### 7.1. Faylni ochish: `fopen()`

```c
FILE *fopen(const char *filename, const char *mode);
```

| Rejim | Ma'nosi |
|---|---|
| `"r"` | O'qish uchun (fayl mavjud bo'lishi kerak) |
| `"w"` | Yozish uchun (mavjud bo'lsa — tozalanadi, bo'lmasa — yaratiladi) |
| `"a"` | Qo'shish uchun (append, oxiriga yoziladi) |
| `"r+"` | O'qish + yozish (fayl mavjud bo'lishi kerak) |
| `"w+"` | O'qish + yozish (tozalab yaratadi) |
| `"a+"` | O'qish + oxiriga qo'shish |
| `"rb"`, `"wb"`, ... | Yuqoridagilarning **binar** versiyalari |

```c
FILE *f = fopen("malumot.txt", "r");
if (f == NULL) {
    perror("Faylni ochib bo'lmadi");  // errno asosida tushunarli xabar chiqaradi
    return 1;
}
```

**Har doim `fopen()` natijasini tekshiring!** Fayl mavjud emasligi, ruxsat yo'qligi kabi sabablar bilan `NULL` qaytishi juda oddiy holat.

### 7.2. Matn rejimi vs Binar rejim: `"r"` vs `"rb"`

Bu farq ayniqsa Windows'da muhim: matn rejimida `\n` va `\r\n` orasidagi konvertatsiya avtomatik amalga oshiriladi. Unix/Linux tizimlarida bu ikkisi funksional jihatdan bir xil, lekin **portativ kod yozish uchun** har doim to'g'ri rejimni tanlash tavsiya etiladi:
- Matn fayllar (txt, csv, kod fayllari) — `"r"`/`"w"`.
- Binar fayllar (rasm, video, tuzilma dump'lari) — `"rb"`/`"wb"` — bu majburiy, aks holda ma'lumot buzilishi mumkin.

### 7.3. O'qish/yozish funksiyalari

**Formatlangan** (matn kabi):
```c
fprintf(f, "Yosh: %d\n", 25);
fscanf(f, "%d", &yosh);
```

**Qator asosida**:
```c
fgets(qator, sizeof(qator), f);
fputs("salom\n", f);
```

**Xom baytlar (binar)**:
```c
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
```

`fread`/`fwrite` **strukturalar yoki massivlarni to'g'ridan-to'g'ri xotiradan faylga** (yoki aksincha) ko'chirish uchun ishlatiladi — bu formatlash xarajatisiz eng tezkor usul.

```c
struct Talaba {
    char ism[50];
    int yosh;
    float baho;
};

struct Talaba t = {"Ali", 20, 4.5};

FILE *f = fopen("talabalar.dat", "wb");
fwrite(&t, sizeof(struct Talaba), 1, f);   // 1 ta strukturani yozadi
fclose(f);

// O'qish:
struct Talaba tOqilgan;
FILE *f2 = fopen("talabalar.dat", "rb");
fread(&tOqilgan, sizeof(struct Talaba), 1, f2);
fclose(f2);
```

**Muhim**: `fread`/`fwrite` ning qaytish qiymati — muvaffaqiyatli o'qilgan/yozilgan **elementlar soni**, baytlar soni emas! Bu qiymatni tekshirish orqali qisman yozuv/o'qishni aniqlash mumkin.

### 7.4. Fayl ichida harakatlanish: `fseek`, `ftell`, `rewind`

```c
int fseek(FILE *stream, long offset, int whence);
long ftell(FILE *stream);
void rewind(FILE *stream);
```

`whence` uchta qiymatdan biri bo'lishi mumkin:
- `SEEK_SET` — fayl boshidan
- `SEEK_CUR` — joriy pozitsiyadan
- `SEEK_END` — fayl oxiridan

```c
FILE *f = fopen("data.bin", "rb");

fseek(f, 0, SEEK_END);
long hajmi = ftell(f);     // fayl hajmini bilib olish trikki
rewind(f);                 // boshiga qaytish (fseek(f, 0, SEEK_SET) ga teng)

printf("Fayl hajmi: %ld bayt\n", hajmi);
```

Bu — faylning **o'lchamini aniqlashning** eng keng tarqalgan (ammo asosiy) usuli.

### 7.5. Faylni yopish: `fclose()`

```c
int fclose(FILE *stream);
```

**Nega bu muhim?** `fclose()` chaqirilganda:
1. Buferdagi barcha yozilmagan ma'lumotlar **darhol diskka yoziladi** (flush qilinadi).
2. Operatsion tizim resurslari (fayl deskriptori) bo'shatiladi.

Agar `fclose()` chaqirmasangiz:
- Kichik dasturlarda, dastur tugaganda OT baribir resurslarni tozalaydi — lekin buferdagi yozilmagan ma'lumot **yo'qolishi mumkin**!
- Katta/uzoq ishlaydigan dasturlarda — fayl deskriptorlari "sizib chiqadi" (file descriptor leak), va oxir-oqibat tizim limitiga yetib, yangi fayllarni ocholmay qolasiz.

```c
FILE *f = fopen("log.txt", "a");
if (f != NULL) {
    fprintf(f, "Voqea qayd etildi\n");
    fclose(f);   // ALBATTA yoping!
}
```

### 7.6. `feof()`, `ferror()`, `clearerr()`

```c
int feof(FILE *stream);     // fayl oxiriga yetildimi?
int ferror(FILE *stream);   // oqimda xato bayrog'i o'rnatilganmi?
void clearerr(FILE *stream); // EOF/xato bayroqlarini tozalash
```

**Klassik xato** — `feof()` ni tsikl shartida noto'g'ri ishlatish:

```c
// NOTO'G'RI namuna:
while (!feof(f)) {
    fscanf(f, "%d", &son);   // oxirgi muvaffaqiyatsiz o'qishda "son" eski qiymatda qoladi,
    printf("%d\n", son);     // lekin baribir chop etiladi - dublikat natija!
}
```

**To'g'ri namuna** — o'qish funksiyasining qaytish qiymatini tekshirish:

```c
while (fscanf(f, "%d", &son) == 1) {
    printf("%d\n", son);
}
```

Sabab: `feof()` faqat **o'qishga urinilgandan va muvaffaqiyatsiz bo'lgandan keyingina** `true` bo'ladi — ya'ni u "oldindan bilib" ishlamaydi, balki "orqaga qarab" tasdiqlaydi.

---

## 8. Xatoliklarni Boshqarish: `errno`, `perror`, `strerror`

```c
#include <errno.h>
#include <string.h>

extern int errno;   // global xato kodi o'zgaruvchisi
```

Ko'plab tizim darajasidagi funksiyalar (shu jumladan ba'zi stdio funksiyalari) xatolik yuz berganda `errno` ni o'rnatadi.

```c
FILE *f = fopen("mavjud_bolmagan_fayl.txt", "r");
if (f == NULL) {
    perror("Xato");                        
    // Chiqishi: "Xato: No such file or directory"
    
    printf("Xato kodi: %d, izoh: %s\n", errno, strerror(errno));
}
```

`perror(const char *s)` — `s` matnini, keyin ikki nuqta, keyin `errno` ga mos matnli tavsifni chop etadi. Bu — fayl xatolarini diagnostika qilishning eng qulay usuli.

---

## 9. Qayta Yo'naltirish: `freopen()`

```c
FILE *freopen(const char *filename, const char *mode, FILE *stream);
```

Bu funksiya mavjud oqimni (masalan, `stdout`) yangi faylga "qayta bog'laydi" — dasturingiz ichida.

```c
freopen("output.txt", "w", stdout);
printf("Bu endi faylga yoziladi!\n");   // ekranga emas!
```

Bu, ayniqsa, dasturni o'zgartirmasdan uning barcha chiqishlarini faylga yozib olish kerak bo'lganda (masalan, log fayllari uchun) foydali.

---

## 10. Unumdorlik va Murakkablik Tahlili

`stdio.h` funksiyalari algoritmik ma'noda "murakkablik" tushunchasiga ega emas (chunki ular ma'lumotlar tuzilmasi emas), lekin amaliy unumdorlik nuqtai nazaridan quyidagilarni bilish muhim:

| Operatsiya | Vaqt xarajati |
|---|---|
| Bufer ichidagi `getchar`/`putchar` | $O(1)$ amortizatsiyalangan |
| `fread`/`fwrite` (katta bloklar) | $O(n)$, lekin kichik doimiy koeffitsient bilan (tez) |
| Har bir belgi uchun alohida syscall (bufer o'chirilgan) | $O(n)$, lekin juda katta doimiy koeffitsient (sekin) |
| `fseek` (tasodifiy kirish) | Odatda $O(1)$ (diskda jismoniy joylashuvga bog'liq) |
| `fscanf`/`sscanf` (format tahlili) | $O(m)$, bu yerda $m$ — format satrining murakkabligi |

**Amaliy xulosa**: katta fayllarni qayta ishlashda, `fread`/`fwrite` ni katta bloklar (masalan, 4KB yoki undan katta) bilan ishlatish, har bir bayt uchun alohida chaqiruv qilishdan sezilarli tezroq.

```c
// SEKIN: har bir bayt uchun alohida chaqiruv
int c;
while ((c = fgetc(f)) != EOF) { /* ... */ }

// TEZROQ: katta bloklar bilan o'qish
char buffer[4096];
size_t o'qilgan;
while ((o'qilgan = fread(buffer, 1, sizeof(buffer), f)) > 0) {
    /* buffer[0..o'qilgan-1] ustida ishlash */
}
```

---

## 11. Boshqa Tillar Bilan Qisqacha Taqqoslash

`stdio.h` — C ga xos, lekin xuddi shunday vazifani boshqa tillar ham o'z usullarida bajaradi:

**C++ (`<iostream>`)** — obyektga yo'naltirilgan, operator ortiqcha yuklash (`<<`, `>>`) orqali:
```cpp
#include <iostream>
std::cout << "Salom\n";
std::cin >> son;
```
C++ da ham `stdio.h` ni ishlatish mumkin (mos kelish uchun), lekin `iostream` tur-xavfsiz (type-safe) — format spetsifikatorlari kerak emas.

**Python** — yanada yuqori darajali, fayl ob'ektlari orqali:
```python
with open("fayl.txt", "r") as f:
    matn = f.read()
print("Salom")
```
Python'da bufer boshqaruvi va fayl yopilishi avtomatik (`with` bloklari orqali), lekin tagida xuddi shunday operatsion tizim darajasidagi I/O mexanizmi ishlaydi.

**Asosiy g'oya**: barcha tillarda "oqim" (stream) abstraksiyasi, "bufer" tushunchasi va "ochish/yopish" jarayoni deyarli bir xil — chunki ular hammasi operatsion tizimning bir xil pastki darajadagi syscall'lariga (`open`, `read`, `write`, `close`) tayanadi. C tilida stdio.h ni o'rganish orqali siz aslida boshqa tillardagi I/O tizimlarining ham asosiy mantig'ini tushunib olasiz.

---

## 12. To'liq Amaliy Misol: Fayldan O'qish va Statistika Chiqarish

```c
#include <stdio.h>
#include <ctype.h>

int main(void) {
    FILE *f = fopen("matn.txt", "r");
    if (f == NULL) {
        perror("Faylni ochishda xatolik");
        return 1;
    }

    long qatorlar = 0, sozlar = 0, belgilar = 0;
    int c, oldingi = '\n';

    while ((c = fgetc(f)) != EOF) {
        belgilar++;
        if (c == '\n') {
            qatorlar++;
        }
        // yangi so'z boshlanishini aniqlash: bo'shliqdan keyin bo'shliq-bo'lmagan belgi
        if (!isspace(c) && isspace(oldingi)) {
            sozlar++;
        }
        oldingi = c;
    }

    if (ferror(f)) {
        fprintf(stderr, "O'qishda xatolik yuz berdi\n");
        fclose(f);
        return 1;
    }

    fclose(f);

    printf("Qatorlar: %ld\n", qatorlar);
    printf("So'zlar:  %ld\n", sozlar);
    printf("Belgilar: %ld\n", belgilar);

    return 0;
}
```

Bu misol quyidagilarni birlashtiradi: `fopen`, xatoni tekshirish, `fgetc` bilan tsikl, `ferror`, `fclose` — ya'ni ushbu darslikda ko'rgan deyarli barcha muhim tushunchalarni.

---

## 13. Eng Ko'p Uchraydigan Xatolar — Xulosa Jadvali

| Xato | Nega yomon | To'g'ri yechim |
|---|---|---|
| `gets()` ishlatish | Buffer overflow | `fgets()` |
| `scanf("%s", ...)` kenglik cheklovsiz | Buffer overflow | `scanf("%9s", ...)` yoki `fgets` |
| `char c = getchar()` | `EOF` bilan xato | `int c = getchar()` |
| `while(!feof(f))` | Oxirgi qatorni ikki marta qayta ishlash | O'qish funksiyasining qaytish qiymatini tekshirish |
| `fflush(stdin)` | Aniqlanmagan xatti-harakat | `getchar()` tsikli bilan buferni tozalash |
| `fopen()` natijasini tekshirmaslik | `NULL` ko'rsatgich bilan ishlash - crash | Har doim `NULL` ni tekshirish |
| `fclose()` chaqirmaslik | Ma'lumot yo'qolishi, resurs sizib chiqishi | Har doim yopish (yoki `fclose` avtomatik chaqiriladigan patternlar) |

---

## 14. Keyingi Qadamlar

Endi siz `stdio.h` ning to'liq arxitekturasini — oqimlar, bufferlash, belgi/qator/formatlangan I/O, fayllar bilan ishlash va xatolarni boshqarishni bilasiz. Keyingi tabiiy qadamlar:

1. **Dinamik xotira bilan fayl o'qish** — fayl hajmini oldindan bilmasdan, `malloc`/`realloc` yordamida moslashuvchan buferlar yaratish.
2. **POSIX past darajadagi I/O** (`open`, `read`, `write`, `close`) — `stdio.h` ning "orqasida" nima yotishini ko'rish.
3. **Ko'p faylli dasturlar** — bir vaqtning o'zida bir nechta faylni ochib, ular orasida ma'lumot ko'chirish.
4. **Binar fayl formatlari** — o'z struktura-asosli fayl formatingizni yaratish (`fread`/`fwrite` chuqurroq qo'llanilishi).

Agar xohlasangiz, keyingi darslikda ana shu mavzulardan birini chuqur ko'rib chiqishimiz mumkin.
