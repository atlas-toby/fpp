#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

/*
 * F++ Native Compiler v0.1 (fppc)
 * Hicbir harici derleyici/assembler/linker kullanmadan
 * dogrudan Windows x86-64 PE (.exe) makine kodu uretir.
 */

#define TEXT_RVA   0x1000
#define TEXT_RAW   0x0200
#define TEXT_SIZE  0x1000

#define RDATA_RVA  0x2000
#define RDATA_RAW  0x1200
#define RDATA_SIZE 0x1000

#define IMAGE_BASE 0x0000000140000000ULL

static uint8_t text_buf[TEXT_SIZE];
static uint32_t text_pos = 0;

static uint8_t rdata_buf[RDATA_SIZE];
static uint32_t str_pos = 0x200; // Ilk 0x200 byte Import/IAT tablolarina ayrildi

// IAT RVA sabitleri (.rdata icinde 0x2000 + offset)
#define IAT_GETSTDHANDLE_RVA (RDATA_RVA + 0x00)
#define IAT_WRITEFILE_RVA    (RDATA_RVA + 0x08)
#define IAT_EXITPROCESS_RVA  (RDATA_RVA + 0x10)

static void emit8(uint8_t b) {
    if (text_pos < TEXT_SIZE) text_buf[text_pos++] = b;
}

static void emit32(uint32_t v) {
    emit8((uint8_t)(v & 0xFF));
    emit8((uint8_t)((v >> 8) & 0xFF));
    emit8((uint8_t)((v >> 16) & 0xFF));
    emit8((uint8_t)((v >> 24) & 0xFF));
}

static void emit_call_iat(uint32_t iat_rva) {
    // FF 15 <rel32>  => call qword ptr [rip + rel32]
    emit8(0xFF);
    emit8(0x15);
    uint32_t next_rip = TEXT_RVA + text_pos + 4;
    int32_t disp = (int32_t)(iat_rva - next_rip);
    emit32((uint32_t)disp);
}

static void emit_lea_rdx_rva(uint32_t target_rva) {
    // 48 8D 15 <rel32> => lea rdx, [rip + rel32]
    emit8(0x48);
    emit8(0x8D);
    emit8(0x15);
    uint32_t next_rip = TEXT_RVA + text_pos + 4;
    int32_t disp = (int32_t)(target_rva - next_rip);
    emit32((uint32_t)disp);
}

static uint32_t add_string(const char *s, uint32_t *out_len) {
    uint32_t rva = RDATA_RVA + str_pos;
    uint32_t len = (uint32_t)strlen(s);
    memcpy(rdata_buf + str_pos, s, len);
    str_pos += len;
    rdata_buf[str_pos++] = '\r';
    rdata_buf[str_pos++] = '\n';
    rdata_buf[str_pos++] = '\0';
    *out_len = len + 2;
    return rva;
}

static void emit_prologue(void) {
    // sub rsp, 0x38
    emit8(0x48); emit8(0x83); emit8(0xEC); emit8(0x38);
    // mov ecx, -11 (STD_OUTPUT_HANDLE)
    emit8(0xB9); emit32(0xFFFFFFF5);
    // call [GetStdHandle]
    emit_call_iat(IAT_GETSTDHANDLE_RVA);
    // mov rbx, rax
    emit8(0x48); emit8(0x89); emit8(0xC3);
}

static void emit_print_str(const char *s) {
    uint32_t len = 0;
    uint32_t str_rva = add_string(s, &len);

    // mov rcx, rbx (hStdOut)
    emit8(0x48); emit8(0x89); emit8(0xD9);
    // lea rdx, [rip + str_rva]
    emit_lea_rdx_rva(str_rva);
    // mov r8d, len
    emit8(0x41); emit8(0xB8); emit32(len);
    // lea r9, [rsp + 0x28] (&written)
    emit8(0x4C); emit8(0x8D); emit8(0x4C); emit8(0x24); emit8(0x28);
    // mov qword ptr [rsp + 0x20], 0 (lpOverlapped = NULL)
    emit8(0x48); emit8(0xC7); emit8(0x44); emit8(0x24); emit8(0x20); emit32(0);
    // call [WriteFile]
    emit_call_iat(IAT_WRITEFILE_RVA);
}

static void emit_epilogue(void) {
    // xor ecx, ecx
    emit8(0x31); emit8(0xC9);
    // call [ExitProcess]
    emit_call_iat(IAT_EXITPROCESS_RVA);
}

static void write_u16(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v & 0xFF);
    p[1] = (uint8_t)((v >> 8) & 0xFF);
}

static void write_u32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v & 0xFF);
    p[1] = (uint8_t)((v >> 8) & 0xFF);
    p[2] = (uint8_t)((v >> 16) & 0xFF);
    p[3] = (uint8_t)((v >> 24) & 0xFF);
}

static void write_u64(uint8_t *p, uint64_t v) {
    write_u32(p, (uint32_t)(v & 0xFFFFFFFFULL));
    write_u32(p + 4, (uint32_t)(v >> 32));
}

static void build_rdata_imports(void) {
    /*
     * .rdata layout (RVA 0x2000):
     * 0x000: IAT (3 entries + NULL = 32 bytes)
     * 0x020: ILT (3 entries + NULL = 32 bytes)
     * 0x040: Import Directory Table (2 entries = 40 bytes)
     * 0x080: "KERNEL32.dll\0"
     * 0x0A0: Hint/Name GetStdHandle
     * 0x0C0: Hint/Name WriteFile
     * 0x0E0: Hint/Name ExitProcess
     */
    const uint32_t iat_off = 0x000;
    const uint32_t ilt_off = 0x020;
    const uint32_t idt_off = 0x040;
    const uint32_t dll_off = 0x080;
    const uint32_t hn1_off = 0x0A0;
    const uint32_t hn2_off = 0x0C0;
    const uint32_t hn3_off = 0x0E0;

    // DLL name
    strcpy((char *)(rdata_buf + dll_off), "KERNEL32.dll");

    // Hint/Name entries (2-byte hint = 0, then ASCII name)
    write_u16(rdata_buf + hn1_off, 0);
    strcpy((char *)(rdata_buf + hn1_off + 2), "GetStdHandle");

    write_u16(rdata_buf + hn2_off, 0);
    strcpy((char *)(rdata_buf + hn2_off + 2), "WriteFile");

    write_u16(rdata_buf + hn3_off, 0);
    strcpy((char *)(rdata_buf + hn3_off + 2), "ExitProcess");

    // ILT & IAT (64-bit RVAs to Hint/Name)
    uint64_t rva1 = RDATA_RVA + hn1_off;
    uint64_t rva2 = RDATA_RVA + hn2_off;
    uint64_t rva3 = RDATA_RVA + hn3_off;

    write_u64(rdata_buf + iat_off + 0, rva1);
    write_u64(rdata_buf + iat_off + 8, rva2);
    write_u64(rdata_buf + iat_off + 16, rva3);
    write_u64(rdata_buf + iat_off + 24, 0);

    write_u64(rdata_buf + ilt_off + 0, rva1);
    write_u64(rdata_buf + ilt_off + 8, rva2);
    write_u64(rdata_buf + ilt_off + 16, rva3);
    write_u64(rdata_buf + ilt_off + 24, 0);

    // Import Directory Table entry 0
    write_u32(rdata_buf + idt_off + 0,  RDATA_RVA + ilt_off); // OriginalFirstThunk (ILT)
    write_u32(rdata_buf + idt_off + 4,  0);                   // TimeDateStamp
    write_u32(rdata_buf + idt_off + 8,  0);                   // ForwarderChain
    write_u32(rdata_buf + idt_off + 12, RDATA_RVA + dll_off); // Name
    write_u32(rdata_buf + idt_off + 16, RDATA_RVA + iat_off); // FirstThunk (IAT)
}

static int write_pe_exe(const char *out_path) {
    uint8_t hdr[0x200];
    memset(hdr, 0, sizeof(hdr));

    // DOS Header
    hdr[0] = 'M'; hdr[1] = 'Z';
    write_u32(hdr + 0x3C, 0x80); // e_lfanew -> PE signature at 0x80

    // PE Signature
    uint8_t *pe = hdr + 0x80;
    pe[0] = 'P'; pe[1] = 'E'; pe[2] = 0; pe[3] = 0;

    // COFF File Header (20 bytes at 0x84)
    uint8_t *coff = pe + 4;
    write_u16(coff + 0, 0x8664); // Machine: AMD64
    write_u16(coff + 2, 2);      // NumberOfSections: 2 (.text, .rdata)
    write_u16(coff + 16, 0xF0);  // SizeOfOptionalHeader: 240 bytes (PE32+)
    write_u16(coff + 18, 0x0022);// Characteristics: EXECUTABLE_IMAGE | LARGE_ADDRESS_AWARE

    // Optional Header PE32+ (240 bytes at 0x98)
    uint8_t *opt = coff + 20;
    write_u16(opt + 0, 0x020B);       // Magic: PE32+
    write_u32(opt + 4, TEXT_SIZE);    // SizeOfCode
    write_u32(opt + 8, RDATA_SIZE);   // SizeOfInitializedData
    write_u32(opt + 16, TEXT_RVA);    // AddressOfEntryPoint
    write_u32(opt + 20, TEXT_RVA);    // BaseOfCode
    write_u64(opt + 24, IMAGE_BASE);  // ImageBase
    write_u32(opt + 32, 0x1000);      // SectionAlignment
    write_u32(opt + 36, 0x0200);      // FileAlignment
    write_u16(opt + 40, 6);           // MajorOperatingSystemVersion
    write_u16(opt + 48, 6);           // MajorSubsystemVersion
    write_u32(opt + 56, 0x3000);      // SizeOfImage (headers + .text + .rdata)
    write_u32(opt + 60, 0x0200);      // SizeOfHeaders
    write_u16(opt + 68, 3);           // Subsystem: WINDOWS_CUI (Console)
    write_u16(opt + 70, 0x8160);      // DllCharacteristics: NX_COMPAT | DYNAMIC_BASE | HIGH_ENTROPY_VA | TERMINAL_SERVER_AWARE
    write_u64(opt + 72, 0x100000);    // SizeOfStackReserve
    write_u64(opt + 80, 0x1000);      // SizeOfStackCommit
    write_u64(opt + 88, 0x100000);    // SizeOfHeapReserve
    write_u64(opt + 96, 0x1000);      // SizeOfHeapCommit
    write_u32(opt + 108, 16);         // NumberOfRvaAndSizes

    // Data Directories (starts at opt + 112)
    // Entry 1: Import Table
    write_u32(opt + 112 + 8,  RDATA_RVA + 0x040);
    write_u32(opt + 112 + 12, 40);
    // Entry 12: IAT
    write_u32(opt + 112 + 96,  RDATA_RVA + 0x000);
    write_u32(opt + 112 + 100, 32);

    // Section Headers (40 bytes each, starts at opt + 240 = 0x188)
    uint8_t *sec1 = opt + 240;
    memcpy(sec1, ".text\0\0\0", 8);
    write_u32(sec1 + 8,  TEXT_SIZE);
    write_u32(sec1 + 12, TEXT_RVA);
    write_u32(sec1 + 16, TEXT_SIZE);
    write_u32(sec1 + 20, TEXT_RAW);
    write_u32(sec1 + 36, 0x60000020); // CODE | EXECUTE | READ

    uint8_t *sec2 = sec1 + 40;
    memcpy(sec2, ".rdata\0\0", 8);
    write_u32(sec2 + 8,  RDATA_SIZE);
    write_u32(sec2 + 12, RDATA_RVA);
    write_u32(sec2 + 16, RDATA_SIZE);
    write_u32(sec2 + 20, RDATA_RAW);
    write_u32(sec2 + 36, 0x40000040); // INITIALIZED_DATA | READ

    build_rdata_imports();

    FILE *f = fopen(out_path, "wb");
    if (!f) return 0;
    fwrite(hdr, 1, sizeof(hdr), f);
    fwrite(text_buf, 1, sizeof(text_buf), f);
    fwrite(rdata_buf, 1, sizeof(rdata_buf), f);
    fclose(f);
    return 1;
}

/* --- Basit F++ Frontend (v0.1: yaz("...") ve NUMARA sabitleri) --- */
typedef struct {
    char name[64];
    double val;
} ConstVar;

static ConstVar cvars[128];
static int cvar_count = 0;

static void set_cvar(const char *name, double val) {
    for (int i = 0; i < cvar_count; i++) {
        if (strcmp(cvars[i].name, name) == 0) { cvars[i].val = val; return; }
    }
    strcpy(cvars[cvar_count].name, name);
    cvars[cvar_count].val = val;
    cvar_count++;
}

static int get_cvar(const char *name, double *out) {
    for (int i = 0; i < cvar_count; i++) {
        if (strcmp(cvars[i].name, name) == 0) { *out = cvars[i].val; return 1; }
    }
    return 0;
}

static void compile_fpp(const char *src) {
    emit_prologue();

    size_t i = 0;
    while (src[i]) {
        while (src[i] && isspace((unsigned char)src[i])) i++;
        if (!src[i]) break;

        if (src[i] == '#' || (src[i] == '/' && src[i + 1] == '/')) {
            while (src[i] && src[i] != '\n') i++;
            continue;
        }

        if (strncmp(src + i, "yaz", 3) == 0 && !isalnum((unsigned char)src[i + 3])) {
            i += 3;
            while (src[i] && isspace((unsigned char)src[i])) i++;
            if (src[i] == '(') i++;
            while (src[i] && isspace((unsigned char)src[i])) i++;

            if (src[i] == '"') {
                i++;
                char buf[512];
                int k = 0;
                while (src[i] && src[i] != '"' && k < 511) buf[k++] = src[i++];
                buf[k] = '\0';
                if (src[i] == '"') i++;
                emit_print_str(buf);
            } else {
                char ident[64];
                int k = 0;
                while ((isalnum((unsigned char)src[i]) || src[i] == '_') && k < 63) ident[k++] = src[i++];
                ident[k] = '\0';
                double v = 0;
                if (get_cvar(ident, &v)) {
                    char numbuf[64];
                    snprintf(numbuf, sizeof(numbuf), "%g", v);
                    emit_print_str(numbuf);
                } else {
                    emit_print_str(ident);
                }
            }
            while (src[i] && src[i] != ';') i++;
            if (src[i] == ';') i++;
        } else if (strncmp(src + i, "NUMARA", 6) == 0 && isspace((unsigned char)src[i + 6])) {
            i += 6;
            while (src[i] && isspace((unsigned char)src[i])) i++;
            char name[64];
            int k = 0;
            while ((isalnum((unsigned char)src[i]) || src[i] == '_') && k < 63) name[k++] = src[i++];
            name[k] = '\0';
            while (src[i] && src[i] != '=') i++;
            if (src[i] == '=') i++;
            while (src[i] && isspace((unsigned char)src[i])) i++;
            double val = atof(src + i);
            set_cvar(name, val);
            while (src[i] && src[i] != ';') i++;
            if (src[i] == ';') i++;
        } else {
            while (src[i] && src[i] != '\n') i++;
        }
    }

    emit_epilogue();
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Kullanim: fppc <giris.fpp> <cikis.exe>\n");
        return 1;
    }

    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        fprintf(stderr, "Dosya acilamadi: %s\n", argv[1]);
        return 1;
    }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc((size_t)len + 1);
    fread(buf, 1, (size_t)len, f);
    buf[len] = '\0';
    fclose(f);

    compile_fpp(buf);
    free(buf);

    if (!write_pe_exe(argv[2])) {
        fprintf(stderr, "EXE yazilamadi: %s\n", argv[2]);
        return 1;
    }

    printf("Basariyla derlendi (Sifirdan x86-64 PE): %s (%u byte makine kodu)\n", argv[2], text_pos);
    return 0;
}
