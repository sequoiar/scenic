#include <glib.h>
#include <stdio.h>

const char* str = "mystring=\"bar you\" Hello tab:\twhat is 1 the \\";


int main(int, char**)
{
    printf("\nString:%s---\n", str);
    gchar* g = g_strescape(str, "\b\r\f\t");

    printf("\nEsc String:%s---\n", g);

    gchar* g2 = g_strcompress(g);


    printf("\nUnEsc String:%s---\n", g2);

    printf("\nDONE.\n");
}


