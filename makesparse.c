// -*- c-basic-offset: 4; c-backslash-column: 79; indent-tabs-mode: nil -*-
// vim:sw=4 ts=4 sts=4 expandtab
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>

#define _FILE_OFFSET_BITS 64

static void check_err(FILE *f, char const *fname)
{
    if (ferror(f)) {
        fprintf(stderr, "Cannot write into %s: %s\n", fname, strerror(errno));
        exit(1);
    }
}

static char const min_hole[256];  // seek for holes biger than this

static void write_hole(FILE *f, char const *fname, off_t len)
{
    if (len == 0) return;

    if (len >= (off_t)sizeof(min_hole)) {
        if (0 != fseeko(f, len, SEEK_CUR)) {
            fprintf(stderr, "Cannot seek %ld bytes in %s: %s\n", len, fname, strerror(errno));
            exit(1);
        }
    } else {
        (void)fwrite(min_hole, 1, len, f);
        check_err(f, fname);
    }
}

int main(int nb_args, char const **args)
{
    if (nb_args != 2) {
        fprintf(stderr, "That's `makesparse dest_file`\n");
        return EXIT_FAILURE;
    }

    FILE *out = fopen(args[1], "w+");
    if (! out) {
        fprintf(stderr, "Cannot open %s: %s\n", args[1], strerror(errno));
        return EXIT_FAILURE;
    }

    off_t current_hole = 0;

    while (! feof(stdin)) {
        char buf[1024];
        size_t l = fread(buf, 1, sizeof(buf), stdin);
        if (ferror(stdin)) {
            fprintf(stderr, "Cannot read from stdin: %s\n", strerror(errno));
            return EXIT_FAILURE;
        }

        // How many zeros do we get?
        unsigned z;
        for (z = 0; z < l && buf[z] == 0; z++) ;
        current_hole += z;

        if (z < l) { // end of hole, flush what we have
            // dig a hole
            write_hole(out, args[1], current_hole);
            current_hole = 0;
            // write the data
            (void)fwrite(buf+z, 1, l - z, out);
            check_err(out, args[1]);
        }
    }

    write_hole(out, args[1], current_hole);
    if (0 != ftruncate(fileno(out), ftello(out))) { // "truncate" the file to this length
        fprintf(stderr, "Cannot truncate %s: %s\n", args[1], strerror(errno));
    }

    return EXIT_SUCCESS;
}

