#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define DEBUG 0

#define ARRAY_SIZE(X) (sizeof(X)/sizeof(X[0]))
#define uint unsigned int

#define ERROR_INVALID_LENGTH -1

#define INDEX_DAC 3

void debug(const char *format, ...) {
#if DEBUG
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
#endif
}

uint from_ascii(char digit) {
    return digit - '0';
}

uint barrel_roll_inc(uint number, uint max) {
    return ((number + 1) % max);
}

uint compute_sliding_acc_product(uint *numbers, uint numbers_length, uint *sequence, uint sequence_length) {
    int index_numbers;
    int index_sequence;
    uint acc_sum = 0;

    index_numbers = numbers_length - 1;
    index_sequence = 0;
    while(index_numbers >= 0) {
        uint prod = numbers[index_numbers] * sequence[index_sequence];
        acc_sum += prod;

        debug("(%d, %d) %d x %d = %d \t sum = %d \n", index_numbers, index_sequence, numbers[index_numbers], sequence[index_sequence], prod, acc_sum);

        index_numbers--;
        index_sequence = barrel_roll_inc(index_sequence, sequence_length);
    }

    return acc_sum;
}

void char_array_to_uint_array(char *digits, uint digitsLength, uint *output) {
    for(int i = 0; i < digitsLength; i++) {
        output[i] = from_ascii(digits[i]);
    }
}

uint compute_dac11(char *boleto, uint len) {
    static uint dac11_sequence[] = {2, 3, 4, 5, 6, 7, 8, 9};
    uint *buffer = calloc(len, sizeof(uint));
    char_array_to_uint_array(boleto, len, buffer);
    
    uint acc_sum = compute_sliding_acc_product(buffer, len, dac11_sequence, ARRAY_SIZE(dac11_sequence));
    uint mod11 = acc_sum % 11;

    debug("%d mod 11 = %d \n", acc_sum, mod11);
    uint result = (mod11 <= 1) ? 0 : (mod11 == 10) ? 1 : mod11;

    free(buffer);

    return result;
}

uint compute_dac10(char *boleto, uint len) {
    static uint dac10_sequence[] = {1, 2};
    uint *buffer = calloc(len, sizeof(uint));
    char_array_to_uint_array(boleto, len, buffer);

    uint acc_sum = compute_sliding_acc_product(buffer, len, dac10_sequence, ARRAY_SIZE(dac10_sequence));
    uint result = acc_sum % 10;
    free(buffer);

    return result;
}

uint get_expected_dac(char *boleto, uint len) {
    if (len <= INDEX_DAC) return ERROR_INVALID_LENGTH;

    return from_ascii(boleto[INDEX_DAC]);
}

uint compute_dac(char *boleto, uint len) {
    switch (len + 1) {
        case 48: return compute_dac11(boleto, len);
        default: return compute_dac10(boleto, len);
    }
}

char* create_copy_discarding_dac_digit(char *boleto, uint boleto_length) {
    char *result = calloc(boleto_length - 1, sizeof(char));
    memcpy(result, boleto, INDEX_DAC);
    strncpy(&result[INDEX_DAC], &boleto[INDEX_DAC + 1], boleto_length - INDEX_DAC - 1);

    return result;
}

int main () {
    char *boletoFull = "846700000017020400820898992824091613518251262995";
    printf("Checking following code: %s (%lu digits) \n", boletoFull, strlen(boletoFull));
    uint expected_dac = get_expected_dac(boletoFull, strlen(boletoFull));

    char *boletoWithoutDAC = create_copy_discarding_dac_digit(boletoFull, strlen(boletoFull));
    printf("Without DAC: %s (%lu digits) \n", boletoWithoutDAC, strlen(boletoWithoutDAC));
    uint computed_dac = compute_dac(boletoWithoutDAC, strlen(boletoWithoutDAC));
    free(boletoWithoutDAC);

    printf("Expected DAC: %u \n", expected_dac);
    printf("Computed DAC: %u \n", computed_dac);

    return 0;
}