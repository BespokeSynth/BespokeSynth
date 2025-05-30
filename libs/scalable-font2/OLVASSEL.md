Skálázható Képernyő Font 2.0
============================

Ez egy könnyen portolható, egyedülálló ANSI C/C++ fejlécfájl, skálázható bitmap és vektor font megjelenítő. Csak
memóriával kapcsolatos libc függőségei vannak és nem használ lebegőpontos utasításokat sem. Nagyon kicsi (kevesebb,
mint 32 kilobájt) és alig foglal memóriát, tökéletes beágyazott rendszerekhez és hobbi operációs rendszer kernelekhez.
Mivel csak hobbiból írtam, ezért bárminemű adományt és támogatást szívesen fogadok, ha hasznosnak bizonyulna a számodra.

 - [ssfn.h](https://gitlab.com/bztsrc/scalable-font2/blob/master/ssfn.h) az SSFN [megjelenítő SDK](https://gitlab.com/bztsrc/scalable-font2/blob/master/docs/API.md)
 - [libsfn](https://gitlab.com/bztsrc/scalable-font2/tree/master/libsfn) a font [manipuláló SDK](https://gitlab.com/bztsrc/scalable-font2/blob/master/docs/libsfn.md)
 - [sfnconv](https://gitlab.com/bztsrc/scalable-font2/tree/master/sfnconv) parancssoros SSFN konvertáló
 - [sfnedit](https://gitlab.com/bztsrc/scalable-font2/tree/master/sfnedit) többplatformos, ablakos SSFN font konvertáló és szerkesztő
 - [sfntest](https://gitlab.com/bztsrc/scalable-font2/tree/master/sfntest) teszt programok és [API](https://gitlab.com/bztsrc/scalable-font2/blob/master/docs/API.md) használati példák

<img alt="Skálázható Képernyő Font Képességek" src="https://gitlab.com/bztsrc/scalable-font2/raw/master/features.png">

Az SSFN megjelenítő nem használ egyetlen meglévő formátumot sem (mert azok nem hatékonyak vagy csak agyamentek), ezért
először is a fontokat [SSFN](https://gitlab.com/bztsrc/scalable-font2/blob/master/docs/sfnconv.md)-é kell konvertálni.

Ehhez adott egy kicsi ANSI C program és egy ablakos szerkesztő is, amik szállítható futtathatók, egyből használhatók
[Windows](https://gitlab.com/bztsrc/scalable-font2/raw/master/ssfn_2.0.0-i686-win.zip),
[MacOSX](https://gitlab.com/bztsrc/scalable-font2/raw/master/ssfn_2.0.0-intel-macosx.zip) és
[Linux](https://gitlab.com/bztsrc/scalable-font2/raw/master/ssfn_2.0.0-amd64.deb) alatt. Csak töltsd le és csomagold ki a
`C:\Program Files (x86)` (Windows) vagy `/Applications` (MacOSX) mappába; nem kell telepíteni. Linux alatt a
`dpkg -i ssfn_*.deb` parancs csomagolja ki.

Szinte minden létező font formátumot támogatnak:

 - OpenType (.otf, .ttf, .sfnt, .cff),
 - TrueType (.ttf),
 - PS Type1 (.pfa, .pfb),
 - PS Type42 (.pfa, .pfb),
 - Webfonts (.woff, .woff2),
 - FontForge SplineFontDB-je (.sfd, vektor és bitmap alapú fontok),
 - X11 Bitmap Distribution Format (.bdf),
 - X11 Portable Compiled Font (.pcf),
 - Linux konzol fontok PC Screen Font (.psf, .psfu),
 - Windows konzol fontok (.fon, .fnt),
 - GRUB PFF2 fontjai (.pf2),
 - GNU unifont (.hex),
 - Bits'N'Picas (.kbits, .kbitx)
 - ByteMap Font (.bmf, színes pixel alapú fontok)
 - Yet Another Font Format (.yaff)
 - ROM kép (.f08, .f10, .f12, .f16, stb.)
 - Portable Network Graphics és TARGA (.png, .tga, a színes pixel alapú fontokhoz),
 - ...és még sok más! Tömörített (.gz) fontokat is kezeli. Bitmap vektorizálás és vektor font raszterirálás egyaránt lehetséges.

Az SSFN fontok használatával rengeteg helyet spórolsz meg, és a megjelenítő is sokkal gyorsabban tud dolgozni velük, mint
más megjelenítők. Lásd az [összehasonlítás](https://gitlab.com/bztsrc/scalable-font2/blob/master/docs/compare.md)t más font
formátumokkal (a sebességmérési adatok a fenti demóhoz az oldal alján találhatók).

Támogasd a fejlesztést adománnyal
---------------------------------

Ha tetszik, vagy hasznosnak találod, szívesen fogadok akármekkora adományt:<br>
<a href="bitcoin:3NBKzD1DHXr7Pd1ntGxvGqjDmcFKSccndf"><img src="https://gitlab.com/bztsrc/scalable-font2/raw/master/donate.png"><br>BTC 3NBKzD1DHXr7Pd1ntGxvGqjDmcFKSccndf</a>

Minta kód
---------

Az SSFN megjelenítő kétféleképp használható: van a normál megjelenítő, néhány libc függvény függőséggel, és van a
speciális megjelenítő operációs rendszer konzolokhoz, ami mindössze egyetlen függvény, és nincs semmi függősége.
(A teljesség kedvéért megjegyzem a normál megjelenítő is bekonfigurálható függőség nélkülire, de ennek a módnak
vannak bizonyos hátrányai.)

### Szimpla Megjelenítő

Tömörítetlen, fix méretű bitmap fontokhoz. A fejléc mindent tartalmaz, nem kell függvénykönyvtárakkal összeszerkeszteni!

```c
#define SSFN_CONSOLEBITMAP_HICOLOR          /* a szimpla megjelenító kiválasztása hicolor pixelekhez */
#include <ssfn.h>

/* bekonfigurálás néhány globális változóval */
ssfn_src = &_binary_console_sfn_start;      /* a használt bitmap font */

ssfn_dst.ptr = 0xE0000000;                  /* a lineáris frémbuffer címe */
ssfn_dst.w = 1024;                          /* szélesség */
ssfn_dst.h = 768;                           /* magasság */
ssfn_dst.p = 4096;                          /* sor hossza bájtban */
ssfn_dst.x = ssfn_dst.y = 0;                /* toll poziciója */
ssfn_dst.fg = 0xFFFF;                       /* előtér színe */

/* UNICODE kódpontok megjelenítése direktben a képernyőn és a toll odébbmozgatása */
ssfn_putc('H');
ssfn_putc('e');
ssfn_putc('l');
ssfn_putc('l');
ssfn_putc(0xF3); /* a hosszú ó kódpontja */
```

Amint látható, ez a megjelenítő nagyon egyszerű, és nagyon kicsi (kevesebb, mint 2k). Csak fix méretű bitmap
fontokat tud megjeleníteni. Cserébe nem foglal memóriát, és nincs libc függősége, ezért nem tud átméretezni,
de kezeli a proporcionális fontokat (például 8x16-os Latin betűk és 16x16-os CJK, kínai, japán és kóreai betűk).
Emiatt egy teljes értékű UNICODE konzolt lehet vele implementálni.

FONTOS MEGJEGYZÉS: a normál megjelenítővel ellentétben nem kezel gzip tömörített vagy vektor fontokat, például a
FreeSans [nem fog működni](https://forum.osdev.org/viewtopic.php?p=349572#p349572) vele. Mindig *kitömörített bitmap fontot*
kell megadni az `ssfn_src` változóban, ezért előzetesen át kell konvertálni a `sfnconv -U -B 16 vektor.sfn konzolfont.sfn`
paranccsal (ahol az `-U` tömörítetlenül ment, a `-B 16` pedig a kívánt bitmap méret pixelben).

FIGYELEM: egy egy font raszterizáló és nem egy konzol függvénykönyvtár. Alapból nem fog vezérlőkaraktereket
(mint `\t`, `\r` vagy `\n`) értelmezni, habár egy define segítségével egy *egyszerű* implementáció bekapcsolható.

FIGYELEM: ha laphibát dob a konzolos megjelenítő, mint [ennek a szerencsétlen flótás](https://forum.osdev.org/viewtopic.php?f=1&t=56005)nak,
akkor az azt jelenti, hogy memória felülírásos hiba van a *TE* kódodban, vagy hibás bemenetet adtál meg. Egész
egyszerűen azért, mert a konzolos megjelenítő egyáltalán nem is foglal memóriát, és csakis az általad `ssfn_dst`-ben
megadott bufferbe ír, amiről *TE* állítod, hogy írható!

### Normál Megjelenítő

Tömörített, átméretezhető, bitmap, pixmap és vektoros fontokhoz egyaránt. Nagyon könnyű a használata, példa kód
hibakezelés nélkül:

```c
#define SSFN_IMPLEMENTATION                         /* a normál megjelenítő kiválasztása */
#include <ssfn.h>

ssfn_t ctx = { 0 };                                 /* a megjelenítő kontextusa */
ssfn_buf_t buf = {                                  /* a cél pixel buffer */
    .ptr = sdlsurface->pixels,                      /* a buffer címe */
    .w = sdlsurface->w,                             /* szélessége */
    .h = sdlsurface->h,                             /* magassága */
    .p = sdlsurface->pitch,                         /* sor hossza bájtban */
    .x = 100,                                       /* toll pozicója */
    .y = 100,
    .fg = 0xFF808080                                /* előtér színe */
};

/* egy vagy több font hozzáadása a kontextushoz. A fontoknak a memóriában kell lenniük */
ssfn_load(&ctx, &_binary_times_sfn_start);          /* lehet különféle stílusú... */
ssfn_load(&ctx, &_binary_timesbold_sfn_start);
ssfn_load(&ctx, &_binary_timesitalic_sfn_start);
ssfn_load(&ctx, &_binary_emoji_sfn_start);          /* ...vagy különféle UNICODE tartomány */
ssfn_load(&ctx, &_binary_cjk_sfn_start);

/* megjelenítés (typeface) kiválasztása */
ssfn_select(&ctx,
    SSFN_FAMILY_SERIF, NULL,                        /* betűcsalád */
    SSFN_STYLE_REGULAR | SSFN_STYLE_UNDERLINE,      /* stílus */
    64                                              /* méret */
);

/* egy glif kirajzolása az UTF-8 sztring elejéről 32 bites pixel bufferre */
/* a sztring elejéről felhasznált bájtok számát adja vissza */
ssfn_render(&ctx, &buf, "Helló");
ssfn_render(&ctx, &buf, "elló");
ssfn_render(&ctx, &buf, "lló");                     /* feltéve, hogy a fontokban van "ll" ligatúra */
ssfn_render(&ctx, &buf, "ó");

/* erőforrások felszabadítása */
ssfn_free(&ctx);                                    /* a megjelenítő kontextus belső buffereinek felszabadítása */
```

Van még tovább, használható C++ osztályként, a font név alapján is kiválaszható, a kirajzolandó szöveg mérete
előre lekérhető például, és az `ssfn_text` pedig egy újonnan allokált pixel buffert ad vissza a szöveggel, lásd
az [API leírás](https://gitlab.com/bztsrc/scalable-font2/blob/master/docs/API.md)át.

Akárcsak a szimpla megjelenítő esetében, a fejléc mindent tartalmaz, nincs szükség függvénykönyvtárakkal való
összeszerkesztésre! A gzip kitömörítőt is tartalmazza ez a 28k-nyi kód, nincs szükség zlib-re!

Font Szerkesztő
---------------

A [konvertáló](https://gitlab.com/bztsrc/scalable-font2/tree/master/sfnconv) parancson túl, ami mindenféle formátumot
megeszik, van egy professzionális szintű, ablakos [SSFN font szerkesztő](https://gitlab.com/bztsrc/scalable-font2/tree/master/sfnedit)
is, ami legalább annyit tud, mint a FontForge, de azzal ellentétben szállítható futtatható (nem kell telepíteni és nem
kell hozzá semmiféle DLL):

<img alt="Skálázható Képernyő Font Szerkesztő" src="https://gitlab.com/bztsrc/scalable-font2/raw/master/docs/sfnedit.png">

FIGYELEM MacOS felhasználók: az SDL2 nincs statikusan belefordítva, helyette az applikáció boundle-be van csomagolva.
Elképzelhető, hogy ezzel gond adódhat MacOS BigSur és afölötti verzióknál. Ha az applikáció nem indul el, mert hiányolja az
SDL2-t, akkor csak le kell tölteni és telepíteni a legfrissebb dmg-t a hivatalos [libsdl.org](http://libsdl.org/download-2.0.php)
oldalról, az megoldja a problémát.

Licensz
-------

Mind a két megjelenítő, a konvertáló és a szerkesztő is az MIT licensz feltételei szerint kerül terjesztésre, abban a
reményben, hogy hasznosnak bizonyulnak.

FONTOS MEGJEGYZÉS: habár a font formátuma MIT licenszű, elképzelhető, hogy az SSFN formátumban tárolt font maga nem az!
Ezért mindig nézd meg a font felhasználási feltételeit! Lásd `sfnconv -d`.

Függőségek
----------

A szimpla megjelenítő semmiféle eljárást sem hív, ezért teljesen függőségmentes (még csak libc vagy fordító beépített
függvény sem kell hozzá). Abszolút semmi, leszámítva a két globális változóját.

Ami a normál megjelenítőt illeti, egy define-al szintén lehet függőség nélküli módban fordítani (néhány megszorítással),
de egyébként minden függősége libc, vagy fordító beépített függvény:
 - `realloc()` és `free()` a libc-ből (stdlib.h)
 - `memcmp()` és `memset()` a libc-ből (string.h)

A vektorfont konvertáló a **freetype2** függvénykönyvtárra épít (alapból bele van fordítva a libsfn-be). A bitmap
konvertálóknak nincs függősége. A libsfn használja a **zlib** függvénykönyvtárat gzip tömörített fontok mentéséhez
(statikusan bele van fordítva a libsfn-be, a kitömörítés meg megy zlib nélkül is). A font vektorizácóhoz a
**potrace** függvénykönyvtárat használja (egy lecsupaszított változat ebből is bele van fordítva statikusan a libsfn-be).

A szerkesztő SDL2-t vagy vészforgatókönyvnek X11-et használ, és persze függ a libsfn-től. Az SDL2 kapcsán fordítható
statikusan és dinamikusan is (a statikushoz kézzel kell fordítani az SDL2-t).

A teszt programok szintén SDL2-t használnak az ablakok kezelésére és a kigenerált szövegek megjelenítésére.

Változások az SSFN 1.0-hoz képest
---------------------------------

Az SSFN 2.0 egy koncepcionálisan más eljárást használ a glifek kigenerálásához, mint az elődje. SSFN 2.0 alatt nemcsak
a glifek lehetnek kevertek (mint SSFN 1.0 esetén), hanem egy glifen belül is keverhetőek a bitmap, pixmap és vektor rétegek,
és a végeredmény sokkal olvashatóbb kis méretben. Hogy ezt megtegye, csak 32 bites pixel buffereket támogat, cserébe elvégzi
az alpha-összemosást.

### Hátrányok

- a megjelenítő határozottan több memóriát igényel, mint az SSFN 1.0 (~64k a korábbi ~24k-val szemben, glif gyorsítótárral pedig akár megabájtokra is rúghat).
- nem adja vissza sem a körvonalakat, sem a kigenerált glifet.

### Előnyök

- egyszerűbb API, kevesebb funkció (betöltés + kiválasztás + megjelenítés + felszabadítás).
- kiváló minőségű, élsimított megjelenítés közvetlenül a 32 bites pixel bufferbe.
- gyorsabb megjelenítés, kevesebb malloc hívással és belső glif gyorsítótárral
- kissebb, kompaktabb font fájlok.
- gzippelt fontok transzparens kezelése.
- nem csak a szabványos, hanem a felhasználó által definiált ligatúrákat is kezeli.
- színes glifek támogatása.
- külön libsfn függvénykönyvtár a fontok manipulálásához.

Ismert hibák
------------

A megjelenítőkben semmi, amiről tudnék. De egyetlen programozó sem tudja tökéletesen leteszteni a saját programját.
Számtalanszor futtattam számtalan különböző SSFN fonttal valgrinden keresztül, mindenféle probléma nélkül. Ha mégis
találnál benne hibát, kérlek jelezd a gitlab-os [hibabejelentő](https://gitlab.com/bztsrc/scalable-font2/issues)ben.

A betöltők esetében ott van a soha véget nem érő harc a FreeType2-vel a toll eltolások pontos lekérdezésénél. Elképzelhető,
hogy néhány glif esetén kézzel kell helyreigazítani miután betöltötted a fontot.

A kernelő információk betöltése SplineFontDB .sfd fájlokból hiányzik, nem lett még implementálva.

Bár a formátum kezeli a koordináta tippeket (hinting grid), a megjelenítő nem foglalkozik ezzel (a libsfn támogatja a
betöltésüket és kimentésüket). Az új bilineáris interpolációs átméretezővel nem hiszem, hogy szükség lenne rájuk, majd
meglátjuk, mint hoz a tesztelés eredménye.

A bitmap fontok vektorizációja hagy kívánnivalót maga után. A potrace nem igazán boldogul 16 x 16-os vagy még kissebb bitképekkel.

Támogatók
---------

Szeretnék köszönetet mondani @mrjbom-nak, aki kitartóan tesztelte és rámutatott néhány használhatósági hiányosságra,
ezáltal segített sokkal jobbá tenni a kódot.

Készítők
--------

- SSFN formátum, konvertáló, szerkesztő és megjelenítők: bzt
- STBI (PD, eredeti zlib dekódoló): Sean Barrett
- potrace (GPL): Peter Selinger
- libimagequant (GPL): Jef Poskanzer, Greg Roelofs, Kornel Lesiński
- freetype2 (GPL): David Turner, Robert Wilhelm, Werner Lemberg
