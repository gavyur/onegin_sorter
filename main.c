#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define FILENAME_READ "onegin.txt"
#define FILENAME_WRITE_SORTED "onegin_sorted.txt"
#define FILENAME_WRITE_BACKSORTED "onegin_backsorted.txt"

//cp1251
#define START_UPPER_LETTERS -64
#define END_UPPER_LETTERS -33
#define START_LOWER_LETTERS -32
#define END_LOWER_LETTERS -1
#define IS_CODED_ALPHA(x) (((x >= START_UPPER_LETTERS) && (x <= END_UPPER_LETTERS)) || ((x >= START_LOWER_LETTERS) && (x <= END_LOWER_LETTERS)))

#define MY_NAME "GavYur"
#define GREET(program, version) printf("#--- " program " v" version " (%s %s) by " MY_NAME "\n\n", __DATE__, __TIME__)

typedef struct
{
    char* str;
    int length;
} String;

int read_file(const char* filename, char** buf_addr, int* len);
int save_file(const char* filename, char* buffer, int buffer_len);
String* split_by_lines(char* buffer, int len, int* lines_count);
int alnum_strcmp(const String str1, const String str2);
int alnum_strcmp_reversed(const String str1, const String str2);
int get_next_alnum_symbol(const char* str, int start_pos);
int get_next_alnum_symbol_reversed(const char* str, int start_pos);
int sort_lines(String* lines, int left, int right, int (*compare)(const String str1, const String str2));
void make_sorted_buffer(String* lines, int lines_len, char* sorted_buffer);
int string_ctor(String* This, const char* str);
int string_destruct(String* This);
int cleanup(String* lines, int lines_count, char* sorted_buffer);

int main()
{
    GREET("Onegin Sorter", "1.0");

    const char* buffer = 0;
    int buffer_len = 0;
    if (read_file(FILENAME_READ, (char**) &(buffer), &buffer_len))
        return 1;
    int lines_count = 0;
    String* lines = split_by_lines((char*) buffer, buffer_len, &lines_count);

    char* sorted_buffer = (char*) calloc(buffer_len, sizeof(*sorted_buffer));

    sort_lines(lines, 0, lines_count - 1, alnum_strcmp);
    make_sorted_buffer(lines, lines_count, sorted_buffer);
    save_file(FILENAME_WRITE_SORTED, sorted_buffer, buffer_len);
    printf("Sorted Onegin was written to %s\n", FILENAME_WRITE_SORTED);

    sort_lines(lines, 0, lines_count - 1, alnum_strcmp_reversed);
    make_sorted_buffer(lines, lines_count, sorted_buffer);
    save_file(FILENAME_WRITE_BACKSORTED, sorted_buffer, buffer_len);
    printf("Backsorted Onegin was written to %s\n", FILENAME_WRITE_BACKSORTED);

    cleanup(lines, lines_count, sorted_buffer);
    return 0;
}

int cleanup(String* lines, int lines_count, char* sorted_buffer)
{
    for (int i = 0; i < lines_count; ++i)
        string_destruct(&lines[i]);
    free(lines);
    free(sorted_buffer);
    return 0;
}

int string_ctor(String* This, const char* str)
{
    This->str = strdup(str);
    if (!This->str)
        return 1;
    This->length = strlen(str);
    return 0;
}

int string_destruct(String* This)
{
    free(This->str);
    This->str = 0;
    This->length = 0;
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
String* split_by_lines(char* buffer, int len, int* lines_count)
{
    *lines_count = 0;
    for (int i = 0; i < len; ++i)
    {
        char cur_char = buffer[i];
        if (cur_char == '\n') ++(*lines_count);
        if ((cur_char == '\r') || (cur_char == '\n'))
            buffer[i] = '\0';
    }
    String* lines = (String*) calloc(*lines_count, sizeof(*lines));
    String first_string = {"", 0};
    string_ctor(&first_string, buffer);
    lines[0] = first_string;
    int lines_pos = 1;
    for (int i = 0; i < len; ++i)
    {
        char cur_char = buffer[i];
        if ((cur_char == '\0') && (i + 1 < len))
        {
            if (buffer[i + 1] != '\0')
            {
                String str = {"", 0};
                string_ctor(&str, &buffer[i + 1]);
                lines[lines_pos] = str;
                ++lines_pos;
            }
        }
    }
    return lines;
}

int alnum_strcmp(const String str1, const String str2) // ATTENTION! compares only alphabetical and numeric symbols!
{
    int pos1 = 0, pos2 = 0;
    while ((pos1 < str1.length) && (pos2 < str2.length))
    {
        pos1 = get_next_alnum_symbol(str1.str, pos1);
        pos2 = get_next_alnum_symbol(str2.str, pos2);
        char sym1 = str1.str[pos1];
        char sym2 = str2.str[pos2];
        if (sym1 > sym2)
            return 1;
        else if (sym1 < sym2)
            return -1;
        ++pos1;
        ++pos2;
    }
    if ((pos1 >= str1.length) && (pos2 < str2.length))
        return -1;
    else if ((pos1 < str1.length) && (pos2 >= str2.length))
        return 1;
    return 0;
}

int alnum_strcmp_reversed(const String str1, const String str2) // ATTENTION! compares only alphabetical and numeric symbols!
{
    int pos1 = str1.length - 1, pos2 = str2.length - 1;
    while ((pos1 >= 0) && (pos2 >= 0))
    {
        pos1 = get_next_alnum_symbol_reversed(str1.str, pos1);
        pos2 = get_next_alnum_symbol_reversed(str2.str, pos2);
        char sym1 = str1.str[pos1];
        char sym2 = str2.str[pos2];
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

int sort_lines(String* lines, int left, int right, int (*compare)(const String str1, const String str2))
{
    int left_backup = left;
    int right_backup = right;
    String temp_for_swap = {"", 0};
    String mid = lines[(left + right) / 2];
    while (left <= right)
    {
        while (((*compare)(lines[left], mid) < 0) && (left <= right_backup))
            left++;
        while (((*compare)(lines[right], mid) > 0) && (right >= left_backup))
            right--;
        if (left <= right)
        {
            temp_for_swap = lines[left];
            lines[left] = lines[right];
            lines[right] = temp_for_swap;
            left++;
            right--;
        }
    }
    if (right > left_backup)
        sort_lines(lines, left_backup, right, compare);
    if (left < right_backup)
        sort_lines(lines, left, right_backup, compare);
    return 0;
}

void make_sorted_buffer(String* lines, int lines_len, char* sorted_buffer)
{
    int buf_position = 0;
    for (int i = 0; i < lines_len; ++i)
    {
        char* line = lines[i].str;
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
