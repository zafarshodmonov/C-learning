# C dasturlash tilida `struct` — To'liq darslik

## Mundarija

1. Struct nima va nega kerak?
2. Struct e'lon qilish va ishlatish
3. Struct xotirada qanday joylashadi (memory layout, padding, alignment)
4. Nested struct (struct ichida struct)
5. Struct massivlari
6. Struct'ga pointer
7. Struct'larni funksiyaga uzatish
8. `typedef` bilan ishlash
9. Self-referential struct (o'z-o'ziga ishora qiluvchi struct) — Linked List asosi
10. Bit-fields
11. `struct` vs `union`
12. Struct ichida funksiya pointerlari (C'da "OOP" taqlidi)
13. C++ bilan taqqoslash (`struct` vs `class`)
14. Xulosa va amaliy mashqlar

---

## 1. Struct nima va nega kerak?

C tilida bizda ibtidoiy (primitive) tiplar bor: `int`, `char`, `float`, `double` va h.k. Lekin haqiqiy dunyoda ma'lumotlar ko'pincha **guruh** holida keladi. Masalan, bitta talabani tasvirlash uchun bizga kerak:

- ismi (`char[]`)
- yoshi (`int`)
- GPA'si (`float`)

Bularni alohida-alohida o'zgaruvchilar sifatida saqlashimiz mumkin:

```c
char name[50];
int age;
float gpa;
```

Lekin bu yondashuv 100 ta talaba bo'lsa, chalkash bo'lib ketadi — qaysi `name` qaysi `age`ga tegishli ekanini boshqarish qiyinlashadi. Bizga **bitta mantiqiy birlik** kerak, ya'ni bir nechta turli tipdagi ma'lumotlarni bitta nom ostida jamlash imkoniyati.

Aynan shu vazifani **`struct`** (structure — struktura) bajaradi. Struct — bu **foydalanuvchi tomonidan yaratilgan yangi ma'lumot tipi** bo'lib, u turli tipdagi bir nechta o'zgaruvchini (ular **member** yoki **field** deb ataladi) bitta yaxlit birlikka birlashtiradi.

> **Muhim tushuncha**: `struct` massiv (`array`) dan farqli o'laroq, **har xil tipdagi** elementlarni birga saqlay oladi. Massiv faqat bir xil tipdagi elementlarni ketma-ket saqlaydi, struct esa — turli tipdagi elementlarni bitta "quti" ichiga joylaydi.

---

## 2. Struct e'lon qilish va ishlatish

### 2.1. Struct'ning umumiy sintaksisi

```c
struct StructName {
    tip1 member1;
    tip2 member2;
    tip3 member3;
};
```

Diqqat qiling: yopuvchi qavsdan (`}`) keyin **nuqta-vergul (`;`) majburiy**. Bu — boshlang'ich dasturchilarning eng ko'p uchraydigan xatolaridan biri, chunki `struct {...}` aslida bir turdagi murakkab **tip e'loni** hisoblanadi, va C'da har qanday e'lon `;` bilan tugaydi.

### 2.2. Misol: Talaba struct'i

```c
struct Student {
    char name[50];
    int age;
    float gpa;
};
```

Bu yerda biz faqat **yangi tip yaratdik** — hali birorta ham xotira ajratilmadi! Bu xuddi `int` tipi mavjudligiga o'xshaydi: `int` deb yozish o'zi xotira band qilmaydi, faqat `int x;` deganda xotira ajraladi. Xuddi shunday, `struct Student` — bu shunchaki "qolip" (blueprint), uning asosida **instance** (nusxa) yaratganimizdagina xotira band bo'ladi.

### 2.3. Struct o'zgaruvchisini yaratish

```c
struct Student s1;   // s1 — struct Student tipidagi o'zgaruvchi
```

Diqqat: C tilida (C++'dan farqli o'laroq) struct o'zgaruvchisini e'lon qilishda **`struct` kalit so'zini yozish majburiy**:

```c
struct Student s1;   // to'g'ri
Student s1;           // XATO! (C++'da to'g'ri, lekin sof C'da yo'q)
```

### 2.4. Member'larga murojaat qilish — nuqta operatori (`.`)

```c
#include <stdio.h>
#include <string.h>

struct Student {
    char name[50];
    int age;
    float gpa;
};

int main(void) {
    struct Student s1;

    strcpy(s1.name, "Aziz Karimov");
    s1.age = 20;
    s1.gpa = 3.8f;

    printf("Ism: %s\n", s1.name);
    printf("Yosh: %d\n", s1.age);
    printf("GPA: %.2f\n", s1.gpa);

    return 0;
}
```

`s1.age` yozuvi "`s1` o'zgaruvchisining `age` member'i" degan ma'noni bildiradi. Nuqta operatori — **struct instance** orqali uning ichidagi member'ga kirish uchun ishlatiladi.

### 2.5. Initialization (boshlang'ich qiymat berish)

Struct'ni e'lon qilish bilan bir vaqtda qiymat berish mumkin:

```c
struct Student s2 = {"Nodira Yusupova", 21, 3.9f};
```

Bu yerda qiymatlar **struct e'lon qilingan tartibda** beriladi: birinchi qiymat birinchi member'ga, ikkinchisi ikkinchisiga va h.k.

**Designated initializer** (C99'dan boshlab) — bu ancha xavfsiz va o'qilishi oson usul, chunki member nomini aniq ko'rsatasiz:

```c
struct Student s3 = {.name = "Bekzod Toshev", .age = 22, .gpa = 3.5f};
```

Bu usulning afzalligi — member'lar tartibini bilish shart emas, va agar keyinchalik struct'ga yangi member qo'shilsa, kod buzilmaydi.

---

## 3. Struct xotirada qanday joylashadi (memory layout)

Bu — struct mavzusidagi eng chuqur va ko'pincha e'tibordan chetda qoladigan qism. To'liq tushunish uchun buni bosqichma-bosqich ko'rib chiqamiz.

### 3.1. Oddiy holat — hech qanday "sehr" yo'q

Quyidagi struct'ni ko'raylik:

```c
struct Point {
    int x;
    int y;
};
```

`int` — 4 bayt (aksariyat zamonaviy tizimlarda). Demak, `struct Point` **8 bayt** egallashi kerak, to'g'rimi? Buni tekshiramiz:

```c
#include <stdio.h>

struct Point {
    int x;
    int y;
};

int main(void) {
    printf("sizeof(struct Point) = %zu\n", sizeof(struct Point));
    return 0;
}
```

Natija: `8`. To'g'ri taxmin qildik — chunki bu holatda **padding** (to'ldiruvchi bayt) kerak emas.

### 3.2. Padding qachon paydo bo'ladi?

Endi quyidagi struct'ni ko'raylik:

```c
struct Mixed {
    char c;   // 1 bayt
    int i;    // 4 bayt
};
```

Aqlga muvofiq javob: `1 + 4 = 5` bayt. Lekin haqiqiy natija — odatda **8 bayt**! Nega?

Bu yerda **alignment (tekislash)** tushunchasi ishga tushadi. Protsessor xotiradan ma'lumotni tasodifiy manzillardan emas, balki **o'z tipiga mos "tekislangan" manzillardan** o'qiganda tezroq ishlaydi. `int` (4 baytli) odatda 4ga bo'linadigan manzildan boshlanishi kerak (bu — uning **alignment requirement**i).

Xotira tartibini vizual ko'raylik:

```
Manzil:   0    1    2    3    4    5    6    7
          [c ][??  padding  ][    i (4 bayt)   ]
```

`c` 0-manzilda joylashadi (1 bayt egallaydi). Keyingi member `i` — `int`, va u 4ga bo'linadigan manzildan (ya'ni 4-manzildan) boshlanishi kerak. Shu sababli kompilyator 1, 2, 3-manzillarni **padding** (bo'sh, ishlatilmaydigan bayt) sifatida qoldiradi, va `i` faqat 4-manzildan boshlanadi. Natijada: `1 (c) + 3 (padding) + 4 (i) = 8 bayt`.

> **Nega bu muhim?** Agar siz katta massivlarda ko'plab struct instance'lar bilan ishlasangiz (masalan, o'yin dvijogida million zarracha), padding sabab ortiqcha xotira behuda sarflanishi mumkin. Buni tushunish — xotira optimizatsiyasi uchun zarur.

### 3.3. Member tartibini o'zgartirish orqali xotirani tejash

Struct member'lar tartibi xotira hajmiga ta'sir qiladi! Solishtiring:

```c
struct Bad {
    char a;   // 1 bayt
    int b;    // 4 bayt
    char c;   // 1 bayt
};
// Natija: odatda 12 bayt (har ikki char'dan keyin padding kerak bo'lishi mumkin)

struct Good {
    int b;    // 4 bayt
    char a;   // 1 bayt
    char c;   // 1 bayt
};
// Natija: odatda 8 bayt (ikkita char yonma-yon, keyin faqat 2 bayt padding)
```

`Bad` struct'da xotira taqsimoti:
```
[a][pad][pad][pad][    b (4)    ][c][pad][pad][pad]
 0   1    2    3    4  5  6  7    8   9   10   11
```
Jami: 12 bayt.

`Good` struct'da:
```
[    b (4)    ][a][c][pad][pad]
 0  1  2  3     4  5   6    7
```
Jami: 8 bayt.

**Qoida**: Member'larni **kattadan kichikka** tartibda joylashtirish (masalan, `double` → `int` → `char`) odatda eng kam padding hosil qiladi, chunki katta tiplar avval o'zining alignment talabini qondiradi, keyin kichik tiplar bo'sh joylarga "yig'ilib" qoladi.

### 3.4. Struct'ning oxiridagi padding

Struct'ning umumiy hajmi (`sizeof`) har doim uning **eng katta member'ining alignment**iga karrali bo'lishi shart — chunki struct'lar massivda ishlatilganda, har bir keyingi element ham to'g'ri tekislangan bo'lishi kerak.

```c
struct Example {
    char c;
    int i;
};
// c(1) + padding(3) + i(4) = 8, va bu 4ga karrali — OK
```

Agar oxirgi member kichik bo'lsa-yu, struct hajmi eng katta member alignment'iga karrali bo'lmasa, oxiriga qo'shimcha **trailing padding** qo'shiladi.

### 3.5. `#pragma pack` — padding'ni o'chirish (ehtiyot bilan!)

Ba'zan (masalan, tarmoq protokollari yoki fayl formatlari bilan ishlaganda) padding'siz, aniq hajmli struct kerak bo'ladi:

```c
#pragma pack(push, 1)
struct Packed {
    char c;
    int i;
};
#pragma pack(pop)
// Endi sizeof(struct Packed) == 5
```

**Ogohlantirish**: Bu protsessorning "unaligned access" qilishiga majbur qiladi, bu esa ba'zi arxitekturalarda (masalan, ARM) sekinroq ishlashi yoki hatto ishlamay qolishi mumkin. Faqat aniq zaruratda (masalan, binary fayl formatini o'qish/yozish) ishlatiladi.

---

## 4. Nested struct (struct ichida struct)

Struct member sifatida boshqa struct'ni ham o'z ichiga olishi mumkin:

```c
struct Date {
    int day;
    int month;
    int year;
};

struct Student {
    char name[50];
    struct Date birth_date;   // nested struct
    float gpa;
};

int main(void) {
    struct Student s1;
    s1.birth_date.day = 15;
    s1.birth_date.month = 3;
    s1.birth_date.year = 2004;
    return 0;
}
```

`s1.birth_date.day` — bu yerda ikkita nuqta operatori ketma-ket ishlatilmoqda: birinchisi `s1`dan `birth_date`ga, ikkinchisi `birth_date`dan `day`ga o'tish uchun.

Xotira jihatidan, nested struct **"flatten"** qilinadi — ya'ni `Date` struct'ining bayt ketma-ketligi `Student` struct'ining ichiga to'g'ridan-to'g'ri joylashtiriladi (pointer orqali emas, aynan qiymat sifatida).

Anonim (nomsiz) nested struct ham mumkin (C11 xususiyati):

```c
struct Student {
    char name[50];
    struct {
        int day, month, year;
    } birth_date;
};

// Endi bevosita: s1.birth_date.day (o'zgarishsiz)
```

---

## 5. Struct massivlari

Bir xil tipdagi ko'plab struct instance'larni saqlash uchun struct massivi ishlatiladi:

```c
#include <stdio.h>
#include <string.h>

struct Student {
    char name[50];
    int age;
};

int main(void) {
    struct Student class[3] = {
        {"Aziz", 20},
        {"Nodira", 21},
        {"Bekzod", 22}
    };

    for (int i = 0; i < 3; i++) {
        printf("%s - %d yosh\n", class[i].name, class[i].age);
    }

    return 0;
}
```

Xotirada `class` massivi — bu 3 ta `struct Student` ning **ketma-ket** joylashuvi (har biri o'zining padding'i bilan birga). `class[1]` manzili `class[0]` manzilidan aniq `sizeof(struct Student)` bayt keyinda joylashadi. Bu massiv indekslash mantig'i oddiy `int[]` massividagi bilan bir xil — faqat "element o'lchami" endi `sizeof(struct Student)`.

---

## 6. Struct'ga pointer

### 6.1. Nega struct pointer kerak?

Struct'lar katta bo'lishi mumkin (masalan, 50+ bayt). Agar struct'ni funksiyaga **qiymat bo'yicha** (by value) uzatsak, uning **butun nusxasi** (copy) yaratiladi — bu vaqt va xotira jihatidan qimmatga tushadi. Shuning uchun struct'lar bilan ishlashda ko'pincha **pointer orqali** (by reference) murojaat qilinadi.

### 6.2. Struct pointer e'lon qilish

```c
struct Student s1 = {"Aziz", 20, 3.7f};
struct Student *ptr = &s1;   // s1'ning manziliga ishora qiluvchi pointer
```

### 6.3. Pointer orqali member'larga murojaat — ikki usul

**1-usul**: Dereference qilib, keyin nuqta operatori:

```c
(*ptr).age = 21;
```

Bu yerda qavslar **majburiy**, chunki `.` operatori `*` ga qaraganda yuqoriroq ustuvorlikka (precedence) ega. Agar `*ptr.age` deb yozsak, kompilyator buni `*(ptr.age)` deb tushunadi — bu esa xato, chunki `ptr` struct emas, pointer, va unda `.` operatori ishlamaydi.

**2-usul**: Arrow operatori (`->`) — bu ancha qulay va keng tarqalgan:

```c
ptr->age = 21;
```

`ptr->age` aynan `(*ptr).age` bilan bir xil ma'noni anglatadi, lekin o'qilishi ancha oson. Amaliyotda deyarli har doim `->` ishlatiladi.

### 6.4. To'liq misol

```c
#include <stdio.h>
#include <string.h>

struct Student {
    char name[50];
    int age;
};

int main(void) {
    struct Student s1;
    struct Student *ptr = &s1;

    strcpy(ptr->name, "Dilnoza");
    ptr->age = 19;

    printf("%s, %d yosh\n", s1.name, s1.age);
    // yoki: printf("%s, %d yosh\n", ptr->name, ptr->age);

    return 0;
}
```

### 6.5. Dinamik xotirada struct (`malloc`)

Struct'ni heap'da (dinamik xotirada) yaratish ham mumkin — bu ayniqsa **runtime'da nechta struct kerakligi noma'lum** bo'lganda foydali:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Student {
    char name[50];
    int age;
};

int main(void) {
    struct Student *ptr = malloc(sizeof(struct Student));
    if (ptr == NULL) {
        printf("Xotira ajratib bo'lmadi!\n");
        return 1;
    }

    strcpy(ptr->name, "Sardor");
    ptr->age = 23;

    printf("%s, %d yosh\n", ptr->name, ptr->age);

    free(ptr);   // xotirani bo'shatishni unutmang!
    return 0;
}
```

`malloc(sizeof(struct Student))` — heap'dan aynan struct hajmicha (padding bilan birga) xotira ajratadi va uning manzilini qaytaradi. Ishlatib bo'lgach, `free(ptr)` chaqirish **shart** — aks holda memory leak yuzaga keladi.

---

## 7. Struct'larni funksiyaga uzatish

### 7.1. Qiymat bo'yicha uzatish (pass by value)

```c
#include <stdio.h>

struct Point {
    int x, y;
};

void print_point(struct Point p) {
    printf("(%d, %d)\n", p.x, p.y);
}

int main(void) {
    struct Point p1 = {3, 5};
    print_point(p1);   // p1'ning BUTUN NUSXASI funksiyaga ko'chiriladi
    return 0;
}
```

Bu yerda `print_point` funksiyasi ichidagi `p` — `p1`ning mustaqil nusxasi. `p`ni o'zgartirish `p1`ga ta'sir qilmaydi. Kichik struct'lar (masalan, 2 ta `int`) uchun bu maqbul, lekin katta struct'lar uchun samarasiz.

### 7.2. Pointer orqali uzatish (pass by reference) — samarali usul

```c
#include <stdio.h>

struct Point {
    int x, y;
};

void move_point(struct Point *p, int dx, int dy) {
    p->x += dx;
    p->y += dy;
}

int main(void) {
    struct Point p1 = {3, 5};
    move_point(&p1, 2, -1);
    printf("(%d, %d)\n", p1.x, p1.y);   // (5, 4)
    return 0;
}
```

Bu holatda struct nusxalanmaydi — faqat uning **manzili** (8 bayt, 64-bitli tizimda) uzatiladi. Funksiya ichida `p->x` orqali qilingan o'zgarish to'g'ridan-to'g'ri asl `p1`ga ta'sir qiladi, chunki `p` va `p1` **bir xil xotira manzilini** ko'rsatmoqda.

### 7.3. `const` bilan himoyalash

Agar funksiya struct'ni faqat **o'qish** uchun ishlatsa-yu, nusxalashning narxidan qochmoqchi bo'lsak — pointer + `const` kombinatsiyasi ideal yechim:

```c
void print_point(const struct Point *p) {
    printf("(%d, %d)\n", p->x, p->y);
    // p->x = 10;  // XATO! const tufayli taqiqlanadi
}
```

Bu — samaradorlik (nusxalash yo'q) va xavfsizlik (funksiya struct'ni tasodifan o'zgartira olmaydi) o'rtasidagi eng yaxshi muvozanat, va professional C kodida standart amaliyot hisoblanadi.

### 7.4. Struct'ni funksiyadan qaytarish

```c
struct Point make_point(int x, int y) {
    struct Point p;
    p.x = x;
    p.y = y;
    return p;   // struct'ning nusxasi qaytariladi
}

int main(void) {
    struct Point p1 = make_point(4, 7);
    return 0;
}
```

Bu yerda ham struct nusxa sifatida qaytariladi (odatda kompilyator **RVO** ga o'xshash optimizatsiyalar qo'llaydi, lekin C standarti buni kafolatlamaydi). Katta struct'lar uchun buning o'rniga chiqish parametrini pointer orqali uzatish tezroq bo'lishi mumkin:

```c
void make_point(struct Point *out, int x, int y) {
    out->x = x;
    out->y = y;
}
```

---

## 8. `typedef` bilan ishlash

Har safar `struct Student s1;` deb yozish charchatadi. `typedef` yordamida struct nomiga **alias** (taxallus) yaratib, `struct` kalit so'zisiz ishlatish mumkin:

```c
typedef struct {
    char name[50];
    int age;
} Student;

int main(void) {
    Student s1;   // "struct" kalit so'zi shart emas!
    s1.age = 20;
    return 0;
}
```

Bu yerda biz struct'ga **teg (tag) nomi bermadik** — u anonim, faqat `typedef` orqali `Student` nomi bilan atalmoqda.

Agar struct o'ziga ishora qilishi kerak bo'lsa (masalan, linked list uchun — 9-bo'limga qarang), teg nomi ham berish tavsiya etiladi:

```c
typedef struct Node {
    int data;
    struct Node *next;   // bu yerda "Node" emas, "struct Node" ishlatiladi
} Node;
```

**Nega bunday?** `typedef` bayonoti **to'liq bajarilgunga qadar** (ya'ni yopuvchi `}` dan keyingi `;` gacha) `Node` nomi hali mavjud emas — kompilyator hali uni tanimaydi. Lekin struct **tegi** (`struct Node`) struct tanasi ichida darhol mavjud bo'ladi, shuning uchun o'z-o'ziga ishora qilish uchun `struct Node *next;` ishlatiladi, `Node *next;` emas.

---

## 9. Self-referential struct — Linked List asosi

Struct o'zining ichida **o'zi tipidagi pointer**ni saqlashi mumkin (lekin o'zi tipidagi **to'g'ridan-to'g'ri qiymatni emas** — bu cheksiz rekursiyaga va cheksiz xotira hajmiga olib kelardi).

```c
typedef struct Node {
    int data;
    struct Node *next;
} Node;
```

Bu — **linked list**ning eng asosiy qurilish bloki. Nega bu ishlaydi, lekin `struct Node next;` (pointer'siz) ishlamaydi?

- `struct Node *next;` — pointer har doim belgilangan hajmga ega (masalan, 8 bayt 64-bitli tizimda), tipidan qat'iy nazar. Kompilyator `sizeof(struct Node)` ni hisoblashda muammoga duch kelmaydi.
- `struct Node next;` — bu holatda `struct Node`ning hajmini hisoblash uchun avval `struct Node`ning o'zi (ya'ni yana `next`) hajmini bilish kerak bo'lar edi — bu **cheksiz regressiya**, kompilyator buni qabul qilmaydi.

### To'liq linked list misoli

```c
#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int data;
    struct Node *next;
} Node;

Node* create_node(int value) {
    Node *new_node = malloc(sizeof(Node));
    new_node->data = value;
    new_node->next = NULL;
    return new_node;
}

int main(void) {
    Node *head = create_node(10);
    head->next = create_node(20);
    head->next->next = create_node(30);

    Node *current = head;
    while (current != NULL) {
        printf("%d -> ", current->data);
        current = current->next;
    }
    printf("NULL\n");

    // Xotirani tozalash
    current = head;
    while (current != NULL) {
        Node *temp = current;
        current = current->next;
        free(temp);
    }

    return 0;
}
```

Natija: `10 -> 20 -> 30 -> NULL`

Bu yerda `head->next->next` — zanjir shaklidagi arrow operatoridan foydalanish: `head`dan `next`ga, undan yana `next`ga o'tish.

---

## 10. Bit-fields

Ba'zan struct member'i uchun to'liq bayt (yoki 4 bayt) emas, balki faqat **bir necha bit** kerak bo'ladi (masalan, boolean flag'lar yoki protokol sarlavhalari uchun). Bunday holda **bit-field** ishlatiladi:

```c
struct Flags {
    unsigned int is_active : 1;   // faqat 1 bit
    unsigned int is_admin  : 1;   // faqat 1 bit
    unsigned int level     : 4;   // 4 bit (0-15 oraliq qiymat)
};
```

```c
#include <stdio.h>

struct Flags {
    unsigned int is_active : 1;
    unsigned int is_admin  : 1;
    unsigned int level     : 4;
};

int main(void) {
    struct Flags f;
    f.is_active = 1;
    f.is_admin = 0;
    f.level = 7;

    printf("sizeof(struct Flags) = %zu\n", sizeof(struct Flags));
    // Odatda 4 (bitta int'ga "siqib" joylashtiriladi)

    return 0;
}
```

Bit-field'lar orqali bir nechta kichik qiymatni bitta `int` ichiga "siqib" xotira tejash mumkin. Lekin bu — **platformaga bog'liq** xususiyat: bit'larning tartibi (endianness ta'sirida) va aniq joylashuvi kompilyatordan kompilyatorga farq qilishi mumkin, shuning uchun **portable** (ko'chma) kod uchun tavsiya etilmaydi — asosan low-level, platforma-spetsifik kodda (masalan, embedded systems, network protokollari) ishlatiladi.

---

## 11. `struct` vs `union`

`union` — struct'ga sirtdan o'xshaydi, lekin fundamental farq bor: **union'ning barcha member'lari bir xil xotira manzilini "bo'lishadi"** (share qiladi), struct'da esa har bir member o'zining alohida joyiga ega.

```c
#include <stdio.h>

struct MyStruct {
    int i;
    float f;
};   // sizeof = 8 (i va f alohida joy egallaydi)

union MyUnion {
    int i;
    float f;
};   // sizeof = 4 (i va f BIR XIL 4 baytni bo'lishadi)

int main(void) {
    printf("struct: %zu\n", sizeof(struct MyStruct));  // 8
    printf("union:  %zu\n", sizeof(union MyUnion));     // 4

    union MyUnion u;
    u.i = 65;
    printf("%d\n", u.i);      // 65
    printf("%f\n", u.f);      // u.i yozilgan xotirani 'float' sifatida talqin qiladi — mazmunsiz raqam chiqadi

    return 0;
}
```

`union`ning hajmi — **eng katta member**ining hajmiga teng (chunki hammasi bir joyni bo'lishadi). Bir vaqtning o'zida faqat **bitta** member "amal qiladi" — boshqasiga yozish avvalgisining qiymatini "buzadi" (chunki xotira bir xil).

`union` odatda xotira tejash kerak bo'lgan holatlarda (masalan, "bir vaqtda faqat bitta tipdagi qiymat saqlanadi" degan holatlarda — **tagged union / variant** pattern) ishlatiladi.

---

## 12. Struct ichida funksiya pointerlari

C'da klassik ma'noda "class" yo'q, lekin struct ichiga **funksiya pointer**larini joylashtirish orqali oddiy obyektga o'xshash xatti-harakatni taqlid qilish mumkin:

```c
#include <stdio.h>

typedef struct {
    int width, height;
    int (*area)(struct Rectangle *self);   // funksiya pointer member
} Rectangle;

int rectangle_area(Rectangle *self) {
    return self->width * self->height;
}

int main(void) {
    Rectangle r = {5, 3, rectangle_area};
    printf("Yuza: %d\n", r.area(&r));   // "metod chaqirish"ga o'xshaydi
    return 0;
}
```

Bu texnika C kutubxonalarida (masalan, GTK, Linux kernel) keng qo'llaniladi — struct ichidagi funksiya pointerlari orqali **polymorphism**ga o'xshash xususiyat yaratiladi (masalan, turli struct'lar bir xil "interfeys"ni turlicha amalga oshirishi mumkin).

---

## 13. C++ bilan taqqoslash (`struct` vs `class`)

Agar siz C++ bilan ham shug'ullansangiz, quyidagi farqlarni bilish foydali:

| Xususiyat | C `struct` | C++ `struct` | C++ `class` |
|---|---|---|---|
| Member funksiyalar (metodlar) | Yo'q (faqat funksiya pointer) | Bor | Bor |
| Default access modifikator | — | `public` | `private` |
| Konstruktor/destruktor | Yo'q | Bor | Bor |
| Meros (inheritance) | Yo'q | Bor | Bor |
| `struct` kalit so'zisiz e'lon | Yo'q (majburiy) | Ha (avtomatik `typedef` kabi) | Ha |

C++'da `struct` va `class` deyarli bir xil — yagona farq **default access level** (`struct` — `public`, `class` — `private`). C'da esa `struct` faqat **ma'lumotlarni guruhlash** uchun, hech qanday metod yoki inkapsulyatsiya tushunchasi yo'q.

```cpp
// C++ da:
struct Point {
    int x, y;
    Point(int x, int y) : x(x), y(y) {}   // konstruktor — sof C'da BOʻLMAYDI
    int distance_from_origin() {           // metod — sof C'da BOʻLMAYDI
        return x*x + y*y;
    }
};
```

---

## 14. Xulosa va amaliy mashqlar

### Asosiy tushunchalar xulosasi

- `struct` — turli tipdagi ma'lumotlarni bitta mantiqiy birlikka birlashtiruvchi foydalanuvchi tipi
- Struct e'lon qilish xotira ajratmaydi; faqat instance yaratilganda xotira band bo'ladi
- **Padding va alignment** tufayli struct hajmi member'lar yig'indisidan katta bo'lishi mumkin — member tartibi buni optimallashtirishi mumkin
- Katta struct'larni funksiyaga uzatishda **pointer** (`->`) ishlatish nusxalash xarajatidan qutqaradi; faqat o'qish uchun `const` qo'shish tavsiya etiladi
- `typedef` — struct nomini soddalashtiradi, lekin self-referential struct'larda `struct Tag *` ishlatish shart
- **Bit-fields** va **union** — xotirani maksimal tejash kerak bo'lgan maxsus holatlar uchun

### Amaliy mashqlar

1. `struct Book` yarating (`title`, `author`, `pages`, `price`) va 5 ta kitobdan iborat massiv yarating, keyin eng qimmat kitobni topib chiqaring.
2. `struct Complex` (kompleks son: `real`, `imag`) yarating va ikkita kompleks sonni qo'shadigan, ko'paytiradigan funksiyalarni pointer orqali (natijani chiqish parametriga yozadigan qilib) yozing.
3. Yuqoridagi `Node` (linked list) tuzilmasini kengaytirib, ro'yxat oxiriga element qo'shadigan (`append`) va berilgan qiymatni ro'yxatdan o'chiradigan (`delete`) funksiyalarni yozing.
4. `sizeof` yordamida turli member tartibidagi (kamida 3 xil tartib) bir xil member'lardan iborat struct'larning hajmini solishtirib, eng kam padding beruvchi tartibni toping.
5. Funksiya pointer'li struct yordamida `Shape` (doira va to'rtburchak uchun) — ikkalasi ham `area()` funksiyasiga ega bo'ladigan — oddiy "polymorphic" tizim yarating.
