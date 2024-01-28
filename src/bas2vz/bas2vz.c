/*
 * Copyright (C) 2024 Rhys Weatherley
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* BASIC token names */
static const char * const basic_token_names[128] = {
/*          0               1               2               3               4               5               6               7 */
/* 0x80 */  "END",          "FOR",          "RESET",        "SET",          "CLS",          "CMD",          "RANDOM",       "NEXT",
/* 0x88 */  "DATA",         "INPUT",        "DIM",          "READ",         "LET",          "GOTO",         "RUN",          "IF",
/* 0x90 */  "RESTORE",      "GOSUB",        "RETURN",       "REM",          "STOP",         "ELSE",         "COPY",         "COLOR",
/* 0x98 */  "VERIFY",       "DEFINT",       "DEFSNG",       "DEFDBL",       "CRUN",         "MODE",         "SOUND",        "RESUME",
/* 0xA0 */  "OUT",          "ON",           "OPEN",         "FIELD",        "GET",          "PUT",          "CLOSE",        "LOAD",
/* 0xA8 */  "MERGE",        "NAME",         "KILL",         "LSET",         "RSET",         "SAVE",         "SYSTEM",       "LPRINT",
/* 0xB0 */  "DEF",          "POKE",         "PRINT",        "CONT",         "LIST",         "LLIST",        "DELETE",       "AUTO",
/* 0xB8 */  "CLEAR",        "CLOAD",        "CSAVE",        "NEW",          "TAB(",         "TO",           "FN",           "USING",
/* 0xC0 */  "VARPTR",       "USR",          "ERL",          "ERR",          "STRING$",      "INSTR",        "POINT",        "TIME$",
/* 0xC8 */  "MEM",          "INKEY$",       "THEN",         "NOT",          "STEP",         "+",            "-",            "*",
/* 0xD0 */  "/",            "^",            "AND",          "OR",           ">",            "=",            "<",            "SGN",
/* 0xD8 */  "INT",          "ABS",          "FRE",          "INP",          "POS",          "SQR",          "RND",          "LOG",
/* 0xE0 */  "EXP",          "COS",          "SIN",          "TAN",          "ATN",          "PEEK",         "CVI",          "CVS",
/* 0xE8 */  "CVD",          "EOF",          "LOC",          "LOF",          "MKI$",         "MKS$",         "MKD$",         "CINT",
/* 0xF0 */  "CSNG",         "CDBL",         "FIX",          "LEN",          "STR$",         "VAL",          "ASC",          "CHR$",
/* 0xF8 */  "LEFT$",        "RIGHT$",       "MID$",         "'",            "",             "",             "",             "",
};

static void write_program_name(FILE *out, const char *filename);
static unsigned tokenize_line
    (FILE *out, const char *input, unsigned input_line,
     unsigned address, char *line);

int main(int argc, char *argv[])
{
    const char *input;
    const char *output;
    unsigned address;
    unsigned input_line;
    char buffer[BUFSIZ];
    FILE *in;
    FILE *out;

    /* Parse the command-line arguments */
    if (argc < 3) {
        fprintf(stderr, "Usage: %s input.bas output.vz\n", argv[0]);
        return 1;
    }
    input = argv[1];
    output = argv[2];

    /* Open the input file */
    if ((in = fopen(input, "r")) == NULL) {
        perror(input);
        return 1;
    }

    /* Open the output file */
    if ((out = fopen(output, "wb")) == NULL) {
        perror(input);
        fclose(in);
        return 1;
    }

    /* Output the VZ file header */
    fputc('V', out);
    fputc('Z', out);
    fputc('F', out);
    fputc('0', out);
    write_program_name(out, input);
    fputc(0xf0, out);
    fputc(0xe9, out);
    fputc(0x7a, out);

    /* Tokenize the lines of BASIC code in the input file */
    address = 0x7ae9;
    while (fgets(buffer, sizeof(buffer), in) != NULL) {
        address = tokenize_line(out, input, input_line, address, buffer);
        if (!address) {
            /* An error occurred */
            fclose(out);
            fclose(in);
            return 1;
        }
        ++input_line;
    }
    fputc(0x00, out);
    fputc(0x00, out);

    /* Clean up and exit */
    fclose(out);
    fclose(in);
    return 0;
}

static void write_program_name(FILE *out, const char *filename)
{
    size_t len = strlen(filename);
    size_t count = 16;
    while (len > 0 && filename[len - 1] != '/' && filename[len - 1] != '\\') {
        --len;
    }
    while (count > 0 && filename[len] != '.' && filename[len] != '\0') {
        int ch = (filename[len++] & 0xFF);
        if (ch >= 0x60 && ch <= 0x7F) {
            /* Convert the program name to upper case */
            ch -= 0x20;
        } else if (ch < 0x20 || ch >= 0x80) {
            /* Ignore characters outside the printable ASCII range */
            continue;
        }
        fputc(ch, out);
        --count;
    }
    while (count > 0) {
        /* Pad the program name with NUL's */
        fputc(0, out);
        --count;
    }
    fputc(0, out); /* Final NUL terminator for the program name */
}

static int find_token(char **line)
{
    int token;
    size_t len1;
    size_t len2;
    for (token = 128; token < 256; ++token) {
        len1 = strlen(basic_token_names[token - 128]);
        len2 = strlen(*line);
        if (len1 > 0 && len2 >= len1 &&
                !strncasecmp(*line, basic_token_names[token - 128], len1)) {
            (*line) += len1;
            return token;
        }
    }
    return -1;
}

static unsigned write_tokens
    (FILE *out, unsigned address, unsigned linenum,
     const unsigned char *tokens, size_t len)
{
    unsigned next_address = address + len + 5;
    fputc(next_address & 0xFF, out);
    fputc((next_address >> 8) & 0xFF, out);
    fputc(linenum & 0xFF, out);
    fputc((linenum >> 8) & 0xFF, out);
    fwrite(tokens, 1, len, out);
    fputc(0x00, out);
    return next_address;
}

static int to_hex(char ch)
{
    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    else if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    else
        return ch - '0';
}

static unsigned tokenize_line
    (FILE *out, const char *filename, unsigned input_line,
     unsigned address, char *line)
{
    static unsigned long last_linenum = 0;
    static int have_last_linenum = 0;
    size_t len;
    unsigned long linenum;
    unsigned char tokens[BUFSIZ];
    char *endptr;
    int ch, quote;
    int rem;

    /* Strip whitespace from the start of the line */
    while (line[0] != '\0' && isspace(line[0])) {
        ++line;
    }

    /* Strip whitespace from the end of the line */
    len = strlen(line);
    while (len > 0 && isspace(line[len - 1])) {
        --len;
    }
    line[len] = '\0';

    /* Ignore empty lines and those starting with ';' or '#' */
    if (len == 0 || line[0] == ';' || line[0] == '#') {
        return address;
    }

    /* Get and validate the line number */
    if (line[0] < '0' || line[0] > '9') {
        fprintf(stderr, "%s:%u: line number expected at start of line\n",
                filename, input_line);
        return 0;
    }
    linenum = strtoul(line, &endptr, 10);
    line = endptr;
    if (have_last_linenum && last_linenum >= linenum) {
        fprintf(stderr, "%s:%u: line numbers are not increasing; %lu to %lu\n",
                filename, input_line, last_linenum, linenum);
        return 0;
    }
    last_linenum = linenum;
    have_last_linenum = 1;
    if (linenum >= 65530U) {
        fprintf(stderr, "%s:%u: invalid line number\n", filename, input_line);
        return 0;
    }

    /* Skip whitespace after the line number */
    while (line[0] != '\0' && isspace(line[0])) {
        ++line;
    }

    /* If the line is empty, change it to a line with "REM" only */
    len = 0;
    if (line[0] == '\0') {
        tokens[len++] = 0x93;
        return write_tokens(out, address, linenum, tokens, len);
    }

    /* Tokenize the rest of the line */
    quote = 0;
    rem = 0;
    while ((ch = *line) != '\0') {
        ch &= 0xFF;
        if (rem) {
            /* We are inside a "REM" statement */
            tokens[len++] = ch;
            ++line;
        } else if (quote) {
            /* We are inside a quoted string */
            if (ch == '\\' && (line[1] == 'x' || line[1] == 'X') &&
                    isxdigit(line[2]) && isxdigit(line[3])) {
                /* Escaped special character */
                ch = to_hex(line[2]) * 16 + to_hex(line[3]);
                line += 3;
            }
            tokens[len++] = ch;
            if (ch == quote) {
                quote = 0;
            }
            ++line;
        } else if (ch == '\'' || ch == '"') {
            /* Start of a quoted string */
            tokens[len++] = ch;
            quote = ch;
            ++line;
        } else {
            /* May be a token or a normal character */
            int token = find_token(&line);
            if (token >= 0) {
                tokens[len++] = (unsigned char)token;
                if (token == 0x93) {
                    rem = 1;
                }
            } else if (ch != ' ') {
                /* Ordinary character outside of a quoted string */
                if (ch >= 'a' && ch <= 'z')
                    tokens[len++] = ch - 'a' + 'A';
                else if (ch < 0x80)
                    tokens[len++] = ch;
                ++line;
            } else {
                /* Collapse multiple spaces into a single space */
                ++line;
                if (line[0] != ' ') {
                    tokens[len++] = ch;
                }
            }
        }
    }
    return write_tokens(out, address, linenum, tokens, len);
}
