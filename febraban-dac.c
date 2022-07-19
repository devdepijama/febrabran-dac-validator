#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define DEBUG 1

#define ARRAY_SIZE(X) (sizeof(X)/sizeof(X[0]))
#define uint unsigned int

#define ERROR_INVALID_LENGTH -1
#define ERROR_WRONG_USAGE -2

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

        //debug("(%d, %d) ", index_numbers, index_sequence);
        debug("%d x %d = %d \t sum = %d \n", numbers[index_numbers], sequence[index_sequence], prod, acc_sum);

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
    uint result = 0;
    
    if (mod11 <= 1) {
        debug("%d <= 1, so result = 0 \n", mod11);
        result = 0;
    } else if (mod11 == 10) {
        debug("%d == 10, so result = 1 \n", mod11);
        result = 1;
    } else {
        debug("result = %d \n", mod11);
        result = mod11;
    }

    free(buffer);

    return result;
}

uint compute_dac10(char *boleto, uint len) {
    static uint dac10_sequence[] = {2, 1};
    uint *buffer = calloc(len, sizeof(uint));
    char_array_to_uint_array(boleto, len, buffer);

    uint acc_sum = compute_sliding_acc_product(buffer, len, dac10_sequence, ARRAY_SIZE(dac10_sequence));
    uint mod10 = acc_sum % 10;
    debug("%d mod 10 = %d \n", acc_sum, mod10);

    free(buffer);

    uint result = 0;
    if (mod10 == 0) {
       result = 0; 
       debug("%d == 0, so result = %d \n", mod10, result);
    } else {
        result = 10 - mod10;
        debug("%d != 0, so result = (10 - %d) => %d \n", mod10, mod10, result);
    }

    return result;
}

uint get_expected_dac(char *boleto, uint len) {
    if (len <= INDEX_DAC) return ERROR_INVALID_LENGTH;

    return from_ascii(boleto[INDEX_DAC]);
}

uint compute_dac(char *boleto, uint len) {
    switch (len + 1) {
        case 48: 
            printf("Using DAC11 algorithm... \n");
            return compute_dac11(boleto, len);

        default: 
            printf("Using DAC10 algorithm... \n");
            return compute_dac10(boleto, len);
    }
}

char* create_copy_discarding_dac_digit(char *boleto, uint boleto_length) {
    char *result = calloc(boleto_length - 1, sizeof(char));
    memcpy(result, boleto, INDEX_DAC);
    strncpy(&result[INDEX_DAC], &boleto[INDEX_DAC + 1], boleto_length - INDEX_DAC - 1);

    return result;
}

void print_usage(char *execName) {
    printf("Incorrect parameters! \n");
    printf("Usage : %s <BOLETO> \n", execName);
}

int main (int argc, char *argv[]) {

    if (argc < 2){
        print_usage(argv[0]);
        return ERROR_WRONG_USAGE;
    }

    char *boletoFull = argv[1];
    printf("Checking following code: %s (%lu digits) \n", boletoFull, strlen(boletoFull));
    uint expected_dac = get_expected_dac(boletoFull, strlen(boletoFull));

    char *boletoWithoutDAC = create_copy_discarding_dac_digit(boletoFull, strlen(boletoFull));
    debug("Without DAC: %s (%lu digits) \n", boletoWithoutDAC, strlen(boletoWithoutDAC));
    uint computed_dac = compute_dac(boletoWithoutDAC, strlen(boletoWithoutDAC));
    free(boletoWithoutDAC);

    debug("Expected DAC: %u \n", expected_dac);
    debug("Computed DAC: %u \n", computed_dac);

    if (expected_dac == computed_dac) {
        printf("DAC is valid! (%d) \n", expected_dac);
    } else {
        printf("DAC is invalid. Expected %d but got %d instead \n", expected_dac, computed_dac);
    }

    return 0;
}