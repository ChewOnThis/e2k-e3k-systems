#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Escape a single line for inclusion inside "..." C string:
   - \  -> \\
   - "  -> \"
   We also strip any '\r'. We DO NOT include the trailing newline;
   the caller will append "\\n" explicitly.
*/
static char *escape_line(const char *line) {
    size_t in_len = strlen(line);
    size_t out_cap = in_len * 2 + 16;  // worst case grow
    char *out = (char *)malloc(out_cap);
    if (!out) return NULL;

    size_t j = 0;
    for (size_t i = 0; i < in_len; ++i) {
        unsigned char c = (unsigned char)line[i];
        if (c == '\r') {
            continue; // drop CRs
        } else if (c == '\\' || c == '\"') {
            if (j + 2 >= out_cap) {
                out_cap *= 2;
                char *tmp = (char *)realloc(out, out_cap);
                if (!tmp) { free(out); return NULL; }
                out = tmp;
            }
            out[j++] = '\\';
            out[j++] = (char)c;
        } else {
            if (j + 1 >= out_cap) {
                out_cap *= 2;
                char *tmp = (char *)realloc(out, out_cap);
                if (!tmp) { free(out); return NULL; }
                out = tmp;
            }
            out[j++] = (char)c;
        }
    }
    out[j] = '\0';
    return out;
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s input.html [output.txt]\n", argv[0]);
        return 1;
    }

    const char *in_path = argv[1];
    const char *out_path = (argc == 3) ? argv[2] : "createHTML.txt";

    FILE *fin = fopen(in_path, "rb");
    if (!fin) {
        perror("Error opening input");
        return 1;
    }
    FILE *fout = fopen(out_path, "wb");
    if (!fout) {
        perror("Error opening output");
        fclose(fin);
        return 1;
    }

    /* Write function header */
    fputs("String createHTML() {\n", fout);
    fputs("  String str = \"\";\n", fout);

    /* Read input line-by-line to preserve structure */
    char *line = NULL;
    size_t line_cap = 0;
    ssize_t n;

    /* Use getline where available; otherwise fallback */
#if defined(_GNU_SOURCE) || defined(__APPLE__)
    while ((n = getline(&line, &line_cap, fin)) != -1) {
        (void)n; // silence unused warning
#else
    /* Portable fallback for environments without getline */
    {
        char buf[8192];
        while (fgets(buf, sizeof(buf), fin)) {
            line = buf;
#endif
            char *escaped = escape_line(line);
            if (!escaped) {
                fprintf(stderr, "Out of memory while escaping.\n");
                fclose(fin);
                fclose(fout);
#if defined(_GNU_SOURCE) || defined(__APPLE__)
                free(line);
#endif
                return 1;
            }
            fputs("  str += \"", fout);
            fputs(escaped, fout);
            fputs("\\n\";\n", fout);
            free(escaped);
#if !defined(_GNU_SOURCE) && !defined(__APPLE__)
        }
#endif
    }

    /* Footer */
    fputs("  return str;\n", fout);
    fputs("}\n", fout);

#if defined(_GNU_SOURCE) || defined(__APPLE__)
    free(line);
#endif
    fclose(fin);
    fclose(fout);

    fprintf(stdout, "Wrote Arduino String function to: %s\n", out_path);
    return 0;
}
