#include "radio_handler.h"
#include "error.h"
#include "serialize.h"
#include <stdio.h>
#include <stdlib.h>

#define COLOR_GREEN "\x1B[32m"
#define COLOR_RED   "\x1B[31m"
#define COLOR_NONE  "\x1B[0m"

static int total_tests = 0;
static int failing_tests = 0;
#define UNIT_TEST(expected_result, description)                                 \
    if( (expected_result) ) {                                                   \
        printf("%sTest Passed:%s %s\n", COLOR_GREEN, COLOR_NONE, description);  \
    } else {                                                                    \
        printf("%sTest Failed:%s %s\n", COLOR_RED, COLOR_NONE, description);    \
        failing_tests++;                                                        \
    }                                                                           \
    total_tests++;

int main()
{
    //report an error
    uint8_t board_id = 7;
    enum BOARD_STATUS err_type = E_BATT_UNDER_VOLTAGE;
    uint8_t mV_high = 47;
    uint8_t mV_low = 123;
    report_error(board_id, err_type, mV_high, mV_low, 0, 0);

    //make sure we can get that error as a serialized thing
    char serialized_error_command[ERROR_COMMAND_LENGTH];
    UNIT_TEST(get_next_serialized_error(serialized_error_command),
              "serialize the error returns true");

    //one error has been reported, and one has been serialized. Attempting to
    //pull out another error should return false
    char tmp[ERROR_COMMAND_LENGTH];
    UNIT_TEST(!get_next_serialized_error(tmp),
              "serialized a second error, only one reported");

    //deserialize the first error we removed
    error_t deserialized_err;
    UNIT_TEST(deserialize_error(&deserialized_err, serialized_error_command),
              "deserialized the error");

    //make sure that all the values we put in we got out
    UNIT_TEST( deserialized_err.board_id == board_id &&
               deserialized_err.err_type == err_type &&
               deserialized_err.byte4 == mV_high &&
               deserialized_err.byte5 == mV_low, "error values aren't what we expected");

    //let's try to randomize 1000 error messages, serialize report them in batches of
    //1-10, then pull them out and compare them
    uint32_t randomized_error_count = 0;
    bool randomized_test_passed = true;
    while(randomized_error_count < 1000) {
        uint8_t num_errors = rand() % 10;
        //generate and report that many errors. Then serialize those
        //errors so we know what _should_ come out of get_next_error
        char serialized_errors[num_errors][ERROR_COMMAND_LENGTH];
        for (int i = 0; i < num_errors; ++i) {
            //randomize an error
            uint8_t board_id = rand() % 16;
            enum BOARD_STATUS error_type = rand() % 20;
            uint8_t byte4 = rand() % 256;
            uint8_t byte5 = rand() % 256;
            uint8_t byte6 = rand() % 256;
            uint8_t byte7 = rand() % 256;
            report_error(board_id, error_type, byte4, byte5, byte6, byte7);
            error_t error = {
                .board_id = board_id,
                .err_type = error_type,
                .byte4 = byte4,
                .byte5 = byte5,
                .byte6 = byte6,
                .byte7 = byte7,
            };
            serialize_error(&error, serialized_errors[i]);
        }

        //call get_next_serialized_error that many times, make sure they
        //all line up
        for (int i = 0; i < num_errors; ++i) {
            char next_error[ERROR_COMMAND_LENGTH];
            get_next_serialized_error(next_error);
            if(strcmp(next_error, serialized_errors[i]) ){
                UNIT_TEST(false, "randomized test");
                randomized_test_passed = false;
            }
        }
        randomized_error_count += num_errors;
    }
    if (randomized_test_passed) {
        UNIT_TEST(true, "randomized test");
    }


    printf("%s Test Results: %i tests, %i passed, %i %sfailed%s\n",
           __FILE__,
           total_tests,
           total_tests - failing_tests,
           failing_tests, failing_tests ? COLOR_RED : COLOR_GREEN, COLOR_NONE);
}
