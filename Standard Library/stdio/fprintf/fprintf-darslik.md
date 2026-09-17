# `fprintf()` funksiyasi — to'liq darslik

## 1. Kirish: `fprintf()` nima va u qayerda joylashadi

`stdio.h` kutubxonasidagi formatlangan chiqish (formatted output) funksiyalari bir oilaga mansub: `printf`, `fprintf`, `sprintf`, `snprintf`. Ularning barchasi bitta umumiy "dvigatel"ga asoslangan — format string va variadic argumentlarni tahlil qilib, natijani qandaydir manzilga yozadi. Farq faqat **qayerga yozishda**:

| Funksiya | Qayerga yozadi |
|---|---|
| `printf` | `stdout` (standart chiqish) |
| `fprintf` | **istalgan `FILE*` stream** — fayl, `stdout`, `stderr` |
| `sprintf` | xotiradagi `char` massiviga (string) |
| `snprintf` | xotiradagi `char` massiviga, hajmi cheklangan holda |

Demak, `printf(...)` aslida ichki jihatdan `fprintf(stdout, ...)` bilan deyarli bir xil ishlaydi. `fprintf` — bu oilaning eng "umumiy" (generic) a'zosi: unga qaysi stream'ga yozish kerakligini o'zingiz ko'rsatasiz. Shu sababli `fprintf`ni tushunish — fayllarga yozish, xato xabarlarini `stderr`ga yo'naltirish va log tizimlarini qurish uchun kalit hisoblanadi.

## 2. Prototip va sintaksis

```c
#include <stdio.h>

int fprintf(FILE *stream, const char *format, ...);
```

Qismlarni ajratib ko'ramiz:

- **`FILE *stream`** — yozish kerak bo'lgan oqim (stream) ko'rsatkichi. Bu `stdout`, `stderr` yoki `fopen()` orqali ochilgan fayl bo'lishi mumkin.
- **`const char *format`** — format string: oddiy matn + format spesifikatorlari (`%d`, `%s`, `%f`, va h.k.).
- **`...`** — variadic argumentlar: format string ichidagi har bir `%`-spesifikatorga mos keluvchi qiymat.
- **Qaytish turi `int`** — funksiya muvaffaqiyatli yozilgan **belgilar (characters) sonini** qaytaradi, xato bo'lsa **manfiy qiymat**.

### `FILE*` nima o'zi?

`FILE` — `stdio.h` ichida e'lon qilingan struct (uning ichki tuzilishi kompilyatorga bog'liq va odatda dasturchiga "opaque", ya'ni yopiq). U stream haqidagi barcha metama'lumotlarni saqlaydi: fayl deskriptori, joriy pozitsiya, bufer holati, xato/EOF flaglari.

C dasturi boshlanganda avtomatik ravishda uchta stream ochiladi va ular global o'zgaruvchilar sifatida mavjud bo'ladi:

```c
extern FILE *stdin;   // standart kirish
extern FILE *stdout;  // standart chiqish
extern FILE *stderr;  // standart xato chiqishi
```

`fprintf`ning birinchi argumenti sifatida shu uchtasidan birini yoki `fopen()` orqali o'zingiz ochgan `FILE*`ni berishingiz mumkin.

## 3. Eng oddiy misollar

### 3.1. `stdout`ga yozish — `printf` bilan ekvivalent

```c
#include <stdio.h>

int main(void) {
    fprintf(stdout, "Salom, dunyo!\n");   // printf("Salom, dunyo!\n"); bilan bir xil
    printf("Bu esa oddiy printf\n");
    return 0;
}
```

### 3.2. `stderr`ga xato xabarini yozish

Bu `fprintf`ning eng ko'p uchraydigan real hayotiy ishlatilishi:

```c
#include <stdio.h>

int main(void) {
    int fayl_topildimi = 0;

    if (!fayl_topildimi) {
        fprintf(stderr, "Xato: fayl topilmadi.\n");
        return 1;
    }
    return 0;
}
```

**Nega aynan `stderr`?** `stdout` va `stderr` — ikkita mustaqil oqim. Terminalda ikkalasi ham ekranda ko'rinadi, lekin ular **alohida yo'naltirilishi (redirect)** mumkin:

```bash
./dastur > natija.txt        # faqat stdout faylga yoziladi
./dastur 2> xatolar.txt      # faqat stderr faylga yoziladi
./dastur > natija.txt 2>&1   # ikkalasi ham bitta faylga
```

Agar xato xabarini `printf` bilan yozsangiz, u `stdout`ga aralashib ketadi va foydalanuvchi (yoki skript) haqiqiy natijadan xatoni ajrata olmay qoladi. Shuning uchun **qoida**: oddiy natijalar — `stdout`ga, diagnostika/xato xabarlari — `stderr`ga.

Yana bir muhim farq: `stdout` odatda **bufer**langan (fayl yoki quvur — pipe'ga yo'naltirilganda to'liq buferlangan), `stderr` esa **buferlanmagan** (unbuffered) yoki chiziq bo'yicha buferlangan bo'ladi — shu sababli xato xabari darhol ko'rinadi, hatto dastur keyinroq qulasa ham.

### 3.3. Faylga yozish

```c
#include <stdio.h>

int main(void) {
    FILE *f = fopen("log.txt", "w");
    if (f == NULL) {
        fprintf(stderr, "Faylni ochib bo'lmadi.\n");
        return 1;
    }

    fprintf(f, "Dastur ishga tushdi.\n");
    fprintf(f, "Foydalanuvchi soni: %d\n", 42);
    fprintf(f, "O'rtacha ball: %.2f\n", 87.5);

    fclose(f);
    return 0;
}
```

Bu yerda `fprintf` xuddi `printf` kabi format spesifikatorlarini qo'llaydi, faqat natija ekranga emas, `f` ko'rsatayotgan faylga yoziladi.

## 4. Format spesifikatorlari — qisqacha eslatma

`fprintf`da qo'llaniladigan format spesifikatorlari `printf` bilan **bir xil** (chunki ular bir xil ichki mexanizmga — `vfprintf`ga asoslangan). Eng ko'p ishlatiladiganlari:

| Spesifikator | Turi | Misol |
|---|---|---|
| `%d` / `%i` | `int` (o'nlik, ishorali) | `fprintf(f, "%d", 42);` |
| `%u` | `unsigned int` | `fprintf(f, "%u", 42u);` |
| `%f` | `double` (o'nlik kasr) | `fprintf(f, "%.2f", 3.14159);` |
| `%e` | `double` (eksponensial) | `fprintf(f, "%e", 12345.6789);` |
| `%c` | `char` | `fprintf(f, "%c", 'A');` |
| `%s` | `char*` (string) | `fprintf(f, "%s", "salom");` |
| `%p` | ko'rsatkich manzili | `fprintf(f, "%p", (void*)&x);` |
| `%x` / `%X` | 16-lik son | `fprintf(f, "%x", 255);` |
| `%%` | literal `%` belgisi | `fprintf(f, "100%%");` |

Kenglik, aniqlik va bayroqlar (`%-10d`, `%05.2f`, `%+d` va h.k.) ham xuddi `printf`dagidek ishlaydi — bu mavzu allaqachon `printf` darsligida batafsil yoritilgan, shuning uchun bu yerda takrorlamaymiz.

## 5. Qaytish qiymati (return value)

`fprintf` `int` qaytaradi:

- **Muvaffaqiyatda** — stream'ga yozilgan **belgilar sonini** qaytaradi (format string kengaytirilgandan keyingi umumiy uzunlik).
- **Xatoda** — **manfiy qiymat** (odatda `EOF`, ya'ni `-1`) qaytaradi va `errno` mos ravishda o'rnatiladi.

```c
#include <stdio.h>

int main(void) {
    int n = fprintf(stdout, "Salom\n");
    printf("Yozilgan belgilar soni: %d\n", n);  // "Salom\n" — 6 ta belgi
    return 0;
}
```

Natija:
```
Salom
Yozilgan belgilar soni: 6
```

### Nega qaytish qiymatini tekshirish kerak?

Ko'pchilik dasturchilar `fprintf`ning qaytish qiymatini e'tiborsiz qoldiradi, lekin production darajasidagi kodda bu **muhim xato tekshiruvi** hisoblanadi — ayniqsa diskka yozayotganda:

```c
#include <stdio.h>
#include <errno.h>
#include <string.h>

int main(void) {
    FILE *f = fopen("/faqat_root_yoza_oladi/log.txt", "w");
    if (f == NULL) {
        fprintf(stderr, "fopen xatosi: %s\n", strerror(errno));
        return 1;
    }

    if (fprintf(f, "Muhim ma'lumot: %d\n", 12345) < 0) {
        fprintf(stderr, "Yozishda xato: %s\n", strerror(errno));
        fclose(f);
        return 1;
    }

    fclose(f);
    return 0;
}
```

Disk to'lib qolganda yoki fayl tizimi faqat-o'qish (read-only) rejimida bo'lganda, `fprintf` xato qaytarishi mumkin — buni tekshirmasangiz, dasturingiz "muvaffaqiyatli yozdim" deb o'ylab, aslida ma'lumot yo'qolishini payqamay qoladi.

## 6. `errno` va `ferror()` bilan chuqurroq xato tekshiruvi

`fprintf` xato qaytarganda, sabab haqida ko'proq ma'lumot olish uchun ikki vosita bor:

### 6.1. `errno`

`fprintf` muvaffaqiyatsiz bo'lganda, tizim darajasidagi `errno` global o'zgaruvchisi o'rnatiladi (masalan, `ENOSPC` — disk joyi yo'q, `EBADF` — noto'g'ri fayl deskriptori). `strerror(errno)` orqali inson o'qiy oladigan xato matnini olish mumkin.

### 6.2. `ferror()`

```c
#include <stdio.h>

int ferror(FILE *stream);
```

Bu funksiya stream'da oldin xato yuz berganmi-yo'qmi, uni tekshiradi (0 — xato yo'q, nolmas qiymat — xato bor). Xato flag'ini tozalash uchun `clearerr(stream)` ishlatiladi.

```c
if (fprintf(f, "ma'lumot\n") < 0 || ferror(f)) {
    fprintf(stderr, "Faylga yozishda muammo yuz berdi.\n");
}
```

## 7. `fprintf` va bufer — nozik jihat

`fprintf` yozgan ma'lumot **darhol diskka tushmaydi** — u avval `FILE*`ning ichki buferiga to'planadi va quyidagi hollarda diskka "tushiriladi" (flush qilinadi):

1. Bufer to'lganda (odatda fayllar uchun **to'liq buferlangan**, hajmi tizimga bog'liq, ko'pincha 4096 bayt).
2. `fflush(stream)` chaqirilganda — qo'lda majburlash.
3. `fclose(stream)` chaqirilganda — fayl yopilganda avtomatik flush bo'ladi.
4. Dastur normal tugaganda (`main`dan `return` yoki `exit()`), ochiq qolgan streamlar avtomatik flush va yopiladi.

```c
#include <stdio.h>
#include <unistd.h> // sleep() uchun

int main(void) {
    FILE *f = fopen("log.txt", "w");
    fprintf(f, "Ishlov boshlandi...\n");
    fflush(f);   // darhol diskka yozilishini kafolatlaydi

    sleep(5);    // shu payt faylni ochib ko'rsangiz, satr allaqachon ko'rinadi

    fprintf(f, "Ishlov tugadi.\n");
    fclose(f);   // bu yerda ham avtomatik flush bo'ladi
    return 0;
}
```

**Amaliy qoida:** uzoq ishlaydigan dasturlarda (masalan, server, daemon) log yozganda va dastur kutilmagan tarzda qulashi mumkin bo'lgan holatlarda, muhim yozuvlardan keyin `fflush()` chaqirish tavsiya etiladi — aks holda buferdagi ma'lumot qulash paytida yo'qolib ketishi mumkin.

## 8. `stdout` vs `stderr` bufer siyosati — solishtirish

| Stream | Terminalga chiqarilganda | Faylga/quvurga yo'naltirilganda |
|---|---|---|
| `stdout` | Qator bo'yicha buferlangan (line-buffered) — `\n`da flush | To'liq buferlangan (full-buffered) — bufer to'lganda flush |
| `stderr` | Buferlanmagan (unbuffered) — har doim darhol | Buferlanmagan yoki minimal buferlangan |

Bu farq amalda muhim natijaga olib keladi: agar dastur `printf` va `fprintf(stderr, ...)`ni aralashtirib ishlatsa va chiqish faylga yo'naltirilgan bo'lsa, **satrlar tartibi buzilishi mumkin** — chunki `stdout` kechroq, `stderr` esa darhol yoziladi. Shu sababli tartibni kafolatlash kerak bo'lgan joylarda `fflush(stdout)`ni `stderr`ga yozishdan oldin chaqirish yaxshi amaliyot hisoblanadi.

## 9. Amaliy misol: log funksiyasi yaratish

Real loyihalarda `fprintf` ko'pincha o'zining log yozish funksiyasi ichida ishlatiladi:

```c
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

void log_yoz(FILE *stream, const char *daraja, const char *format, ...) {
    time_t hozir = time(NULL);
    struct tm *vaqt = localtime(&hozir);

    // Vaqt tamg'asi
    fprintf(stream, "[%02d:%02d:%02d] [%s] ",
            vaqt->tm_hour, vaqt->tm_min, vaqt->tm_sec, daraja);

    // Foydalanuvchi format string va argumentlarini o'tkazish
    va_list args;
    va_start(args, format);
    vfprintf(stream, format, args);   // fprintf ning "va_list" versiyasi
    va_end(args);

    fprintf(stream, "\n");
    fflush(stream);
}

int main(void) {
    log_yoz(stdout, "INFO", "Dastur ishga tushdi, versiya %s", "1.2.0");
    log_yoz(stderr, "ERROR", "Faylni ochib bo'lmadi: %s", "config.json");
    return 0;
}
```

Natija (taxminan):
```
[14:23:07] [INFO] Dastur ishga tushdi, versiya 1.2.0
[14:23:07] [ERROR] Faylni ochib bo'lmadi: config.json
```

Bu yerda muhim nuqta: **o'zingizning variadic funksiyangiz ichida `fprintf`ni to'g'ridan-to'g'ri chaqira olmaysiz**, chunki sizda argumentlar allaqachon `va_list` ko'rinishida. Shu sababli `fprintf`ning `va_list` qabul qiluvchi "opa-singlisi" — `vfprintf` ishlatiladi. Bu — `fprintf` oilasining muhim qismi:

| Funksiya | `...` (variadic) | `va_list` |
|---|---|---|
| `stdout`ga | `printf` | `vprintf` |
| `FILE*`ga | `fprintf` | `vfprintf` |
| stringga | `sprintf` | `vsprintf` |
| stringga (xavfsiz) | `snprintf` | `vsnprintf` |

## 10. Amaliy misol: CSV fayl yaratish

```c
#include <stdio.h>

typedef struct {
    char ism[50];
    int yosh;
    double maosh;
} Xodim;

int main(void) {
    Xodim xodimlar[] = {
        {"Aziz", 28, 4500000.0},
        {"Malika", 34, 6200000.0},
        {"Bobur", 22, 3100000.0}
    };
    int soni = sizeof(xodimlar) / sizeof(xodimlar[0]);

    FILE *csv = fopen("xodimlar.csv", "w");
    if (csv == NULL) {
        fprintf(stderr, "CSV faylni yaratib bo'lmadi.\n");
        return 1;
    }

    fprintf(csv, "Ism,Yosh,Maosh\n");   // sarlavha qatori

    for (int i = 0; i < soni; i++) {
        fprintf(csv, "%s,%d,%.2f\n",
                xodimlar[i].ism, xodimlar[i].yosh, xodimlar[i].maosh);
    }

    fclose(csv);
    printf("%d ta yozuv xodimlar.csv fayliga yozildi.\n", soni);
    return 0;
}
```

Bu misol `fprintf`ning eng keng tarqalgan amaliy qo'llanilishlaridan birini ko'rsatadi — **strukturaviy ma'lumotlarni formatlangan matn faylga aylantirish**.

## 11. `fprintf` va format string xavfsizligi

`printf` darsligida keltirilgan **format string zaifligi** (format string vulnerability) `fprintf`ga ham to'liq tegishli, chunki ular bir xil mexanizmga asoslangan:

```c
// XAVFLI — foydalanuvchi kiritgan matnni to'g'ridan-to'g'ri format sifatida berish
char foydalanuvchi_matni[100];
fgets(foydalanuvchi_matni, sizeof(foydalanuvchi_matni), stdin);
fprintf(stdout, foydalanuvchi_matni);   // XATO! Agar matnda %s, %n bo'lsa — falokat

// XAVFSIZ — foydalanuvchi matnini argument sifatida berish
fprintf(stdout, "%s", foydalanuvchi_matni);
```

Agar foydalanuvchi kiritgan satrda `%s` yoki `%n` kabi spesifikatorlar bo'lsa-yu, siz uni to'g'ridan-to'g'ri format sifatida uzatsangiz, dastur mos argumentlarni stackdan "o'qishga" urinadi — bu xotira sizib chiqishi (information leak) yoki hatto xotirani yozib qo'yishga (`%n` orqali) olib kelishi mumkin. **Qoida: foydalanuvchidan kelgan matnni hech qachon format string sifatida ishlatmang — uni har doim `%s` argumenti sifatida bering.**

## 12. `fprintf` ning `printf` oilasidagi o'rni — umumlashtirish

```
         format string + variadic args
                    |
                    v
         +----------------------+
         |   ichki mexanizm     |
         |  (vfprintf asosida)  |
         +----------------------+
           /        |        \
          /         |         \
     stdout'ga   FILE*'ga    string'ga
         |            |            |
      printf()   fprintf()    sprintf()/snprintf()
```

Amalda ko'p C standart kutubxona implementatsiyalarida `printf(fmt, ...)` shunchaki `fprintf(stdout, fmt, ...)`ni chaqiradi, xolos — ya'ni `printf` `fprintf`ning **maxsus holati**.

## 13. Xulosa — asosiy qoidalar

1. `fprintf(stream, format, ...)` — `printf`ning umumlashtirilgan versiyasi; `stream` o'rniga `stdout`, `stderr` yoki `fopen()`dan olingan `FILE*` beriladi.
2. Oddiy natijalarni `stdout`ga, xato/diagnostika xabarlarini **har doim `stderr`ga** yozing.
3. Qaytish qiymati — yozilgan belgilar soni; xatoda manfiy qiymat qaytadi va buni tekshirish yaxshi amaliyot.
4. Fayllarga yozish buferlangan; muhim yozuvlardan keyin `fflush()` yoki `fclose()` orqali diskka majburlab tushirish mumkin/kerak.
5. `va_list` argumentlari bilan ishlaganda `fprintf` emas, `vfprintf` ishlatiladi.
6. Foydalanuvchi kiritgan matnni **hech qachon** to'g'ridan-to'g'ri format string sifatida bermang — bu xavfsizlik zaifligi.

## 14. Mashqlar

1. Fayl nomini va matnni argument sifatida oluvchi, faylni `"a"` (append) rejimida ochib, oxiriga sana-vaqt bilan birga yozuv qo'shuvchi `dnevnik_yoz()` funksiyasini yozing.
2. Yuqoridagi `xodimlar.csv` misolini kengaytirib, agar `fopen` yoki biror `fprintf` chaqiruvi xato qaytarsa, dastur mos xato xabari bilan `stderr`ga yozib, darhol to'xtaydigan qilib qayta yozing.
3. `vfprintf` yordamida, xato darajasiga qarab (`"DEBUG"`, `"INFO"`, `"ERROR"`) turli streamlarga (masalan, `DEBUG`/`INFO` — faylga, `ERROR` — ham faylga, ham `stderr`ga) yozadigan kengaytirilgan log funksiyasini loyihalang.
4. Format string zaifligini namoyish etuvchi kichik dastur yozing (faqat lokal, o'quv maqsadida) va uni `%s` argumenti bilan tuzatilgan versiyasi bilan solishtiring.
