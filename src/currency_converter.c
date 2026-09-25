/*
 * Currency Converter in C
 *
 * Author: Toufique Ahmed
 * Academic context: Introduction to Programming, first semester, 2022
 * Institution: Quaid-e-Awam University of Engineering, Science and
 *              Technology (QUEST), Nawabshah
 *
 * This is a cleaned and portable edition of the original academic project.
 * It uses fixed exchange rates retained from the original implementation and
 * must not be used for real financial decisions.
 */

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CURRENCY_COUNT 11
#define INPUT_SIZE 128

typedef struct {
    const char *code;
    const char *name;
} Currency;

static const Currency CURRENCIES[CURRENCY_COUNT] = {
    {"PKR", "Pakistani Rupee"},
    {"INR", "Indian Rupee"},
    {"SAR", "Saudi Riyal"},
    {"USD", "United States Dollar"},
    {"EUR", "Euro"},
    {"CNY", "Chinese Yuan"},
    {"MYR", "Malaysian Ringgit"},
    {"AUD", "Australian Dollar"},
    {"BDT", "Bangladeshi Taka"},
    {"GBP", "Pound Sterling"},
    {"CAD", "Canadian Dollar"}
};

/*
 * Fixed educational exchange rates from the original 2022 project.
 * RATES[source][target] gives the value of one source unit in target units.
 */
static const double RATES[CURRENCY_COUNT][CURRENCY_COUNT] = {
    /*          PKR      INR      SAR      USD      EUR      CNY      MYR      AUD      BDT      GBP      CAD */
    /* PKR */ {1.0,     0.30,    0.014,   0.0036,  0.0034,  0.025,   0.016,   0.0055,  0.38,    0.0031,  0.0050},
    /* INR */ {3.37,    1.0,     0.046,   0.012,   0.012,   0.085,   0.055,   0.019,   1.28,    0.010,   0.017},
    /* SAR */ {73.59,   21.84,   1.0,     0.27,    0.25,    1.85,    1.21,    0.40,    28.04,   0.22,    0.37},
    /* USD */ {276.30,  81.98,   3.75,    1.0,     0.95,    6.95,    4.52,    1.52,    105.26,  0.84,    1.38},
    /* EUR */ {291.43,  86.47,   3.96,    1.05,    1.0,     7.33,    4.77,    1.60,    111.05,  0.89,    1.46},
    /* CNY */ {40.03,   11.78,   0.54,    0.14,    0.14,    1.0,     0.65,    0.22,    15.14,   0.12,    0.20},
    /* MYR */ {61.50,   18.12,   0.83,    0.22,    0.21,    1.54,    1.0,     0.34,    23.26,   0.19,    0.30},
    /* AUD */ {183.34,  53.95,   2.47,    0.66,    0.62,    4.58,    2.98,    1.0,     69.36,   0.56,    0.91},
    /* BDT */ {2.64,    0.78,    0.036,   0.0095,  0.0090,  0.066,   0.043,   0.014,   1.0,     0.0080,  0.013},
    /* GBP */ {329.44,  97.06,   4.45,    1.18,    1.12,    8.23,    5.36,    1.80,    124.63,  1.0,     1.63},
    /* CAD */ {201.62,  59.40,   2.72,    0.72,    0.69,    5.04,    3.28,    1.10,    76.27,   0.61,    1.0}
};

static bool read_line(char *buffer, size_t size);
static bool contains_only_whitespace(const char *text);
static bool parse_integer(const char *text, int *value);
static bool parse_positive_amount(const char *text, double *value);
static bool prompt_for_choice(const char *prompt, int minimum, int maximum,
                              int *choice);
static bool prompt_for_amount(const char *prompt, double *amount);
static bool prompt_to_continue(bool *should_continue);
static void print_currency_menu(void);
static void save_conversion(int source, int target, double amount,
                            double result, double rate);

int main(void)
{
    bool should_continue = true;

    puts("============================================================");
    puts("                  CURRENCY CONVERTER");
    puts("============================================================");
    puts("Educational demonstration using fixed exchange rates.");

    while (should_continue) {
        int source_choice;
        int target_choice;
        double amount;
        double result;
        double rate;

        print_currency_menu();

        if (!prompt_for_choice("Select the source currency (1-11): ",
                               1, CURRENCY_COUNT, &source_choice)) {
            puts("\nInput ended. Exiting the program.");
            break;
        }

        do {
            if (!prompt_for_choice("Select the target currency (1-11): ",
                                   1, CURRENCY_COUNT, &target_choice)) {
                puts("\nInput ended. Exiting the program.");
                return EXIT_SUCCESS;
            }

            if (target_choice == source_choice) {
                puts("Source and target currencies must be different.");
            }
        } while (target_choice == source_choice);

        if (!prompt_for_amount("Enter a positive amount: ", &amount)) {
            puts("\nInput ended. Exiting the program.");
            break;
        }

        --source_choice;
        --target_choice;

        rate = RATES[source_choice][target_choice];
        result = amount * rate;

        if (!isfinite(result)) {
            puts("The amount is too large to convert safely.");
        } else {
            puts("\nConversion result");
            puts("-----------------");
            printf("%.2f %s = %.2f %s\n",
                   amount, CURRENCIES[source_choice].code,
                   result, CURRENCIES[target_choice].code);
            printf("Rate used: 1 %s = %.6f %s\n",
                   CURRENCIES[source_choice].code, rate,
                   CURRENCIES[target_choice].code);

            save_conversion(source_choice, target_choice, amount, result, rate);
        }

        if (!prompt_to_continue(&should_continue)) {
            puts("\nInput ended. Exiting the program.");
            break;
        }
    }

    puts("\nThank you for using the Currency Converter.");
    return EXIT_SUCCESS;
}

static bool read_line(char *buffer, size_t size)
{
    size_t newline_position;

    if (fgets(buffer, (int)size, stdin) == NULL) {
        return false;
    }

    newline_position = strcspn(buffer, "\n");
    if (buffer[newline_position] == '\n') {
        buffer[newline_position] = '\0';
    } else {
        int character;

        while ((character = getchar()) != '\n' && character != EOF) {
            /* Discard characters that did not fit in the input buffer. */
        }
    }

    return true;
}

static bool contains_only_whitespace(const char *text)
{
    while (*text != '\0') {
        if (!isspace((unsigned char)*text)) {
            return false;
        }
        ++text;
    }

    return true;
}

static bool parse_integer(const char *text, int *value)
{
    char *end;
    long parsed_value;

    errno = 0;
    parsed_value = strtol(text, &end, 10);

    if (text == end || errno == ERANGE || parsed_value < INT_MIN ||
        parsed_value > INT_MAX || !contains_only_whitespace(end)) {
        return false;
    }

    *value = (int)parsed_value;
    return true;
}

static bool parse_positive_amount(const char *text, double *value)
{
    char *end;
    double parsed_value;

    errno = 0;
    parsed_value = strtod(text, &end);

    if (text == end || errno == ERANGE || !isfinite(parsed_value) ||
        parsed_value <= 0.0 || !contains_only_whitespace(end)) {
        return false;
    }

    *value = parsed_value;
    return true;
}

static bool prompt_for_choice(const char *prompt, int minimum, int maximum,
                              int *choice)
{
    char input[INPUT_SIZE];
    int parsed_choice;

    for (;;) {
        printf("%s", prompt);
        fflush(stdout);

        if (!read_line(input, sizeof input)) {
            return false;
        }

        if (parse_integer(input, &parsed_choice) &&
            parsed_choice >= minimum && parsed_choice <= maximum) {
            *choice = parsed_choice;
            return true;
        }

        printf("Invalid choice. Enter a number from %d to %d.\n",
               minimum, maximum);
    }
}

static bool prompt_for_amount(const char *prompt, double *amount)
{
    char input[INPUT_SIZE];

    for (;;) {
        printf("%s", prompt);
        fflush(stdout);

        if (!read_line(input, sizeof input)) {
            return false;
        }

        if (parse_positive_amount(input, amount)) {
            return true;
        }

        puts("Invalid amount. Enter a number greater than zero.");
    }
}

static bool prompt_to_continue(bool *should_continue)
{
    char input[INPUT_SIZE];
    char answer;

    for (;;) {
        printf("\nPerform another conversion? (y/n): ");
        fflush(stdout);

        if (!read_line(input, sizeof input)) {
            return false;
        }

        if (sscanf(input, " %c", &answer) == 1) {
            answer = (char)tolower((unsigned char)answer);

            if (answer == 'y' || answer == 'n') {
                *should_continue = answer == 'y';
                return true;
            }
        }

        puts("Invalid response. Enter y for yes or n for no.");
    }
}

static void print_currency_menu(void)
{
    int index;

    puts("\nAvailable currencies");
    puts("--------------------");

    for (index = 0; index < CURRENCY_COUNT; ++index) {
        printf("%2d. %-3s - %s\n",
               index + 1, CURRENCIES[index].code, CURRENCIES[index].name);
    }

    putchar('\n');
}

static void save_conversion(int source, int target, double amount,
                            double result, double rate)
{
    FILE *history;
    time_t current_time;
    struct tm *local_time;
    char timestamp[20] = "Unknown time";

    history = fopen("History.txt", "a");
    if (history == NULL) {
        fputs("Warning: the conversion history could not be saved.\n", stderr);
        return;
    }

    current_time = time(NULL);
    local_time = localtime(&current_time);
    if (local_time != NULL) {
        (void)strftime(timestamp, sizeof timestamp, "%Y-%m-%d %H:%M:%S",
                       local_time);
    }

    fprintf(history,
            "%s | %.2f %s -> %.2f %s | rate: %.6f\n",
            timestamp,
            amount, CURRENCIES[source].code,
            result, CURRENCIES[target].code,
            rate);

    if (fclose(history) != 0) {
        fputs("Warning: the conversion history may not have been saved fully.\n",
              stderr);
    }
}
