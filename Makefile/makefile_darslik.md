# Makefile darsligi: `s21_string` loyihasi misolida

Bu darslik sizning Makefile'ingizni **satrma-satr** tushuntiradi. Har bir bo'limda avval nazariya, keyin shu nazariya Makefile'ingizning qaysi qismida ishlatilgani ko'rsatiladi. Texnik terminlar (target, prerequisite, recipe va h.k.) xalqaro shaklda qoldirildi.

---

## Mundarija

1. Make nima va nega kerak
2. Asosiy g'oya: qoida (rule)
3. Make qanday qaror qiladi: vaqt tamg'alari va dependency graph
4. Sintaksis qoidalari
5. O'zgaruvchilar
6. Funksiyalar va almashtirishlar
7. Shartli direktivalar (`ifeq`)
8. Pattern rule va automatic variables
9. Recipe ichki tuzilishi: `@`, `\`, har qator alohida shell
10. `.PHONY` targetlar
11. Makefile'ingizning to'liq tahlili
12. Kompilyatsiya va linking tushunchalari (`ar`, `ranlib`, `-l`, `--coverage`)
13. `make` yozganda aynan nima sodir bo'ladi (trace)
14. Ehtiyot bo'ladigan joylar va yaxshilash takliflari
15. Foydali buyruqlar va debugging
16. Mashqlar
17. Qisqa shpargalka

---

## 1. Make nima va nega kerak

C dasturini qo'lda kompilyatsiya qilish quyidagicha:

```bash
gcc -std=c11 -Wall -Wextra -Werror -c s21_strlen.c -o s21_strlen.o
gcc -std=c11 -Wall -Wextra -Werror -c s21_memcpy.c -o s21_memcpy.o
... (yana 30 ta fayl)
ar rcs s21_string.a *.o
```

Bu muammolar tug'diradi:

- Har safar hamma buyruqni qo'lda yozish kerak.
- Bitta faylni o'zgartirsangiz ham hammasini qayta kompilyatsiya qilish vaqt oladi.
- Test, coverage, tozalash kabi qo'shimcha ishlar uchun yana buyruqlar kerak.

**Make** shu ishlarni avtomatlashtiradi. U `Makefile` faylini o'qiydi va:

1. Qaysi fayl qaysi fayldan hosil bo'lishini biladi (**bog'liqliklar**).
2. Faqat **o'zgargan** fayllarni qayta quradi (**inkremental build**).
3. Bir so'z bilan (`make test`, `make clean`) murakkab buyruqlar ketma-ketligini ishga tushiradi.

> Make faqat C uchun emas. U har qanday "fayl A fayl B dan hosil bo'ladi" tipidagi vazifalar uchun ishlaydi. Bu darslik **GNU Make** haqida.

---

## 2. Asosiy g'oya: qoida (rule)

Makefile qoidalardan (rules) iborat. Qoidaning umumiy shakli:

```make
target: prerequisites
	recipe
```

| Qism | Ma'nosi |
|---|---|
| **target** | Hosil qilinadigan narsa (odatda fayl nomi, yoki "vazifa" nomi) |
| **prerequisites** | Target hosil bo'lishi uchun kerak bo'ladigan fayllar (dependency ham deyiladi) |
| **recipe** | Targetni hosil qiluvchi shell buyruqlari |

**Muhim:** recipe qatorlari **TAB** belgisi bilan boshlanishi shart (probel emas!). Bu Make'ning eng mashhur "tuzog'i".

Sizning Makefile'ingizdan misol:

```make
$(TARGET): $(OBJECTS)
	ar rcs $(TARGET) $(OBJECTS)
	ranlib $(TARGET)
```

O'zgaruvchilar o'rniga qiymatlarni qo'ysak:

```make
s21_string.a: s21_strlen.o s21_memcpy.o ...
	ar rcs s21_string.a s21_strlen.o s21_memcpy.o ...
	ranlib s21_string.a
```

O'qilishi: "`s21_string.a` faylini hosil qilish uchun avval hamma `.o` fayllar tayyor bo'lishi kerak; keyin `ar` va `ranlib` buyruqlarini ishga tushir."

---

## 3. Make qanday qaror qiladi

### 3.1. Vaqt tamg'alari (timestamps)

Make har bir target va prerequisite'ning **oxirgi o'zgarish vaqtini** (mtime) solishtiradi:

- Target fayli **mavjud emas** → recipe bajariladi.
- Biror prerequisite targetdan **yangiroq** → recipe bajariladi.
- Aks holda → target "yangi" (up to date), hech narsa qilinmaydi.

Shuning uchun `make`ni ikkinchi marta yozsangiz, `make: 's21_string.a' is up to date.` xabarini ko'rasiz.

### 3.2. Dependency graph

Make bog'liqliklardan **graf** quradi va uni pastdan yuqoriga qarab bajaradi:

```
all
 └── s21_string.a
      ├── s21_strlen.o  ← s21_strlen.c, s21_string.h
      ├── s21_memcpy.o  ← s21_memcpy.c, s21_string.h
      └── ...
```

Make grafni **rekursiv** aylanadi: `all` kerak → `s21_string.a` kerak → uning `.o` fayllari kerak → ularning `.c` fayllari mavjudmi, o'zgarganmi?

### 3.3. Default goal

Agar `make` ni argumentsiz yozsangiz, u Makefile'dagi **birinchi targetni** quradi. Sizda bu:

```make
all: $(TARGET)
```

Shu sababli `make` ≡ `make all`. Shuning uchun `all` odatda faylning boshida turadi.

---

## 4. Sintaksis qoidalari

**Izohlar** `#` bilan boshlanadi:

```make
# The tests deliberately call the standard functions ...
```

**Uzun qatorni davom ettirish** oxiriga `\` qo'yish bilan:

```make
TEST_CFLAGS = $(CFLAGS) -Wno-stringop-truncation -Wno-stringop-overflow \
              -Wno-memset-transposed-args
```

`\` dan keyin hech narsa (probel ham) bo'lmasligi kerak. Make qatorlarni bitta qator deb o'qiydi.

**Bo'sh qatorlar** e'tiborga olinmaydi, o'qilishni yaxshilash uchun ishlatiladi.

**TAB va probel:** recipe faqat TAB bilan boshlanadi. Direktivalar (`ifeq`, `endif`) va o'zgaruvchi e'lonlari esa TAB bilan **boshlanmasligi** kerak, aks holda Make ularni recipe deb o'ylaydi.

---

## 5. O'zgaruvchilar

### 5.1. E'lon qilish va ishlatish

```make
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Werror
```

Ishlatish: `$(CC)` yoki `${CC}`. Bir harfli o'zgaruvchilar uchun qavssiz `$X` ham ishlaydi, lekin **doim qavs ishlating**.

Shell o'zgaruvchisi bilan adashtirmang: Make o'zgaruvchisi `$(NAME)`, shell'niki `$NAME`. Recipe ichida shell `$` belgisini ishlatish uchun `$$` yozish kerak (masalan `$$HOME`).

### 5.2. Tayinlash operatorlari

| Operator | Nomi | Ma'nosi |
|---|---|---|
| `=` | recursive (lazy) | Qiymat **ishlatilgan paytda** hisoblanadi |
| `:=` | simple (immediate) | Qiymat **e'lon qilingan paytda** bir marta hisoblanadi |
| `?=` | conditional | Faqat o'zgaruvchi hali belgilanmagan bo'lsa tayinlaydi |
| `+=` | append | Mavjud qiymatga qo'shadi |

Farqni misolda ko'ramiz:

```make
A = $(B)
B = salom
# $(A) → "salom"  (chunki A ishlatilganda B allaqachon belgilangan)

C := $(D)
D = salom
# $(C) → ""  (C e'lon qilinganda D hali yo'q edi)
```

**Sizning Makefile'ingizda** barcha o'zgaruvchilar `=` bilan e'lon qilingan. Bu muhim oqibatga olib keladi:

```make
CHECK_LIBS = $(shell pkg-config --libs check 2>/dev/null)
```

Bu yerda `$(shell ...)` **har safar** `$(CHECK_LIBS)` ishlatilganda qayta bajariladi. Kichik loyihada bu sezilmaydi, lekin katta loyihada `:=` ishlatish tezroq (buyruq bir marta bajariladi). Buni 14-bo'limda yana eslaymiz.

### 5.3. Buyruq qatoridan qiymat berish

```bash
make CC=clang
make CFLAGS="-O2"
```

Buyruq qatoridagi qiymat Makefile ichidagi `=` tayinlashdan **ustun** turadi. Shuning uchun kompilyatorni almashtirish oson.

---

## 6. Funksiyalar va almashtirishlar

Make'da funksiya chaqirish shakli: `$(funksiya argument1,argument2,...)`.

### 6.1. `$(wildcard ...)` — fayllarni qidirish

```make
SOURCES = $(wildcard s21_*.c)
TEST_SOURCES = $(wildcard tests/*.c)
```

Joriy papkadagi `s21_` bilan boshlanib `.c` bilan tugaydigan barcha fayllar ro'yxatini beradi. Masalan: `s21_strlen.c s21_memcpy.c s21_strchr.c`.

Qulaylik: yangi `s21_xxx.c` fayl qo'shsangiz, Makefile'ni o'zgartirish shart emas.

### 6.2. Substitution reference: `$(VAR:eski=yangi)`

```make
OBJECTS = $(SOURCES:.c=.o)
```

`SOURCES` ichidagi har bir so'z oxiridagi `.c` ni `.o` ga almashtiradi:

```
s21_strlen.c s21_memcpy.c  →  s21_strlen.o s21_memcpy.o
```

Bu `$(patsubst %.c,%.o,$(SOURCES))` ning qisqa yozuvi.

### 6.3. `$(shell ...)` — shell buyrug'ini ishga tushirish

```make
OS = $(shell uname -s)
```

Buyruq chiqishini (stdout) o'zgaruvchiga qaytaradi. `uname -s` Linux'da `Linux`, macOS'da `Darwin` beradi. Yangi qator belgilari probelga aylantiriladi.

`2>/dev/null` xato xabarlarini (stderr) tashlab yuboradi. Bu yerda: agar `pkg-config` o'rnatilmagan bo'lsa, ekranga xato chiqmasin.

### 6.4. `$(strip ...)` — ortiqcha probellarni olib tashlash

```make
ifeq ($(strip $(CHECK_LIBS)),)
```

Boshi va oxiridagi probellarni olib tashlaydi, o'rtadagi ketma-ket probellarni bittaga aylantiradi. Bu "o'zgaruvchi bo'shmi?" tekshiruvini ishonchli qiladi (faqat probeldan iborat qiymat ham bo'sh hisoblanadi).

---

## 7. Shartli direktivalar

Make fayl o'qilayotgan paytda shart bo'yicha qismlarni qo'shishi yoki tashlab ketishi mumkin.

```make
ifeq (a, b)
  # a == b bo'lsa
else
  # aks holda
endif
```

Boshqa variantlar: `ifneq` (teng emas), `ifdef` (o'zgaruvchi belgilangan), `ifndef`.

**Muhim:** bu direktivalar Make'ning **o'qish bosqichida** ishlaydi, recipe bajarilayotgan paytda emas. Recipe ichidagi shartlar uchun shell'ning `if` konstruksiyasi ishlatiladi.

### 7.1. Kompilyator turini aniqlash

```make
ifeq ($(shell $(CC) --version 2>/dev/null | grep -ci clang),0)
TEST_CFLAGS = $(CFLAGS) -Wno-stringop-truncation -Wno-stringop-overflow \
              -Wno-memset-transposed-args
else
TEST_CFLAGS = $(CFLAGS)
endif
```

Tahlil:

- `$(CC) --version` kompilyator versiyasini chiqaradi.
- `grep -ci clang`: `-c` mos qatorlar **sonini** beradi, `-i` katta-kichik harfni farqlamaydi.
- Natija `0` bo'lsa, "clang" so'zi topilmagan → bu **haqiqiy GCC**.
- GCC bo'lsa, test fayllar uchun ba'zi ogohlantirishlar o'chiriladi (`-Wno-...`). Sababi izohda yozilgan: testlar ataylab standart funksiyalarni "g'alati" o'lchamlar bilan chaqiradi.
- Clang bo'lsa (macOS'da `gcc` aslida clang'ga havola), bu flaglar noma'lum bo'lgani uchun qo'shilmaydi.

Shunday qilib, bitta Makefile ham Linux (GCC), ham macOS (Clang) da ishlaydi.

### 7.2. Ichma-ich shartlar va Check kutubxonasini topish

```make
ifeq ($(strip $(CHECK_LIBS)),)
ifeq ($(OS), Darwin)
CHECK_CFLAGS = -I/opt/homebrew/include -I/usr/local/include
CHECK_LIBS = -L/opt/homebrew/lib -L/usr/local/lib -lcheck
else
CHECK_LIBS = -lcheck -lm -lpthread -lrt -lsubunit
endif
endif
```

Mantiq (algoritm):

```
1. pkg-config orqali Check flaglarini olishga urin
2. Agar CHECK_LIBS bo'sh bo'lsa (pkg-config topa olmadi):
     a. macOS bo'lsa → Homebrew yo'llarini qo'lda ko'rsat
     b. aks holda (Linux) → standart kutubxonalar ro'yxatini ber
3. Agar pkg-config ishlagan bo'lsa → hech narsa o'zgarmaydi
```

Bu **fallback** (zaxira) strategiyasi: avval avtomatik usul, bo'lmasa qo'lda.

Flaglar ma'nosi:

| Flag | Ma'nosi |
|---|---|
| `-I<yo'l>` | Header fayllar qidiriladigan papka |
| `-L<yo'l>` | Kutubxona fayllari qidiriladigan papka |
| `-lcheck` | `libcheck` kutubxonasini ulash |
| `-lm`, `-lpthread`, `-lrt`, `-lsubunit` | Math, thread, real-time va subunit kutubxonalari (Check ularga tayanadi) |

---

## 8. Pattern rule va automatic variables

### 8.1. Pattern rule

Har bir `.c` fayl uchun alohida qoida yozish o'rniga **shablon** yoziladi:

```make
%.o: %.c s21_string.h
	$(CC) $(CFLAGS) -c $< -o $@
```

`%` — "istalgan qism" (stem). Make `s21_strlen.o` kerak bo'lganda `%` = `s21_strlen` deb oladi va prerequisite'lar `s21_strlen.c` va `s21_string.h` bo'ladi.

### 8.2. Automatic variables

Make recipe ichida avtomatik to'ldiradigan maxsus o'zgaruvchilar:

| O'zgaruvchi | Ma'nosi | Bu qoidadagi qiymati (misol) |
|---|---|---|
| `$@` | Target nomi | `s21_strlen.o` |
| `$<` | **Birinchi** prerequisite | `s21_strlen.c` |
| `$^` | Barcha prerequisite'lar | `s21_strlen.c s21_string.h` |

Nima uchun bu yerda `$<` ishlatilgan, `$^` emas? Chunki `gcc -c` ga faqat `.c` faylni berish kerak. Header (`s21_string.h`) kompilyatorga `#include` orqali o'zi ulanadi. Agar `$^` ishlatilsa, `.h` fayl ham buyruqqa tushib qoladi.

Natijada har bir fayl uchun bajariladigan buyruq:

```bash
gcc -std=c11 -Wall -Wextra -Werror -c s21_strlen.c -o s21_strlen.o
```

### 8.3. Header dependency nima uchun qo'shilgan?

`s21_string.h` prerequisite ro'yxatida bo'lgani uchun, header o'zgarsa **hamma** `.o` fayllar qayta kompilyatsiya bo'ladi. Bu to'g'ri xatti-harakat: header'dagi struct yoki funksiya imzosi o'zgarsa, eski `.o` fayllar yaroqsiz bo'lib qolishi mumkin.

---

## 9. Recipe ichki tuzilishi

### 9.1. Har bir qator alohida shell'da bajariladi

```make
foo:
	cd tests
	ls        # bu HALI HAM joriy papkada ishlaydi, tests ichida emas!
```

Har bir recipe qatori **yangi shell jarayoni**. `cd` faqat o'sha qator uchun ta'sir qiladi. Bir shell'da bajarish uchun `cd tests && ls` yoki `\` bilan bitta qatorga birlashtirish kerak.

### 9.2. Buyruq echosi va `@`

Odatda Make bajarayotgan buyruqni ekranga chiqaradi. Buyruq oldiga `@` qo'ysangiz, chiqarilmaydi:

```make
	@echo "report is in $(REPORT_DIR)/index.html"
```

Bu yerda `@` bo'lmasa, ekranda avval `echo "report is ..."` buyrug'i, keyin uning natijasi (ikki marta) ko'rinardi.

### 9.3. `-` prefiksi (Makefile'ingizda yo'q, lekin bilish kerak)

Buyruq oldiga `-` qo'ysangiz, uning xatosi e'tiborga olinmaydi va Make davom etadi. Odatda esa bitta buyruq xato (nol bo'lmagan exit code) qaytarsa, Make **to'xtaydi**.

Sizda `rm -f` ishlatilgan: `-f` fayl mavjud bo'lmasa ham xato bermaydi. Shu sababli `make clean` ni ketma-ket ikki marta yozish xavfsiz.

### 9.4. Recipe ichida qatorni davom ettirish

```make
	$(CC) $(TEST_CFLAGS) $(CHECK_CFLAGS) $(TEST_SOURCES) $(TARGET) $(CHECK_LIBS) \
		-o $(TEST_TARGET)
```

Bu bitta shell buyrug'i. Ikkinchi qator boshidagi TAB `\` dan keyin shell'ga uzatilmaydi (Make uni olib tashlaydi).

---

## 10. `.PHONY` targetlar

Ba'zi targetlar **fayl emas, vazifa** nomi: `all`, `test`, `clean`. Agar loyiha papkasida tasodifan `clean` degan fayl paydo bo'lsa, Make "`clean` fayli allaqachon bor va prerequisite yo'q, demak yangi" deb hisoblab, recipe'ni bajarmasligi mumkin.

Buni oldini olish uchun:

```make
.PHONY: all test gcov_report style clean
```

`.PHONY` ga kiritilgan targetlar uchun Make fayl mavjudligini tekshirmaydi va recipe **har doim** bajariladi.

Qoida: agar target fayl hosil qilmasa, uni `.PHONY` ga qo'shing.

---

## 11. Makefile'ingizning to'liq tahlili

### 11.1. Sozlamalar bloki (1–15-qatorlar)

```make
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Werror
GCOV_FLAGS = --coverage
```

- `CC`: kompilyator.
- `CFLAGS`: `-std=c11` (C11 standarti), `-Wall -Wextra` (ko'p ogohlantirishlarni yoqish), `-Werror` (**ogohlantirishlarni xatoga aylantirish**: ogohlantirish bo'lsa build to'xtaydi).
- `GCOV_FLAGS`: coverage yig'ish uchun.

```make
TARGET = s21_string.a
TEST_TARGET = tests/s21_test
REPORT_DIR = report
```

Asosiy nomlar bir joyda turibdi. Keyin o'zgartirish kerak bo'lsa, faqat shu yerni tahrirlaysiz. Bu **DRY** (Don't Repeat Yourself) tamoyili.

`.a` kengaytmasi: **static library** (statik kutubxona), ya'ni `.o` fayllar arxivi.

```make
SOURCES = $(wildcard s21_*.c)
OBJECTS = $(SOURCES:.c=.o)
TEST_SOURCES = $(wildcard tests/*.c)
```

Fayl ro'yxatlari avtomatik shakllanadi (6.1 va 6.2 bo'limlar).

```make
OS = $(shell uname -s)
CHECK_CFLAGS = $(shell pkg-config --cflags check 2>/dev/null)
CHECK_LIBS = $(shell pkg-config --libs check 2>/dev/null)
```

`pkg-config` kutubxona uchun kerakli flaglarni beradigan vosita. `--cflags` kompilyatsiya flaglarini (`-I...`), `--libs` linking flaglarini (`-l...`) qaytaradi.

### 11.2. Shartlar bloki (17–34-qatorlar)

7-bo'limda batafsil ko'rilgan: kompilyator turiga qarab `TEST_CFLAGS`, va Check kutubxonasi topilmasa zaxira yo'llar.

### 11.3. `all` va kutubxonani qurish (36–40-qatorlar)

```make
all: $(TARGET)

$(TARGET): $(OBJECTS)
	ar rcs $(TARGET) $(OBJECTS)
	ranlib $(TARGET)
```

- `ar rcs`: arxiv yaratish. `r` — fayllarni qo'shish/almashtirish, `c` — arxiv yo'q bo'lsa yaratish (ogohlantirishsiz), `s` — symbol index yozish.
- `ranlib`: arxivga symbol index yozadi. Aslida `ar ... s` buni allaqachon qiladi, shuning uchun `ranlib` ko'pincha ortiqcha; lekin ba'zi eski tizimlarda (masalan eski macOS) kerak bo'lgan va zarari yo'q.

### 11.4. `test` targeti (45–48-qatorlar)

```make
test: $(TARGET)
	$(CC) $(TEST_CFLAGS) $(CHECK_CFLAGS) $(TEST_SOURCES) $(TARGET) $(CHECK_LIBS) \
		-o $(TEST_TARGET)
	./$(TEST_TARGET)
```

- Avval kutubxona qurilishi kerak (`test: $(TARGET)`).
- Keyin test `.c` fayllar kompilyatsiya qilinib, **kutubxona bilan** va Check bilan **linking** qilinadi va `tests/s21_test` bajariladigan fayli hosil bo'ladi.
- `./$(TEST_TARGET)` uni ishga tushiradi. Agar testlar yiqilsa, dastur nol bo'lmagan kod qaytaradi va Make `Error` bilan to'xtaydi.

**Linking tartibi muhim:** `$(TEST_SOURCES) $(TARGET) $(CHECK_LIBS)`. Linker chapdan o'ngga o'qiydi: avval test kodi qaysi funksiyalarga muhtojligini bilib oladi, keyin ularni keyingi kutubxonalardan qidiradi. Kutubxonalarni oldinga qo'ysangiz, ko'p tizimlarda `undefined reference` xatosi chiqadi.

### 11.5. `gcov_report` targeti (50–58-qatorlar)

```make
gcov_report: clean
	$(CC) $(TEST_CFLAGS) $(GCOV_FLAGS) $(CHECK_CFLAGS) $(SOURCES) $(TEST_SOURCES) \
		$(CHECK_LIBS) -o $(TEST_TARGET)
	./$(TEST_TARGET)
	lcov --capture --directory . --output-file coverage.info --no-external \
		--exclude '*/tests/*' --ignore-errors inconsistent,unsupported,empty
	genhtml coverage.info --output-directory $(REPORT_DIR) \
		--ignore-errors inconsistent,unsupported,empty,category
	@echo "report is in $(REPORT_DIR)/index.html"
```

Bosqichlar:

1. **`clean` prerequisite sifatida**: avval eski build va eski coverage fayllari o'chiriladi. Bu zarur, chunki coverage ma'lumotlari faqat toza kompilyatsiyadan keyin to'g'ri bo'ladi.
2. **Kutubxona o'rniga to'g'ridan-to'g'ri `.c` fayllar** ($(SOURCES)) kompilyatsiya qilinadi, `--coverage` bilan. Har bir source uchun `.gcno` (kompilyatsiya paytida struktura ma'lumoti) fayl hosil bo'ladi.
3. **Testlar ishga tushiriladi**: dastur ishlaganda `.gcda` fayllar (nechta marta qaysi qator bajarilgani) yoziladi.
4. **`lcov --capture`**: `.gcda/.gcno` fayllardan `coverage.info` yig'adi. `--no-external` tizim header'larini hisobga olmaydi, `--exclude '*/tests/*'` testlarning o'z kodini hisobdan chiqaradi.
5. **`genhtml`**: `coverage.info` dan HTML hisobot yaratadi.
6. `--ignore-errors ...`: ba'zi lcov/gcov versiyalaridagi mos kelmaslik xatolarini e'tiborsiz qoldiradi.
7. `@echo`: foydalanuvchiga hisobot qayerdaligini aytadi.

**Coverage (qamrov)** — testlar kodning necha foizini bajarganini o'lchaydi. Foiz qancha yuqori bo'lsa, kodning shuncha ko'p qismi sinovdan o'tgan.

### 11.6. `style` targeti (60–63-qatorlar)

```make
style:
	cp ../materials/linters/.clang-format .
	clang-format -n $(SOURCES) $(TEST_SOURCES) s21_string.h
	rm -f .clang-format
```

- Uslub sozlamasi (`.clang-format`) vaqtincha nusxalanadi, chunki `clang-format` uni joriy papkadan qidiradi.
- `-n` (dry-run): fayllarni **o'zgartirmaydi**, faqat uslub xatolarini ko'rsatadi.
- Oxirida vaqtincha fayl o'chiriladi.

Diqqat: `../materials/...` yo'li faqat sizning papkalar tuzilishingizda ishlaydi.

### 11.7. `clean` targeti (65–69-qatorlar)

```make
clean:
	rm -f $(OBJECTS) $(TARGET) $(TEST_TARGET)
	rm -f *.gcda *.gcno *.gcov tests/*.gcda tests/*.gcno tests/*.gcov
	rm -f coverage.info
	rm -rf $(REPORT_DIR) $(TEST_TARGET).dSYM
```

Build paytida hosil bo'lgan hamma narsani o'chiradi:

| Fayl | Nima |
|---|---|
| `.o` | Object fayllar |
| `.a` | Statik kutubxona |
| `.gcda`, `.gcno`, `.gcov` | Coverage ma'lumotlari |
| `coverage.info` | lcov yig'gan ma'lumot |
| `report/` | HTML hisobot papkasi (`-r` rekursiv, papka uchun) |
| `*.dSYM` | macOS debug symbol papkasi |

---

## 12. Kompilyatsiya va linking tushunchalari

C dasturi qurilish bosqichlari:

```
.c  --(kompilyatsiya, -c)-->  .o  --(arxivlash, ar)-->  .a
                                                        │
test.c --(kompilyatsiya)--> test.o ──(linking)──────────┴──> bajariladigan fayl
```

- **`-c`**: faqat kompilyatsiya qil, linking qilma. Natija `.o` (object).
- **`-o <fayl>`**: natija fayl nomi.
- **Static library (`.a`)**: `.o` fayllar arxivi. Linking paytida kerakli qismlar bajariladigan faylga **nusxalanadi**.
- **`-l<nom>`**: `lib<nom>.a` yoki `lib<nom>.so` ni ulash. `-lcheck` = `libcheck`.
- **Linker** nomlarni (funksiya nomlari) hal qiladi. Topa olmasa `undefined reference` xatosi beradi.

**Nima uchun `test` uchun kutubxona ishlatilgan, `gcov_report` uchun esa `.c` fayllar?** Test uchun tayyor `.a` yetarli. Coverage uchun esa har bir source **`--coverage` flagi bilan** kompilyatsiya qilinishi kerak, tayyor `.a` esa bu flagsiz qurilgan.

---

## 13. `make` yozganda aynan nima sodir bo'ladi

Faraz qilaylik, papkada `s21_strlen.c`, `s21_memcpy.c` va `s21_string.h` bor. Birinchi marta `make` yozamiz.

**1-bosqich, o'qish (parse):** Make butun Makefile'ni o'qiydi. O'zgaruvchilarni tayyorlaydi, `ifeq` shartlarini hal qiladi (shu paytda `pkg-config`, `uname`, `gcc --version` bajariladi), qoidalar ro'yxatini yig'adi.

**2-bosqich, maqsadni tanlash:** Argument yo'q → birinchi target `all`.

**3-bosqich, grafni aylanish:**

```
all → s21_string.a → s21_strlen.o, s21_memcpy.o
```

**4-bosqich, bajarish** (pastdan yuqoriga):

```bash
gcc -std=c11 -Wall -Wextra -Werror -c s21_strlen.c -o s21_strlen.o
gcc -std=c11 -Wall -Wextra -Werror -c s21_memcpy.c -o s21_memcpy.o
ar rcs s21_string.a s21_strlen.o s21_memcpy.o
ranlib s21_string.a
```

Endi `s21_memcpy.c` ni o'zgartirib, yana `make` yozsak, faqat u qayta kompilyatsiya qilinadi va arxiv yangilanadi. `s21_strlen.o` ga tegilmaydi.

`s21_string.h` o'zgarsa? Ikkala `.o` ham qayta quriladi.

---

## 14. Ehtiyot bo'ladigan joylar va yaxshilash takliflari

Quyidagilar xato emas, lekin chuqurroq tushunish va kelajakda yaxshilash uchun.

1. **`=` va `$(shell ...)`**: `CHECK_LIBS`, `OS` kabi o'zgaruvchilar har ishlatilganda shell'ni qayta chaqiradi. `:=` bunday holatda samaraliroq:
   ```make
   OS := $(shell uname -s)
   ```
   Lekin ehtiyot bo'ling: `:=` da tartib muhim (o'zgaruvchi ishlatishdan **oldin** e'lon qilingan bo'lishi kerak).

2. **`test` doim qayta kompilyatsiya qiladi**: `test` `.PHONY`, va uning recipe'i test dasturini har safar yig'adi. Bu kichik loyihada muammo emas.

3. **Header bog'liqliklari qo'pol**: faqat `s21_string.h` sanalgan. Agar boshqa header qo'shilsa, uni ham qo'shish kerak yoki GCC'ning `-MMD -MP` flaglari bilan bog'liqliklar avtomatik yaratilishi mumkin.

4. **`clean` va `gcov_report`**: `gcov_report` ning `clean` ga bog'liqligi natijasida `s21_string.a` ham o'chib ketadi. Shundan keyin `make test` uni qayta quradi. Bu ataylab qilingan: coverage bilan qurilgan va oddiy `.o` fayllar aralashib qolmasin.

5. **`style` targetidagi nisbiy yo'l**: `../materials/...` boshqa papkalar tuzilishida ishlamaydi. Yo'l o'zgaruvchiga chiqarilsa (`LINTER_DIR = ../materials/linters`) o'zgartirish osonlashadi.

6. **Parallel build**: `make -j4` bilan bir necha fayl bir vaqtda kompilyatsiya qilinadi. `.o` qoidalari mustaqil bo'lgani uchun bu xavfsiz.

7. **`$(RM)`**: Make'da `rm -f` uchun tayyor `RM` o'zgaruvchisi bor. Ko'chma (portable) yozuv uchun ishlatish mumkin.

---

## 15. Foydali buyruqlar va debugging

| Buyruq | Vazifasi |
|---|---|
| `make` | Default goal (`all`) ni quradi |
| `make test` | Aniq targetni quradi |
| `make -n` | **Dry run**: buyruqlarni bajarmasdan faqat ko'rsatadi |
| `make -B` | Hamma narsani majburan qayta quradi |
| `make -j4` | 4 ta parallel jarayon |
| `make -k` | Xatodan keyin ham mustaqil targetlarni davom ettiradi |
| `make -p` | Make'ning barcha ichki qoidalari va o'zgaruvchilarini chiqaradi |
| `make CC=clang` | O'zgaruvchini buyruq qatoridan almashtirish |
| `make -f boshqa.mk` | Boshqa nomli Makefile'ni ishlatish |

**O'zgaruvchi qiymatini ko'rish** uchun vaqtincha qo'shing:

```make
$(info SOURCES = $(SOURCES))
$(info OBJECTS = $(OBJECTS))
```

Yoki `$(warning ...)`, `$(error ...)` funksiyalari.

**Eng ko'p uchraydigan xatolar:**

| Xato xabari | Sabab |
|---|---|
| `*** missing separator` | Recipe TAB bilan emas, probel bilan boshlangan (yoki `:` unutilgan) |
| `No rule to make target 'X'` | Prerequisite fayl mavjud emas yoki nomi noto'g'ri |
| `Nothing to be done for 'X'` | Target allaqachon yangi |
| `undefined reference to ...` | Linking paytida kutubxona ulanmagan yoki tartibi noto'g'ri |
| `Error 1` / `Error 2` | Recipe ichidagi buyruq xato bilan tugadi |

---

## 16. Mashqlar

1. `make -n` ni ishga tushiring va chiqishni sizning qo'lda taxmin qilgan buyruqlaringiz bilan solishtiring.
2. Bitta `.c` faylga `touch` qiling (`touch s21_strlen.c`) va `make` ni yozing. Nechta buyruq bajarildi? Nima uchun?
3. `s21_string.h` ga `touch` qiling. Endi nechta fayl qayta kompilyatsiya bo'ladi?
4. `Makefile` ga `$(info OBJECTS = $(OBJECTS))` qo'shing va natijani kuzating.
5. `make CC=clang` bilan ishga tushiring. `TEST_CFLAGS` qaysi shoxdan o'tadi?
6. `CHECK_LIBS` ni `:=` ga o'zgartiring. Ishlashda farq bormi?
7. **Qiyinroq:** `make count` degan yangi target qo'shing, u `s21_*.c` fayllar sonini chiqarsin (`wc -l` va `$(SOURCES)` yordamida). Uni `.PHONY` ga qo'shishni unutmang.
8. **Qiyinroq:** `-MMD -MP` flaglari bilan header bog'liqliklarini avtomatlashtiring va `-include $(OBJECTS:.o=.d)` qatorini qo'shing.

---

## 17. Qisqa shpargalka

```make
# Izoh
VAR = qiymat            # lazy
VAR := qiymat           # immediate
VAR ?= qiymat           # agar yo'q bo'lsa
VAR += qo'shimcha       # qo'shish

$(VAR)                  # ishlatish
$(wildcard *.c)        # fayllar ro'yxati
$(SRC:.c=.o)            # kengaytmani almashtirish
$(shell buyruq)         # shell natijasi
$(strip  x  )           # probellarni tozalash

ifeq (a,b) ... else ... endif

target: prereq1 prereq2
	buyruq              # TAB bilan!

%.o: %.c
	$(CC) -c $< -o $@   # $@ target, $< birinchi prereq, $^ hamma prereq

.PHONY: all clean       # fayl bo'lmagan targetlar
```

**Eslab qoling:**

- Make **fayl vaqtlarini** solishtiradi va faqat kerakli narsani quradi.
- Recipe qatori **TAB** bilan boshlanadi va har biri **alohida shell**.
- Fayl hosil qilmaydigan targetlar `.PHONY` ga kiradi.
- `=` — lazy, `:=` — darhol hisoblanadi.
- `$@` — target, `$<` — birinchi prerequisite.
- Linking'da kutubxonalar tartibi muhim.
