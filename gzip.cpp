#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "zlib.h"
#define CHUNK 1024 

char* inf(const char *in, int len)//, char *out)
{
    int ret;
    unsigned have;
    z_stream strm;
    char *s = (char*)malloc(CHUNK);
    char *out = s;
    

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit2(&strm, 16+MAX_WBITS);

    if (ret != Z_OK)
        return NULL;

    strm.avail_in = len;
    strm.next_in = (Bytef*)in;

    int off = 0;

    do {
        strm.avail_out = CHUNK;
        strm.next_out = (Bytef*)out;
        
        inflate(&strm, Z_NO_FLUSH);
       
        have = CHUNK - strm.avail_out;

        off+=have;

        s = (char*)realloc(s,off+CHUNK);
        out = s+off;
        
    } while (strm.avail_out == 0);

    (void)inflateEnd(&strm);

    return s;
}

/*int main(int argc, char **argv)
{

    unsigned char in[CHUNK*1024];

    char *out = malloc(CHUNK);

    int len = fread(in, 1, CHUNK*1024, stdin);


    out = inf(in,len,out);

    printf(out);
}
*/
