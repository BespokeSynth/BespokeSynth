Scalable Screen Font 2.0
========================

This is a portable, single ANSI C/C++ header file, a scalable bitmap and vector font renderer. It has only memory
related libc dependency and it does not use floating point numbers. It's extremely small (less than 32 kilobytes)
and it is very easy on memory, perfect for embedded systems and hobby OS kernels. It was a hobby project for me,
so donations and contributions would be much appreciated if it turns out to be useful to you.

 - [ssfn.h](https://gitlab.com/bztsrc/scalable-font2/blob/master/ssfn.h) the SSFN [renderer SDK](https://gitlab.com/bztsrc/scalable-font2/blob/master/docs/API.md)
 - [libsfn](https://gitlab.com/bztsrc/scalable-font2/tree/master/libsfn) the font [manipulator SDK](https://gitlab.com/bztsrc/scalable-font2/blob/master/docs/libsfn.md)
 - [sfnconv](https://gitlab.com/bztsrc/scalable-font2/tree/master/sfnconv) a command line SSFN converter tool
 - [sfnedit](https://gitlab.com/bztsrc/scalable-font2/tree/master/sfnedit) multiplatform SSFN font converter and editor with a GUI
 - [sfntest](https://gitlab.com/bztsrc/scalable-font2/tree/master/sfntest) test applications and [API](https://gitlab.com/bztsrc/scalable-font2/blob/master/docs/API.md) usage examples

<img alt="Scalable Screen Font Features" src="https://gitlab.com/bztsrc/scalable-font2/raw/master/features.png">

SSFN renderer does not use existing font formats directly (because most formats are inefficient or just insane),
so you first have to compress those into [SSFN](https://gitlab.com/bztsrc/scalable-font2/blob/master/docs/sfnconv.md).

There's a small ANSI C utility and also a GUI editor to do that, which I provide in a portable executable form for
[Windows](https://gitlab.com/bztsrc/scalable-font2/raw/master/ssfn_2.0.0-i686-win.zip),
[MacOSX](https://gitlab.com/bztsrc/scalable-font2/raw/master/ssfn_2.0.0-intel-macosx.zip) and
[Linux](https://gitlab.com/bztsrc/scalable-font2/raw/master/ssfn_2.0.0-amd64.deb). Just download the zip and extract to
`C:\Program Files (x86)` (Windows) or `/Applications` (MacOSX); no installation required. For Linux, use `dpkg -i ssfn_*.deb`.

They support importing virtually all font formats:

 - OpenType (.otf, .ttf, .sfnt, .cff),
 - TrueType (.ttf),
 - PS Type1 (.pfa, .pfb),
 - PS Type42 (.pfa, .pfb),
 - Webfonts (.woff, .woff2),
 - FontForge's SplineFontDB (.sfd, both vector and bitmap fonts),
 - X11 Bitmap Distribution Format (.bdf),
 - X11 Portable Compiled Font (.pcf),
 - Linux Console PC Screen Fonts (.psf, .psfu),
 - Windows Console Fonts (.fon, .fnt),
 - GRUB's PFF2 Fonts (.pf2),
 - GNU unifont (.hex),
 - Bits'N'Picas (.kbits, .kbitx)
 - ByteMap Font (.bmf, colorful pixel fonts)
 - Yet Another Font Format (.yaff)
 - ROM image (.f08, .f10, .f12, .f16, etc.)
 - Portable Network Graphics and TARGA (.png, .tga, for pixel fonts),
 - ...and others! Compressed variants (.gz) supported on-the-fly. Vectorizing bitmap fonts and rasterizing vector fonts also possible.

Using SSFN means your fonts will require less space, and also the renderer can work a lot faster than other renderer
libraries. Check out [comparison](https://gitlab.com/bztsrc/scalable-font2/blob/master/docs/compare.md) with other font
formats (performance measurements for the feature demo above can be found at the bottom of that page).

Support the Development by Donation
-----------------------------------

If you like it or find it useful, your donation of any amount will be very much appreciated:<br>
<a href="bitcoin:3NBKzD1DHXr7Pd1ntGxvGqjDmcFKSccndf"><img src="https://gitlab.com/bztsrc/scalable-font2/raw/master/donate.png"><br>BTC 3NBKzD1DHXr7Pd1ntGxvGqjDmcFKSccndf</a>

Example Code
------------

The SSFN renderer comes in two flavours: there's the normal renderer with a few functions and libc dependency, and a
specialized renderer for OS kernel consoles with just one function and no dependencies at all. (Just for the records,
with a single define you can configure the normal render for dependency-free static memory management too, although
that mode has certain limitiations.)

### Simple Renderer

For uncompressed, fixed size bitmap fonts only. The header contains everything, no additional linking required!

```c
#define SSFN_CONSOLEBITMAP_TRUECOLOR        /* use the special renderer for 32 bit truecolor packed pixels */
#include <ssfn.h>

/* set up context by global variables */
ssfn_src = &_binary_console_sfn_start;      /* the bitmap font to use */

ssfn_dst.ptr = 0xE0000000;                  /* address of the linear frame buffer */
ssfn_dst.w = 1024;                          /* width */
ssfn_dst.h = 768;                           /* height */
ssfn_dst.p = 4096;                          /* bytes per line */
ssfn_dst.x = ssfn_dst.y = 0;                /* pen position */
ssfn_dst.fg = 0xFFFFFF;                     /* foreground color */

/* render UNICODE codepoints directly to the screen and then adjust pen position */
ssfn_putc('H');
ssfn_putc('e');
ssfn_putc('l');
ssfn_putc('l');
ssfn_putc('o');
```

As you see this renderer implementation is very simple, extremely small (less than 2k). It can only render
unscaled bitmap fonts. It does not allocate memory nor need libc, so it can't scale, but it can handle
proportional fonts (like 8x16 for Latin letters, and 16x16 for CJK ideograms). Therefore you can implement
a true UNICODE console with this renderer. It also works with paletted, hicolor and truecolor modes.

IMPORTANT NOTE: unlike the normal renderer, this one does not handle gzip compressed nor vector fonts, for example
FreeSans [won't work](https://forum.osdev.org/viewtopic.php?p=349572#p349572) with it. Always pass an *inflated bitmap font*
in `ssfn_src`, so you must convert apriori with the `sfnconv -U -B 16 vector.sfn consolefont.sfn` command (where
`-U` means save uncompressed, and `-B 16` specifies the desired bitmap size in pixels).

NOTE: this is a font rasterizer, not a console library. By default, it won't interpret control codes (like
`\t`, `\r` or `\n`) for you, however you can turn a *basic* implementation on with a define.

NOTE: if you get page faults with the console renderer like [this miserable lost soul](https://forum.osdev.org/viewtopic.php?f=1&t=56005),
that means *YOUR* code has memory corruption issues or you've given invalid input. This is so because the
console renderer simply doesn't allocate any memory at all, and it only writes to a buffer you specify in
`ssfn_dst` and *YOU* claim to be writeable!

### Normal Renderer

For compressed, scaled, bitmap, pixmap or vector fonts. Very easy to use, here's an example without error handling:

```c
#define SSFN_IMPLEMENTATION                         /* use the normal renderer implementation */
#include <ssfn.h>

ssfn_t ctx = { 0 };                                 /* the renderer context */
ssfn_buf_t buf = {                                  /* the destination pixel buffer */
    .ptr = sdlsurface->pixels,                      /* address of the buffer */
    .w = sdlsurface->w,                             /* width */
    .h = sdlsurface->h,                             /* height */
    .p = sdlsurface->pitch,                         /* bytes per line */
    .x = 100,                                       /* pen position */
    .y = 100,
    .fg = 0xFF808080                                /* foreground color */
};

/* add one or more fonts to the context. Fonts must be already in memory */
ssfn_load(&ctx, &_binary_times_sfn_start);          /* you can add different styles... */
ssfn_load(&ctx, &_binary_timesbold_sfn_start);
ssfn_load(&ctx, &_binary_timesitalic_sfn_start);
ssfn_load(&ctx, &_binary_emoji_sfn_start);          /* ...or different UNICODE ranges */
ssfn_load(&ctx, &_binary_cjk_sfn_start);

/* select the typeface to use */
ssfn_select(&ctx,
    SSFN_FAMILY_SERIF, NULL,                        /* family */
    SSFN_STYLE_REGULAR | SSFN_STYLE_UNDERLINE,      /* style */
    64                                              /* size */
);

/* rasterize the first glyph in an UTF-8 string into a 32 bit packed pixel buffer */
/* returns how many bytes were consumed from the string */
ssfn_render(&ctx, &buf, "Hello");
ssfn_render(&ctx, &buf, "ello");
ssfn_render(&ctx, &buf, "llo");                     /* assuming there's a ligature for "ll" in the font */
ssfn_render(&ctx, &buf, "o");

/* free resources */
ssfn_free(&ctx);                                    /* free the renderer context's internal buffers */
```

There's more, you can use the C++ wrapper class, you can select font by it's name and you can also query the
bounding box for example, and `ssfn_text` will render entire strings into newly allocated pixel buffers,
read the [API reference](https://gitlab.com/bztsrc/scalable-font2/blob/master/docs/API.md). This only works
for truecolor pixels, because it needs a separate alpha-channel to work properly.

As with the simple renderer, the header contains everything, no additional linking required! Gzip uncompressor
also included in this 28k of code, no need to link with zlib!

Font Editor
-----------

Besides of the [converter](https://gitlab.com/bztsrc/scalable-font2/tree/master/sfnconv) that can import various
font formats, there's also a professional [SSFN font editor](https://gitlab.com/bztsrc/scalable-font2/tree/master/sfnedit)
available, which is in par with FontForge as long as the feature set concerned, but unlike FontForge it is a portable
executable (no installation nor third party DLLs required):

<img alt="Scalable Screen Font Editor" src="https://gitlab.com/bztsrc/scalable-font2/raw/master/docs/sfnedit.png">

NOTE for MacOS users: SDL2 is not statically compiled in, instead it is boundled with the application. It is possible that
on MacOS BigSur and above this doesn't work. If the application doesn't start because it's missing SDL2, just download and
install the latest dmg from official [libsdl.org](http://libsdl.org/download-2.0.php) to fix.

License
-------

Both the renderers, the converter and editor utilities are licensed under the terms of MIT license in the hope
that they will be useful.

IMPORTANT NOTE: although the file format is licensed under MIT, it is possible that the font stored in a SSFN
file is NOT! Always consult the license field in the font's header! See `sfnconv -d`.

Dependencies
------------

The simple renderer calls no functions at all and therefore has no dependencies whatsoever (not even libc
nor compiler built-ins). Absolutely nothing save for it's two global variables.

As for the normal renderer, with a define you can also compile it dependency-free (with some limitations)
but all dependencies are provided as built-ins by gcc or by libc:
 - `realloc()` and `free()` from libc (stdlib.h)
 - `memcmp()` and `memset()` from libc (string.h)

The scalable font converter is built on the **freetype2** library to read vector font files (compiled
statically into libsfn by default). The bitmap font converter has no dependencies. Libsfn uses **zlib**
to write gzip deflate compressed files on-the-fly (statically linked, read is supported without
zlib). Font vectorization needs **potrace** (also a stripped down version statically linked).

The editor uses SDL2 with an X11 fallback, and naturally depends on libsfn. For SDL2, it can be compiled
with both static and dynamic linkage (static requires that you manually compile SDL2).

The test applications also use SDL2 to create a window and display the rendered texts.

Changes to SSFN 1.0
-------------------

SSFN 2.0 uses a conceptually different method to rasterize and scale glyphs. SSFN 2.0 does not only support mixed
glyphs in a font (like SSFN 1.0), but it can also mix bitmap, pixmap and vector layers within one glyph, and the
result is much more readable in small rendering sizes. To do that, it only supports 32 bit packed pixel buffers,
but takes care of the alpha-blending.

### Disadvantages

- the renderer uses considerably more memory than SSFN 1.0 (~64k vs. ~24k, and with glyph cache enabled probably megabytes).
- it does not return the outline nor the rasterized glyph any more.

### Advantages

- simpler API, fewer functions (load + select + render + free).
- high quality, anti-aliased rendering directly to 32 bit pixel buffers.
- faster rendering with less malloc calls and internal glyph caching.
- smaller, more compact font files.
- transparent uncompression of gzipped fonts.
- support for non-standardized and user-defined ligatures.
- support for color glyphs.
- separate libsfn library to manipulate font files.

Known Bugs
----------

Nothing that I know of in the renderer. But no programmer can test their own code properly. I've ran the SSFN renderer
through valgrind with many different font files with all the tests without problems. If you find a bug, please
use the [Issue Tracker](https://gitlab.com/bztsrc/scalable-font2/issues) on gitlab and let me know.

With the importers, there's the neverending issue with querying the advance offsets correctly from FreeType2. You might
need to adjust some manually.

Loading kerning information from SplineFontDB .sfd files is not implemented as of yet.

Hinting is supported by the format, but not implementeted by the renderer (libsfn supports loading and saving the hinting
grids). With the new bilinear interpolation scaler I'm not sure it's needed, we'll see how testing goes.

Vectorizing bitmap fonts could use some work. Potrace is not the best on 16 x 16 bitmaps.

Contributors
------------

I'd like to say thanks to @mrjbom, who tested the library throughfully and pointed out many usability issues and
by that helped me a lot to improve this code.

Authors
-------

- SSFN format, converter, editor and renderers: bzt
- STBI (PD, original zlib decoder): Sean Barrett
- potrace (GPL): Peter Selinger
- libimagequant (GPL): Jef Poskanzer, Greg Roelofs, Kornel Lesiński
- freetype2 (GPL): David Turner, Robert Wilhelm, Werner Lemberg
