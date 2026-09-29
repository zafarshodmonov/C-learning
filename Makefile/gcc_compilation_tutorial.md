# GCC yordamida Compilation jarayoni: to'liq darslik

> Maqsad: C dasturi `.c` faylidan ishga tushadigan `executable` gacha qanday yo'l bosib o'tishini **har bir bosqichda o'z ko'zingiz bilan ko'rib** o'rganish va shu bilimni `Makefile` bilan bog'lash.
>
> Barcha buyruqlar Linux (GCC, GNU binutils) uchun. Chiqishlardagi manzillar va ba'zi tafsilotlar sizning kompyuteringizda biroz farq qilishi mumkin.

---

## Mundarija

1. [Umumiy manzara](#1-umumiy-manzara)
2. [Namunaviy loyiha](#2-namunaviy-loyiha)
3. [Bosqich 1: Preprocessing](#3-bosqich-1-preprocessing--e)
4. [Bosqich 2: Compilation](#4-bosqich-2-compilation--s)
5. [Bosqich 3: Assembly](#5-bosqich-3-assembly--c)
6. [Object fayl ichida: nm, objdump, readelf](#6-object-fayl-ichida-nm-objdump-readelf)
7. [Bosqich 4: Linking](#7-bosqich-4-linking)
8. [Separate compilation](#8-separate-compilation-nega-har-bir-c-fayl-alohida-compile-qilinadi)
9. [Libraries: static va dynamic](#9-libraries-static-va-dynamic)
10. [Header fayllar va include mexanizmi](#10-header-fayllar-va-include-mexanizmi)
11. [Warning flag lari](#11-warning-flaglari)
12. [Debug va Sanitizers](#12-debug-va-sanitizers)
13. [Optimization](#13-optimization)
14. [Coverage: gcov va lcov](#14-coverage-gcov-va-lcov)
15. [Avtomatik dependency generatsiya](#15--mmd--mp-avtomatik-dependency-generatsiya)
16. [Xato qaysi bosqichdan chiqdi?](#16-xato-qaysi-bosqichdan-chiqdi)
17. [GCC va Makefile: to'liq bog'lanish](#17-gcc-va-makefile-toliq-boglanish)
18. [Mashqlar](#18-mashqlar)
19. [Cheat sheet](#19-cheat-sheet)

---

## 1. Umumiy manzara

`gcc main.c -o app` deb yozganda bitta buyruq ishlaganday ko'rinadi. Aslida `gcc` bu **driver** (dispetcher) dastur: u ichkarida bir nechta alohida dasturlarni ketma-ket chaqiradi.

```
 main.c                (source code)
   │
   │  1. PREPROCESSING      (cpp / cc1 -E)     gcc -E
   ▼
 main.i                (preprocessed source: #include, #define ochilgan)
   │
   │  2. COMPILATION        (cc1)              gcc -S
   ▼
 main.s                (assembly code)
   │
   │  3. ASSEMBLY           (as)               gcc -c
   ▼
 main.o                (object file: machine code, hali to'liq emas)
   │
   │  4. LINKING            (collect2 → ld)    gcc main.o ... -o app
   ▼
 app                   (executable file)
```

| Bosqich | Kirish | Chiqish | Kim bajaradi | Flag |
|---|---|---|---|---|
| Preprocessing | `.c` | `.i` | preprocessor (`cpp`) | `-E` |
| Compilation | `.i` | `.s` | compiler (`cc1`) | `-S` |
| Assembly | `.s` | `.o` | assembler (`as`) | `-c` |
| Linking | `.o` + libraries | executable | linker (`ld`) | (flagsiz) |

**Muhim qoida:** `-E`, `-S`, `-c` flag lari jarayonni **o'sha bosqichda to'xtatadi**.

### Translation unit

Preprocessing tugagach, bitta `.c` fayl va u `#include` qilgan hamma narsa birlashib **translation unit** deb ataladigan bitta katta matn bo'ladi. Compiler faqat translation unit ni ko'radi. U boshqa `.c` fayllardan bexabar. Bu fikr keyingi barcha bo'limlar uchun kalit.

### Ichki jarayonni ko'rish

Gcc aslida qaysi dasturlarni chaqirayotganini ko'rish uchun:

```bash
gcc -v main.c -o app
```

Chiqishda `cc1` (preprocessor + compiler), `as` (assembler), `collect2` (`ld` ni chaqiruvchi wrapper) qatorlarini ko'rasiz.

Barcha oraliq fayllarni saqlab qolish uchun:

```bash
gcc -save-temps main.c -o app
# main.i, main.s, main.o, app hosil bo'ladi
```

---

## 2. Namunaviy loyiha

Darslik davomida shu uchta fayldan foydalanamiz. Ularni yangi papkada yarating.

**`mathutils.h`**

```c
#ifndef MATHUTILS_H
#define MATHUTILS_H

extern int counter;          /* boshqa faylda e'lon qilingan global o'zgaruvchi */

int add(int a, int b);
int square(int x);

#endif
```

**`mathutils.c`**

```c
#include "mathutils.h"

int counter = 0;             /* global o'zgaruvchi: ta'rif (definition) */

int add(int a, int b)
{
    counter++;
    return a + b;
}

int square(int x)
{
    return x * x;
}
```

**`main.c`**

```c
#include <stdio.h>
#include "mathutils.h"

#define GREETING "Salom"

int main(void)
{
    int s = add(2, 3);
    printf("%s: sum=%d, square=%d, calls=%d\n", GREETING, s, square(s), counter);
    return 0;
}
```

Tez tekshiruv:

```bash
gcc main.c mathutils.c -o app
./app
# Salom: sum=5, square=25, calls=1
```

Endi shu bitta buyruq ichida nima bo'layotganini bosqichma-bosqich ochamiz.

---

## 3. Bosqich 1: Preprocessing (`-E`)

```bash
gcc -E main.c -o main.i
less main.i
```

### Preprocessor nima qiladi

Preprocessor **C tilini tushunmaydi**, u faqat matn bilan ishlaydi. Vazifalari:

1. **`#include`** — ko'rsatilgan faylni o'sha joyga aynan nusxalab qo'yadi.
2. **`#define`** — makroslarni matn sifatida almashtiradi.
3. **`#ifdef / #ifndef / #if / #else / #endif`** — shartli ravishda kodni qoldiradi yoki olib tashlaydi.
4. **Izohlarni (`/* */`, `//`) o'chiradi.**
5. Qator-davomi `\` larni birlashtiradi.

### `main.i` ni o'qish

Fayl juda uzun bo'ladi (bir necha yuz qator), chunki `<stdio.h>` ichidagi hamma narsa shu yerga kirgan. Oxiriga qarang:

```bash
tail -n 20 main.i
```

Taxminan quyidagini ko'rasiz:

```c
# 3 "mathutils.h" 1
extern int counter;
int add(int a, int b);
int square(int x);
# 4 "main.c" 2

int main(void)
{
    int s = add(2, 3);
    printf("%s: sum=%d, square=%d, calls=%d\n", "Salom", s, square(s), counter);
    return 0;
}
```

E'tibor bering:

- `GREETING` o'rniga `"Salom"` yozilgan (makros almashgan).
- `#define`, `#include` qatorlari yo'qolgan.
- `# 3 "mathutils.h" 1` kabi qatorlar **linemarker** deb ataladi. Ular compiler ga xato xabarida qaysi fayl va qator ko'rsatishni aytadi.

### Makroslar: `-D` flag

Makrosni kod ichiga yozmasdan, buyruq qatoridan berish mumkin:

```bash
gcc -E -DGREETING='"Hello"' main.c | tail -n 5
```

Bu `Makefile` da `-DDEBUG` yoki `-DVERSION=2` kabi ko'p uchraydi. Quyidagi kod bilan sinab ko'ring:

```c
#ifdef DEBUG
    printf("debug: s=%d\n", s);
#endif
```

```bash
gcc -E main.c          | grep debug    # hech narsa chiqmaydi
gcc -E -DDEBUG main.c  | grep debug    # qator paydo bo'ladi
```

### Makroslar xavfi (qavslar)

Preprocessor faqat matnni almashtiradi, shuning uchun:

```c
#define SQUARE(x) x * x
int r = SQUARE(2 + 3);      /* 2 + 3 * 2 + 3 = 11, 25 emas! */
```

To'g'risi:

```c
#define SQUARE(x) ((x) * (x))
```

Buni `gcc -E` bilan o'zingiz tekshirib ko'ring: nima xato ketganini darhol ko'rasiz.

### Include qidiruv yo'llari

```c
#include <stdio.h>       /* <...> : avval -I papkalar, keyin tizim papkalari */
#include "mathutils.h"   /* "..." : avval shu fayl turgan papka, keyin -I, keyin tizim */
```

`-I` flag qo'shimcha qidiruv papkasini beradi:

```bash
gcc -Iinclude -c main.c       # include/ papkasidan ham qidiradi
```

Gcc qaysi papkalarda qidirishini ko'rish:

```bash
echo | gcc -E -Wp,-v -x c - 2>&1 | less
```

### Foydali: dependency ro'yxati

```bash
gcc -M  main.c      # main.c bog'liq bo'lgan BARCHA header lar (tizimnikilar ham)
gcc -MM main.c      # faqat sizning header laringiz
```

Chiqish:

```
main.o: main.c mathutils.h
```

Bu aynan `Makefile` qoidasi formati. 15-bo'limda bundan qanday foydalanishni ko'ramiz.

---

## 4. Bosqich 2: Compilation (`-S`)

```bash
gcc -S main.c -o main.s
cat main.s
```

### Compiler nima qiladi

Preprocessed C kodini **assembly** tiliga aylantiradi. Ichkarida:

1. **Lexical analysis** — matnni token larga bo'ladi (`int`, `main`, `(`, ...).
2. **Parsing** — token lardan sintaksis daraxti (AST) quradi. **Syntax error** lar shu yerda chiqadi.
3. **Semantic analysis** — tur tekshiruvi, o'zgaruvchi e'lon qilinganmi va h.k.
4. **Optimization** — kodni tezroq/kichikroq qiladi (`-O` flag lari).
5. **Code generation** — mashina arxitekturasiga (x86-64, ARM, ...) mos assembly chiqaradi.

### `main.s` ni o'qish

Default holatda AT&T sintaksisi chiqadi. Intel sintaksisini xohlasangiz:

```bash
gcc -S -masm=intel -fno-asynchronous-unwind-tables main.c -o main.s
```

Ikkinchi flag `.cfi_*` direktivalarini olib tashlab, o'qishni osonlashtiradi. Taxminiy natija:

```asm
        .intel_syntax noprefix
        .section .rodata
.LC0:
        .string "Salom"
.LC1:
        .string "%s: sum=%d, square=%d, calls=%d\n"
        .text
        .globl  main
main:
        push    rbp
        mov     rbp, rsp
        sub     rsp, 16
        mov     esi, 3
        mov     edi, 2
        call    add
        mov     DWORD PTR [rbp-4], eax
        ...
        call    square
        ...
        call    printf
```

Diqqat qiling:

- `call add`, `call square`, `call printf` — compiler bu funksiyalar **qayerda** ekanini bilmaydi. U faqat "shu nomdagi funksiyani chaqir" deb yozib qo'yadi. Manzilni keyinroq **linker** topadi.
- `.LC0`, `.LC1` — string literal lar `.rodata` (read-only data) bo'limiga tushgan.
- `main` — `.globl` bilan belgilangan, ya'ni boshqa fayllarga ko'rinadi.

Assembly ni o'qishni yaxshi bilish uchun sizning Assembly (NASM) darslaringiz bevosita yordam beradi.

### Nega compiler `add` funksiyasini bilmasa ham xato bermadi?

Chunki `mathutils.h` dagi **prototype** (`int add(int a, int b);`) unga funksiya *mavjud ekanligini* va *imzosini* aytdi. Ta'rif (body) boshqa translation unit da. Buni sinab ko'rish uchun `#include "mathutils.h"` qatorini vaqtincha o'chirib compile qiling:

```bash
gcc -c main.c
```

Zamonaviy GCC (14 va undan yuqori) `implicit declaration of function 'add'` ni **xato** (error) deb beradi, eski versiyalar ogohlantirish (warning) bergan. Bu **compilation** bosqichidagi xato.

---

## 5. Bosqich 3: Assembly (`-c`)

```bash
gcc -c main.c -o main.o
gcc -c mathutils.c -o mathutils.o
file main.o
# main.o: ELF 64-bit LSB relocatable, x86-64, ...
```

### Assembler nima qiladi

`.s` faylni **mashina kodiga** (binary) aylantiradi va **object file** hosil qiladi. Linux da format **ELF** (Executable and Linkable Format).

Muhim so'z: **relocatable**. Object fayl hali *to'liq emas*:

- Uning ichidagi kod manzillari 0 dan boshlanadi (haqiqiy manzil hali noma'lum).
- `add`, `square`, `printf` chaqiruvlari uchun manzil joylari **bo'sh qoldirilgan**.
- Bo'sh joylar ro'yxati (**relocation entries**) faylda saqlanadi, linker ularni to'ldiradi.

`./main.o` ni ishga tushirib bo'lmaydi:

```bash
./main.o
# Permission denied / cannot execute binary file
```

### ELF bo'limlari (sections)

| Section | Nima saqlaydi |
|---|---|
| `.text` | mashina kodi (funksiyalar) |
| `.data` | boshlang'ich qiymati **nolga teng bo'lmagan** global/static o'zgaruvchilar |
| `.bss` | boshlang'ich qiymati nol yoki berilmagan global/static o'zgaruvchilar (faylda joy egallamaydi) |
| `.rodata` | o'qish uchungina ma'lumot: string literal, `const` global lar |
| `.symtab` | **symbol table** (funksiya va o'zgaruvchi nomlari) |
| `.strtab` | symbol nomlari matni |
| `.rela.text` | **relocation** yozuvlari |

---

## 6. Object fayl ichida: nm, objdump, readelf

Bu uchta vosita — linker xatolarini tushunishning asosiy quroli.

### 6.1 `nm` — symbol table

```bash
nm mathutils.o
```

```
0000000000000000 T add
0000000000000000 B counter
000000000000001d T square
```

```bash
nm main.o
```

```
                 U add
                 U counter
0000000000000000 T main
                 U printf
                 U square
```

**Harflar ma'nosi:**

| Harf | Ma'nosi |
|---|---|
| `T` | `.text` da **aniqlangan** (defined), global funksiya |
| `t` | `.text` da aniqlangan, **local** (`static`) funksiya |
| `U` | **Undefined** — bu faylda ishlatilgan, lekin ta'rifi boshqa joyda |
| `D` | `.data` da aniqlangan global o'zgaruvchi |
| `B` | `.bss` da aniqlangan global o'zgaruvchi |
| `R` | `.rodata` (read-only) da aniqlangan |
| `W` | weak symbol |

Katta harf = **global** (boshqa fayllarga ko'rinadi), kichik harf = **local**.

**Nazorat savoli:** `main.o` dagi `U add` va `mathutils.o` dagi `T add` juftligi nimani anglatadi? — `main.o` "menga `add` kerak" deydi, `mathutils.o` "menda `add` bor" deydi. **Linker vazifasi aynan shu juftliklarni topib bog'lash.**

### 6.2 `objdump` — disassembly

```bash
objdump -d main.o                 # kodni disassemble qilish
objdump -d -M intel main.o        # Intel sintaksisi
objdump -h main.o                 # section lar ro'yxati
objdump -s -j .rodata main.o      # .rodata ning xom baytlari
```

`objdump -dr main.o` (`-r` — relocation ham ko'rsatadi):

```
  1a:   e8 00 00 00 00          call   1f <main+0x1f>
                        1b: R_X86_64_PLT32      add-0x4
```

`e8 00 00 00 00` — `call` buyrug'i, **manzil qismi nol** (hali noma'lum). Ostidagi `R_X86_64_PLT32 add` esa: "linker, bu joyga `add` manzilini yoz" degan ko'rsatma.

### 6.3 `readelf` — ELF tuzilmasi

```bash
readelf -h main.o        # ELF header
readelf -S main.o        # section lar
readelf -s main.o        # symbol table (nm dan batafsilroq)
readelf -r main.o        # relocation yozuvlari
```

---

## 7. Bosqich 4: Linking

```bash
gcc main.o mathutils.o -o app
./app
```

### Linker nima qiladi

1. **Symbol resolution** — har bir `U` (undefined) symbol uchun boshqa object fayl yoki kutubxonada `T`/`D`/`B` (defined) symbol topadi. Topa olmasa — `undefined reference` xatosi.
2. **Section merging** — barcha object fayllarning `.text` larini bitta `.text` ga, `.data` larini bitta `.data` ga va h.k. birlashtiradi.
3. **Relocation** — yakuniy manzillar aniq bo'lgach, `call add` dagi bo'sh manzil joylarini to'ldiradi.
4. **Startup kod qo'shish** — `_start`, `crt1.o`, `crti.o`... Dastur aslida `main` dan emas, `_start` dan boshlanadi; u libc ni sozlab, keyin `main` ni chaqiradi.
5. **Libraries bilan bog'lash** — default holda `libc` avtomatik ulanadi.

Shuni tekshiring:

```bash
nm app | grep -E ' (add|square|main|_start|counter)$'
```

Endi `add` va `square` da `U` yo'q, ularning aniq manzillari bor.

```bash
objdump -d -M intel app | grep -A12 '<main>:'
```

Endi `call` yonida haqiqiy manzil turibdi.

### 7.1 Tipik linker xatolari

**(a) `undefined reference to 'square'`**

`mathutils.o` ni bermasdan link qilib ko'ring:

```bash
gcc main.o -o app
# undefined reference to `add'
# undefined reference to `square'
# undefined reference to `counter'
```

Sabab: `main.o` da `U add`, lekin hech qaysi berilgan faylda `T add` yo'q. **Bu compile xatosi emas, link xatosi**, chunki `.c → .o` muvaffaqiyatli o'tgan.

**(b) `multiple definition of 'counter'`**

Agar `mathutils.h` ichiga `int counter = 0;` (extern siz) yozib, uni ikkala `.c` fayl include qilsa, ikkala `.o` da ham `B counter` bo'ladi. Linker qaysi birini tanlashni bilmaydi:

```
multiple definition of `counter'; mathutils.o: first defined here
```

**Qoida:** header da faqat **e'lon** (declaration): `extern int counter;`, funksiya prototype lari. **Ta'rif** (definition) faqat bitta `.c` faylda.

**(c) `undefined reference to 'main'`**

Hech bir faylda `main` yo'q (yoki `-c` bilan compile qilib, link qilishni unutgansiz).

**(d) `-lm` yetishmayotgani**

`sqrt`, `pow`, `sin` kabi `<math.h>` funksiyalari `libm` da:

```c
#include <math.h>
#include <stdio.h>
int main(int argc, char **argv) { printf("%f\n", sqrt(argc)); return 0; }
```

```bash
gcc m.c -o m        # undefined reference to `sqrt'  (ko'p tizimlarda)
gcc m.c -o m -lm    # ishlaydi
```

(Argument compile vaqtida ma'lum konstanta bo'lsa, compiler `sqrt` ni o'zi hisoblab qo'yishi mumkin. Shuning uchun bu yerda `argc` ishlatildi.)

### 7.2 Linkerni bevosita ko'rish

```bash
gcc -Wl,--verbose main.o mathutils.o -o app | head -50   # linker script
gcc -Wl,-Map=app.map main.o mathutils.o -o app           # xarita fayli
```

`-Wl,<opsiya>` — "shu opsiyani linker ga uzat" degani.

---

## 8. Separate compilation: nega har bir `.c` fayl alohida compile qilinadi

Ikki yondashuv:

```bash
# A) Hammasi birdan
gcc main.c mathutils.c -o app

# B) Alohida bosqichlar
gcc -c main.c
gcc -c mathutils.c
gcc main.o mathutils.o -o app
```

Natija bir xil, lekin B ning afzalligi:

- 100 ta `.c` faylli loyihada bitta faylni o'zgartirsangiz, **faqat o'shani** qayta compile qilasiz (`.o` lar tayyor), so'ng link qilasiz.
- Aynan shu g'oya `make` ning asosi: `make` fayl o'zgarish vaqtini (timestamp) solishtirib, qaysi `.o` eskirganini aniqlaydi.

**Sinab ko'ring:**

```bash
touch mathutils.c        # faqat mathutils.c yangilandi
# faqat shuni qayta compile qilish yetarli:
gcc -c mathutils.c && gcc main.o mathutils.o -o app
```

---

## 9. Libraries: static va dynamic

Kutubxona — bir nechta object fayl to'plami.

### 9.1 Static library (`.a`)

Static library — `.o` fayllar **arxivi**. Link vaqtida kerakli `.o` lar arxivdan **executable ichiga nusxalab qo'yiladi**.

```bash
gcc -c mathutils.c
ar rcs libmathutils.a mathutils.o     # arxiv yaratish
ar t libmathutils.a                   # ichidagi fayllar ro'yxati
nm libmathutils.a                     # ichidagi symbol lar
```

`ar rcs` dagi harflar: `r` — fayllarni qo'sh/almashtir, `c` — arxiv yo'q bo'lsa jimgina yarat, `s` — symbol index yoz (`ranlib` bilan bir xil).

Ishlatish:

```bash
gcc main.c -L. -lmathutils -o app_static
```

- `-L.` — kutubxonani **qayerdan** qidirish (joriy papka).
- `-lmathutils` — `libmathutils.a` yoki `libmathutils.so` ni qidir (`lib` prefiksi va kengaytma **yozilmaydi**).

Sizning loyihangizdagi `s21_string.a` aynan shunday kutubxona. Uning `Makefile` qoidasi:

```make
s21_string.a: $(OBJ)
	ar rcs $@ $^
```

### 9.2 Dynamic (shared) library (`.so`)

Dynamic library executable ichiga **nusxalanmaydi**. Dastur ishga tushganda **dynamic loader** (`ld.so`) uni xotiraga yuklaydi. Ko'p dastur bitta `.so` ni umumiy ishlatadi.

```bash
gcc -fPIC -c mathutils.c                      # position-independent code
gcc -shared -o libmathutils.so mathutils.o    # shared library
gcc main.c -L. -lmathutils -o app_dyn
./app_dyn
# error while loading shared libraries: libmathutils.so: cannot open shared object file
```

Bu xato **runtime** xatosi: link o'tdi, lekin dastur ishga tushganda loader `.so` ni topa olmadi. Yechimlar:

```bash
# 1) Vaqtincha:
LD_LIBRARY_PATH=. ./app_dyn

# 2) Executable ichiga yo'lni yozib qo'yish (rpath):
gcc main.c -L. -lmathutils -Wl,-rpath,'$ORIGIN' -o app_dyn
./app_dyn

# 3) Doimiy: .so ni /usr/local/lib ga qo'yib, `sudo ldconfig`
```

Qaysi `.so` lar kerakligini ko'rish:

```bash
ldd app_dyn
#   libmathutils.so => ./libmathutils.so
#   libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6
```

**`-fPIC`** nima? Shared library xotirada **turli manzillarga** yuklanishi mumkin, shuning uchun kod manzillarga qat'iy bog'lanmasligi (position-independent) kerak.

### 9.3 Static vs Dynamic

| | Static (`.a`) | Dynamic (`.so`) |
|---|---|---|
| Link vaqtida | kod nusxalanadi | faqat havola qoladi |
| Executable hajmi | katta | kichik |
| Tarqatish | bitta fayl yetadi | `.so` ham kerak |
| Kutubxona yangilanishi | qayta link kerak | `.so` ni almashtirish yetarli |
| Xotira | har dastur o'z nusxasida | umumiy ishlatiladi |
| Xato turi | link vaqtida | link yoki **runtime** da |

Agar `.a` va `.so` ikkalasi ham bor bo'lsa, `-l` default holda **`.so` ni afzal ko'radi**. To'liq statik link:

```bash
gcc -static main.c -L. -lmathutils -o app_full_static
```

### 9.4 Link tartibi muhim!

Static kutubxonalarda linker fayllarni **chapdan o'ngga** ko'radi. U kutubxonadan faqat *shu paytgacha kerak bo'lib qolgan* symbol larni oladi.

```bash
gcc -L. -lmathutils main.c -o app     # XATO bo'lishi mumkin: undefined reference
gcc main.c -L. -lmathutils -o app     # TO'G'RI
```

**Qoida:** object fayllar avval, kutubxonalar keyin (`-lm`, `-lcheck` ham shunday). Agar kutubxonalar bir-biriga bog'liq bo'lsa (`A` `B` ni ishlatadi), `A` ni `B` dan oldin yozing.

Bu Check kutubxonasi bilan test yozganda muhim:

```make
$(CC) $(TEST_OBJ) s21_string.a -o test -lcheck -lm -lpthread -lrt -lsubunit
```

Kutubxonalar **oxirida**. (Kerakli kutubxonalar to'plami tizimga qarab farq qiladi.)

---

## 10. Header fayllar va include mexanizmi

### 10.1 Header ichida nima bo'lishi kerak

**Ha:** funksiya prototype lari, `extern` o'zgaruvchi e'lonlari, `typedef`, `struct` tavsiflari, `#define` makroslar, `static inline` funksiyalar.

**Yo'q:** oddiy funksiya body si, `extern` siz global o'zgaruvchi ta'rifi (aks holda `multiple definition`).

### 10.2 Include guard

```c
#ifndef MATHUTILS_H
#define MATHUTILS_H
/* ... */
#endif
```

Nega kerak? Agar `a.h` ni `b.h` ham, `main.c` ham include qilsa, `a.h` mazmuni ikki marta kiradi, natijada `struct` ni ikki marta ta'riflash xatosi (`redefinition of 'struct ...'`) chiqadi. Guard shuni oldini oladi. Muqobili (deyarli hamma kompilyator qo'llaydi, lekin standartda emas): `#pragma once`.

### 10.3 Header o'zgarsa nima bo'ladi?

`mathutils.h` ni o'zgartirsangiz (masalan, `add` imzosini), `main.o` **eskirgan** bo'ladi, chunki u shu header asosida compile qilingan. Lekin oddiy `main.o: main.c` qoidasi bilan `make` buni **sezmaydi**! Yechim — 15-bo'lim.

---

## 11. Warning flag lari

Compiler ko'p xatoni **ruxsat beradi**, lekin ogohlantiradi. Doim yoqib qo'ying:

| Flag | Nima qiladi |
|---|---|
| `-Wall` | asosiy ogohlantirishlar to'plami (aslida "hammasi" emas) |
| `-Wextra` | qo'shimcha ogohlantirishlar (ishlatilmagan parametr, signed/unsigned solishtirish...) |
| `-Werror` | har bir warning ni **xatoga** aylantiradi |
| `-pedantic` | standartdan chetlanishlarni ko'rsatadi |
| `-std=c11` | til standartini tanlaydi (`c99`, `c11`, `c17`, `gnu11`) |
| `-Wshadow` | o'zgaruvchi boshqasini "yopib" qo'yganda |
| `-Wconversion` | ma'lumot yo'qotadigan konversiyalar |

Tavsiya etilgan boshlang'ich to'plam:

```bash
gcc -std=c11 -Wall -Wextra -Werror -pedantic -c main.c
```

**Misol.** Quyidagi kodni compile qiling:

```c
#include <stdio.h>
int main(void)
{
    int x;
    printf("%d\n", x);        /* x initsializatsiya qilinmagan */
    return 0;
}
```

```bash
gcc -Wall -Wextra x.c
# warning: 'x' is used uninitialized
```

Flagsiz jimgina o'tib ketardi. Ogohlantirish qanday **chuqurroq** bo'lishi (`-O2` bilan flow analysis kuchayadi) ham muhim: ba'zi warning lar faqat optimization yoqilganda chiqadi.

---

## 12. Debug va Sanitizers

### 12.1 `-g` — debug info

```bash
gcc -g -O0 -c main.c
gdb ./app
```

`-g` executable ga funksiya/o'zgaruvchi nomlari va qator raqamlarini yozadi (kod tezligiga ta'sir qilmaydi, faqat fayl kattalashadi). `-O0` bilan birga ishlating: optimizatsiya kodni qayta tartiblab, debug qilishni qiyinlashtiradi.

Debug uchun mo'ljallangan optimizatsiya darajasi: `-Og`.

### 12.2 Sanitizers

Compiler dastur ichiga **tekshiruv kodi** qo'shadi. Runtime da xato topilsa, aniq joyni ko'rsatadi.

```bash
gcc -g -fsanitize=address,undefined -fno-omit-frame-pointer main.c mathutils.c -o app_san
./app_san
```

- **AddressSanitizer (`address`)** — buffer overflow, use-after-free, memory leak.
- **UndefinedBehaviorSanitizer (`undefined`)** — signed integer overflow, null pointer dereference, noto'g'ri shift.

**Misol:**

```c
#include <stdlib.h>
int main(void)
{
    int *a = malloc(3 * sizeof(int));
    a[3] = 10;             /* chegaradan tashqari yozish */
    free(a);
    return 0;
}
```

```bash
gcc -g -fsanitize=address x.c -o x && ./x
# ERROR: AddressSanitizer: heap-buffer-overflow ...
```

**Muhim:** sanitizer flag ini compile **va** link bosqichlarida ham berish kerak. Makefile da `CFLAGS` va `LDFLAGS` ikkalasiga qo'shing.

Muqobil vosita: `valgrind ./app` (recompile talab qilmaydi, lekin sekinroq).

---

## 13. Optimization

| Flag | Ma'nosi |
|---|---|
| `-O0` | optimizatsiyasiz (default). Tez compile, debug qulay |
| `-O1` | oddiy optimizatsiyalar |
| `-O2` | tavsiya etilgan release darajasi |
| `-O3` | agressiv (loop unrolling, vektorlashtirish); kod hajmi ortishi mumkin |
| `-Os` | hajm bo'yicha optimizatsiya |
| `-Og` | debug uchun ma'qul optimizatsiya |

### Assembly farqini ko'rish

```c
/* opt.c */
int sum(int n)
{
    int s = 0;
    for (int i = 1; i <= n; i++)
        s += i;
    return s;
}
```

```bash
gcc -S -O0 -masm=intel -fno-asynchronous-unwind-tables opt.c -o opt0.s
gcc -S -O2 -masm=intel -fno-asynchronous-unwind-tables opt.c -o opt2.s
wc -l opt0.s opt2.s
```

`-O2` da ko'pincha loop o'rniga Gauss formulasiga o'xshash yopiq shakl chiqadi. Compiler `O(n)` sikl o'rnini `O(1)` ifoda bilan almashtirishi mumkin. Bu **compiler nima qila olishini** ko'rsatuvchi yaxshi misol.

### Optimization va "noaniq xatti-harakat"

Undefined behavior (UB) bor kodda `-O2` kutilmagan natija berishi mumkin. Masalan, `signed` overflow UB, shuning uchun compiler `x + 1 > x` shartini har doim `true` deb hisoblashi mumkin. `-O0` da ishlagan, `-O2` da buzilgan kod ko'pincha UB belgisidir. Bunday holatda `-fsanitize=undefined` yordam beradi.

---

## 14. Coverage: gcov va lcov

Coverage — testlar kodning **qaysi qatorlarini** ishga tushirganini o'lchash. Sizning `Makefile` dagi `gcov_report` maqsadi aynan shu.

### 14.1 Jarayon

```bash
gcc --coverage -O0 -g main.c mathutils.c -o app_cov
```

`--coverage` (= `-fprofile-arcs -ftest-coverage` va link vaqtida `-lgcov`) ikki ish qiladi:

1. Compile paytida `.gcno` fayl yaratadi (kod tuzilmasi haqida).
2. Kodga hisoblagichlar qo'shadi; dastur ishlaganda `.gcda` fayl yaratiladi (nechta marta bajarilgani).

```bash
./app_cov              # .gcda fayllar hosil bo'ladi
gcov main.c mathutils.c
cat mathutils.c.gcov
```

`.gcov` faylda har bir qator oldida bajarilish soni turadi (`#####` = hech qachon bajarilmagan).

### 14.2 lcov — HTML hisobot

```bash
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory report
# brauzerda report/index.html ni oching
```

### 14.3 Makefile da

Muhim nuqta: coverage flag lari **`.o` fayllarni compile qilganda ham, link qilganda ham** kerak. Shuning uchun ko'pincha coverage uchun alohida `CFLAGS` ishlatiladi:

```make
gcov_report: CFLAGS += --coverage
gcov_report: LDFLAGS += --coverage
gcov_report: clean test
	lcov --capture --directory . --output-file coverage.info
	genhtml coverage.info --output-directory report
```

Coverage bilan `-O0` ishlating: optimizatsiya qatorlarni birlashtirib, hisobotni chalkashtiradi.

---

## 15. `-MMD -MP`: avtomatik dependency generatsiya

**Muammo.** Quyidagi qoida bilan:

```make
main.o: main.c
	gcc -c main.c
```

`mathutils.h` o'zgarganda `main.o` qayta compile **qilinmaydi**, chunki `make` header ni bilmaydi. Natija: eski `main.o` yangi header bilan mos kelmay, tushunish qiyin xatolar (yoki jimgina noto'g'ri ishlash).

**Yechim.** Compiler o'zi dependency larni topib, `.d` fayl yozsin:

```bash
gcc -MMD -MP -c main.c
cat main.d
```

```make
main.o: main.c mathutils.h

mathutils.h:
```

- `-MMD` — compile bilan birga `.d` fayl yaratadi (tizim header larisiz).
- `-MP` — har bir header uchun bo'sh "phony" qoida qo'shadi. Header o'chirib yuborilsa `make` "No rule to make target 'x.h'" xatosi bermaydi.

Makefile da ulash:

```make
CFLAGS += -MMD -MP
-include $(OBJ:.o=.d)
```

`-include` (tire bilan) — fayl mavjud bo'lmasa xato bermaydi (birinchi build da `.d` lar hali yo'q).

---

## 16. Xato qaysi bosqichdan chiqdi?

Xato xabaridan bosqichni aniqlash — debug tezligi uchun asosiy ko'nikma.

| Xabar | Bosqich | Odatiy sabab |
|---|---|---|
| `fatal error: xxx.h: No such file or directory` | Preprocessing | header yo'q yoki `-I` unutilgan |
| `error: expected ';' before ...` | Compilation (parsing) | sintaksis xatosi |
| `error: 'x' undeclared` | Compilation (semantic) | o'zgaruvchi e'lon qilinmagan |
| `implicit declaration of function` | Compilation | prototype yo'q / header include qilinmagan |
| `warning: ... [-Wall]` | Compilation | shubhali kod |
| `undefined reference to 'f'` | **Linking** | `.o` yoki kutubxona berilmagan, `-l` unutilgan, tartib xato |
| `multiple definition of 'x'` | **Linking** | ta'rif header da yoki ikki `.c` da |
| `cannot find -lxxx` | **Linking** | kutubxona topilmadi (`-L` unutilgan yoki o'rnatilmagan) |
| `error while loading shared libraries` | **Runtime (loader)** | `.so` topilmadi (`LD_LIBRARY_PATH`/rpath) |
| `Segmentation fault` | Runtime | xotira xatosi (sanitizer/gdb ishlating) |

**Oltin qoida:** `.c → .o` bosqichida chiqqan xato — sizning kodingizdagi (yoki header dagi) muammo. `.o → executable` bosqichida chiqqan xato esa — **fayllar/kutubxonalar ro'yxati** haqidagi muammo.

---

## 17. GCC va Makefile: to'liq bog'lanish

### 17.1 Tarjima jadvali

| Qo'lda gcc buyrug'i | Makefile ifodasi |
|---|---|
| `gcc` | `$(CC)` |
| `-Wall -Wextra -std=c11` | `$(CFLAGS)` |
| `-I`, `-D` | odatda `$(CPPFLAGS)` (preprocessor uchun) |
| `-L`, `-fsanitize`, `--coverage` (link uchun) | `$(LDFLAGS)` |
| `-lm`, `-lcheck` | `$(LDLIBS)` |
| `gcc -c x.c -o x.o` | `%.o: %.c` → `$(CC) $(CFLAGS) -c $< -o $@` |
| `gcc a.o b.o -o app` | `app: a.o b.o` → `$(CC) $^ -o $@ $(LDLIBS)` |
| `ar rcs lib.a a.o b.o` | `lib.a: a.o b.o` → `ar rcs $@ $^` |

Avtomatik o'zgaruvchilar: `$@` — target, `$<` — birinchi prerequisite, `$^` — barcha prerequisite lar.

### 17.2 Namunaviy loyiha uchun Makefile

> Diqqat: recipe qatorlari **Tab** bilan boshlanishi shart.

```make
CC       = gcc
CFLAGS   = -std=c11 -Wall -Wextra -Werror -pedantic -MMD -MP
LDFLAGS  =
LDLIBS   =

SRC = main.c mathutils.c
OBJ = $(SRC:.c=.o)
DEP = $(OBJ:.o=.d)

all: app

app: $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEP)

clean:
	rm -f $(OBJ) $(DEP) app

.PHONY: all clean
```

Bu Makefile qo'lda bajargan barcha bosqichlaringizni avtomatlashtiradi:

```
make   ─►  gcc ... -c main.c     -o main.o       (bosqich 1+2+3)
       ─►  gcc ... -c mathutils.c -o mathutils.o  (bosqich 1+2+3)
       ─►  gcc main.o mathutils.o -o app          (bosqich 4)
```

Qayta `make` yozsangiz: "make: Nothing to be done for 'all'." Faqat `mathutils.c` ni o'zgartirsangiz, faqat `mathutils.o` qayta compile bo'ladi va keyin link.

### 17.3 Static library bilan variant

```make
LIB = libmathutils.a
LIBOBJ = mathutils.o

$(LIB): $(LIBOBJ)
	ar rcs $@ $^

app: main.o $(LIB)
	$(CC) main.o -L. -lmathutils -o $@
```

### 17.4 Sinov: `make -n` va `make -B`

```bash
make -n        # nima bajarilishini ko'rsatadi, bajarmaydi
make -B        # hamma narsani majburan qayta build qiladi
make V=1       # (agar Makefile qo'llasa) batafsil chiqish
```

`make -n` orqali Makefile ishlatayotgan **haqiqiy gcc buyruqlarini** ko'rib, ularni shu darslikdagi bosqichlar bilan solishtiring.

---

## 18. Mashqlar

Har bir mashqni bajarib, natijani o'zingiz tekshiring.

**1-mashq (pipeline).** `main.c` ni `-save-temps` bilan compile qiling. `main.i` necha qatordan iborat? Nega `main.c` dan ancha katta?

**2-mashq (makros).** `#define SQUARE(x) x * x` yozing, `SQUARE(2 + 3)` ni `gcc -E` bilan oching va natijani tushuntiring. Keyin uni to'g'rilang.

**3-mashq (nm).** `nm` bilan `main.o` va `mathutils.o` ni solishtiring. `U` va `T` juftliklarini toping. `counter` `mathutils.o` da qaysi harf bilan turibdi? Nega `B`? Agar `int counter = 5;` deb o'zgartirsangiz harf nima bo'ladi?

**4-mashq (linker xatosi).** Ataylab uchta xato yarating: (a) `mathutils.o` ni link ga bermang, (b) ta'rifni header ga ko'chirib, ikkala `.c` da include qiling, (c) `sqrt` ni `-lm` siz ishlating. Har birining xabarini o'qib, qaysi bosqich ekanini ayting.

**5-mashq (relocation).** `objdump -dr main.o` va `objdump -d app` natijalarini solishtiring. `call add` yonidagi baytlar qanday o'zgardi?

**6-mashq (libraries).** `libmathutils.a` va `libmathutils.so` ni yarating. Ikkalasiga qarshi `main.c` ni link qiling. `ls -l` bilan executable hajmini solishtiring. `ldd` bilan farqni ko'ring. `.so` ni boshqa papkaga ko'chirsangiz nima bo'ladi?

**7-mashq (link tartibi).** `gcc -L. -lmathutils main.c` va `gcc main.c -L. -lmathutils` ni statik kutubxona bilan sinab, farqni tushuntiring.

**8-mashq (header dependency).** 17.2 dagi Makefile dan `-MMD -MP` va `-include $(DEP)` ni olib tashlang. `make`, keyin `mathutils.h` ga bo'sh qator qo'shing va yana `make` qiling: nima bo'ldi? Keyin ularni qaytarib qo'yib, farqni ko'ring.

**9-mashq (optimization).** `sum(n)` funksiyasini `-O0` va `-O2` da assembly ga o'tkazing. Nechta qator? Sikl saqlanib qoldimi?

**10-mashq (sanitizer).** 12.2 dagi `a[3] = 10;` xatosini `-fsanitize=address` bilan ushlang. Xabarning qaysi qismi xato qatorini ko'rsatayotganini toping.

**11-mashq (coverage).** `add` va `square` ni testlaydigan kichik `test.c` yozing. `square` ni chaqirmasangiz, `gcov` da uning qatorlari `#####` bo'lishini tekshiring.

**12-mashq (yakuniy).** O'z loyihangizdagi (`s21_string`) Makefile ning har bir qoidasi uchun: qo'lda yozganda qaysi `gcc`/`ar` buyrug'iga teng ekanini, va qaysi **bosqichni** bajarayotganini yozib chiqing.

---

## 19. Cheat sheet

```bash
# --- Bosqichlar ---
gcc -E  x.c -o x.i          # preprocess
gcc -S  x.c -o x.s          # -> assembly
gcc -c  x.c -o x.o          # -> object
gcc a.o b.o -o app          # link
gcc -save-temps x.c         # barcha oraliq fayllarni saqla
gcc -v x.c                  # ichki dasturlarni ko'rsat

# --- Tekshirish ---
nm x.o                      # symbol lar (T, U, B, D, R, t)
objdump -d -M intel x.o     # disassembly
objdump -dr x.o             # + relocation
readelf -S x.o              # section lar
readelf -s x.o              # symbol table
ldd app                     # dynamic bog'lanishlar
file x.o                    # fayl turi

# --- Preprocessor ---
-Iinclude   -DNAME   -DNAME=val   -UNAME
gcc -MM x.c                 # dependency ro'yxati
gcc -MMD -MP -c x.c         # x.d generatsiya

# --- Kutubxona ---
ar rcs libx.a a.o b.o                     # static
gcc -fPIC -c a.c && gcc -shared -o libx.so a.o   # shared
gcc main.c -L. -lx -o app                 # ishlatish (libs OXIRIDA)
gcc ... -Wl,-rpath,'$ORIGIN'              # runtime yo'lini yozish
LD_LIBRARY_PATH=. ./app                   # vaqtincha yo'l

# --- Sifat ---
-std=c11 -Wall -Wextra -Werror -pedantic
-g -O0                                    # debug
-O2                                       # release
-fsanitize=address,undefined              # compile va link da!
--coverage                                # compile va link da!
```

### Yodda tuting

1. **Compiler faqat bitta translation unit ni ko'radi.** Boshqa fayllardagi narsalarni prototype orqali "ishonib" qabul qiladi.
2. **Linker `U` ni `T`/`D`/`B` bilan juftlaydi.** `undefined reference` = juft topilmadi. `multiple definition` = juft bittadan ko'p.
3. **Object fayl to'liq emas** (relocatable). Manzillar faqat link vaqtida aniqlanadi.
4. **Kutubxonalar oxirida.** Link tartibi muhim.
5. **`make` = gcc buyruqlarini timestamp asosida avtomatlashtirish.** Header dependency larni `-MMD -MP` bilan avtomatlashtiring.
6. **Xato xabaridan bosqichni aniqlang**, keyin nimani tekshirishni biling.

---

*Keyingi qadam: 17-bo'limdagi Makefile ni o'zingiz noldan yozib chiqing, so'ng o'z `s21_string` loyihangizning Makefile'ini shu darslik nuqtai nazaridan qayta o'qing: har bir satr endi tanish bo'lishi kerak.*
