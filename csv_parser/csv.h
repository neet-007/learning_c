/*
    **Spec**

    *Fields are separated by commas.
    *A field may be enclosed in double-quote characters "...".
    *A quoted field may contain commas but not newlines.
    *A quoted field may contain double-quote characters ",represented by "".
    *Fields may be empty; "" and an empty string both represent an empty field.
    *Leading and trailing white space is preserved.
    *char acsvgetli ne(F1LE af) ;
    *reads one line from open input file f ;
    *assumes that input lines are terminated by \r, \n, \r\n, or EOF.
    *returns pointer to line, with terminator removed, or N U L L if EOF occurred.
    *line may be of arbitrary length; returns NULL if memory limit exceeded.
    *line must be treated as read-only storage;
    *caller must make a copy to preserve or change contents.
    *char acsvf i el d ( i n t n) ;
    *fields are numbered from 0.
    *returns n-th field from last line read by csvgetl i ne;
    *returns N U L L if n < 0 or beyond last field.
    *fields are separated by commas.
    *fields may be surrounded by "..."; such quotes are removed;
    *within "... "," " is replaced by " and comma is not a separator.
    *in unquoted fields, quotes are regular characters.
    *there can be an arbitrary number of fields of any length;
    *returns NULL if memory limit exceeded.
    *field must be treated as read-only storage;
    *caller must make a copy to preserve or change contents.
    *behavior undefined if called before csvgetline is called.
    *in t c s v n f i e l d(void) ;
    *returns number of fields on last line read by csvgetline.
    *behavior undefined if called before csvgetline is called
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char *csv_get_line(FILE *fin) ; /* read next input line */
extern char *csv_field (int n) ; /* return field n */
extern int csv_n_field(void) ; /* return number of fields */
