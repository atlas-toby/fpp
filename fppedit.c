#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include <windows.h>

#define MAX_LINES 1024
#define MAX_COL   512

typedef enum {
    MODE_NORMAL,
    MODE_INSERT,
    MODE_COMMAND
} EditorMode;

static char lines[MAX_LINES][MAX_COL];
static int line_count = 1;
static int cur_y = 0;
static int cur_x = 0;
static EditorMode mode = MODE_NORMAL;
static char cmd_buf[128] = "";
static char status_msg[256] = "F++ Vim Editoru | i: Yaz | ESC: Normal | :w Kaydet | :run Calistir | :build Derle | :q Cik";
static char filename[256] = "yeni.fpp";
static int running = 1;

static const char *fpp_keywords[] = {
    "NUMARA", "doğruMu", "dogruMu", "culuk", "cul", "yaz", "hak_team", "foto",
    "bak_fonksiyon", "sal", "dosyaGezginiRiskAltında", "dosyaGezginiRiskAltinda",
    "temizle", "yrenk", "dur_kardeşim", "dur_kardesim", "aradur", "uyari", "uyarı",
    "eğer", "eger", "yada_böyleyse", "yada_boyleyse", "değilse", "degilse",
    "döngü_icerisine_sok_bilader_ne_yapalım", "dongu_icerisine_sok_bilader_ne_yapalim",
    "aynen", "yok", "Renk", NULL
};

static int is_keyword(const char *word) {
    for (int i = 0; fpp_keywords[i]; i++) {
        if (strcmp(word, fpp_keywords[i]) == 0) return 1;
    }
    return 0;
}

static void load_file(const char *path) {
    strncpy(filename, path, sizeof(filename) - 1);
    FILE *f = fopen(path, "rb");
    if (!f) {
        line_count = 1;
        lines[0][0] = '\0';
        return;
    }
    line_count = 0;
    while (line_count < MAX_LINES && fgets(lines[line_count], MAX_COL, f)) {
        size_t len = strlen(lines[line_count]);
        while (len > 0 && (lines[line_count][len - 1] == '\n' || lines[line_count][len - 1] == '\r')) {
            lines[line_count][--len] = '\0';
        }
        line_count++;
    }
    if (line_count == 0) {
        line_count = 1;
        lines[0][0] = '\0';
    }
    fclose(f);
}

static int save_file(void) {
    FILE *f = fopen(filename, "wb");
    if (!f) return 0;
    for (int i = 0; i < line_count; i++) {
        fprintf(f, "%s\n", lines[i]);
    }
    fclose(f);
    return 1;
}

static void set_color(HANDLE h, WORD attr) {
    SetConsoleTextAttribute(h, attr);
}

static void print_highlighted_line(HANDLE h, const char *line) {
    int i = 0;
    int len = (int)strlen(line);
    while (i < len) {
        if (line[i] == '#' || (line[i] == '/' && line[i + 1] == '/')) {
            set_color(h, FOREGROUND_INTENSITY); // Gri yorum
            printf("%s", line + i);
            break;
        }
        if (line[i] == '"' || line[i] == '\'') {
            char q = line[i];
            set_color(h, FOREGROUND_GREEN | FOREGROUND_INTENSITY); // Yesil string/char
            putchar(line[i++]);
            while (i < len && line[i] != q) putchar(line[i++]);
            if (i < len) putchar(line[i++]);
            continue;
        }
        if (isdigit((unsigned char)line[i])) {
            set_color(h, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY); // Mor sayi
            while (i < len && (isdigit((unsigned char)line[i]) || line[i] == '.')) putchar(line[i++]);
            continue;
        }
        if (isalpha((unsigned char)line[i]) || line[i] == '_' || (unsigned char)line[i] >= 0x80) {
            char word[128];
            int k = 0;
            int start = i;
            while (i < len && (isalnum((unsigned char)line[i]) || line[i] == '_' || (unsigned char)line[i] >= 0x80) && k < 127) {
                word[k++] = line[i++];
            }
            word[k] = '\0';
            if (is_keyword(word)) {
                set_color(h, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY); // Sari anahtar kelime
            } else {
                set_color(h, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            }
            fwrite(line + start, 1, (size_t)(i - start), stdout);
            continue;
        }
        set_color(h, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY); // Turkuaz operatorler
        putchar(line[i++]);
    }
    set_color(h, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

static void draw_screen(void) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(h, &csbi);
    int rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    int edit_rows = rows - 2;
    if (edit_rows < 5) edit_rows = 5;

    COORD origin = {0, 0};
    SetConsoleCursorPosition(h, origin);

    int start_line = 0;
    if (cur_y >= edit_rows) start_line = cur_y - edit_rows + 1;

    for (int r = 0; r < edit_rows; r++) {
        int idx = start_line + r;
        if (idx < line_count) {
            set_color(h, FOREGROUND_INTENSITY);
            printf("%3d | ", idx + 1);
            print_highlighted_line(h, lines[idx]);
        } else {
            set_color(h, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
            printf("  ~ ");
        }
        printf("\x1b[K\n");
    }

    // Status bar
    set_color(h, BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    const char *mstr = (mode == MODE_NORMAL) ? "NORMAL" : (mode == MODE_INSERT ? "INSERT" : "COMMAND");
    printf(" [%s] %s | Satir %d/%d, Sutun %d | %s\x1b[K\n",
           mstr, filename, cur_y + 1, line_count, cur_x + 1, status_msg);
    set_color(h, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

    if (mode == MODE_COMMAND) {
        printf(":%s\x1b[K", cmd_buf);
    } else {
        printf("\x1b[K");
        COORD cpos;
        cpos.X = (SHORT)(6 + cur_x);
        cpos.Y = (SHORT)(cur_y - start_line);
        SetConsoleCursorPosition(h, cpos);
    }
    fflush(stdout);
}

static void execute_command(void) {
    if (strcmp(cmd_buf, "q") == 0) {
        running = 0;
    } else if (strcmp(cmd_buf, "w") == 0) {
        if (save_file()) snprintf(status_msg, sizeof(status_msg), "Kaydedildi: %s", filename);
        else snprintf(status_msg, sizeof(status_msg), "Kaydedilemedi!");
    } else if (strcmp(cmd_buf, "wq") == 0) {
        save_file();
        running = 0;
    } else if (strcmp(cmd_buf, "run") == 0) {
        save_file();
        system("cls");
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "fpp.exe \"%s\"", filename);
        system(cmd);
        printf("\n--- Editore donmek icin bir tusa bas ---");
        _getch();
        system("cls");
        snprintf(status_msg, sizeof(status_msg), "fpp.exe ile calistirildi.");
    } else if (strcmp(cmd_buf, "build") == 0) {
        save_file();
        system("cls");
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "fppc.exe \"%s\" cikti.exe", filename);
        system(cmd);
        printf("\n--- Editore donmek icin bir tusa bas ---");
        _getch();
        system("cls");
        snprintf(status_msg, sizeof(status_msg), "fppc.exe ile cikti.exe uretildi.");
    } else {
        snprintf(status_msg, sizeof(status_msg), "Bilinmeyen komut: :%s", cmd_buf);
    }
    cmd_buf[0] = '\0';
    mode = MODE_NORMAL;
}

static void clamp_cursor(void) {
    if (cur_y < 0) cur_y = 0;
    if (cur_y >= line_count) cur_y = line_count - 1;
    int len = (int)strlen(lines[cur_y]);
    if (cur_x < 0) cur_x = 0;
    if (cur_x > len) cur_x = len;
}

int main(int argc, char **argv) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    if (argc >= 2) load_file(argv[1]);
    system("cls");

    while (running) {
        clamp_cursor();
        draw_screen();

        int ch = _getch();
        if (ch == 0 || ch == 224) {
            int ext = _getch();
            if (ext == 72) cur_y--;      // Up
            else if (ext == 80) cur_y++; // Down
            else if (ext == 75) cur_x--; // Left
            else if (ext == 77) cur_x++; // Right
            continue;
        }

        if (mode == MODE_NORMAL) {
            if (ch == 'i') {
                mode = MODE_INSERT;
                strcpy(status_msg, "INSERT MODU (Cikmak icin ESC)");
            } else if (ch == ':') {
                mode = MODE_COMMAND;
                cmd_buf[0] = '\0';
            } else if (ch == 'h') cur_x--;
            else if (ch == 'l') cur_x++;
            else if (ch == 'k') cur_y--;
            else if (ch == 'j') cur_y++;
            else if (ch == 'x') {
                int len = (int)strlen(lines[cur_y]);
                if (cur_x < len) {
                    memmove(&lines[cur_y][cur_x], &lines[cur_y][cur_x + 1], (size_t)(len - cur_x));
                }
            }
        } else if (mode == MODE_INSERT) {
            if (ch == 27) { // ESC
                mode = MODE_NORMAL;
                strcpy(status_msg, "NORMAL MOD");
            } else if (ch == '\r') { // Enter
                if (line_count < MAX_LINES - 1) {
                    for (int i = line_count; i > cur_y + 1; i--) {
                        strcpy(lines[i], lines[i - 1]);
                    }
                    strcpy(lines[cur_y + 1], lines[cur_y] + cur_x);
                    lines[cur_y][cur_x] = '\0';
                    line_count++;
                    cur_y++;
                    cur_x = 0;
                }
            } else if (ch == '\b') { // Backspace
                if (cur_x > 0) {
                    int len = (int)strlen(lines[cur_y]);
                    memmove(&lines[cur_y][cur_x - 1], &lines[cur_y][cur_x], (size_t)(len - cur_x + 1));
                    cur_x--;
                } else if (cur_y > 0) {
                    int prev_len = (int)strlen(lines[cur_y - 1]);
                    if (prev_len + (int)strlen(lines[cur_y]) < MAX_COL - 1) {
                        strcat(lines[cur_y - 1], lines[cur_y]);
                        for (int i = cur_y; i < line_count - 1; i++) {
                            strcpy(lines[i], lines[i + 1]);
                        }
                        line_count--;
                        cur_y--;
                        cur_x = prev_len;
                    }
                }
            } else if (ch >= 32 && ch <= 126) {
                int len = (int)strlen(lines[cur_y]);
                if (len < MAX_COL - 2) {
                    memmove(&lines[cur_y][cur_x + 1], &lines[cur_y][cur_x], (size_t)(len - cur_x + 1));
                    lines[cur_y][cur_x] = (char)ch;
                    cur_x++;
                }
            }
        } else if (mode == MODE_COMMAND) {
            if (ch == 27) {
                mode = MODE_NORMAL;
                cmd_buf[0] = '\0';
            } else if (ch == '\r') {
                execute_command();
            } else if (ch == '\b') {
                size_t len = strlen(cmd_buf);
                if (len > 0) cmd_buf[len - 1] = '\0';
            } else if (ch >= 32 && ch <= 126) {
                size_t len = strlen(cmd_buf);
                if (len < sizeof(cmd_buf) - 2) {
                    cmd_buf[len] = (char)ch;
                    cmd_buf[len + 1] = '\0';
                }
            }
        }
    }

    system("cls");
    return 0;
}
