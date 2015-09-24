#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define DO_REVERSED_SORT 0
#define FILENAME_READ "onegin.txt"
#define FILENAME_WRITE "onegin_sorted.txt"

#if DO_REVERSED_SORT == 1
    #define STRCMP alnum_strcmp_reversed
#else
    #define STRCMP alnum_strcmp
#endif // IS_REVERSED_SORT

//cp1251
#define START_UPPER_LETTERS -64
#define END_UPPER_LETTERS -33
#define START_LOWER_LETTERS -32
#define END_LOWER_LETTERS -1
#define IS_CODED_ALPHA(x) (((x >= START_UPPER_LETTERS) && (x <= END_UPPER_LETTERS)) || ((x >= START_LOWER_LETTERS) && (x <= END_LOWER_LETTERS)))

#define MY_NAME "GavYur"
#define GREET(program, version) printf("#--- " program " v" version " (%s %s) by " MY_NAME "\n\n", __DATE__, __TIME__)

int read_file(const char* filename, char** buf_addr, int* len);
int save_file(const char* filename, char* buffer, int buffer_len);
char** split_by_lines(char* buffer, int len, int* lines_count);
int alnum_strcmp(const char* str1, const char* str2);
int alnum_strcmp_reversed(const char* str1, const char* str2);
int get_next_alnum_symbol(const char* str, int start_pos);
int get_next_alnum_symbol_reversed(const char* str, int start_pos);
void sort_lines(char** lines, int len);
void make_sorted_buffer(char** lines, int lines_len, char* sorted_buffer);

int main()
{
    GREET("Onegin Sorter", "1.0");

    const char* buffer = 0;
    int buffer_len = 0;
    if (read_file(FILENAME_READ, (char**) &(buffer), &buffer_len))
        return 1;
    int lines_count = 0;
    char** lines = split_by_lines((char*) buffer, buffer_len, &lines_count);
    sort_lines(lines, lines_count);
    char* sorted_buffer = (char*) calloc(buffer_len, sizeof(*sorted_buffer));
    make_sorted_buffer(lines, lines_count, sorted_buffer);
    save_file(FILENAME_WRITE, sorted_buffer, buffer_len);
    printf("Sorted Onegin was written to %s\n", FILENAME_WRITE);

    free(lines[0]);
    free(lines);
    free(sorted_buffer);
    return 0;
}

int read_file(const char* filename, char** buf_addr, int* len)
{
    FILE* stream = fopen(filename, "rb");
    if (!stream)
        return 1;
    if (fseek(stream, 0, SEEK_END))
        return 2;
    *len = ftell(stream);
    rewind(stream);
    const char* buffer = (char*) calloc(*len, sizeof(*buffer)); // I love Pushkin <3
    fread((char*) buffer, *len, sizeof(*buffer), stream);
    fclose(stream);
    *buf_addr = (char*) buffer;
    return 0;
}

int save_file(const char* filename, char* buffer, int buffer_len)
{
    FILE* stream = fopen(filename, "wb");
    if (!stream)
        return 1;
    fwrite(buffer, sizeof(*buffer), buffer_len, stream);
    fclose(stream);
    return 0;
}

// sorry, Pushkin, for non-constant buffer, but I need to replace \n and \r to \0 :(
// replacing special symbols doesn't change your great poem!
char** split_by_lines(char* buffer, int len, int* lines_count)
{
    *lines_count = 0;
    for (int i = 0; i < len; ++i)
    {
        char cur_char = buffer[i];
        if (cur_char == '\n') ++(*lines_count);
        if ((cur_char == '\r') || (cur_char == '\n'))
            buffer[i] = '\0';
    }
    char** text = (char**) calloc(*lines_count, sizeof(*text));
    text[0] = buffer;
    int text_pos = 1;
    for (int i = 0; i < len; ++i)
    {
        char cur_char = buffer[i];
        if ((cur_char == '\0') && (i + 1 < len))
        {
            if (buffer[i + 1] != '\0')
            {
                text[text_pos] = &buffer[i + 1];
                ++text_pos;
            }
        }
    }
    return text;
}

int alnum_strcmp(const char* str1, const char* str2) // ATTENTION! compares only alphabetical and numeric symbols!
{
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    int pos1 = 0, pos2 = 0;
    while ((pos1 < len1) && (pos2 < len2))
    {
        pos1 = get_next_alnum_symbol(str1, pos1);
        pos2 = get_next_alnum_symbol(str2, pos2);
        char sym1 = str1[pos1];
        char sym2 = str2[pos2];
        if (sym1 > sym2)
            return 1;
        else if (sym1 < sym2)
            return -1;
        ++pos1;
        ++pos2;
    }
    if ((pos1 >= len1) && (pos2 < len2))
        return -1;
    else if ((pos1 < len1) && (pos2 >= len2))
        return 1;
    return 0;
}

int alnum_strcmp_reversed(const char* str1, const char* str2) // ATTENTION! compares only alphabetical and numeric symbols!
{
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    int pos1 = len1 - 1, pos2 = len2 - 1;
    while ((pos1 >= 0) && (pos2 >= 0))
    {
        pos1 = get_next_alnum_symbol_reversed(str1, pos1);
        pos2 = get_next_alnum_symbol_reversed(str2, pos2);
        char sym1 = str1[pos1];
        char sym2 = str2[pos2];
        if (sym1 > sym2)
            return 1;
        else if (sym1 < sym2)
            return -1;
        --pos1;
        --pos2;
    }
    if ((pos1 < 0) && (pos2 >= 0))
        return -1;
    else if ((pos2 < 0) && (pos1 >= 0))
        return 1;
    return 0;
}

int get_next_alnum_symbol_reversed(const char* str, int start_pos)
{
    while (start_pos >= 0)
    {
        char sym = str[start_pos];
        if (isalnum(sym) || IS_CODED_ALPHA(sym))
            return start_pos;
        --start_pos;
    }
    return start_pos;
}

int get_next_alnum_symbol(const char* str, int start_pos)
{
    char sym = str[start_pos];
    while (sym != '\0')
    {
        if (isalnum(sym) || IS_CODED_ALPHA(sym))
            return start_pos;
        ++start_pos;
        sym = str[start_pos];
    }
    return start_pos - 1;
}

//TODO: write qsort here
void sort_lines(char** lines, int len)
{
    for (int i = 0; i < len - 1; ++i)
    {
        for (int j = 0; j < len - i - 1; ++j)
        {
            if (STRCMP(lines[j], lines[j + 1]) > 0)
            {
                char* backup = lines[j];
                lines[j] = lines[j + 1];
                lines[j + 1] = backup;
            }
        }
    }
 }

void make_sorted_buffer(char** lines, int lines_len, char* sorted_buffer)
{
    int buf_position = 0;
    for (int i = 0; i < lines_len; ++i)
    {
        char* line = lines[i];
        int line_pos = 0;
        char sym = line[line_pos];
        while (sym != '\0')
        {
            sorted_buffer[buf_position] = sym;
            ++buf_position;
            ++line_pos;
            sym = line[line_pos];
        }
        sorted_buffer[buf_position] = '\n';
        ++buf_position;
    }
}
