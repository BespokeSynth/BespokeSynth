/*
 * ssfn.h
 * https://gitlab.com/bztsrc/scalable-font2
 *
 * Copyright (C) 2020 - 2024 bzt
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * @brief Scalable Screen Font renderers
 *
 */

#ifndef _SSFN_H_
#define _SSFN_H_

#define SSFN_VERSION 0x0200

#ifdef  __cplusplus
extern "C" {
# ifndef __THROW
#  define __THROW throw()
# endif
#else
# ifndef __THROW
#  define __THROW
# endif
#endif
/* if stdint.h was not included before us */
#if !defined(_STDINT) && !defined(_STDINT_H)
typedef unsigned char       uint8_t;
typedef unsigned short int  uint16_t;
typedef short int           int16_t;
typedef unsigned int        uint32_t;
#ifndef _UINT64_T
typedef unsigned long int   uint64_t;
#endif
#endif

/***** file format *****/

/* magic bytes */
#define SSFN_MAGIC "SFN2"
#define SSFN_COLLECTION "SFNC"
#define SSFN_ENDMAGIC "2NFS"

/* ligatures area */
#define SSFN_LIG_FIRST          0xF000
#define SSFN_LIG_LAST           0xF8FF

/* font family group in font type byte */
#define SSFN_TYPE_FAMILY(x)     ((x)&15)
#define SSFN_FAMILY_SERIF       0
#define SSFN_FAMILY_SANS        1
#define SSFN_FAMILY_DECOR       2
#define SSFN_FAMILY_MONOSPACE   3
#define SSFN_FAMILY_HAND        4

/* font style flags in font type byte */
#define SSFN_TYPE_STYLE(x)      (((x)>>4)&15)
#define SSFN_STYLE_REGULAR      0
#define SSFN_STYLE_BOLD         1
#define SSFN_STYLE_ITALIC       2
#define SSFN_STYLE_USRDEF1      4     /* user defined variant 1 */
#define SSFN_STYLE_USRDEF2      8     /* user defined variant 2 */

/* contour commands */
#define SSFN_CONTOUR_MOVE       0
#define SSFN_CONTOUR_LINE       1
#define SSFN_CONTOUR_QUAD       2
#define SSFN_CONTOUR_CUBIC      3

/* glyph fragments, kerning groups and hinting grid info */
#define SSFN_FRAG_CONTOUR       0
#define SSFN_FRAG_BITMAP        1
#define SSFN_FRAG_PIXMAP        2
#define SSFN_FRAG_KERNING       3
#define SSFN_FRAG_HINTING       4

/* main SSFN header, 32 bytes */
#ifndef _MSC_VER
#define _pack __attribute__((packed))
#else
#define _pack
#pragma pack(push)
#pragma pack(1)
#endif
typedef struct {
    uint8_t     magic[4];             /* SSFN magic bytes */
    uint32_t    size;                 /* total size in bytes */
    uint8_t     type;                 /* font family and style */
    uint8_t     features;             /* format features and revision */
    uint8_t     width;                /* overall width of the font */
    uint8_t     height;               /* overall height of the font */
    uint8_t     baseline;             /* horizontal baseline in grid pixels */
    uint8_t     underline;            /* position of under line in grid pixels */
    uint16_t    fragments_offs;       /* offset of fragments table */
    uint32_t    characters_offs;      /* characters table offset */
    uint32_t    ligature_offs;        /* ligatures table offset */
    uint32_t    kerning_offs;         /* kerning table offset */
    uint32_t    cmap_offs;            /* color map offset */
} _pack ssfn_font_t;
#ifdef _MSC_VER
#pragma pack(pop)
#endif

/***** renderer API *****/
#define SSFN_FAMILY_ANY      0xff     /* select the first loaded font */
#define SSFN_FAMILY_BYNAME   0xfe     /* select font by its unique name */

/* additional styles not stored in fonts */
#define SSFN_STYLE_UNDERLINE   16     /* under line glyph */
#define SSFN_STYLE_STHROUGH    32     /* strike through glyph */
#define SSFN_STYLE_NOAA        64     /* no anti-aliasing */
#define SSFN_STYLE_NOKERN     128     /* no kerning */
#define SSFN_STYLE_NODEFGLYPH 256     /* don't draw default glyph */
#define SSFN_STYLE_NOCACHE    512     /* don't cache rasterized glyph */
#define SSFN_STYLE_NOHINTING 1024     /* no auto hinting grid (not used as of now) */
#define SSFN_STYLE_RTL       2048     /* render right-to-left */
#define SSFN_STYLE_ABS_SIZE  4096     /* scale absoulte height */
#define SSFN_STYLE_NOSMOOTH  8192     /* no edge-smoothing for bitmaps */

/* error codes */
#define SSFN_OK                 0     /* success */
#define SSFN_ERR_ALLOC         -1     /* allocation error */
#define SSFN_ERR_BADFILE       -2     /* bad SSFN file format */
#define SSFN_ERR_NOFACE        -3     /* no font face selected */
#define SSFN_ERR_INVINP        -4     /* invalid input */
#define SSFN_ERR_BADSTYLE      -5     /* bad style */
#define SSFN_ERR_BADSIZE       -6     /* bad size */
#define SSFN_ERR_NOGLYPH       -7     /* glyph (or kerning info) not found */

#define SSFN_SIZE_MAX         192     /* biggest size we can render */
#define SSFN_ITALIC_DIV         4     /* italic angle divisor, glyph top side pushed width / this pixels */
#define SSFN_PREC               4     /* precision in bits */

/* destination frame buffer context */
typedef struct {
    uint8_t *ptr;                     /* pointer to the buffer */
    int w;                            /* width (positive: ARGB, negative: ABGR pixels) */
    int h;                            /* height */
    uint16_t p;                       /* pitch, bytes per line */
    int x;                            /* cursor x */
    int y;                            /* cursor y */
    uint32_t fg;                      /* foreground color */
    uint32_t bg;                      /* background color */
} ssfn_buf_t;

/* cached bitmap struct */
#define SSFN_DATA_MAX       65536
typedef struct {
    uint16_t p;                       /* data buffer pitch, bytes per line */
    uint8_t h;                        /* data buffer height */
    uint8_t o;                        /* overlap of glyph, scaled to size */
    uint8_t x;                        /* advance x, scaled to size */
    uint8_t y;                        /* advance y, scaled to size */
    uint8_t a;                        /* ascender, scaled to size */
    uint8_t d;                        /* descender, scaled to size */
    uint8_t data[SSFN_DATA_MAX];      /* data buffer */
} ssfn_glyph_t;

/* character metrics */
typedef struct {
    uint8_t t;                        /* type and overlap */
    uint8_t n;                        /* number of fragments */
    uint8_t w;                        /* width */
    uint8_t h;                        /* height */
    uint8_t x;                        /* advance x */
    uint8_t y;                        /* advance y */
} ssfn_chr_t;

#ifdef SSFN_PROFILING
#include <string.h>
#include <sys/time.h>
#endif

/* renderer context */
typedef struct {
#ifdef SSFN_MAXLINES
    const ssfn_font_t *fnt[5][16];    /* static font registry */
#else
    const ssfn_font_t **fnt[5];       /* dynamic font registry */
#endif
    const ssfn_font_t *s;             /* explicitly selected font */
    const ssfn_font_t *f;             /* font selected by best match */
    ssfn_glyph_t ga;                  /* glyph sketch area */
    ssfn_glyph_t *g;                  /* current glyph pointer */
#ifdef SSFN_MAXLINES
    uint16_t p[SSFN_MAXLINES*2];
#else
    ssfn_glyph_t ***c[17];            /* glyph cache */
    uint16_t *p;
    char **bufs;                      /* allocated extra buffers */
#endif
    ssfn_chr_t *rc;                   /* pointer to current character */
    int numbuf, lenbuf, np, ap, ox, oy, ax;
    int mx, my, lx, ly;               /* move to coordinates, last coordinates */
    int len[5];                       /* number of fonts in registry */
    int family;                       /* required family */
    int style;                        /* required style */
    int size;                         /* required size */
    int line;                         /* calculate line height */
#ifdef SSFN_PROFILING
    uint64_t lookup, raster, blit, kern;/* profiling accumulators */
#endif
} ssfn_t;

/***** API function protoypes *****/

uint32_t ssfn_utf8(char **str);                                                   /* decode UTF-8 sequence */

/* normal renderer */
int ssfn_load(ssfn_t *ctx, const void *data);                                     /* add an SSFN to context */
int ssfn_select(ssfn_t *ctx, int family, const char *name, int style, int size);  /* select font to use */
int ssfn_render(ssfn_t *ctx, ssfn_buf_t *dst, const char *str);                   /* render a glyph to a pixel buffer */
int ssfn_bbox(ssfn_t *ctx, const char *str, int *w, int *h, int *left, int *top); /* get bounding box */
ssfn_buf_t *ssfn_text(ssfn_t *ctx, const char *str, unsigned int fg);             /* renders text to a newly allocated buffer */
int ssfn_mem(ssfn_t *ctx);                                                        /* return how much memory is used */
void ssfn_free(ssfn_t *ctx);                                                      /* free context */
#define ssfn_error(err) (err<0&&err>=-7?ssfn_errstr[-err]:"Unknown error")        /* return string for error code */
extern const char *ssfn_errstr[];

/* simple renderer */
extern ssfn_font_t *ssfn_src;                                                     /* font buffer */
extern ssfn_buf_t ssfn_dst;                                                       /* destination frame buffer */
int ssfn_putc(uint32_t unicode);                                                  /* render console bitmap font */

/***** renderer implementations *****/

/*** these go for both renderers ***/
#if (defined(SSFN_IMPLEMENTATION) || defined(SSFN_CONSOLEBITMAP_PALETTE) || \
    defined(SSFN_CONSOLEBITMAP_HICOLOR) || defined(SSFN_CONSOLEBITMAP_TRUECOLOR)) && !defined(SSFN_COMMON)
#define SSFN_COMMON

/**
 * Error code strings
 */
const char *ssfn_errstr[] = { "",
    "Memory allocation error",
    "Bad file format",
    "No font face found",
    "Invalid input value",
    "Invalid style",
    "Invalid size",
    "Glyph not found"
};

/**
 * Decode an UTF-8 multibyte, advance string pointer and return UNICODE. Watch out, no input checks
 *
 * @param **s pointer to an UTF-8 string pointer
 * @return unicode, and *s moved to next multibyte sequence
 */
uint32_t ssfn_utf8(char **s)
{
    uint32_t c = **s;

    if((**s & 128) != 0) {
        if(!(**s & 32)) { c = ((**s & 0x1F)<<6)|(*(*s+1) & 0x3F); *s += 1; } else
        if(!(**s & 16)) { c = ((**s & 0xF)<<12)|((*(*s+1) & 0x3F)<<6)|(*(*s+2) & 0x3F); *s += 2; } else
        if(!(**s & 8)) { c = ((**s & 0x7)<<18)|((*(*s+1) & 0x3F)<<12)|((*(*s+2) & 0x3F)<<6)|(*(*s+3) & 0x3F); *s += 3; }
        else c = 0;
    }
    (*s)++;
    return c;
}
#endif

#ifdef SSFN_IMPLEMENTATION
/*** normal renderer (ca. 28k, fully featured with error checking) ***/

# ifndef NULL
#  define NULL (void*)0
# endif
# ifndef size_t
#  ifndef __SIZE_TYPE__
#  define __SIZE_TYPE__ uint32_t
#  endif
   typedef __SIZE_TYPE__ size_t;
# endif
# ifndef inline
#  ifdef _MSC_VER
#   define inline __inline
#  else
#   define inline __inline__
#  endif
# endif

#ifndef _STRING_H_
extern int memcmp (const void *__s1, const void *__s2, size_t __n) __THROW;
extern void *memset (void *__s, int __c, size_t __n) __THROW;
#endif

/* Clang does not have built-ins */
# ifndef SSFN_memcmp
#  ifdef __builtin_memcmp
#   define SSFN_memcmp __builtin_memcmp
#  else
#   ifndef SSFN_MAXLINES
#    define SSFN_memcmp memcmp
#   else
static int SSFN_memcmp(const void *__s1, const void *__s2, size_t __n)
{   unsigned char *a = (unsigned char *)__s1, *b = (unsigned char *)__s2;
    if(__n > 0) { while(__n-- > 0) { if(*a != *b) { return *a - *b; } a++; b++; } } return 0; }
#   endif
#  endif
# endif

# ifndef SSFN_memset
#  ifdef __builtin_memset
#   define SSFN_memset __builtin_memset
#  else
#   ifndef SSFN_MAXLINES
#    define SSFN_memset memset
#   else
static void *SSFN_memset(void *__s, int __c, size_t __n)
{ unsigned char *a = __s; if(__n > 0) { while(__n-- > 0) *a++ = __c; } return __s; }
#   endif
#  endif
# endif

# ifndef SSFN_MAXLINES

#  ifndef SSFN_realloc
#   ifdef __builtin_realloc
#    define SSFN_realloc __builtin_realloc
#   else
#    define SSFN_realloc realloc
     extern void *realloc (void *__ptr, size_t __size) __THROW;
#   endif
#  endif

#  ifndef SSFN_free
#   ifdef __builtin_free
#    define SSFN_free __builtin_free
#   else
#    define SSFN_free free
     extern void free (void *p) __THROW;
#   endif
#  endif

# endif /* if !SSFN_MAXLINES */

/*** Private functions ***/

/* parse character table */
static uint8_t *_ssfn_c(const ssfn_font_t *font, const char *str, int *len, uint32_t *unicode)
{
    uint32_t i, j, u = -1U;
    uint16_t *l;
    uint8_t *ptr, *s;

    *len = 0; *unicode = 0;
    if(!font || !font->characters_offs || !str || !*str) return NULL;

    if(font->ligature_offs) {
        for(l = (uint16_t*)((uint8_t*)font + font->ligature_offs), i = 0; l[i] && u == -1U; i++) {
            for(ptr = (uint8_t*)font + l[i], s = (uint8_t*)str; *ptr && *ptr == *s; ptr++, s++);
            if(!*ptr) { u = SSFN_LIG_FIRST + i; break; }
        }
    }
    if(u == -1U) {
        /* inline ssfn_utf8 to workaround -O2 bug in gcc 11.1 */
        s = (uint8_t*)str; u = *s;
        if((*s & 128) != 0) {
            if(!(*s & 32)) { u = ((*s & 0x1F)<<6)|(*(s+1) & 0x3F); s++; } else
            if(!(*s & 16)) { u = ((*s & 0xF)<<12)|((*(s+1) & 0x3F)<<6)|(*(s+2) & 0x3F); s += 2; } else
            if(!(*s & 8)) { u = ((*s & 0x7)<<18)|((*(s+1) & 0x3F)<<12)|((*(s+2) & 0x3F)<<6)|(*(s+3) & 0x3F); s += 3; }
            else u = 0;
        }
        s++;
    }
    *len = (int)(s - (uint8_t*)str);
    *unicode = u;
    for(ptr = (uint8_t*)font + font->characters_offs, i = 0; i < 0x110000; i++) {
        if(ptr[0] == 0xFF) { i += 65535; ptr++; }
        else if((ptr[0] & 0xC0) == 0xC0) { j = (((ptr[0] & 0x3F) << 8) | ptr[1]); i += j; ptr += 2; }
        else if((ptr[0] & 0xC0) == 0x80) { j = (ptr[0] & 0x3F); i += j; ptr++; }
        else {
            if(i == u) return ptr;
            ptr += 6 + ptr[1] * (ptr[0] & 0x40 ? 6 : 5);
        }
    }
    return NULL;
}

/* add a line to contour */
static void _ssfn_l(ssfn_t *ctx, int p, int h, int x, int y)
{
    if(x < 0 || y < 0 || x >= p || y >= h || (
        ((ctx->lx + (1 << (SSFN_PREC-1))) >> SSFN_PREC) == ((x + (1 << (SSFN_PREC-1))) >> SSFN_PREC) &&
        ((ctx->ly + (1 << (SSFN_PREC-1))) >> SSFN_PREC) == ((y + (1 << (SSFN_PREC-1))) >> SSFN_PREC))) return;
#ifdef SSFN_MAXLINES
    if(ctx->np >= SSFN_MAXLINES*2-2) return;
#else
    if(ctx->ap <= ctx->np) {
        ctx->ap = ctx->np + 512;
        ctx->p = (uint16_t*)SSFN_realloc(ctx->p, ctx->ap * sizeof(uint16_t));
        if(!ctx->p) { ctx->ap = ctx->np = 0; return; }
    }
#endif
    if(!ctx->np) {
        ctx->p[0] = ctx->mx;
        ctx->p[1] = ctx->my;
        ctx->np += 2;
    }
    ctx->p[ctx->np+0] = x;
    ctx->p[ctx->np+1] = y;
    ctx->np += 2;
    ctx->lx = x; ctx->ly = y;
}

/* add a Bezier curve to contour */
static void _ssfn_b(ssfn_t *ctx, int p,int h, int x0,int y0, int x1,int y1, int x2,int y2, int x3,int y3, int l)
{
    int m0x, m0y, m1x, m1y, m2x, m2y, m3x, m3y, m4x, m4y,m5x, m5y;
    if(l<4 && (x0!=x3 || y0!=y3)) {
        m0x = ((x1-x0)/2) + x0;     m0y = ((y1-y0)/2) + y0;
        m1x = ((x2-x1)/2) + x1;     m1y = ((y2-y1)/2) + y1;
        m2x = ((x3-x2)/2) + x2;     m2y = ((y3-y2)/2) + y2;
        m3x = ((m1x-m0x)/2) + m0x;  m3y = ((m1y-m0y)/2) + m0y;
        m4x = ((m2x-m1x)/2) + m1x;  m4y = ((m2y-m1y)/2) + m1y;
        m5x = ((m4x-m3x)/2) + m3x;  m5y = ((m4y-m3y)/2) + m3y;
        _ssfn_b(ctx, p,h, x0,y0, m0x,m0y, m3x,m3y, m5x,m5y, l+1);
        _ssfn_b(ctx, p,h, m5x,m5y, m4x,m4y, m2x,m2y, x3,y3, l+1);
    }
    if(l) _ssfn_l(ctx, p,h, x3, y3);
}

#ifndef SSFN_MAXLINES
static void _ssfn_fc(ssfn_t *ctx)
{
    int i, j, k;
    if(!ctx) return;
    for(k = 0; k <= 16; k++)
        if(ctx->c[k]) {
            for(j = 0; j < 256; j++)
                if(ctx->c[k][j]) {
                    for(i = 0; i < 256; i++)
                        if(ctx->c[k][j][i]) SSFN_free(ctx->c[k][j][i]);
                    SSFN_free(ctx->c[k][j]);
                }
            SSFN_free(ctx->c[k]);
            ctx->c[k] = NULL;
        }
}

/*
 * gzip deflate uncompressor from stb_image.h with minor modifications to reduce dependency
 * stb_image - v2.23 - public domain image loader - http://nothings.org/stb_image.h
 */
#define SSFN__ZFAST_BITS  9
#define SSFN__ZFAST_MASK  ((1 << SSFN__ZFAST_BITS) - 1)

typedef struct
{
   uint16_t fast[1 << SSFN__ZFAST_BITS];
   uint16_t firstcode[16];
   int maxcode[17];
   uint16_t firstsymbol[16];
   unsigned char  size[288];
   uint16_t value[288];
} _ssfn__zhuffman;

inline static int _ssfn__bitreverse16(int n)
{
  n = ((n & 0xAAAA) >>  1) | ((n & 0x5555) << 1);
  n = ((n & 0xCCCC) >>  2) | ((n & 0x3333) << 2);
  n = ((n & 0xF0F0) >>  4) | ((n & 0x0F0F) << 4);
  n = ((n & 0xFF00) >>  8) | ((n & 0x00FF) << 8);
  return n;
}

inline static int _ssfn__bit_reverse(int v, int bits)
{
   return _ssfn__bitreverse16(v) >> (16-bits);
}

static int _ssfn__zbuild_huffman(_ssfn__zhuffman *z, unsigned char *sizelist, int num)
{
   int i,k=0;
   int code, next_code[16], sizes[17];

   SSFN_memset(sizes, 0, sizeof(sizes));
   SSFN_memset(z->fast, 0, sizeof(z->fast));
   for (i=0; i < num; ++i)
      ++sizes[sizelist[i]];
   sizes[0] = 0;
   for (i=1; i < 16; ++i)
      if (sizes[i] > (1 << i))
         return 0;
   code = 0;
   for (i=1; i < 16; ++i) {
      next_code[i] = code;
      z->firstcode[i] = (uint16_t) code;
      z->firstsymbol[i] = (uint16_t) k;
      code = (code + sizes[i]);
      if (sizes[i])
         if (code-1 >= (1 << i)) return 0;
      z->maxcode[i] = code << (16-i);
      code <<= 1;
      k += sizes[i];
   }
   z->maxcode[16] = 0x10000;
   for (i=0; i < num; ++i) {
      int s = sizelist[i];
      if (s) {
         int c = next_code[s] - z->firstcode[s] + z->firstsymbol[s];
         uint16_t fastv = (uint16_t) ((s << 9) | i);
         z->size [c] = (unsigned char) s;
         z->value[c] = (uint16_t) i;
         if (s <= SSFN__ZFAST_BITS) {
            int j = _ssfn__bit_reverse(next_code[s],s);
            while (j < (1 << SSFN__ZFAST_BITS)) {
               z->fast[j] = fastv;
               j += (1 << s);
            }
         }
         ++next_code[s];
      }
   }
   return 1;
}

typedef struct
{
   unsigned char *zbuffer;
   int num_bits;
   uint32_t code_buffer;

   char *zout;
   char *zout_start;
   char *zout_end;

   _ssfn__zhuffman z_length, z_distance;
} _ssfn__zbuf;

inline static unsigned char _ssfn__zget8(_ssfn__zbuf *z)
{
   return *z->zbuffer++;
}

static void _ssfn__fill_bits(_ssfn__zbuf *z)
{
   do {
      z->code_buffer |= (unsigned int) _ssfn__zget8(z) << z->num_bits;
      z->num_bits += 8;
   } while (z->num_bits <= 24);
}

inline static unsigned int _ssfn__zreceive(_ssfn__zbuf *z, int n)
{
   unsigned int k;
   if (z->num_bits < n) _ssfn__fill_bits(z);
   k = z->code_buffer & ((1 << n) - 1);
   z->code_buffer >>= n;
   z->num_bits -= n;
   return k;
}

static int _ssfn__zhuffman_decode_slowpath(_ssfn__zbuf *a, _ssfn__zhuffman *z)
{
   int b,s,k;
   k = _ssfn__bit_reverse(a->code_buffer, 16);
   for (s=SSFN__ZFAST_BITS+1; ; ++s)
      if (k < z->maxcode[s])
         break;
   if (s == 16) return -1;
   b = (k >> (16-s)) - z->firstcode[s] + z->firstsymbol[s];
   a->code_buffer >>= s;
   a->num_bits -= s;
   return z->value[b];
}

inline static int _ssfn__zhuffman_decode(_ssfn__zbuf *a, _ssfn__zhuffman *z)
{
   int b,s;
   if (a->num_bits < 16) _ssfn__fill_bits(a);
   b = z->fast[a->code_buffer & SSFN__ZFAST_MASK];
   if (b) {
      s = b >> 9;
      a->code_buffer >>= s;
      a->num_bits -= s;
      return b & 511;
   }
   return _ssfn__zhuffman_decode_slowpath(a, z);
}

static int _ssfn__zexpand(_ssfn__zbuf *z, char *zout)
{
   char *q;
   unsigned int cur, limit;
#ifdef __GNUC__
#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wuse-after-free"
#endif
   z->zout = zout; cur = (unsigned int) (z->zout - z->zout_start); limit = (unsigned int) (z->zout_end - z->zout_start);
   if(limit == 8) { if(z->zout_start[0] != 'S' || z->zout_start[1] != 'F' || z->zout_start[2] != 'N') return 0; limit = *((uint32_t*)&z->zout_start[4]); } else return 0;
   q = (char *) SSFN_realloc(z->zout_start, limit);
   if (q == NULL) return 0;
   z->zout_start = q; z->zout = q + cur; z->zout_end = q + limit;
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
   return 1;
}

static int _ssfn__zlength_base[31]={3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258,0,0};
static int _ssfn__zlength_extra[31]={0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0,0,0};
static int _ssfn__zdist_base[32]={1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577,0,0};
static int _ssfn__zdist_extra[32]={0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

static int _ssfn__parse_huffman_block(_ssfn__zbuf *a)
{
   char *zout = a->zout;
   for(;;) {
      int z = _ssfn__zhuffman_decode(a, &a->z_length);
      if (z < 256) {
         if (z < 0) return 0;
         if (zout >= a->zout_end) { if (!_ssfn__zexpand(a, zout)) return 0; zout = a->zout; }
         *zout++ = (char) z;
      } else {
         unsigned char *p;
         int len,dist;
         if (z == 256) { a->zout = zout; return 1; }
         z -= 257;
         len = _ssfn__zlength_base[z];
         if (_ssfn__zlength_extra[z]) len += _ssfn__zreceive(a, _ssfn__zlength_extra[z]);
         z = _ssfn__zhuffman_decode(a, &a->z_distance);
         if (z < 0) return 0;
         dist = _ssfn__zdist_base[z];
         if (_ssfn__zdist_extra[z]) dist += _ssfn__zreceive(a, _ssfn__zdist_extra[z]);
         if (zout - a->zout_start < dist) return 0;
         if (zout + len > a->zout_end) {
            if (!_ssfn__zexpand(a, zout)) return 0;
            zout = a->zout;
         }
         p = (unsigned char *) (zout - dist);
         if (dist == 1) {unsigned char v = *p;if (len) { do *zout++ = v; while (--len); }
         } else { if (len) { do *zout++ = *p++; while (--len); } }
      }
   }
}

static int _ssfn__compute_huffman_codes(_ssfn__zbuf *a)
{
   static unsigned char length_dezigzag[19] = { 16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15 };
   _ssfn__zhuffman z_codelength;
   unsigned char lencodes[286+32+137];
   unsigned char codelength_sizes[19];
   int i,n;

   int hlit  = _ssfn__zreceive(a,5) + 257;
   int hdist = _ssfn__zreceive(a,5) + 1;
   int hclen = _ssfn__zreceive(a,4) + 4;
   int ntot  = hlit + hdist;

   SSFN_memset(codelength_sizes, 0, sizeof(codelength_sizes));
   for (i=0; i < hclen; ++i) {
      int s = _ssfn__zreceive(a,3);
      codelength_sizes[length_dezigzag[i]] = (unsigned char) s;
   }
   if (!_ssfn__zbuild_huffman(&z_codelength, codelength_sizes, 19)) return 0;

   n = 0;
   while (n < ntot) {
      int c = _ssfn__zhuffman_decode(a, &z_codelength);
      if (c < 0 || c >= 19) return 0;
      if (c < 16)
         lencodes[n++] = (unsigned char) c;
      else {
         unsigned char fill = 0;
         if (c == 16) {
            c = _ssfn__zreceive(a,2)+3;
            if (n == 0) return 0;
            fill = lencodes[n-1];
         } else
         if (c == 17) c = _ssfn__zreceive(a,3)+3;
         else { c = _ssfn__zreceive(a,7)+11; }
         if (ntot - n < c) return 0;
         SSFN_memset(lencodes+n, fill, c);
         n += c;
      }
   }
   if (n != ntot) return 0;
   if (!_ssfn__zbuild_huffman(&a->z_length, lencodes, hlit)) return 0;
   if (!_ssfn__zbuild_huffman(&a->z_distance, lencodes+hlit, hdist)) return 0;
   return 1;
}

inline static int _ssfn__parse_uncompressed_block(_ssfn__zbuf *a)
{
   unsigned char header[4];
   int len,nlen,k;
   if (a->num_bits & 7)
      _ssfn__zreceive(a, a->num_bits & 7);
   k = 0;
   while (a->num_bits > 0) {
      header[k++] = (unsigned char) (a->code_buffer & 255);
      a->code_buffer >>= 8;
      a->num_bits -= 8;
   }
   while (k < 4)
      header[k++] = _ssfn__zget8(a);
   len  = header[1] * 256 + header[0];
   nlen = header[3] * 256 + header[2];
   if (nlen != (len ^ 0xffff)) return 0;
   if (a->zout + len > a->zout_end)
      if (!_ssfn__zexpand(a, a->zout)) return 0;
   for(k = 0; k < len; k++)
       a->zout[k] = a->zbuffer[k];
   a->zbuffer += len;
   a->zout += len;
   return 1;
}

static unsigned char _ssfn__zdefault_length[288], _ssfn__zdefault_distance[32];
static void _ssfn__init_zdefaults(void)
{
   int i;
   for (i=0; i <= 143; ++i) _ssfn__zdefault_length[i] = 8;
   for (   ; i <= 255; ++i) _ssfn__zdefault_length[i] = 9;
   for (   ; i <= 279; ++i) _ssfn__zdefault_length[i] = 7;
   for (   ; i <= 287; ++i) _ssfn__zdefault_length[i] = 8;

   for (i=0; i <=  31; ++i) _ssfn__zdefault_distance[i] = 5;
}

static int _ssfn__parse_zlib(_ssfn__zbuf *a)
{
   int fin, type;
   a->num_bits = 0;
   a->code_buffer = 0;
   do {
      fin = _ssfn__zreceive(a,1);
      type = _ssfn__zreceive(a,2);
      if (type == 0) { if (!_ssfn__parse_uncompressed_block(a)) return 0; }
      else if (type == 3) { return 0; }
      else {
         if (type == 1) {
            if (!_ssfn__zbuild_huffman(&a->z_length  , _ssfn__zdefault_length  , 288)) return 0;
            if (!_ssfn__zbuild_huffman(&a->z_distance, _ssfn__zdefault_distance,  32)) return 0;
         } else {
            if (!_ssfn__compute_huffman_codes(a)) return 0;
         }
         if (!_ssfn__parse_huffman_block(a)) return 0;
      }
   } while (!fin);
   return 1;
}

char *_ssfn_zlib_decode(const char *buffer)
{
   _ssfn__zbuf a;
   char *p = (char *) SSFN_realloc(NULL, 8);
   if (p == NULL) return NULL;
   a.zbuffer = (unsigned char *) buffer;
   a.zout_start = p; a.zout = p; a.zout_end = p + 8;
   _ssfn__init_zdefaults();
   if (_ssfn__parse_zlib(&a)) { return a.zout_start; } else { SSFN_free(a.zout_start); return NULL; }
}
#endif /* if !SSFN_MAXLINES */

/*** Public API implementation ***/

/**
 * Load a font or font collection into renderer context
 *
 * @param ctx rendering context
 * @param font SSFN font or font collection in memory
 * @return error code
 */
int ssfn_load(ssfn_t *ctx, const void *data)
{
    const ssfn_font_t *font = (const ssfn_font_t *)data;
    ssfn_font_t *fnt, *end;
    int family;
#ifndef SSFN_MAXLINES
    uint8_t c, r, *ptr = (uint8_t *)font;
#endif
    if(!ctx || !font)
        return SSFN_ERR_INVINP;
    if(((uint8_t *)font)[0] == 0x1f && ((uint8_t *)font)[1] == 0x8b) {
#ifdef SSFN_MAXLINES
        return SSFN_ERR_BADFILE;
#else
        ptr += 2;
        if(*ptr++ != 8) return SSFN_ERR_BADFILE;
        c = *ptr++; ptr += 6;
        if(c & 4) { r = *ptr++; r += (*ptr++ << 8); ptr += r; }
        if(c & 8) { while(*ptr++ != 0); }
        if(c & 16) { while(*ptr++ != 0); }
        font = (ssfn_font_t*)_ssfn_zlib_decode((const char*)ptr);
        if(!font) return SSFN_ERR_BADFILE;
        ctx->bufs = (char**)SSFN_realloc(ctx->bufs, (ctx->numbuf + 1) * sizeof(char*));
        if(!ctx->bufs) { ctx->numbuf = 0; return SSFN_ERR_ALLOC; }
        ctx->bufs[ctx->numbuf++] = (char*)font;
        ctx->lenbuf += font->size;
#endif
    }
    if(!SSFN_memcmp(font->magic, SSFN_COLLECTION, 4)) {
        end = (ssfn_font_t*)((uint8_t*)font + font->size);
        for(fnt = (ssfn_font_t*)((uint8_t*)font + 8); fnt < end && !ssfn_load(ctx, (const void *)fnt);
            fnt = (ssfn_font_t*)((uint8_t*)fnt + fnt->size));
    } else {
        family = SSFN_TYPE_FAMILY(font->type);
        if(SSFN_memcmp(font->magic, SSFN_MAGIC, 4) || SSFN_memcmp((uint8_t*)font + font->size - 4, SSFN_ENDMAGIC, 4) ||
            family > SSFN_FAMILY_HAND || font->fragments_offs >= font->size || font->characters_offs >= font->size ||
            font->ligature_offs >= font->size || font->kerning_offs >= font->size || font->cmap_offs >= font->size ||
            font->fragments_offs >= font->characters_offs) {
                return SSFN_ERR_BADFILE;
        } else {
            ctx->len[family]++;
#ifdef SSFN_MAXLINES
            if(ctx->len[family] > 15) return SSFN_ERR_ALLOC;
#else
            ctx->fnt[family] = (const ssfn_font_t**)SSFN_realloc(ctx->fnt[family], ctx->len[family]*sizeof(void*));
            if(!ctx->fnt[family]) {
                ctx->len[family] = 0;
                return SSFN_ERR_ALLOC;
            } else
#endif
                ctx->fnt[family][ctx->len[family]-1] = font;
        }
#ifndef SSFN_MAXLINES
        _ssfn_fc(ctx);
#endif
    }
    return SSFN_OK;
}

/**
 * Free renderer context
 *
 * @param ctx rendering context
 */
void ssfn_free(ssfn_t *ctx)
{
#ifndef SSFN_MAXLINES
    int i;
#endif
    if(!ctx) return;
#ifndef SSFN_MAXLINES
    _ssfn_fc(ctx);
    if(ctx->bufs) {
        for(i = 0; i < ctx->numbuf; i++)
            if(ctx->bufs[i]) SSFN_free(ctx->bufs[i]);
        SSFN_free(ctx->bufs);
    }
    for(i = 0; i < 5; i++)
        if(ctx->fnt[i]) SSFN_free(ctx->fnt[i]);
    if(ctx->p) SSFN_free(ctx->p);
#endif
    SSFN_memset(ctx, 0, sizeof(ssfn_t));
}

/**
 * Returns how much memory a context consumes
 *
 * @param ctx rendering context
 * @return total memory used by that context in bytes
 */
int ssfn_mem(ssfn_t *ctx)
{
#ifdef SSFN_MAXLINES
    return ctx ? sizeof(ssfn_t) : 0;
#else
    int i, j, k, ret = sizeof(ssfn_t);

    if(!ctx) return 0;
    for(i = 0; i < 5; i++) ret += ctx->len[i] * sizeof(ssfn_font_t*);
    ret += ctx->lenbuf;
    for(k = 0; k <= 16; k++) {
        if(ctx->c[k]) {
            for(j = 0; j < 256; j++)
                if(ctx->c[k][j]) {
                    for(i = 0; i < 256; i++)
                        if(ctx->c[k][j][i]) ret += 8 + ctx->c[k][j][i]->p * ctx->c[k][j][i]->h;
                    ret += 256 * sizeof(void*);
                }
            ret += 256 * sizeof(void*);
        }
    }
    if(ctx->p) ret += ctx->ap * sizeof(uint16_t);
    return ret;
#endif
}

/**
 * Set up rendering parameters
 *
 * @param ctx rendering context
 * @param family one of SSFN_FAMILY_*
 * @param name NULL or UTF-8 string if family is SSFN_FAMILY_BYNAME
 * @param style OR'd values of SSFN_STYLE_*
 * @param size how big glyph it should render, 8 - 192
 * @return error code
 */
int ssfn_select(ssfn_t *ctx, int family, const char *name, int style, int size)
{
    int i, j, l;

    if(!ctx) return SSFN_ERR_INVINP;
#ifndef SSFN_MAXLINES
    _ssfn_fc(ctx);
#endif
    if((style & ~0x5FFF)) return SSFN_ERR_BADSTYLE;
    if(size < 8 || size > SSFN_SIZE_MAX) return SSFN_ERR_BADSIZE;

    if(family == SSFN_FAMILY_BYNAME) {
        if(!name || !name[0]) return SSFN_ERR_INVINP;
        for(l=0; name[l]; l++);
        for(i=0; i < 5; i++) {
            for(j=0; j < ctx->len[i]; j++) {
                if(!SSFN_memcmp(name, (uint8_t*)&ctx->fnt[i][j]->magic + sizeof(ssfn_font_t), l)) {
                    ctx->s = ctx->fnt[i][j];
                    goto familyfound;
                }
            }
        }
        return SSFN_ERR_NOFACE;
    } else {
        if(family != SSFN_FAMILY_ANY && (family > SSFN_FAMILY_HAND || !ctx->len[family])) return SSFN_ERR_NOFACE;
        ctx->s = NULL;
    }
familyfound:
    ctx->f = NULL;
    ctx->family = family;
    ctx->style = style;
    ctx->size = size;
    ctx->line = 0;
    return SSFN_OK;
}

/**
 * Render a glyph to a pixel buffer
 *
 * @param ctx rendering context
 * @param dst destination buffer
 * @param str pointer to an UTF-8 string
 * @return number of bytes parsed in str (zero means end of string) or error code
 */

int ssfn_render(ssfn_t *ctx, ssfn_buf_t *dst, const char *str)
{
    ssfn_font_t **fl;
    uint8_t *ptr = NULL, *frg, *end, *tmp, color, ci = 0, cb = 0, cs, dec[65536];
    uint16_t r[640];
    uint32_t unicode, P, O, *Op, *Ol, sR, sG, sB, sA, bA;
    int ret = 0, i, j, k, l, p, m, n, o, s, x, y, w, h, H, a, A, b, B, nr, uix, uax;
    int ox, oy, y0, y1, Y0, Y1, x0, x1, X0, X1, X2, xs, ys, yp, pc, fB, fG, fR, fA, bB, bG, bR, dB, dG, dR, dA;
#ifdef SSFN_PROFILING
    struct timeval tv0, tv1, tvd;
    gettimeofday(&tv0, NULL);
#endif
#define PUTPIXEL  O = *Ol;bR = (O >> (16 - cs)) & 0xFF; bG = (O >> 8) & 0xFF; bB = (O >> cs) & 0xFF; bA = (O >> 24) & 0xFF;\
    bB += ((fB - bB) * fA) >> 8; bG += ((fG - bG) * fA) >> 8; bR += ((fR - bR) * fA) >> 8; bA += ((fA - bA) * fA) >> 8;\
    *Ol = (bA << 24) | (bR << (16 - cs)) | (bG << 8) | (bB << cs);

    if(!ctx || !str) return SSFN_ERR_INVINP;
    if(!*str) return 0;
    if(*str == '\r') { dst->x = 0; return 1; }
    if(*str == '\n') { dst->x = 0; dst->y += ctx->line ? ctx->line : ctx->size; return 1; }

    if(ctx->s) {
        ctx->f = ctx->s;
        ptr = _ssfn_c(ctx->f, str, &ret, &unicode);
    } else {
        /* find best match */
        p = ctx->family;
        ctx->f = NULL;
again:  if(p >= SSFN_FAMILY_BYNAME) { n = 0; m = 4; } else n = m = p;
        for(; n <= m; n++) {
            fl = (ssfn_font_t **)ctx->fnt[n];
            if(ctx->style & 3) {
                /* check if we have a specific ctx->f for the requested style and size */
                for(i=0;i<ctx->len[n];i++)
                    if(((fl[i]->type>>4) & 3) == (ctx->style & 3) && fl[i]->height == ctx->size &&
                        (ptr = _ssfn_c(fl[i], str, &ret, &unicode))) { ctx->f = fl[i]; break; }
                /* if not, check if we have the requested size (for bitmap fonts) */
                if(!ptr)
                    for(i=0;i<ctx->len[n];i++)
                        if(fl[i]->height == ctx->size && (ptr = _ssfn_c(fl[i], str, &ret, &unicode))) { ctx->f = fl[i]; break; }
                /* if neither size+style nor size matched, look for style match */
                if(!ptr)
                    for(i=0;i<ctx->len[n];i++)
                        if(((fl[i]->type>>4) & 3) == (ctx->style & 3) && (ptr = _ssfn_c(fl[i], str, &ret, &unicode)))
                            { ctx->f = fl[i]; break; }
                /* if bold italic was requested, check if we have at least bold or italic */
                if(!ptr && (ctx->style & 3) == 3)
                    for(i=0;i<ctx->len[n];i++)
                        if(((fl[i]->type>>4) & 3) && (ptr = _ssfn_c(fl[i], str, &ret, &unicode))) { ctx->f = fl[i]; break; }
            }
            /* last resort, get the first ctx->f which has a glyph for this multibyte, no matter style */
            if(!ptr) {
                for(i=0;i<ctx->len[n];i++)
                    if((ptr = _ssfn_c(fl[i], str, &ret, &unicode))) { ctx->f = fl[i]; break; }
            }
        }
        /* if glyph still not found, try any family group */
        if(!ptr && p != SSFN_FAMILY_ANY) { p = SSFN_FAMILY_ANY; goto again; }
    }
    if(!ptr) {
        if(ctx->style & SSFN_STYLE_NODEFGLYPH) return SSFN_ERR_NOGLYPH;
        else {
            unicode = 0;
            if(ctx->family >= SSFN_FAMILY_BYNAME) { n = 0; m = 4; } else n = m = ctx->family;
            for(; n <= m && !ptr; n++)
                if(ctx->len[n] && ctx->fnt[n][0] && !(*((uint8_t*)ctx->fnt[n][0] + ctx->fnt[n][0]->characters_offs) & 0x80))
                    { ctx->f = ctx->fnt[n][0]; ptr = (uint8_t*)ctx->f + ctx->f->characters_offs; }
        }
        if(!ptr) return SSFN_ERR_NOGLYPH;
    }
    if(!ctx->f || !ctx->f->height || !ctx->size) return SSFN_ERR_NOFACE;
    if((unicode >> 16) > 0x10) return SSFN_ERR_INVINP;
    ctx->rc = (ssfn_chr_t*)ptr; ptr += sizeof(ssfn_chr_t);
    H = (ctx->style & SSFN_STYLE_ABS_SIZE) || SSFN_TYPE_FAMILY(ctx->f->type) == SSFN_FAMILY_MONOSPACE || !ctx->f->baseline ?
        ctx->size : ctx->size * ctx->f->height / ctx->f->baseline;

#ifdef SSFN_PROFILING
    gettimeofday(&tv1, NULL); tvd.tv_sec = tv1.tv_sec - tv0.tv_sec; tvd.tv_usec = tv1.tv_usec - tv0.tv_usec;
    if(tvd.tv_usec < 0) { tvd.tv_sec--; tvd.tv_usec += 1000000L; }
    ctx->lookup += tvd.tv_sec * 1000000L + tvd.tv_usec;
    memcpy(&tv0, &tv1, sizeof(struct timeval));
#endif
    /* render glyph into cache */
#ifndef SSFN_MAXLINES
    if(!(ctx->style & SSFN_STYLE_NOCACHE) && ctx->c[unicode >> 16] && ctx->c[unicode >> 16][(unicode >> 8) & 0xFF] &&
        ctx->c[unicode >> 16][(unicode >> 8) & 0xFF][unicode & 0xFF]) {
            ctx->g = ctx->c[unicode >> 16][(unicode >> 8) & 0xFF][unicode & 0xFF];
    } else
#endif
    {
        h = ctx->style & SSFN_STYLE_NOAA ? H : (ctx->size > ctx->f->height ? (ctx->size + 4) & ~3 : ctx->f->height);
        ci = (ctx->style & SSFN_STYLE_ITALIC) && !(SSFN_TYPE_STYLE(ctx->f->type) & SSFN_STYLE_ITALIC);
        cb = (ctx->style & SSFN_STYLE_BOLD) && !(SSFN_TYPE_STYLE(ctx->f->type) & SSFN_STYLE_BOLD) ? (ctx->f->height+64)>>6 : 0;
        w = (ctx->rc->w * h + ctx->f->height - 1) / ctx->f->height;
        if(w > SSFN_SIZE_MAX) { h = h * SSFN_SIZE_MAX / w; w = SSFN_SIZE_MAX; }
        p = w + (ci ? h / SSFN_ITALIC_DIV : 0) + cb;
        /* failsafe, should never happen */
        if(p * h >= SSFN_DATA_MAX) return SSFN_ERR_BADSIZE;
#ifndef SSFN_MAXLINES
        if(!(ctx->style & SSFN_STYLE_NOCACHE)) {
            if(!ctx->c[unicode >> 16]) {
                ctx->c[unicode >> 16] = (ssfn_glyph_t***)SSFN_realloc(NULL, 256 * sizeof(void*));
                if(!ctx->c[unicode >> 16]) return SSFN_ERR_ALLOC;
                SSFN_memset(ctx->c[unicode >> 16], 0, 256 * sizeof(void*));
            }
            if(!ctx->c[unicode >> 16][(unicode >> 8) & 0xFF]) {
                ctx->c[unicode >> 16][(unicode >> 8) & 0xFF] = (ssfn_glyph_t**)SSFN_realloc(NULL, 256 * sizeof(void*));
                if(!ctx->c[unicode >> 16][(unicode >> 8) & 0xFF]) return SSFN_ERR_ALLOC;
                SSFN_memset(ctx->c[unicode >> 16][(unicode >> 8) & 0xFF], 0, 256 * sizeof(void*));
            }
            ctx->g = ctx->c[unicode >> 16][(unicode >> 8) & 0xFF][unicode & 0xFF] = (ssfn_glyph_t*)SSFN_realloc(NULL, p * h + 8);
            if(!ctx->c[unicode >> 16][(unicode >> 8) & 0xFF][unicode & 0xFF]) return SSFN_ERR_ALLOC;
        } else
#endif
            ctx->g = &ctx->ga;
        x = (ctx->rc->x > 0 && ci ? (ctx->f->height - ctx->f->baseline) * h / SSFN_ITALIC_DIV / ctx->f->height : 0);
        ctx->g->p = p;
        ctx->g->h = h;
        ctx->g->x = (ctx->rc->x + x > 255 ? 255 : ctx->rc->x + x);
        ctx->g->y = ctx->rc->y;
        ctx->g->o = (ctx->rc->t & 0x3F) + x;
        SSFN_memset(&ctx->g->data, 0xFF, p * h);
        color = 0xFE; ctx->g->a = ctx->g->d = 0;
        for(n = 0; n < ctx->rc->n; n++) {
            if(ptr[0] == 255 && ptr[1] == 255) { color = ptr[2]; ptr += ctx->rc->t & 0x40 ? 6 : 5; continue; }
            x = ((ptr[0] + cb) << SSFN_PREC) * h / ctx->f->height; y = (ptr[1] << SSFN_PREC) * h / ctx->f->height;
            if(ctx->rc->t & 0x40) { m = (ptr[5] << 24) | (ptr[4] << 16) | (ptr[3] << 8) | ptr[2]; ptr += 6; }
            else { m = (ptr[4] << 16) | (ptr[3] << 8) | ptr[2]; ptr += 5; }
            frg = (uint8_t*)ctx->f +  m;
            if(!(frg[0] & 0x80)) {
                /* contour */
                j = (frg[0] & 0x3F);
                if(frg[0] & 0x40) { j <<= 8; j |= frg[1]; frg++; }
                j++; frg++; tmp = frg; frg += (j+3)/4; ctx->np = 0;
                for(i = 0; i < j; i++) {
                    k = (frg[0] << SSFN_PREC) * h / ctx->f->height + x; m = (frg[1] << SSFN_PREC) * h / ctx->f->height + y;
                    switch((tmp[i >> 2] >> ((i & 3) << 1)) & 3) {
                        case SSFN_CONTOUR_MOVE: ctx->mx = ctx->lx = k; ctx->my = ctx->ly = m; frg += 2; break;
                        case SSFN_CONTOUR_LINE: _ssfn_l(ctx, p << SSFN_PREC, h << SSFN_PREC, k, m); frg += 2; break;
                        case SSFN_CONTOUR_QUAD:
                            a = (frg[2] << SSFN_PREC) * h / ctx->f->height + x; A = (frg[3] << SSFN_PREC) * h / ctx->f->height + y;
                            _ssfn_b(ctx, p << SSFN_PREC,h << SSFN_PREC, ctx->lx,ctx->ly, ((a-ctx->lx)/2)+ctx->lx,
                                ((A-ctx->ly)/2)+ctx->ly, ((k-a)/2)+a,((A-m)/2)+m, k,m, 0);
                            frg += 4;
                        break;
                        case SSFN_CONTOUR_CUBIC:
                            a = (frg[2] << SSFN_PREC) * h / ctx->f->height + x; A = (frg[3] << SSFN_PREC) * h / ctx->f->height + y;
                            b = (frg[4] << SSFN_PREC) * h / ctx->f->height + x; B = (frg[5] << SSFN_PREC) * h / ctx->f->height + y;
                            _ssfn_b(ctx, p << SSFN_PREC,h << SSFN_PREC, ctx->lx,ctx->ly, a,A, b,B, k,m, 0);
                            frg += 6;
                        break;
                    }
                }
                /* close path */
                if(ctx->mx != ctx->lx || ctx->my != ctx->ly) { ctx->p[ctx->np+0] = ctx->mx; ctx->p[ctx->np+1] = ctx->my; ctx->np += 2; }
                /* add rasterized vector layers to cached glyph */
                if(ctx->np > 4) {
                    for(b = A = B = o = 0; b < h; b++, B += p) {
                        a = b << SSFN_PREC;
                        for(nr = 0, i = 0; i < ctx->np - 3; i += 2) {
                            if( (ctx->p[i+1] < a && ctx->p[i+3] >= a) ||
                                (ctx->p[i+3] < a && ctx->p[i+1] >= a)) {
                                    if((ctx->p[i+1] >> SSFN_PREC) == (ctx->p[i+3] >> SSFN_PREC))
                                        x = (((int)ctx->p[i]+(int)ctx->p[i+2])>>1);
                                    else
                                        x = ((int)ctx->p[i]) + ((a - (int)ctx->p[i+1])*
                                            ((int)ctx->p[i+2] - (int)ctx->p[i])/
                                            ((int)ctx->p[i+3] - (int)ctx->p[i+1]));
                                    x >>= SSFN_PREC;
                                    if(ci) x += (h - b) / SSFN_ITALIC_DIV;
                                    if(cb && !o) {
                                        if(ctx->g->data[B + x] != color) { o = -cb; A = cb; }
                                        else { o = cb; A = -cb; }
                                    }
                                    for(k = 0; k < nr && x > r[k]; k++);
                                    for(l = nr; l > k; l--) r[l] = r[l-1];
                                    r[k] = x;
                                    nr++;
                            }
                        }
                        if(nr > 1 && nr & 1) { r[nr - 2] = r[nr - 1]; nr--; }
                        if(nr) {
                            if(ctx->g->d < y + b) ctx->g->d = y + b;
                            for(i = 0; i < nr - 1; i += 2) {
                                l = r[i] + o; m = r[i + 1] + A;
                                if(l < 0) l = 0;
                                if(m > p) m = p;
                                if(i > 0 && l < r[i - 1] + A) l = r[i - 1] + A;
                                for(; l < m; l++)
                                    ctx->g->data[B + l] = ctx->g->data[B + l] == color ? 0xFF : color;
                            }
                        }
                    }
                }
            } else if((frg[0] & 0x60) == 0x00) {
                /* bitmap */
                B = ((frg[0] & 0x1F) + 1) << 3; A = frg[1] + 1; x >>= SSFN_PREC; y >>= SSFN_PREC;
                b = B * h / ctx->f->height; a = A * h / ctx->f->height;
                if(ctx->g->d < y + a) ctx->g->d = y + a;
                frg += 2;
                for(j = 0; j < a; j++) {
                    k = j * A / a;
                    l = (y + j) * p + x + (ci ? (h - y - j) / SSFN_ITALIC_DIV : 0);
                    for(i = 0; i < b; i++) {
                        m = i * B / b;
                        if(frg[(k * B + m) >> 3] & (1 << (m & 7))) {
                            for(o = 0; o <= cb; o++)
                                ctx->g->data[l + i + o] = color;
                        }
                    }
                }
                if(!(ctx->style & (SSFN_STYLE_NOAA|SSFN_STYLE_NOSMOOTH))) {
                    m = color == 0xFD ? 0xFC : 0xFD; o = y * p + p + x;
                    for(k = h; k > ctx->f->height + 4; k -= 2*ctx->f->height) {
                        for(j = 1, l = o; j < a - 1; j++, l += p)
                            for(i = 1; i < b - 1; i++) {
                                if(ctx->g->data[l + i] == 0xFF && (ctx->g->data[l + i - p] == color ||
                                    ctx->g->data[l + i + p] == color) && (ctx->g->data[l + i - 1] == color ||
                                    ctx->g->data[l + i + 1] == color)) ctx->g->data[l + i] = m;
                            }
                        for(j = 1, l = o; j < a - 1; j++, l += p)
                            for(i = 1; i < b - 1; i++) {
                                if(ctx->g->data[l + i] == m) ctx->g->data[l + i] = color;
                            }
                    }
                }
            } else if((frg[0] & 0x60) == 0x20) {
                /* pixmap */
                k = (((frg[0] & 0x1F) << 8) | frg[1]) + 1; B = frg[2] + 1; A = frg[3] + 1; x >>= SSFN_PREC; y >>= SSFN_PREC;
                b = B * h / ctx->f->height; a = A * h / ctx->f->height;
                if(ctx->g->d < y + a) ctx->g->d = y + a;
                frg += 4; end = frg + k; i = 0;
                while(frg < end) {
                    l = ((*frg++) & 0x7F) + 1;
                    if(frg[-1] & 0x80) {
                        while(l--) dec[i++] = *frg;
                        frg++;
                    } else while(l--) dec[i++] = *frg++;
                }
                for(j = 0; j < a; j++) {
                    k = j * A / a * B;
                    l = (y + j) * p + x + (ci ? (h - y - j) / SSFN_ITALIC_DIV : 0);
                    for(i = 0; i < b; i++) {
                        m = dec[k + i * B / b];
                        if(m != 0xFF) ctx->g->data[l + i] = m;
                    }
                }
            }
            color = 0xFE;
        }
        ctx->g->a = ctx->f->baseline;
        if(ctx->g->d > ctx->g->a + 1) ctx->g->d -= ctx->g->a + 1; else ctx->g->d = 0;
#ifdef SSFN_DEBUGGLYPH
        printf("\nU+%06X size %d p %d h %d base %d under %d overlap %d ascender %d descender %d advance x %d advance y %d cb %d\n",
            unicode, ctx->size,p,h,ctx->f->baseline,ctx->f->underline,ctx->g->o,ctx->g->a,ctx->g->d,ctx->g->x,ctx->g->y,cb);
        for(j = 0; j < h; j++) { printf("%3d: ", j); for(i = 0; i < p; i++) { if(ctx->g->data[j*p+i] == 0xFF) printf(j == ctx->g->a ? "_" : "."); else printf("%x", ctx->g->data[j*p+i] & 0xF); } printf("\n"); }
#endif
#ifdef SSFN_PROFILING
        gettimeofday(&tv1, NULL); tvd.tv_sec = tv1.tv_sec - tv0.tv_sec; tvd.tv_usec = tv1.tv_usec - tv0.tv_usec;
        if(tvd.tv_usec < 0) { tvd.tv_sec--; tvd.tv_usec += 1000000L; }
        ctx->raster += tvd.tv_sec * 1000000L + tvd.tv_usec;
        memcpy(&tv0, &tv1, sizeof(struct timeval));
#endif
    }
    if(dst) {
        /* blit glyph from cache into buffer */
        h = H;
        if(h > ctx->line) ctx->line = h;
        w = ctx->g->p * h / ctx->g->h;
        s = ((ctx->g->x - ctx->g->o) * h + ctx->f->height - 1) / ctx->f->height;
        n = ctx->size > 16 ? 2 : 1;
        if(w < n) w = n;
        if(s < n) s = n;
        if(ctx->g->x) {
            ctx->ox = ox = ((ctx->g->o * h + ctx->f->height - 1) / ctx->f->height) + (ctx->style & SSFN_STYLE_RTL ? w : 0);
            ctx->oy = oy = (ctx->g->a * h + ctx->f->height - 1) / ctx->f->height;
        } else { ctx->ox = ox = w / 2; ctx->oy = oy = 0; }
        if(dst->ptr) {
            j = dst->w < 0 ? -dst->w : dst->w;
            cs = dst->w < 0 ? 16 : 0;
            cb = (h + 64) >> 6; uix = w > s ? w : s; uax = 0;
            n = (ctx->f->underline * h + ctx->f->height - 1) / ctx->f->height;
#ifdef SSFN_DEBUGGLYPH
            printf("Scaling to w %d h %d (glyph %d %d, cache %d %d, font %d)\n",
                w,h,ctx->rc->w,ctx->rc->h,ctx->g->p,ctx->g->h,ctx->f->height);
#endif
            fR = (dst->fg >> 16) & 0xFF; fG = (dst->fg >> 8) & 0xFF; fB = (dst->fg >> 0) & 0xFF; fA = (dst->fg >> 24) & 0xFF;
            bR = (dst->bg >> 16) & 0xFF; bG = (dst->bg >> 8) & 0xFF; bB = (dst->bg >> 0) & 0xFF; O = 0xFF000000;
            Op = (uint32_t*)(dst->ptr + dst->p * (dst->y - oy) + ((dst->x - ox) << 2));
            for (y = 0; y < h && dst->y + y - oy < dst->h; y++, Op += dst->p >> 2) {
                if(dst->y + y - oy < 0) continue;
                y0 = (y << 8) * ctx->g->h / h; Y0 = y0 >> 8; y1 = ((y + 1) << 8) * ctx->g->h / h; Y1 = y1 >> 8; Ol = Op;
                for (x = 0; x < w && dst->x + x - ox < j; x++, Ol++) {
                    if(dst->x + x - ox < 0) continue;
                    m = 0; sR = sG = sB = sA = bA = 0;
                    if(!dst->bg) {
                        /* real linear frame buffers should be accessed only as uint32_t on 32 bit boundary */
                        O = *Ol;
                        bA = (O >> 24) & 0xFF;
                        bR = (O >> (16 - cs)) & 0xFF;
                        bG = (O >> 8) & 0xFF;
                        bB = (O >> cs) & 0xFF;
                    }
                    x0 = (x << 8) * ctx->g->p / w; X0 = x0 >> 8; x1 = ((x + 1) << 8) * ctx->g->p / w; X1 = x1 >> 8;
                    for(ys = y0; ys < y1; ys += 256) {
                        if(ys >> 8 == Y0) { yp = 256 - (ys & 0xFF); ys &= ~0xFF; if(yp > y1 - y0) yp = y1 - y0; }
                        else if(ys >> 8 == Y1) yp = y1 & 0xFF; else yp = 256;
                        X2 = (ys >> 8) * ctx->g->p;
                        for(xs = x0; xs < x1; xs += 256) {
                            if (xs >> 8 == X0) {
                                k = 256 - (xs & 0xFF); xs &= ~0xFF; if(k > x1 - x0) k = x1 - x0; pc = k == 256 ? yp : (k * yp)>>8;
                            } else
                            if (xs >> 8 == X1) { k = x1 & 0xFF; pc = k == 256 ? yp : (k * yp) >> 8; }
                            else pc = yp;
                            m += pc;
                            k = ctx->g->data[X2 + (xs >> 8)];
                            if(k == 0xFF) {
                                sB += bB * pc; sG += bG * pc; sR += bR * pc; sA += 255;
                            } else {
                                if(k == 0xFE || !ctx->f->cmap_offs) {
                                    dB = fB; dG = fG; dR = fR; dA = fA;
                                } else {
                                    P = *((uint32_t*)((uint8_t*)ctx->f + ctx->f->cmap_offs + (k << 2)));
                                    dR = (P >> 16) & 0xFF; dG = (P >> 8) & 0xFF; dB = (P >> 0) & 0xFF; dA = (P >> 24) & 0xFF;
                                }
                                if(dA == 255) {
                                    sB += dB * pc; sG += dG * pc; sR += dR * pc; sA += dA * pc;
                                } else {
                                    sB += (dB * dA + bB * (255 - dA)) * pc / 255; sG += (dG * dA + bG * (255 - dA)) * pc / 255;
                                    sR += (dR * dA + bR * (255 - dA)) * pc / 255; sA += dA * pc;
                                }
                            }
                        }
                    }
                    if(m) { sR /= m; sG /= m; sB /= m; sA /= m; } else { sR >>= 8; sG >>= 8; sB >>= 8; sA >>= 8; }
                    if(ctx->style & SSFN_STYLE_NOAA) sA = sA > 127 ? 255 : 0;
                    if(sA > 15) {
                        *Ol = ((sA > 255 ? 255 : (sA > bA ? sA : bA)) << 24) |
                            ((sR > 255 ? 255 : sR) << (16 - cs)) | ((sG > 255 ? 255 : sG) << 8) | ((sB > 255 ? 255 : sB) << cs);
                        if(y == n) { if(uix > x) { uix = x; } if(uax < x) { uax = x; } }
                    }
                }
            }
            if(ctx->style & SSFN_STYLE_UNDERLINE) {
                uix -= cb + 1; uax += cb + 2;
                if(uax < uix) uax = uix + 1;
                k = (w > s ? w : s);
                Op = (uint32_t*)(dst->ptr + dst->p * (dst->y - oy + n) + ((dst->x - ox - 1) << 2));
                for (y = n; y < n + cb && dst->y + y - oy < dst->h; y++, Op += dst->p >> 2) {
                    if(dst->y + y - oy < 0) continue;
                    for (Ol = Op, x = 0; x <= k && dst->x + x - ox < j; x++, Ol++) {
                        if(dst->x + x - ox < 0 || (x > uix && x < uax)) continue;
                        PUTPIXEL;
                    }
                }
            }
            if(ctx->style & SSFN_STYLE_STHROUGH) {
                n = (h >> 1); k = (w > s ? w : s) + 1;
                Op = (uint32_t*)(dst->ptr + dst->p * (dst->y - oy + n) + ((dst->x - ox - 1) << 2));
                for (y = n; y < n + cb && dst->y + y - oy < dst->h; y++, Op += dst->p >> 2) {
                    if(dst->y + y - oy < 0) continue;
                    for (Ol = Op, x = 0; x <= k && dst->x + x - ox < j; x++, Ol++) {
                        if(dst->x + x - ox < 0) continue;
                        PUTPIXEL;
                    }
                }
            }
#ifdef SSFN_PROFILING
            gettimeofday(&tv1, NULL); tvd.tv_sec = tv1.tv_sec - tv0.tv_sec;tvd.tv_usec = tv1.tv_usec - tv0.tv_usec;
            if(tvd.tv_usec < 0) { tvd.tv_sec--; tvd.tv_usec += 1000000L; }
            ctx->blit += tvd.tv_sec * 1000000L + tvd.tv_usec;
            memcpy(&tv0, &tv1, sizeof(struct timeval));
#endif
        }
        /* add advance and kerning */
        ctx->ax = (ctx->style & SSFN_STYLE_RTL ? -s : s);
        dst->x += ctx->ax;
        dst->y += (ctx->g->y * h + ctx->f->height - 1) / ctx->f->height;
        ptr = (uint8_t*)str + ret;
        if(!(ctx->style & SSFN_STYLE_NOKERN) && ctx->f->kerning_offs && _ssfn_c(ctx->f, (const char*)ptr, &i, &P) && P > 32) {
            ptr = (uint8_t*)ctx->rc + sizeof(ssfn_chr_t);
            /* check all kerning fragments, because we might have both vertical and horizontal kerning offsets */
            for(n = 0; n < ctx->rc->n; n++) {
                if(ptr[0] == 255 && ptr[1] == 255) { ptr += ctx->rc->t & 0x40 ? 6 : 5; continue; }
                x = ptr[0];
                if(ctx->rc->t & 0x40) { m = (ptr[5] << 24) | (ptr[4] << 16) | (ptr[3] << 8) | ptr[2]; ptr += 6; }
                else { m = (ptr[4] << 16) | (ptr[3] << 8) | ptr[2]; ptr += 5; }
                frg = (uint8_t*)ctx->f +  m;
                if((frg[0] & 0xE0) == 0xC0) {
                    k = (((frg[0] & 0x1F) << 8) | frg[1]) + 1; frg += 2;
                    while(k--) {
                        m = ((frg[2] & 0xF) << 16) | (frg[1] << 8) | frg[0];
                        if(P >= (uint32_t)m && P <= (uint32_t)(((frg[5] & 0xF) << 16) | (frg[4] << 8) | frg[3])) {
                            P -= m;
                            m = ctx->f->kerning_offs + ((((frg[2] >> 4) & 0xF) << 24) | (((frg[5] >> 4) & 0xF) << 16) |
                                (frg[7] << 8) | frg[6]);
                            tmp = (uint8_t*)ctx->f + m;
                            while(tmp < (uint8_t*)ctx->f + ctx->f->size - 4) {
                                if((tmp[0] & 0x7F) < P) {
                                    P -= (tmp[0] & 0x7F) + 1;
                                    tmp += 2 + (tmp[0] & 0x80 ? 0 : tmp[0] & 0x7F);
                                } else {
                                    y = (int)((signed char)tmp[1 + ((tmp[0] & 0x80) ? 0 : P)]) * h / ctx->f->height;
                                    if(x) dst->x += y; else dst->y += y;
                                    break;
                                }
                            }
                            break;
                        }
                        frg += 8;
                    }
                }
            }
#ifdef SSFN_PROFILING
            gettimeofday(&tv1, NULL); tvd.tv_sec = tv1.tv_sec - tv0.tv_sec; tvd.tv_usec = tv1.tv_usec - tv0.tv_usec;
            if(tvd.tv_usec < 0) { tvd.tv_sec--; tvd.tv_usec += 1000000L; }
            ctx->kern += tvd.tv_sec * 1000000L + tvd.tv_usec;
#endif
        }
    }
    return ret;
}

/**
 * Get the bounding box for a string

 * @param ctx rendering context
 * @param str string to measure
 * @param w returned width
 * @param h returned height
 * @param left returned left margin
 * @param top returned ascender
 * @return error code
 */
int ssfn_bbox(ssfn_t *ctx, const char *str, int *w, int *h, int *left, int *top)
{
    ssfn_buf_t buf;
    int ret, f = 1, l = 0, t = 0;

    if(!ctx || !str) return SSFN_ERR_INVINP;
    if(w) {*w = 0;} if(h) {*h = 0;} if(top) {*top = 0;} if(left) {*left = 0;}
    if(!*str) return SSFN_OK;
    SSFN_memset(&buf, 0, sizeof(ssfn_buf_t)); ctx->line = 0;
    while((ret = ssfn_render(ctx, &buf, str))) {
        if(ret < 0 || !ctx->g) return ret;
        if(f) { f = 0; l = ctx->ox; buf.x += l; }
        if(ctx->g->x) {
            if(ctx->oy > t) t = ctx->oy;
        } else {
            if(buf.w < ctx->g->p) buf.w = ctx->g->p;
            buf.h += ctx->g->y ? ctx->g->y : ctx->g->h;
        }
        str += ret;
    }
    if((ctx->style & SSFN_STYLE_ITALIC) && !(SSFN_TYPE_STYLE(ctx->f->type) & SSFN_STYLE_ITALIC))
        buf.x +=  ctx->size / SSFN_ITALIC_DIV - l;
    if(ctx->g->x) { if(w) {*w = buf.x;} if(h) {*h = ctx->line;} if(left) {*left = l;} if(top) {*top = t;} }
    else { if(w) {*w = buf.w;} if(h) {*h = buf.y;} if(top) {*top = 0;} if(left) {*left = 0;} }
    return SSFN_OK;
}

/**
 * Render text to a newly allocated pixel buffer
 *
 * @param ctx rendering context
 * @param str string to measure
 * @return a newly allocated pixel buffer or NULL
 */
ssfn_buf_t *ssfn_text(ssfn_t *ctx, const char *str, unsigned int fg)
{
#ifndef SSFN_MAXLINES
    ssfn_buf_t *buf;
    int ret;

    if(!ctx || !str) return NULL;
    buf = (ssfn_buf_t*)SSFN_realloc(NULL, sizeof(ssfn_buf_t));
    if(!buf) return NULL;
    SSFN_memset(buf, 0, sizeof(ssfn_buf_t));
    buf->fg = fg;
    if(!*str || ssfn_bbox(ctx, str, (int*)&buf->w, (int*)&buf->h, (int*)&buf->x, (int*)&buf->y) != SSFN_OK)
        return buf;
    buf->p = buf->w * sizeof(uint32_t);
    buf->ptr = (uint8_t*)SSFN_realloc(NULL, buf->p * buf->h);
    SSFN_memset(buf->ptr, 0, buf->p * buf->h);
    while((ret = ssfn_render(ctx, buf, str)) > 0)
        str += ret;
    if(ret != SSFN_OK) { SSFN_free(buf->ptr); SSFN_free(buf); return NULL; }
    return buf;
#else
    (void)ctx;
    (void)str;
    (void)fg;
    return NULL;
#endif
}

#endif /* SSFN_IMPLEMENTATION */

#if defined(SSFN_CONSOLEBITMAP_PALETTE) || defined(SSFN_CONSOLEBITMAP_HICOLOR) || defined(SSFN_CONSOLEBITMAP_TRUECOLOR)
/*** special console bitmap font renderer (ca. 1.5k, no dependencies, no memory allocation and no error checking) ***/

/**
 * public variables to configure
 */
ssfn_font_t *ssfn_src;          /* font buffer with an inflated bitmap font */
ssfn_buf_t ssfn_dst;            /* destination frame buffer */

/**
 * Minimal OS kernel console renderer
 *
 * @param unicode character
 * @return error code
 */
int ssfn_putc(uint32_t unicode)
{
# ifdef SSFN_CONSOLEBITMAP_PALETTE
#  define SSFN_PIXEL  uint8_t
# else
#  ifdef SSFN_CONSOLEBITMAP_HICOLOR
#   define SSFN_PIXEL uint16_t
#  else
#   define SSFN_PIXEL uint32_t
#  endif
# endif
    register SSFN_PIXEL *o, *p;
    register uint8_t *ptr, *chr = NULL, *frg;
    register int i, j, k, l, m, y = 0, w, s = ssfn_dst.p / sizeof(SSFN_PIXEL);

    if(!ssfn_src || ssfn_src->magic[0] != 'S' || ssfn_src->magic[1] != 'F' || ssfn_src->magic[2] != 'N' ||
        ssfn_src->magic[3] != '2' || !ssfn_dst.ptr || !ssfn_dst.p) return SSFN_ERR_INVINP;
    w = ssfn_dst.w < 0 ? -ssfn_dst.w : ssfn_dst.w;
    for(ptr = (uint8_t*)ssfn_src + ssfn_src->characters_offs, i = 0; i < 0x110000; i++) {
        if(ptr[0] == 0xFF) { i += 65535; ptr++; }
        else if((ptr[0] & 0xC0) == 0xC0) { j = (((ptr[0] & 0x3F) << 8) | ptr[1]); i += j; ptr += 2; }
        else if((ptr[0] & 0xC0) == 0x80) { j = (ptr[0] & 0x3F); i += j; ptr++; }
        else { if((uint32_t)i == unicode) { chr = ptr; break; } ptr += 6 + ptr[1] * (ptr[0] & 0x40 ? 6 : 5); }
    }
#ifdef SSFN_CONSOLEBITMAP_CONTROL
    i = ssfn_src->height; j = ssfn_dst.h - i - (ssfn_dst.h % i);
    if(chr && w) {
        if(unicode == '\t') ssfn_dst.x -= ssfn_dst.x % chr[4];
        if(ssfn_dst.x + chr[4] > w) { ssfn_dst.x = 0; ssfn_dst.y += i; }
    }
    if(unicode == '\n') ssfn_dst.y += i;
    if(j > 0 && ssfn_dst.y > j) {
        ssfn_dst.y = j;
        for(k = 0; k < j; k++)
            for(l = 0; l < ssfn_dst.p; l++) ssfn_dst.ptr[k * ssfn_dst.p + l] = ssfn_dst.ptr[(k + i) * ssfn_dst.p + l];
    }
    if(unicode == '\r' || unicode == '\n') { ssfn_dst.x = 0; return SSFN_OK; }
#endif
    if(!chr) return SSFN_ERR_NOGLYPH;
    ptr = chr + 6; o = (SSFN_PIXEL*)(ssfn_dst.ptr + ssfn_dst.y * ssfn_dst.p + ssfn_dst.x * sizeof(SSFN_PIXEL));
    for(i = 0; i < chr[1]; i++, ptr += chr[0] & 0x40 ? 6 : 5) {
        if(ptr[0] == 255 && ptr[1] == 255) continue;
        frg = (uint8_t*)ssfn_src + (chr[0] & 0x40 ? ((ptr[5] << 24) | (ptr[4] << 16) | (ptr[3] << 8) | ptr[2]) :
            ((ptr[4] << 16) | (ptr[3] << 8) | ptr[2]));
        if((frg[0] & 0xE0) != 0x80) continue;
        if(ssfn_dst.bg) {
            for(; y < ptr[1] && (!ssfn_dst.h || ssfn_dst.y + y < ssfn_dst.h); y++, o += s) {
                for(p = o, j = 0; j < chr[2] && (!w || ssfn_dst.x + j < w); j++, p++) *p = ssfn_dst.bg;
            }
        } else { o += (int)(ptr[1] - y) * s; y = ptr[1]; }
        k = ((frg[0] & 0x1F) + 1) << 3; j = frg[1] + 1; frg += 2;
        for(m = 1; j && (!ssfn_dst.h || ssfn_dst.y + y < ssfn_dst.h); j--, y++, o += s)
            for(p = o, l = 0; l < k; l++, p++, m <<= 1) {
                if(m > 0x80) { frg++; m = 1; }
                if(ssfn_dst.x + l >= 0 && (!w || ssfn_dst.x + l < w)) {
                    if(*frg & m) *p = ssfn_dst.fg; else
                    if(ssfn_dst.bg) *p = ssfn_dst.bg;
                }
            }
    }
    if(ssfn_dst.bg)
        for(; y < chr[3] && (!ssfn_dst.h || ssfn_dst.y + y < ssfn_dst.h); y++, o += s) {
            for(p = o, j = 0; j < chr[2] && (!w || ssfn_dst.x + j < w); j++, p++) *p = ssfn_dst.bg;
        }
    ssfn_dst.x += chr[4]; ssfn_dst.y += chr[5];
    return SSFN_OK;
}

#endif /* SSFN_CONSOLEBITMAP */

#ifdef  __cplusplus
}
/*** SSFN C++ Wrapper Class ***/
#include <string>

namespace SSFN {
#ifndef SSFN_IMPLEMENTATION
    class Font {
        public:
            Font();
            ~Font();

        public:
            int Load(const std::string &data);
            int Load(const void *data, int len);
            int Select(int family, const std::string &name, int style, int size);
            int Select(int family, const char *name, int style, int size);
            int Render(ssfn_buf_t *dst, const std::string &str);
            int Render(ssfn_buf_t *dst, const char *str);
            int BBox(const std::string &str, int *w, int *h, int *left, int *top);
            int BBox(const char *str, int *w, int *h, int *left, int *top);
            ssfn_buf_t *Text(const std::string &str, unsigned int fg);
            ssfn_buf_t *Text(const char *str, unsigned int fg);
            int LineHeight();
            int Mem();
            const std::string ErrorStr(int err);
    };
#else
    class Font {
        private:
            ssfn_t ctx;

        public:
            Font() { SSFN_memset(&this->ctx, 0, sizeof(ssfn_t)); }
            ~Font() { ssfn_free(&this->ctx); }

        public:
            int Load(const std::string &data) { return ssfn_load(&this->ctx,reinterpret_cast<const void*>(data.data())); }
            int Load(const void *data) { return ssfn_load(&this->ctx, data); }
            int Select(int family, const std::string &name, int style, int size)
                { return ssfn_select(&this->ctx, family, (char*)name.data(), style, size); }
            int Select(int family, char *name, int style, int size) { return ssfn_select(&this->ctx,family,name,style,size); }
            int Render(ssfn_buf_t *dst, const std::string &str) { return ssfn_render(&this->ctx,dst,(const char *)str.data()); }
            int Render(ssfn_buf_t *dst, const char *str) { return ssfn_render(&this->ctx, dst, str); }
            int BBox(const std::string &str, int *w, int *h, int *left, int *top)
                { return ssfn_bbox(&this->ctx, (const char *)str.data(), w, h, left, top); }
            int BBox(const char *str, int *w, int *h, int *left, int *top) { return ssfn_bbox(&this->ctx,str,w,h,left,top); }
            ssfn_buf_t *Text(const std::string &str, unsigned int fg)
                { return ssfn_text(&this->ctx,(const char*)str.data(), fg); }
            ssfn_buf_t *Text(const char *str, unsigned int fg) { return ssfn_text(&this->ctx, str, fg); }
            int LineHeight() { return this->ctx.line ? this->ctx.line : this->ctx.size; }
            int Mem() { return ssfn_mem(&this->ctx); }
            const std::string ErrorStr(int err) { return std::string(ssfn_error(err)); }
    };
#endif
}
#endif
/*               */
#endif /* _SSFN_H_ */
