// Compilation:
// gcc `pkg-config --cflags --libs glib-2.0` -o test test_escape.c

#include <glib.h>
#include <stdio.h>


const char* str = "mystring=\"bar you\" Hello\twhat\fis 1\b the\r \\";

int main(int argc, char** argv)
{
    char* input_str;
    gchar *g, *g2;
    //FILE *input = stdin;
    input_str = argv[1];

    //printf("a toi de jouer tiit: ");
    //fscanf(input, "%s", input_str);
    printf("Input : %s\n", input_str);

    g = g_strescape(input_str, "\n\t\b\f\r");
    printf("\nInput after escaping: %s---\n", g);

    g2 = g_strcompress(g);
    printf("\nInput restored: %s---\n", g2);

    printf("\nDONE.\n");
}

