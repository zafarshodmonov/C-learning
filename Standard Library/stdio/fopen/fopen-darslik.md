# `fopen()` funksiyasi — to'liq darslik

## 1. Kirish: fayllar bilan ishlash nega `fopen()`dan boshlanadi

`stdio.h` kutubxonasida faylga yozish (`fprintf`, `fwrite`) yoki fayldan o'qish (`fscanf`, `fread`) uchun avval o'sha faylni **stream** sifatida "ochish" kerak. Bu vazifani `fopen()` bajaradi — u operatsion tizimdan fayl bilan ishlash uchun ruxsat so'raydi va muvaffaqiyatli bo'lsa, keyingi barcha operatsiyalarda ishlatiladigan `FILE*` ko'rsatkichini qaytaradi.

`fopen()` — bu C dasturi bilan fayl tizimi (filesystem) o'rtasidagi "ko'prik". U ikki narsani bajaradi:
1. Faylni belgilangan **rejimda** (o'qish, yozish, qo'shish va h.k.) ochadi.
2. Shu fayl uchun `stdio.h`ning ichki bufer tizimini o'rnatadi — shundan keyingina `fprintf`, `fgets`, `fread` kabi yuqori darajali funksiyalar ishlay boshlaydi.

## 2. Prototip va sintaksis

```c
#include <stdio.h>

FILE *fopen(const char *filename, const char *mode);
```

- **`filename`** — ochilishi kerak bo'lgan faylning nomi (yoki to'liq yo'li).
- **`mode`** — qanday rejimda ochish kerakligini bildiruvchi string.
- **Qaytish qiymati** — muvaffaqiyatda `FILE*` ko'rsatkich, xatoda **`NULL`**.

```c
FILE *f = fopen("malumot.txt", "r");
if (f == NULL) {
    fprintf(stderr, "Faylni ochib bo'lmadi.\n");
    return 1;
}
```

**Muhim qoida:** `fopen()`ning qaytish qiymatini **har doim** `NULL`ga tekshirish shart. Fayl mavjud bo'lmasligi, ruxsat yetishmasligi, disk to'lib qolishi kabi sabablarga ko'ra `fopen()` muvaffaqiyatsiz bo'lishi juda oddiy holat — buni e'tiborsiz qoldirish `NULL` ko'rsatkichni keyinroq `fprintf(f, ...)` ichida ishlatib, **segmentation fault**ga olib keladi.

## 3. Rejimlar (mode) — to'liq jadval

| Rejim | Ma'nosi | Fayl mavjud bo'lmasa | Fayl mavjud bo'lsa |
|---|---|---|---|
| `"r"` | O'qish (read) | `NULL` qaytaradi | Boshidan o'qiladi |
| `"w"` | Yozish (write) | Yangi fayl yaratiladi | **Mazmuni butunlay o'chiriladi (truncate)** |
| `"a"` | Qo'shish (append) | Yangi fayl yaratiladi | Oxiriga yoziladi, eski mazmun saqlanadi |
| `"r+"` | O'qish + yozish | `NULL` qaytaradi | Boshidan o'qish/yozish, mazmun saqlanadi |
| `"w+"` | O'qish + yozish | Yangi fayl yaratiladi | **Mazmuni o'chiriladi**, keyin o'qish/yozish |
| `"a+"` | O'qish + qo'shish | Yangi fayl yaratiladi | Oxiriga yoziladi, boshidan o'qish mumkin |

Ikkilik (binary) fayllar uchun rejim oxiriga `b` qo'shiladi: `"rb"`, `"wb"`, `"ab"`, `"rb+"` yoki `"r+b"` va h.k. (Windows'da matn va ikkilik rejim orasida farq bor — matn rejimida `\n` avtomatik `\r\n`ga aylantiriladi; Linux/Unix'da bu farq amalda yo'q, lekin portativ kod yozish uchun `b` belgisini qo'yish yaxshi amaliyot.)

### 3.1. Rejimlarni tanlashda tez-tez uchraydigan xatolar

```c
// XATO: log faylni har safar dastur ishga tushganda tozalab yuboradi
FILE *log = fopen("dastur.log", "w");   // "w" — eski loglarni o'chiradi!

// TO'G'RI: eski loglar saqlanadi, yangi yozuvlar oxiriga qo'shiladi
FILE *log = fopen("dastur.log", "a");
```

```c
// XATO: mavjud faylni o'qish/yozish uchun ochmoqchi bo'lib, "w+" ishlatish
FILE *f = fopen("sozlamalar.json", "w+");  // mazmun darhol o'chib ketadi!

// TO'G'RI: mazmunni saqlab qolib, o'qish/yozish uchun
FILE *f = fopen("sozlamalar.json", "r+");
```

## 4. Xato tekshiruvi va `errno`

`fopen()` `NULL` qaytarganda, sabab haqida batafsil ma'lumotni `errno` va `strerror()` orqali olish mumkin:

```c
#include <stdio.h>
#include <errno.h>
#include <string.h>

int main(void) {
    FILE *f = fopen("mavjud_bolmagan_fayl.txt", "r");
    if (f == NULL) {
        fprintf(stderr, "Xato: %s (%s)\n", strerror(errno), "mavjud_bolmagan_fayl.txt");
        return 1;
    }
    fclose(f);
    return 0;
}
```

Natija (misol):
```
Xato: No such file or directory (mavjud_bolmagan_fayl.txt)
```

Eng ko'p uchraydigan `errno` qiymatlari:

| `errno` konstantasi | Ma'nosi |
|---|---|
| `ENOENT` | Fayl yoki katalog mavjud emas |
| `EACCES` | Ruxsat yetishmaydi (permission denied) |
| `EISDIR` | Berilgan yo'l — fayl emas, katalog |
| `ENOSPC` | Diskda bo'sh joy qolmagan |
| `EMFILE` | Bitta jarayon uchun ochiq fayllar soni chegarasiga yetildi |

## 5. `fopen()`dan foydalanishning to'liq hayotiy sikli

Har bir `fopen()` chaqiruvi oxir-oqibat mos `fclose()` bilan yakunlanishi **shart**. Aks holda:

- Fayl resurslari (file descriptor) tizim tomonidan band bo'lib qoladi — bu resurs sizib chiqishi (resource leak).
- Buferga yozilgan, lekin hali diskka tushirilmagan ma'lumot **yo'qolib ketishi** mumkin (chunki `fclose()` avtomatik `fflush()` ham bajaradi).

```c
#include <stdio.h>

int main(void) {
    FILE *f = fopen("natija.txt", "w");
    if (f == NULL) {
        perror("fopen");     // errno asosida avtomatik xato xabari chiqaradi
        return 1;
    }

    fprintf(f, "Hisoblash yakunlandi.\n");

    if (fclose(f) != 0) {
        perror("fclose");    // fclose ham muvaffaqiyatsiz bo'lishi mumkin!
        return 1;
    }

    return 0;
}
```

**Diqqat:** `fclose()`ning ham qaytish qiymati bor va u ham xato qaytarishi mumkin (masalan, oxirgi flush paytida diskka yozish muvaffaqiyatsiz bo'lsa). Ko'pchilik buni unutadi, lekin ma'lumot yaxlitligi muhim bo'lgan dasturlarda buni ham tekshirish kerak.

`perror()` funksiyasi haqida qisqacha: u sizning matningizni `": "` bilan ajratib, so'ng `strerror(errno)` natijasini chiqaradi — `stderr`ga. Yuqoridagi misolda `perror("fopen")` chiqishi taxminan: `fopen: Permission denied`.

## 6. Bir nechta faylni bir vaqtda ochish

Dasturda bir vaqtning o'zida bir nechta `FILE*` bilan ishlash mumkin — har biri mustaqil `FILE` strukturasiga ega bo'ladi:

```c
#include <stdio.h>

int main(void) {
    FILE *kirish = fopen("manba.txt", "r");
    FILE *chiqish = fopen("nusxa.txt", "w");

    if (kirish == NULL || chiqish == NULL) {
        fprintf(stderr, "Fayllarni ochib bo'lmadi.\n");
        if (kirish) fclose(kirish);
        if (chiqish) fclose(chiqish);
        return 1;
    }

    int belgi;
    while ((belgi = fgetc(kirish)) != EOF) {
        fputc(belgi, chiqish);
    }

    fclose(kirish);
    fclose(chiqish);
    printf("Fayl nusxalandi.\n");
    return 0;
}
```

Bu yerda diqqat qiling: ikkala `fopen()` chaqiruvidan keyin **ikkalasini ham** `NULL`ga tekshiryapmiz, va agar biri muvaffaqiyatli ochilib, ikkinchisi ochilmasa ham, ochilgan faylni albatta yopamiz — resurs sizib chiqmasligi uchun.

Har bir jarayon (process) uchun bir vaqtning o'zida ochiq bo'lishi mumkin bo'lgan fayllar soniga **chegara** bor (odatda operatsion tizimda `ulimit -n` orqali sozlanadi, ko'pincha 1024 atrofida). Ko'p fayl ochib, yopishni unutsangiz, oxir-oqibat yangi `fopen()` chaqiruvlari `NULL` qaytara boshlaydi (`errno == EMFILE`).

## 7. `fopen()` va bufer turi

Fayl ochilganda, `stdio.h` avtomatik ravishda unga bufer biriktiradi (odatda **to'liq buferlangan**, hajmi tizimga bog'liq). Buni `setvbuf()` orqali o'zgartirish mumkin — bu, albatta, `fopen()`dan **keyin**, lekin faylga birinchi operatsiyadan **oldin** chaqirilishi kerak:

```c
FILE *f = fopen("tezkor_log.txt", "w");
if (f != NULL) {
    setvbuf(f, NULL, _IONBF, 0);  // buferlashni butunlay o'chirish
    fprintf(f, "Har bir yozuv darhol diskka tushadi.\n");
}
```

Bu real vaqt (real-time) log tizimlarida foydali — lekin diskka har safar to'g'ridan-to'g'ri yozish sekinroq ishlaydi, shuning uchun faqat kerak bo'lgandagina ishlatiladi.

## 8. `fopen()` ning ichki mexanizmi — nima sodir bo'ladi

Yuqori darajada `fopen()` chaqirilganda, quyidagi bosqichlar yuz beradi (tizimga bog'liq, lekin umumiy model):

1. C kutubxonasi `mode` stringini tahlil qiladi (`"r"`, `"w+b"` va h.k.) va operatsion tizimga mos flaglarni tayyorlaydi (masalan, POSIX tizimlarida `open()` chaqiruvi uchun `O_RDONLY`, `O_WRONLY`, `O_CREAT`, `O_TRUNC`, `O_APPEND`).
2. Operatsion tizimning past darajadagi `open()` (yoki Windows'da `CreateFile`) funksiyasi chaqiriladi — bu fayl uchun **fayl deskriptori (file descriptor)** qaytaradi.
3. Agar bu muvaffaqiyatsiz bo'lsa (fayl yo'q, ruxsat yo'q va h.k.), `fopen()` `NULL` qaytaradi va `errno` o'rnatiladi.
4. Muvaffaqiyatli bo'lsa, xotirada `FILE` strukturasi ajratiladi (`malloc` orqali) — unda fayl deskriptori, bufer, joriy pozitsiya va holat flaglari saqlanadi.
5. Shu `FILE*` ko'rsatkich dasturchiga qaytariladi.

Shu sababli `fopen()` — bu Unix/Linux tizimlaridagi past darajadagi `open()` funksiyasi ustiga qurilgan **yuqori darajadagi qatlam** (bufer va qulay interfeys qo'shadi). Agar sizga bufer kerak bo'lmasa yoki fayl deskriptori bilan to'g'ridan-to'g'ri ishlash kerak bo'lsa, POSIX tizimlarida `open()`, `read()`, `write()` funksiyalaridan foydalanish mumkin — lekin bu alohida mavzu.

## 9. Nisbiy va absolyut yo'llar (relative vs absolute paths)

```c
FILE *f1 = fopen("malumot.txt", "r");           // joriy ishchi katalog (cwd)ga nisbatan
FILE *f2 = fopen("./data/malumot.txt", "r");     // joriy katalogdagi "data" papkasi
FILE *f3 = fopen("/home/zafar/malumot.txt", "r"); // absolyut yo'l
```

Dastur qaysi katalogdan ishga tushirilishiga qarab (`cwd` — current working directory), nisbiy yo'llar turlicha natija berishi mumkin. Ishonchli dasturlarda odatda absolyut yo'llar yoki dastur ishga tushirilgan joyga nisbatan aniq hisoblangan yo'llar ishlatiladi.

## 10. To'liq amaliy misol: sozlamalar faylini xavfsiz ochish

```c
#include <stdio.h>
#include <errno.h>
#include <string.h>

FILE *sozlamalarni_och(const char *fayl_nomi) {
    FILE *f = fopen(fayl_nomi, "r");

    if (f == NULL) {
        if (errno == ENOENT) {
            fprintf(stderr, "'%s' topilmadi — standart sozlamalar ishlatiladi.\n", fayl_nomi);
        } else {
            fprintf(stderr, "'%s' ochilmadi: %s\n", fayl_nomi, strerror(errno));
        }
        return NULL;
    }

    return f;
}

int main(void) {
    FILE *f = sozlamalarni_och("config.ini");

    if (f == NULL) {
        // standart sozlamalar bilan davom etish
        printf("Standart sozlamalar bilan ishga tushirilmoqda.\n");
    } else {
        printf("Sozlamalar fayli muvaffaqiyatli ochildi.\n");
        fclose(f);
    }

    return 0;
}
```

Bu misol xato turini ajratib (`ENOENT` — fayl yo'q — bu ko'pincha jiddiy xato emas, standart qiymatlarga o'tsa bo'ladi) va boshqa xatolardan (masalan, ruxsat yo'qligi) farqlab ishlov berishni ko'rsatadi.

## 11. `fopen()` bilan bog'liq tez-tez uchraydigan xatolar (checklist)

1. **`NULL` tekshirilmasligi** — eng keng tarqalgan xato. `fopen()` muvaffaqiyatsiz bo'lishi mumkinligini har doim eslang.
2. **Noto'g'ri rejim tanlash** — masalan, mavjud faylni saqlab qolish kerak bo'lsa `"w"` o'rniga `"a"` yoki `"r+"` ishlatish kerakligini unutish.
3. **`fclose()` chaqirilmasligi** — resurs sizib chiqishi va buferdagi ma'lumot yo'qolishi xavfi.
4. **Bir xil faylni ikki marta yozish rejimida ochish** — bu tizimga bog'liq noaniq xatti-harakatlarga olib kelishi mumkin.
5. **`fopen()`dan qaytgan ko'rsatkichni ikki marta `fclose()` qilish** — bu "double free"ga o'xshash muammo, **undefined behavior**.

## 12. Xulosa

- `fopen(filename, mode)` faylni ochib, `FILE*` yoki xatoda `NULL` qaytaradi — natijani **har doim** tekshiring.
- Rejim tanlovi (`r`, `w`, `a`, va `+` variantlari) faylning mavjud mazmuniga nima bo'lishini belgilaydi — ayniqsa `"w"` mavjud mazmunni **o'chirib yuborishini** unutmang.
- Xato sababini `errno` + `strerror()` yoki qulay usulda `perror()` orqali aniqlash mumkin.
- Har bir `fopen()` mos `fclose()` bilan yakunlanishi shart — bu ham resurslarni, ham ma'lumot yaxlitligini himoya qiladi.
- `fopen()` — past darajadagi operatsion tizim chaqiruvlari (`open()`) ustiga qurilgan, bufer va qulay interfeys qo'shuvchi yuqori darajadagi qatlam.

## 13. Mashqlar

1. Foydalanuvchidan fayl nomini kiritishni so'rab, uni `"r"` rejimida ochishga harakat qiluvchi, agar fayl topilmasa aniq xabar bilan qayta so'rovchi (masalan, 3 marta) dastur yozing.
2. Berilgan faylni `"r+"` rejimida ochib, agar fayl mavjud bo'lmasa avtomatik ravishda `"w+"` bilan yaratib beradigan `xavfsiz_och()` funksiyasini yozing (ya'ni "ochish, aks holda yaratish" mantiqini qo'lda amalga oshiring).
3. Bir vaqtning o'zida 3 ta faylni ochib (`"r"`, `"w"`, `"a"` rejimlarida), ulardan birortasi muvaffaqiyatsiz bo'lsa, oldin ochilganlarining barchasini to'g'ri yopib, dasturdan chiqadigan kod yozing.
4. `setvbuf()` yordamida bitta faylni buferlanmagan (`_IONBF`) va boshqasini to'liq buferlangan (`_IOFBF`) rejimda ochib, ikkalasiga ham 100,000 marta bitta belgi yozib, ishlash tezligini solishtiring.
