#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <ctype.h>
#include <locale.h>
#include <wchar.h>

#define DATA_FILE "students.txt"

typedef struct {
    int tt;
    char sbd[32];       /* Chuoi giu nguyen so 0 dau. */
    char name[160];
    char school[256];
    char subject[80];
    double scores[3];
} Student;

/* Doc ca dong de ho ten/ten truong co the chua khoang trang.
   Tra ve 0 neu EOF/loi; dong qua dai duoc bo va yeu cau nhap lai. */
static int input_line(const char *prompt, char *buffer, size_t size)
{
    for (;;) {
        fputs(prompt, stdout);
        if (fflush(stdout) == EOF)
            return 0;
        if (fgets(buffer, (int)size, stdin) == NULL)
            return 0;
        char *newline = strchr(buffer, '\n');
        if (newline != NULL) {
            *newline = '\0';
        } else if (!feof(stdin)) {
            int ch = getchar();
            if (ch != '\n' && ch != EOF) {
                while ((ch = getchar()) != '\n' && ch != EOF) { }
                fputs("Du lieu qua dai. Hay nhap lai.\n", stderr);
                continue;
            }
        }
        buffer[strcspn(buffer, "\r")] = '\0';
        int has_text = 0;
        for (const unsigned char *p = (const unsigned char *)buffer; *p; ++p) {
            if (!isspace(*p))
                has_text = 1;
        }
        if (!has_text || strchr(buffer, '|') != NULL) {
            fputs("Khong de trong hoac nhap dau |.\n", stderr);
            continue;
        }
        return 1;
    }
}

static int input_integer(const char *prompt, int min, int max, int *value)
{
    char text[64];
    while (input_line(prompt, text, sizeof text)) {
        char *end;
        errno = 0;
        long n = strtol(text, &end, 10);
        while (isspace((unsigned char)*end))
            ++end;
        if (errno == 0 && end != text && *end == '\0'
            && n >= min && n <= max) {
            *value = (int)n;
            return 1;
        }
        fprintf(stderr, "Nhap so nguyen tu %d den %d.\n", min, max);
    }
    return 0;
}

static int input_score(const char *prompt, double *value)
{
    char text[64];
    while (input_line(prompt, text, sizeof text)) {
        char *end;
        errno = 0;
        double n = strtod(text, &end);
        while (isspace((unsigned char)*end))
            ++end;
        if (errno == 0 && end != text && *end == '\0'
            && isfinite(n) && n >= 0.0 && n <= 100.0) {
            *value = n;
            return 1;
        }
        fputs("Nhap diem tu 0 den 100, dung dau cham thap phan.\n", stderr);
    }
    return 0;
}

/* wcwidth do so cot hien thi, khong phai so byte UTF-8.
   Ho tro ten tieng Viet, ke ca dau duoc luu thanh ky tu ket hop.
   wcwidth la ham POSIX (Linux/macOS/WSL). */
static int text_width(const char *text)
{
    mbstate_t state = {0};
    int width = 0;
    while (*text) {
        wchar_t ch;
        size_t n = mbrtowc(&ch, text, strlen(text), &state);
        if (n == (size_t)-1 || n == (size_t)-2) {
            memset(&state, 0, sizeof state);
            ++text;
            ++width;
        } else if (n == 0) {
            break;
        } else {
            int w = wcwidth(ch);
            width += w < 0 ? 1 : w;
            text += n;
        }
    }
    return width;
}

static const char * const headers[9] = {
    "TT", "SBD", "Họ và tên", "Tên trường", "Tên môn",
    "Bài 1", "Bài 2", "Bài 3", "Tổng"
};

static void measure(const char * const cells[9], int widths[9])
{
    for (int i = 0; i < 9; ++i) {
        int n = text_width(cells[i]);
        if (n > widths[i])
            widths[i] = n;
    }
}

static int print_row(FILE *out, const char * const cells[9],
                     const int widths[9])
{
    for (int i = 0; i < 9; ++i) {
        int spaces = widths[i] - text_width(cells[i]);
        int right = i == 0 || i >= 5;
        if (right) {
            for (int n = 0; n < spaces; ++n)
                if (fputc(' ', out) == EOF) return 0;
        }
        if (fputs(cells[i], out) == EOF) return 0;
        if (!right) {
            for (int n = 0; n < spaces; ++n)
                if (fputc(' ', out) == EOF) return 0;
        }
        if (fputs(i == 8 ? "\n" : " | ", out) == EOF) return 0;
    }
    return 1;
}

static int print_header(FILE *out, const int widths[9])
{
    if (!print_row(out, headers, widths)) return 0;
    for (int i = 0; i < 9; ++i) {
        for (int n = 0; n < widths[i]; ++n)
            if (fputc('-', out) == EOF) return 0;
        if (fputs(i == 8 ? "\n" : "-+-", out) == EOF) return 0;
    }
    return 1;
}

/* Tach 9 cot theo |, bo khoang trang hai dau moi cot.
   0: dong trong/duong ke; 1: dong du lieu; -1: sai so cot. */
static int split_row(char *line, const char *cells[9])
{
    if (strspn(line, "-+ \t\r\n") == strlen(line)) return 0;
    int count = 0;
    char *start = line;
    for (;;) {
        if (count == 9) return -1;
        char *end = strchr(start, '|');
        if (end != NULL) *end = '\0';
        while (isspace((unsigned char)*start)) ++start;
        size_t len = strlen(start);
        while (len > 0 && isspace((unsigned char)start[len - 1]))
            start[--len] = '\0';
        cells[count++] = start;
        if (end == NULL) break;
        start = end + 1;
    }
    return count == 9 ? 1 : -1;
}

/* Pass 1: do do rong cac cot. Pass 2: hien thi bang da can cot. */
static int scan_table(FILE *fp, int widths[9], FILE *out)
{
    char line[2048];
    size_t line_no = 0;
    if (fseek(fp, 0L, SEEK_SET) != 0) return 0;
    while (fgets(line, sizeof line, fp) != NULL) {
        ++line_no;
        if (strchr(line, '\n') == NULL && !feof(fp)) {
            fputs("Dong du lieu qua dai.\n", stderr);
            return 0;
        }
        const char *cells[9];
        int status = split_row(line, cells);
        if (status < 0) {
            fprintf(stderr, "Dong %zu khong du 9 cot phan cach boi |.\n", line_no);
            return 0;
        }
        if (status == 0 || strcmp(cells[0], "TT") == 0) continue;
        if (out == NULL)
            measure(cells, widths);
        else if (!print_row(out, cells, widths))
            return 0;
    }
    return !ferror(fp);
}

static int read_file(void)
{
    FILE *fp = fopen(DATA_FILE, "r");
    if (fp == NULL) {
        perror("Khong mo duoc students.txt de doc");
        return 0;
    }
    int widths[9] = {0};
    measure(headers, widths);
    int ok = scan_table(fp, widths, NULL);
    if (ok && fputc('\n', stdout) == EOF) ok = 0;
    if (ok) ok = print_header(stdout, widths);
    if (ok) ok = scan_table(fp, widths, stdout);
    if (fclose(fp) == EOF) ok = 0;
    if (fflush(stdout) == EOF) ok = 0;
    if (!ok) fputs("Khong doc/hien thi duoc bang.\n", stderr);
    return ok;
}

static int write_student(void)
{
    Student s;
    if (!input_integer("TT: ", 1, INT_MAX, &s.tt)
        || !input_line("SBD: ", s.sbd, sizeof s.sbd)
        || !input_line("Ho va ten: ", s.name, sizeof s.name)
        || !input_line("Ten truong: ", s.school, sizeof s.school)
        || !input_line("Ten mon: ", s.subject, sizeof s.subject)
        || !input_score("Diem bai 1: ", &s.scores[0])
        || !input_score("Diem bai 2: ", &s.scores[1])
        || !input_score("Diem bai 3: ", &s.scores[2])) {
        fputs("Dung nhap; chua ghi ban ghi nay.\n", stderr);
        return 0;
    }

    /* a+b: doc byte de kiem tra cuoi file; ghi luon noi vao cuoi.
       Neu file chua ton tai, fopen tao file moi. */
    FILE *fp = fopen(DATA_FILE, "a+b");
    if (fp == NULL) {
        perror("Khong mo duoc students.txt de ghi");
        return 0;
    }
    char tt[32], a[32], b[32], c[32], total_text[32];
    double total = s.scores[0] + s.scores[1] + s.scores[2];
    snprintf(tt, sizeof tt, "%d", s.tt);
    snprintf(a, sizeof a, "%.2f", s.scores[0]);
    snprintf(b, sizeof b, "%.2f", s.scores[1]);
    snprintf(c, sizeof c, "%.2f", s.scores[2]);
    snprintf(total_text, sizeof total_text, "%.2f", total);
    const char *cells[9] = {
        tt, s.sbd, s.name, s.school, s.subject, a, b, c, total_text
    };
    int widths[9] = {0};
    measure(headers, widths);
    int ok = scan_table(fp, widths, NULL);
    measure(cells, widths);
    if (fseek(fp, 0L, SEEK_END) != 0) ok = 0;
    long length = ok ? ftell(fp) : -1L;
    if (length < 0)
        ok = 0;
    if (ok && length == 0) {
        ok = print_header(fp, widths);
    } else if (ok) {
        /* Them xuong dong neu file duoc sua tay va thieu newline cuoi.
           File duoc luu UTF-8, khong co chuyen doi xuong dong phuc tap. */
        ok = fseek(fp, -1L, SEEK_END) == 0;
        int last = ok ? fgetc(fp) : EOF;
        if (last == EOF)
            ok = 0;
        /* Dinh vi khi chuyen tu doc sang ghi tren update stream. */
        if (fseek(fp, 0L, SEEK_END) != 0)
            ok = 0;
        if (ok && last != '\n')
            ok = fputc('\n', fp) != EOF;
    }
    if (ok) ok = print_row(fp, cells, widths);
    if (fflush(fp) == EOF)
        ok = 0;
    if (fclose(fp) == EOF)
        ok = 0;
    if (ok)
        printf("Da ghi vao students.txt. Tong diem: %.2f\n", total);
    else
        fputs("Ghi file that bai; hay kiem tra noi dung file.\n", stderr);
    return ok;
}

int main(void)
{
    /* LC_CTYPE theo terminal UTF-8; giu LC_NUMERIC mac dinh de dung dau cham. */
    if (setlocale(LC_CTYPE, "") == NULL || MB_CUR_MAX == 1) {
        if (setlocale(LC_CTYPE, "C.UTF-8") == NULL) {
            fputs("Can moi truong UTF-8 de can cot tieng Viet.\n", stderr);
            return EXIT_FAILURE;
        }
    }
    int had_error = 0;
    for (;;) {
        puts("\n===== FILE I/O =====");
        puts("1. Doc va hien thi students.txt");
        puts("2. Nhap va ghi them du lieu vao students.txt");
        puts("0. Thoat");
        int choice;
        if (!input_integer("Chon: ", 0, 2, &choice)) {
            if (ferror(stdin))
                had_error = 1;
            break;
        }
        if (choice == 0)
            break;
        int ok = choice == 1 ? read_file() : write_student();
        if (!ok)
            had_error = 1;
    }
    if (fflush(stdout) == EOF)
        had_error = 1;
    return had_error ? EXIT_FAILURE : EXIT_SUCCESS;
}
