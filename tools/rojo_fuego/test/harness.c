// Headless mGBA runner for Rojo Fuego+ testing.
// usage: harness ROM SCRIPT
// Script commands (one per line, # comments):
//   run N              advance N frames with no keys
//   hold KEYS N        hold KEYS for N frames (KEYS: A B S=select T=start R L U D < >, e.g. "AB")
//   mash KEYS N        tap KEYS every other frame for N frames
//   shot FILE          write the current frame as a binary PPM
//   w8/w16/w32 ADDR V  write memory (hex)
//   r8/r16/r32 ADDR    print memory (hex)
//   dump ADDR LEN FILE write memory to a file
//   copy SRC DST LEN   copy memory (hex)
//   save FILE / load FILE   savestate
#include <mgba/core/core.h>
#include <mgba/core/log.h>
#include <mgba/gba/core.h>
#include <mgba-util/vfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void nolog(struct mLogger *l, int c, enum mLogLevel lv, const char *f, va_list a) { (void)l; (void)c; (void)lv; (void)f; (void)a; }
static struct mLogger sLogger = { .log = nolog };

static struct mCore *core;
static color_t *buffer;
static unsigned w, h;

static int keymask(const char *s)
{
    int m = 0;
    for (; *s; s++) switch (*s) {
        case 'A': m |= 1 << 0; break;
        case 'B': m |= 1 << 1; break;
        case 'S': m |= 1 << 2; break;
        case 'T': m |= 1 << 3; break;
        case '>': m |= 1 << 4; break;
        case '<': m |= 1 << 5; break;
        case 'U': m |= 1 << 6; break;
        case 'D': m |= 1 << 7; break;
        case 'R': m |= 1 << 8; break;
        case 'L': m |= 1 << 9; break;
    }
    return m;
}

static void frames(int keys, int n, int mash)
{
    for (int i = 0; i < n; i++) {
        core->setKeys(core, (mash && (i & 1)) ? 0 : keys);
        core->runFrame(core);
    }
    core->setKeys(core, 0);
}

static void shot(const char *file)
{
    FILE *f = fopen(file, "wb");
    fprintf(f, "P6\n%u %u\n255\n", w, h);
    for (unsigned i = 0; i < w * h; i++) {
        uint32_t c = buffer[i];
        unsigned char px[3] = { c & 0xFF, (c >> 8) & 0xFF, (c >> 16) & 0xFF };
        fwrite(px, 1, 3, f);
    }
    fclose(f);
}

int main(int argc, char **argv)
{
    if (argc < 3) { fprintf(stderr, "usage: harness ROM SCRIPT\n"); return 1; }
    mLogSetDefaultLogger(&sLogger);
    core = mCoreFind(argv[1]);
    if (!core) { fprintf(stderr, "no core\n"); return 1; }
    core->init(core);
    mCoreInitConfig(core, NULL);
    core->desiredVideoDimensions(core, &w, &h);
    buffer = calloc(w * h, sizeof(color_t));
    core->setVideoBuffer(core, buffer, w);
    if (!mCoreLoadFile(core, argv[1])) { fprintf(stderr, "load failed\n"); return 1; }
    struct VFile *sav = VFileMemChunk(NULL, 0);
    core->loadSave(core, sav);
    core->reset(core);

    FILE *sc = fopen(argv[2], "r");
    char line[512];
    while (fgets(line, sizeof line, sc)) {
        char cmd[32] = "", a[256] = "", b[256] = "", c[256] = "";
        if (line[0] == '#' || sscanf(line, "%31s %255s %255s %255s", cmd, a, b, c) < 1)
            continue;
        if (!strcmp(cmd, "run")) frames(0, atoi(a), 0);
        else if (!strcmp(cmd, "hold")) frames(keymask(a), atoi(b), 0);
        else if (!strcmp(cmd, "mash")) frames(keymask(a), atoi(b), 1);
        else if (!strcmp(cmd, "shot")) shot(a);
        else if (!strcmp(cmd, "w8")) core->busWrite8(core, strtoul(a, 0, 16), strtoul(b, 0, 16));
        else if (!strcmp(cmd, "w16")) core->busWrite16(core, strtoul(a, 0, 16), strtoul(b, 0, 16));
        else if (!strcmp(cmd, "w32")) core->busWrite32(core, strtoul(a, 0, 16), strtoul(b, 0, 16));
        else if (!strcmp(cmd, "r8")) printf("%s = %02X\n", a, core->busRead8(core, strtoul(a, 0, 16)));
        else if (!strcmp(cmd, "r16")) printf("%s = %04X\n", a, core->busRead16(core, strtoul(a, 0, 16)));
        else if (!strcmp(cmd, "r32")) printf("%s = %08X\n", a, core->busRead32(core, strtoul(a, 0, 16)));
        else if (!strcmp(cmd, "copy")) {
            unsigned src = strtoul(a, 0, 16), dst = strtoul(b, 0, 16), len = strtoul(c, 0, 16);
            for (unsigned i = 0; i < len; i++) core->busWrite8(core, dst + i, core->busRead8(core, src + i));
        }
        else if (!strcmp(cmd, "dump")) {
            unsigned addr = strtoul(a, 0, 16), len = strtoul(b, 0, 16);
            FILE *f = fopen(c, "wb");
            for (unsigned i = 0; i < len; i++) fputc(core->busRead8(core, addr + i), f);
            fclose(f);
        }
        else if (!strcmp(cmd, "save")) {
            size_t sz = core->stateSize(core);
            void *st = malloc(sz);
            core->saveState(core, st);
            FILE *f = fopen(a, "wb"); fwrite(st, 1, sz, f); fclose(f); free(st);
        }
        else if (!strcmp(cmd, "load")) {
            size_t sz = core->stateSize(core);
            void *st = malloc(sz);
            FILE *f = fopen(a, "rb");
            if (!f) { fprintf(stderr, "no state %s\n", a); return 1; }
            if (fread(st, 1, sz, f) != sz) {
                fprintf(stderr, "incomplete state %s\n", a);
                fclose(f); free(st); return 1;
            }
            fclose(f);
            core->loadState(core, st); free(st);
        }
        fflush(stdout);
    }
    core->deinit(core);
    return 0;
}
