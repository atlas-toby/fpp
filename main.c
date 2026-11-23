#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <windows.h>
#include <shlobj.h>

typedef enum {
    TOK_EOF,
    TOK_KW_NUMARA,
    TOK_KW_DOGRUMU,
    TOK_KW_CULUK,
    TOK_KW_CUL,
    TOK_KW_YAZ,
    TOK_KW_HAK_TEAM,
    TOK_KW_FOTO,
    TOK_KW_BAK_FONKSIYON,
    TOK_KW_SAL,
    TOK_KW_DOSYA_RISK,
    TOK_KW_TEMIZLE,
    TOK_KW_YRENK,
    TOK_KW_DUR_KARDESIM,
    TOK_KW_ARADUR,
    TOK_KW_UYARI,
    TOK_KW_EGER,
    TOK_KW_YADA_BOYLE,
    TOK_KW_DEGILSE,
    TOK_KW_DONGU,
    TOK_BOOL_AYNEN,
    TOK_BOOL_YOK,
    TOK_IDENT,
    TOK_NUMBER,
    TOK_BIGINT,
    TOK_STRING,
    TOK_CHAR,
    TOK_ASSIGN,
    TOK_EQ,
    TOK_NEQ,
    TOK_LT,
    TOK_GT,
    TOK_LE,
    TOK_GE,
    TOK_PLUS,
    TOK_MINUS,
    TOK_STAR,
    TOK_SLASH,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_COMMA,
    TOK_DOT,
    TOK_SEMI
} FppTokenType;

typedef enum {
    VAL_NUMARA,
    VAL_BIGINT,
    VAL_DOGRUMU,
    VAL_CULUK,
    VAL_CUL
} ValType;

typedef struct {
    FppTokenType type;
    char text[512];
    double number;
} Token;

typedef struct {
    char name[64];
    ValType type;
    double num_val;
    int bool_val;
    char str_val[512];
    char char_val;
} Var;

typedef struct {
    char name[64];
    char body[4096];
} Func;

static const char *src;
static size_t pos = 0;
static Token current_tok;
static Var vars[256];
static int var_count = 0;
static Func funcs[64];
static int func_count = 0;

static int is_ident_start(unsigned char c) {
    return isalpha(c) || c == '_' || c >= 0x80;
}

static int is_ident_char(unsigned char c) {
    return isalnum(c) || c == '_' || c >= 0x80;
}

static void next_token(void) {
    while (src[pos] && isspace((unsigned char)src[pos])) pos++;

    if (src[pos] == '#' || (src[pos] == '/' && src[pos + 1] == '/')) {
        while (src[pos] && src[pos] != '\n') pos++;
        next_token();
        return;
    }

    if (src[pos] == '\0') {
        current_tok.type = TOK_EOF;
        current_tok.text[0] = '\0';
        return;
    }

    unsigned char c = (unsigned char)src[pos];

    if (is_ident_start(c)) {
        int i = 0;
        while (is_ident_char((unsigned char)src[pos]) && i < 511) {
            current_tok.text[i++] = src[pos++];
        }
        current_tok.text[i] = '\0';

        if (strcmp(current_tok.text, "NUMARA") == 0) current_tok.type = TOK_KW_NUMARA;
        else if (strcmp(current_tok.text, "doğruMu") == 0 || strcmp(current_tok.text, "dogruMu") == 0) current_tok.type = TOK_KW_DOGRUMU;
        else if (strcmp(current_tok.text, "culuk") == 0) current_tok.type = TOK_KW_CULUK;
        else if (strcmp(current_tok.text, "cul") == 0) current_tok.type = TOK_KW_CUL;
        else if (strcmp(current_tok.text, "yaz") == 0) current_tok.type = TOK_KW_YAZ;
        else if (strcmp(current_tok.text, "hak_team") == 0) current_tok.type = TOK_KW_HAK_TEAM;
        else if (strcmp(current_tok.text, "foto") == 0) current_tok.type = TOK_KW_FOTO;
        else if (strcmp(current_tok.text, "bak_fonksiyon") == 0) current_tok.type = TOK_KW_BAK_FONKSIYON;
        else if (strcmp(current_tok.text, "sal") == 0) current_tok.type = TOK_KW_SAL;
        else if (strcmp(current_tok.text, "dosyaGezginiRiskAltında") == 0 || strcmp(current_tok.text, "dosyaGezginiRiskAltinda") == 0) current_tok.type = TOK_KW_DOSYA_RISK;
        else if (strcmp(current_tok.text, "temizle") == 0) current_tok.type = TOK_KW_TEMIZLE;
        else if (strcmp(current_tok.text, "yrenk") == 0) current_tok.type = TOK_KW_YRENK;
        else if (strcmp(current_tok.text, "dur_kardeşim") == 0 || strcmp(current_tok.text, "dur_kardesim") == 0) current_tok.type = TOK_KW_DUR_KARDESIM;
        else if (strcmp(current_tok.text, "aradur") == 0) current_tok.type = TOK_KW_ARADUR;
        else if (strcmp(current_tok.text, "uyari") == 0 || strcmp(current_tok.text, "uyarı") == 0) current_tok.type = TOK_KW_UYARI;
        else if (strcmp(current_tok.text, "eğer") == 0 || strcmp(current_tok.text, "eger") == 0) current_tok.type = TOK_KW_EGER;
        else if (strcmp(current_tok.text, "yada_böyleyse") == 0 || strcmp(current_tok.text, "yada_boyleyse") == 0) current_tok.type = TOK_KW_YADA_BOYLE;
        else if (strcmp(current_tok.text, "değilse") == 0 || strcmp(current_tok.text, "degilse") == 0) current_tok.type = TOK_KW_DEGILSE;
        else if (strcmp(current_tok.text, "döngü_icerisine_sok_bilader_ne_yapalım") == 0 || strcmp(current_tok.text, "dongu_icerisine_sok_bilader_ne_yapalim") == 0) current_tok.type = TOK_KW_DONGU;
        else if (strcmp(current_tok.text, "aynen") == 0) current_tok.type = TOK_BOOL_AYNEN;
        else if (strcmp(current_tok.text, "yok") == 0) current_tok.type = TOK_BOOL_YOK;
        else current_tok.type = TOK_IDENT;
        return;
    }

    if (isdigit(c)) {
        int i = 0;
        int has_dot = 0;
        while ((isdigit((unsigned char)src[pos]) || src[pos] == '.') && i < 511) {
            if (src[pos] == '.') has_dot = 1;
            current_tok.text[i++] = src[pos++];
        }
        current_tok.text[i] = '\0';

        if (!has_dot && i > 15) {
            current_tok.type = TOK_BIGINT;
        } else {
            current_tok.type = TOK_NUMBER;
            current_tok.number = atof(current_tok.text);
        }
        return;
    }

    if (c == '"') {
        pos++;
        int i = 0;
        while (src[pos] && src[pos] != '"' && i < 511) {
            current_tok.text[i++] = src[pos++];
        }
        current_tok.text[i] = '\0';
        if (src[pos] == '"') pos++;
        current_tok.type = TOK_STRING;
        return;
    }

    if (c == '\'') {
        pos++;
        current_tok.text[0] = src[pos] ? src[pos++] : '\0';
        current_tok.text[1] = '\0';
        if (src[pos] == '\'') pos++;
        current_tok.type = TOK_CHAR;
        return;
    }

    if (c == '=' && src[pos + 1] == '=') {
        pos += 2;
        current_tok.type = TOK_EQ;
        strcpy(current_tok.text, "==");
        return;
    }
    if (c == '!' && src[pos + 1] == '=') {
        pos += 2;
        current_tok.type = TOK_NEQ;
        strcpy(current_tok.text, "!=");
        return;
    }
    if (c == '<' && src[pos + 1] == '=') {
        pos += 2;
        current_tok.type = TOK_LE;
        strcpy(current_tok.text, "<=");
        return;
    }
    if (c == '>' && src[pos + 1] == '=') {
        pos += 2;
        current_tok.type = TOK_GE;
        strcpy(current_tok.text, ">=");
        return;
    }

    pos++;
    switch (c) {
        case '=': current_tok.type = TOK_ASSIGN; break;
        case '<': current_tok.type = TOK_LT; break;
        case '>': current_tok.type = TOK_GT; break;
        case '+': current_tok.type = TOK_PLUS; break;
        case '-': current_tok.type = TOK_MINUS; break;
        case '*': current_tok.type = TOK_STAR; break;
        case '/': current_tok.type = TOK_SLASH; break;
        case '(': current_tok.type = TOK_LPAREN; break;
        case ')': current_tok.type = TOK_RPAREN; break;
        case '{': current_tok.type = TOK_LBRACE; break;
        case '}': current_tok.type = TOK_RBRACE; break;
        case ',': current_tok.type = TOK_COMMA; break;
        case '.': current_tok.type = TOK_DOT; break;
        case ';': current_tok.type = TOK_SEMI; break;
        default:
            fprintf(stderr, "Bilinmeyen karakter: '%c'\n", c);
            exit(1);
    }
}

static void expect(FppTokenType type) {
    if (current_tok.type != type) {
        fprintf(stderr, "Sozdizimi hatasi! Beklenen token: %d, Gelen: %d (%s)\n", type, current_tok.type, current_tok.text);
        exit(1);
    }
    next_token();
}

static Var *find_var(const char *name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(vars[i].name, name) == 0) return &vars[i];
    }
    return NULL;
}

static Var *upsert_var(const char *name) {
    Var *v = find_var(name);
    if (v) return v;
    strcpy(vars[var_count].name, name);
    return &vars[var_count++];
}

static Func *find_func(const char *name) {
    for (int i = 0; i < func_count; i++) {
        if (strcmp(funcs[i].name, name) == 0) return &funcs[i];
    }
    return NULL;
}

static double parse_expr(void);
static void parse_stmt(void);

static double run_sal(void) {
    expect(TOK_LPAREN);
    int min_v = (int)parse_expr();
    expect(TOK_COMMA);
    int max_v = (int)parse_expr();
    expect(TOK_RPAREN);
    if (max_v < min_v) {
        int tmp = min_v;
        min_v = max_v;
        max_v = tmp;
    }
    return (double)(min_v + rand() % (max_v - min_v + 1));
}

static void read_aradur_line(char *out, size_t max_len) {
    fflush(stdout);
    if (fgets(out, (int)max_len, stdin)) {
        size_t len = strlen(out);
        while (len > 0 && (out[len - 1] == '\n' || out[len - 1] == '\r')) {
            out[--len] = '\0';
        }
    } else {
        out[0] = '\0';
    }
}

static double parse_factor(void) {
    if (current_tok.type == TOK_NUMBER) {
        double v = current_tok.number;
        next_token();
        return v;
    }
    if (current_tok.type == TOK_MINUS) {
        next_token();
        return -parse_factor();
    }
    if (current_tok.type == TOK_KW_SAL) {
        next_token();
        return run_sal();
    }
    if (current_tok.type == TOK_KW_ARADUR) {
        next_token();
        expect(TOK_LPAREN);
        expect(TOK_RPAREN);
        char buf[128];
        read_aradur_line(buf, sizeof(buf));
        return atof(buf);
    }
    if (current_tok.type == TOK_IDENT) {
        Var *v = find_var(current_tok.text);
        if (!v || v->type != VAL_NUMARA) {
            fprintf(stderr, "Gecersiz NUMARA degiskeni: %s\n", current_tok.text);
            exit(1);
        }
        next_token();
        return v->num_val;
    }
    if (current_tok.type == TOK_LPAREN) {
        next_token();
        double v = parse_expr();
        expect(TOK_RPAREN);
        return v;
    }
    fprintf(stderr, "Beklenmeyen ifade!\n");
    exit(1);
}

static double parse_term(void) {
    double v = parse_factor();
    while (current_tok.type == TOK_STAR || current_tok.type == TOK_SLASH) {
        FppTokenType op = current_tok.type;
        next_token();
        double rhs = parse_factor();
        if (op == TOK_STAR) v *= rhs;
        else v /= rhs;
    }
    return v;
}

static double parse_expr(void) {
    double v = parse_term();
    while (current_tok.type == TOK_PLUS || current_tok.type == TOK_MINUS) {
        FppTokenType op = current_tok.type;
        next_token();
        double rhs = parse_term();
        if (op == TOK_PLUS) v += rhs;
        else v -= rhs;
    }
    return v;
}

static int parse_condition(void) {
    if (current_tok.type == TOK_BOOL_AYNEN) {
        next_token();
        return 1;
    }
    if (current_tok.type == TOK_BOOL_YOK) {
        next_token();
        return 0;
    }
    if (current_tok.type == TOK_STRING) {
        char lhs[512];
        strcpy(lhs, current_tok.text);
        next_token();
        FppTokenType op = current_tok.type;
        if (op != TOK_EQ && op != TOK_NEQ) {
            fprintf(stderr, "String karsilastirmada == veya != bekleniyordu!\n");
            exit(1);
        }
        next_token();
        char rhs[512];
        if (current_tok.type == TOK_STRING) {
            strcpy(rhs, current_tok.text);
            next_token();
        } else if (current_tok.type == TOK_IDENT) {
            Var *rv = find_var(current_tok.text);
            if (!rv || rv->type != VAL_CULUK) {
                fprintf(stderr, "Gecersiz culuk degiskeni!\n");
                exit(1);
            }
            strcpy(rhs, rv->str_val);
            next_token();
        } else {
            fprintf(stderr, "String deger bekleniyordu!\n");
            exit(1);
        }
        return (op == TOK_EQ) ? (strcmp(lhs, rhs) == 0) : (strcmp(lhs, rhs) != 0);
    }

    if (current_tok.type == TOK_IDENT) {
        Var *v = find_var(current_tok.text);
        if (!v) {
            fprintf(stderr, "Tanimsiz degisken: %s\n", current_tok.text);
            exit(1);
        }
        if (v->type == VAL_DOGRUMU) {
            next_token();
            if (current_tok.type == TOK_EQ || current_tok.type == TOK_NEQ) {
                FppTokenType op = current_tok.type;
                next_token();
                int rval = 0;
                if (current_tok.type == TOK_BOOL_AYNEN) { rval = 1; next_token(); }
                else if (current_tok.type == TOK_BOOL_YOK) { rval = 0; next_token(); }
                else if (current_tok.type == TOK_IDENT) {
                    Var *rv = find_var(current_tok.text);
                    if (!rv || rv->type != VAL_DOGRUMU) exit(1);
                    rval = rv->bool_val;
                    next_token();
                } else {
                    fprintf(stderr, "aynen veya yok bekleniyordu!\n");
                    exit(1);
                }
                return (op == TOK_EQ) ? (v->bool_val == rval) : (v->bool_val != rval);
            }
            return v->bool_val;
        }
        if (v->type == VAL_CULUK) {
            next_token();
            FppTokenType op = current_tok.type;
            if (op != TOK_EQ && op != TOK_NEQ) {
                fprintf(stderr, "culuk karsilastirmada == veya != bekleniyordu!\n");
                exit(1);
            }
            next_token();
            char rhs[512];
            if (current_tok.type == TOK_STRING) {
                strcpy(rhs, current_tok.text);
                next_token();
            } else if (current_tok.type == TOK_IDENT) {
                Var *rv = find_var(current_tok.text);
                if (!rv || rv->type != VAL_CULUK) exit(1);
                strcpy(rhs, rv->str_val);
                next_token();
            } else {
                exit(1);
            }
            return (op == TOK_EQ) ? (strcmp(v->str_val, rhs) == 0) : (strcmp(v->str_val, rhs) != 0);
        }
    }

    double lhs = parse_expr();
    if (current_tok.type == TOK_EQ || current_tok.type == TOK_NEQ ||
        current_tok.type == TOK_LT || current_tok.type == TOK_GT ||
        current_tok.type == TOK_LE || current_tok.type == TOK_GE) {
        FppTokenType op = current_tok.type;
        next_token();
        double rhs = parse_expr();
        switch (op) {
            case TOK_EQ: return lhs == rhs;
            case TOK_NEQ: return lhs != rhs;
            case TOK_LT: return lhs < rhs;
            case TOK_GT: return lhs > rhs;
            case TOK_LE: return lhs <= rhs;
            case TOK_GE: return lhs >= rhs;
            default: return 0;
        }
    }
    return lhs != 0.0;
}

static void skip_block(void) {
    if (current_tok.type != TOK_LBRACE) {
        fprintf(stderr, "Blok '{' ile baslamali!\n");
        exit(1);
    }
    int depth = 1;
    while (src[pos] && depth > 0) {
        if (src[pos] == '"') {
            pos++;
            while (src[pos] && src[pos] != '"') pos++;
            if (src[pos] == '"') pos++;
            continue;
        }
        if (src[pos] == '{') depth++;
        else if (src[pos] == '}') depth--;
        pos++;
    }
    if (depth != 0) {
        fprintf(stderr, "Kapanmamis blok '}'!\n");
        exit(1);
    }
    next_token();
}

static void run_block(void) {
    expect(TOK_LBRACE);
    while (current_tok.type != TOK_RBRACE && current_tok.type != TOK_EOF) {
        parse_stmt();
    }
    expect(TOK_RBRACE);
}

static void run_hak_team(void) {
    mciSendStringA("close fpp_bgm", NULL, 0, NULL);
    mciSendStringA("open \"sarki.mp3\" type mpegvideo alias fpp_bgm", NULL, 0, NULL);
    mciSendStringA("play fpp_bgm", NULL, 0, NULL);

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    WORD old_attr = csbi.wAttributes;
    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);

    const char *lines[] = {
        "[HAK_TEAM] Baglanti kuruluyor -> netlify-edge-01.internal ...",
        "[HAK_TEAM] TLS el sikismasi asiliyor [OK]",
        "[HAK_TEAM] Netlify sunucu dugumleri taranıyor (104.198.14.52)...",
        "[HAK_TEAM] Payload enjekte ediliyor: 0xFPP_OVERDRIVE...",
        "[HAK_TEAM] Yetki yukseltme tamamlandi -> ROOT@NETLIFY-SIM",
        "[HAK_TEAM] Veri akisi basladi... (Simulasyon tamamlandi!)"
    };
    for (int i = 0; i < 6; i++) {
        printf("%s\n", lines[i]);
        fflush(stdout);
        Sleep(450);
    }
    SetConsoleTextAttribute(hConsole, old_attr);
}

static void run_foto(const char *path) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
        "powershell -NoProfile -Command \""
        "Add-Type -AssemblyName System.Windows.Forms; "
        "Add-Type -AssemblyName System.Drawing; "
        "$f = New-Object System.Windows.Forms.Form; "
        "$f.Text = 'F++ Foto'; "
        "$f.StartPosition = 'CenterScreen'; "
        "$img = [System.Drawing.Image]::FromFile('%s'); "
        "$f.ClientSize = New-Object System.Drawing.Size($img.Width, $img.Height); "
        "$pb = New-Object System.Windows.Forms.PictureBox; "
        "$pb.Dock = 'Fill'; "
        "$pb.Image = $img; "
        "$pb.SizeMode = 'Zoom'; "
        "$f.Controls.Add($pb); "
        "$f.TopMost = $true; "
        "[void]$f.ShowDialog()\"",
        path);
    system(cmd);
}

static void run_dosya_risk(void) {
    char path[MAX_PATH];
    if (SHGetFolderPathA(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, path) != S_OK) {
        fprintf(stderr, "Masaustu yolu bulunamadi!\n");
        return;
    }
    strncat(path, "\\dosya.txt", sizeof(path) - strlen(path) - 1);
    FILE *f = fopen(path, "wb");
    if (!f) {
        fprintf(stderr, "Masaustune dosya yazilamadi: %s\n", path);
        return;
    }
    const char *msg = "portal çıkmış mı";
    fwrite(msg, 1, strlen(msg), f);
    fclose(f);
}

static void run_yrenk(const char *color_name) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    WORD attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    if (strcmp(color_name, "kirmizi") == 0 || strcmp(color_name, "kırmızı") == 0) {
        attr = FOREGROUND_RED | FOREGROUND_INTENSITY;
    } else if (strcmp(color_name, "yesil") == 0 || strcmp(color_name, "yeşil") == 0) {
        attr = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    } else if (strcmp(color_name, "mavi") == 0) {
        attr = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    } else if (strcmp(color_name, "sari") == 0 || strcmp(color_name, "sarı") == 0) {
        attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    } else if (strcmp(color_name, "mor") == 0) {
        attr = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    } else if (strcmp(color_name, "turkuaz") == 0) {
        attr = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    } else if (strcmp(color_name, "beyaz") == 0 || strcmp(color_name, "sifirla") == 0) {
        attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    }
    SetConsoleTextAttribute(hConsole, attr);
}

static void call_func(Func *fn) {
    const char *saved_src = src;
    size_t saved_pos = pos;
    Token saved_tok = current_tok;

    src = fn->body;
    pos = 0;
    next_token();
    while (current_tok.type != TOK_EOF) {
        parse_stmt();
    }

    src = saved_src;
    pos = saved_pos;
    current_tok = saved_tok;
}

static void assign_existing_var(Var *v) {
    if (v->type == VAL_NUMARA) {
        v->num_val = parse_expr();
    } else if (v->type == VAL_DOGRUMU) {
        if (current_tok.type == TOK_BOOL_AYNEN) { v->bool_val = 1; next_token(); }
        else if (current_tok.type == TOK_BOOL_YOK) { v->bool_val = 0; next_token(); }
        else { fprintf(stderr, "aynen veya yok bekleniyordu!\n"); exit(1); }
    } else if (v->type == VAL_CULUK) {
        if (current_tok.type == TOK_KW_ARADUR) {
            next_token(); expect(TOK_LPAREN); expect(TOK_RPAREN);
            read_aradur_line(v->str_val, sizeof(v->str_val));
        } else {
            strcpy(v->str_val, current_tok.text);
            expect(TOK_STRING);
        }
    } else if (v->type == VAL_CUL) {
        v->char_val = current_tok.text[0];
        expect(TOK_CHAR);
    }
}

static void parse_stmt(void) {
    if (current_tok.type == TOK_KW_NUMARA) {
        next_token();
        char name[64];
        strcpy(name, current_tok.text);
        expect(TOK_IDENT);
        expect(TOK_ASSIGN);
        Var *v = upsert_var(name);
        if (current_tok.type == TOK_BIGINT) {
            v->type = VAL_BIGINT;
            strcpy(v->str_val, current_tok.text);
            next_token();
        } else {
            v->type = VAL_NUMARA;
            v->num_val = parse_expr();
        }
        expect(TOK_SEMI);
    } else if (current_tok.type == TOK_KW_DOGRUMU) {
        next_token();
        char name[64];
        strcpy(name, current_tok.text);
        expect(TOK_IDENT);
        expect(TOK_ASSIGN);
        Var *v = upsert_var(name);
        v->type = VAL_DOGRUMU;
        if (current_tok.type == TOK_BOOL_AYNEN) {
            v->bool_val = 1;
            next_token();
        } else if (current_tok.type == TOK_BOOL_YOK) {
            v->bool_val = 0;
            next_token();
        } else {
            fprintf(stderr, "doğruMu icin 'aynen' veya 'yok' bekleniyordu!\n");
            exit(1);
        }
        expect(TOK_SEMI);
    } else if (current_tok.type == TOK_KW_CULUK) {
        next_token();
        char name[64];
        strcpy(name, current_tok.text);
        expect(TOK_IDENT);
        expect(TOK_ASSIGN);
        Var *v = upsert_var(name);
        v->type = VAL_CULUK;
        if (current_tok.type == TOK_KW_ARADUR) {
            next_token();
            expect(TOK_LPAREN);
            expect(TOK_RPAREN);
            read_aradur_line(v->str_val, sizeof(v->str_val));
        } else {
            strcpy(v->str_val, current_tok.text);
            expect(TOK_STRING);
        }
        expect(TOK_SEMI);
    } else if (current_tok.type == TOK_KW_CUL) {
        next_token();
        char name[64];
        strcpy(name, current_tok.text);
        expect(TOK_IDENT);
        expect(TOK_ASSIGN);
        Var *v = upsert_var(name);
        v->type = VAL_CUL;
        if (current_tok.type == TOK_KW_ARADUR) {
            next_token();
            expect(TOK_LPAREN);
            expect(TOK_RPAREN);
            char tmp[128];
            read_aradur_line(tmp, sizeof(tmp));
            v->char_val = tmp[0];
        } else {
            v->char_val = current_tok.text[0];
            expect(TOK_CHAR);
        }
        expect(TOK_SEMI);
    } else if (current_tok.type == TOK_KW_YAZ) {
        next_token();
        expect(TOK_LPAREN);
        if (current_tok.type == TOK_STRING) {
            printf("%s\n", current_tok.text);
            next_token();
        } else if (current_tok.type == TOK_CHAR) {
            printf("%c\n", current_tok.text[0]);
            next_token();
        } else if (current_tok.type == TOK_BOOL_AYNEN) {
            printf("aynen\n");
            next_token();
        } else if (current_tok.type == TOK_BOOL_YOK) {
            printf("yok\n");
            next_token();
        } else if (current_tok.type == TOK_BIGINT) {
            printf("%s\n", current_tok.text);
            next_token();
        } else if (current_tok.type == TOK_IDENT) {
            Var *v = find_var(current_tok.text);
            if (!v) {
                fprintf(stderr, "Tanimsiz degisken: %s\n", current_tok.text);
                exit(1);
            }
            if (v->type == VAL_CULUK || v->type == VAL_BIGINT) {
                printf("%s\n", v->str_val);
                next_token();
            } else if (v->type == VAL_CUL) {
                printf("%c\n", v->char_val);
                next_token();
            } else if (v->type == VAL_DOGRUMU) {
                printf("%s\n", v->bool_val ? "aynen" : "yok");
                next_token();
            } else {
                double val = parse_expr();
                printf("%g\n", val);
            }
        } else {
            double val = parse_expr();
            printf("%g\n", val);
        }
        expect(TOK_RPAREN);
        expect(TOK_SEMI);
    } else if (current_tok.type == TOK_KW_EGER) {
        next_token();
        expect(TOK_LPAREN);
        int cond = parse_condition();
        expect(TOK_RPAREN);
        int handled = 0;
        if (cond) {
            run_block();
            handled = 1;
        } else {
            skip_block();
        }
        while (current_tok.type == TOK_KW_YADA_BOYLE) {
            next_token();
            expect(TOK_LPAREN);
            int elif_cond = parse_condition();
            expect(TOK_RPAREN);
            if (!handled && elif_cond) {
                run_block();
                handled = 1;
            } else {
                skip_block();
            }
        }
        if (current_tok.type == TOK_KW_DEGILSE) {
            next_token();
            if (current_tok.type == TOK_LPAREN) {
                next_token();
                expect(TOK_RPAREN);
            }
            if (!handled) {
                run_block();
            } else {
                skip_block();
            }
        }
    } else if (current_tok.type == TOK_KW_DONGU) {
        next_token();
        expect(TOK_LPAREN);
        size_t cond_pos = pos;
        Token cond_tok = current_tok;
        while (1) {
            pos = cond_pos;
            current_tok = cond_tok;
            int cond = parse_condition();
            expect(TOK_RPAREN);
            if (!cond) {
                skip_block();
                break;
            }
            run_block();
        }
    } else if (current_tok.type == TOK_KW_BAK_FONKSIYON) {
        next_token();
        char fname[64];
        strcpy(fname, current_tok.text);
        expect(TOK_IDENT);
        expect(TOK_LPAREN);
        expect(TOK_RPAREN);
        if (current_tok.type != TOK_LBRACE) {
            fprintf(stderr, "Fonksiyon govdesi '{' ile baslamali!\n");
            exit(1);
        }
        size_t body_start = pos;
        int depth = 1;
        while (src[pos] && depth > 0) {
            if (src[pos] == '{') depth++;
            else if (src[pos] == '}') depth--;
            if (depth > 0) pos++;
        }
        if (depth != 0) {
            fprintf(stderr, "Kapanmamis fonksiyon govdesi '}'!\n");
            exit(1);
        }
        size_t body_len = pos - body_start;
        if (body_len >= 4095) body_len = 4095;
        Func *fn = &funcs[func_count++];
        strcpy(fn->name, fname);
        memcpy(fn->body, src + body_start, body_len);
        fn->body[body_len] = '\0';
        pos++;
        next_token();
    } else if (current_tok.type == TOK_KW_HAK_TEAM) {
        next_token();
        expect(TOK_LPAREN);
        expect(TOK_RPAREN);
        expect(TOK_SEMI);
        run_hak_team();
    } else if (current_tok.type == TOK_KW_FOTO) {
        next_token();
        expect(TOK_LPAREN);
        char path[512];
        if (current_tok.type == TOK_STRING) {
            strcpy(path, current_tok.text);
            next_token();
        } else if (current_tok.type == TOK_IDENT) {
            Var *v = find_var(current_tok.text);
            if (!v || v->type != VAL_CULUK) {
                fprintf(stderr, "foto() icin culuk (string) yol gerekli!\n");
                exit(1);
            }
            strcpy(path, v->str_val);
            next_token();
        } else {
            fprintf(stderr, "foto() icin dosya yolu gerekli!\n");
            exit(1);
        }
        expect(TOK_RPAREN);
        expect(TOK_SEMI);
        run_foto(path);
    } else if (current_tok.type == TOK_KW_DOSYA_RISK) {
        next_token();
        expect(TOK_LPAREN);
        expect(TOK_RPAREN);
        expect(TOK_SEMI);
        run_dosya_risk();
    } else if (current_tok.type == TOK_KW_TEMIZLE) {
        next_token();
        expect(TOK_LPAREN);
        expect(TOK_RPAREN);
        expect(TOK_SEMI);
        system("cls");
    } else if (current_tok.type == TOK_KW_YRENK) {
        next_token();
        expect(TOK_LPAREN);
        if (current_tok.type == TOK_IDENT && strcmp(current_tok.text, "Renk") == 0) {
            next_token();
            expect(TOK_DOT);
        }
        char color[64];
        strcpy(color, current_tok.text);
        if (current_tok.type == TOK_IDENT || current_tok.type == TOK_STRING) {
            next_token();
        } else {
            fprintf(stderr, "yrenk() icin Renk.kirmizi gibi bir renk bekleniyordu!\n");
            exit(1);
        }
        expect(TOK_RPAREN);
        expect(TOK_SEMI);
        run_yrenk(color);
    } else if (current_tok.type == TOK_KW_DUR_KARDESIM) {
        next_token();
        expect(TOK_LPAREN);
        int ms = (int)parse_expr();
        expect(TOK_RPAREN);
        expect(TOK_SEMI);
        Sleep((DWORD)ms);
    } else if (current_tok.type == TOK_KW_UYARI) {
        next_token();
        expect(TOK_LPAREN);
        char msg[512] = "F++ Uyari!";
        char title[128] = "F++";
        if (current_tok.type == TOK_STRING) {
            strcpy(msg, current_tok.text);
            next_token();
            if (current_tok.type == TOK_COMMA) {
                next_token();
                strcpy(title, msg);
                strcpy(msg, current_tok.text);
                expect(TOK_STRING);
            }
        }
        expect(TOK_RPAREN);
        expect(TOK_SEMI);
        MessageBoxA(NULL, msg, title, MB_OK | MB_ICONWARNING | MB_TOPMOST);
    } else if (current_tok.type == TOK_IDENT) {
        char ident_name[64];
        strcpy(ident_name, current_tok.text);
        next_token();
        if (current_tok.type == TOK_LPAREN) {
            next_token();
            expect(TOK_RPAREN);
            expect(TOK_SEMI);
            Func *fn = find_func(ident_name);
            if (!fn) {
                fprintf(stderr, "Tanimsiz fonksiyon: %s\n", ident_name);
                exit(1);
            }
            call_func(fn);
        } else if (current_tok.type == TOK_ASSIGN) {
            next_token();
            Var *v = find_var(ident_name);
            if (!v) {
                fprintf(stderr, "Tanimsiz degisken: %s\n", ident_name);
                exit(1);
            }
            assign_existing_var(v);
            expect(TOK_SEMI);
        } else {
            fprintf(stderr, "Gecersiz ifade: %s\n", ident_name);
            exit(1);
        }
    } else {
        fprintf(stderr, "Gecersiz komut: %s\n", current_tok.text);
        exit(1);
    }
}

int main(int argc, char **argv) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    srand((unsigned int)time(NULL));

    if (argc < 2) {
        printf("Kullanim: fpp <dosya.fpp>\n");
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

    src = buf;
    next_token();
    while (current_tok.type != TOK_EOF) {
        parse_stmt();
    }

    free(buf);
    return 0;
}
