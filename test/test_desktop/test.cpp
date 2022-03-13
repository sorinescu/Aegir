#include <stddef.h>
#include <stdint.h>
#include <CircularBufferLogger.hpp>
#include <MovingAverage.hpp>
#include <unity.h>

// void setUp(void) {
// // set stuff up here
// }

// void tearDown(void) {
// // clean stuff up here
// }

void test_CircularBufferLogger(void) {
    CircularBufferLogger<uint16_t> buf(2);

    buf.append(1);
    TEST_ASSERT_EQUAL(1, buf.at(0));
    TEST_ASSERT_EQUAL(0, buf.at(1));

    buf.append(2);
    TEST_ASSERT_EQUAL(1, buf.at(0));
    TEST_ASSERT_EQUAL(2, buf.at(1));
    TEST_ASSERT_EQUAL(0, buf.at(2));

    buf.append(3);
    TEST_ASSERT_EQUAL(2, buf.at(0));
    TEST_ASSERT_EQUAL(3, buf.at(1));
    TEST_ASSERT_EQUAL(0, buf.at(2));

    buf.append(4);
    TEST_ASSERT_EQUAL(3, buf.at(0));
    TEST_ASSERT_EQUAL(4, buf.at(1));
    TEST_ASSERT_EQUAL(0, buf.at(2));
}

void test_Uint16MovingAverage_add(void) {
    Uint16MovingAverage<2> avg;

    avg.add(4);
    avg.add(2);

    // Samples are now [4, 2]
    TEST_ASSERT_EQUAL(3, avg.avg());

    // Samples are now [2, 10]
    avg.add(10);
    TEST_ASSERT_EQUAL(6, avg.avg());
}

void test_Uint16MovingAverage_outliers_low(void) {
    // Will reject samples that deviate more than 10% from median
    Uint16MovingAverage<3> avg(10);

    avg.add(108);
    avg.add(101);
    avg.add(103);

    // Samples are now [108, 101, 103] and the median is 103.
    // Since there are no outliers, the average is (108 + 101 + 103) / 3 = 104.
    TEST_ASSERT_EQUAL(103, avg.median());
    TEST_ASSERT_EQUAL(104, avg.avg());

    // Add a small outlier "x". Since the elements will now be [101, 103, x], the new median is 101.
    // To be considered an outlier, x must be less than (101 - 10% ~= 91).
    // Let's choose 90.
    // The average should be (101 + 103) / 2 = 102.
    avg.add(90);
    TEST_ASSERT_EQUAL(101, avg.median());
    TEST_ASSERT_EQUAL(102, avg.avg());
}

void test_Uint16MovingAverage_outliers_high(void) {
    // Will reject samples that deviate more than 10% from median
    Uint16MovingAverage<3> avg(10);

    avg.add(108);
    avg.add(101);
    avg.add(103);

    // Samples are now [108, 101, 103] and the median is 103.
    // Since there are no outliers, the average is (108 + 101 + 103) / 3 = 104.
    TEST_ASSERT_EQUAL(103, avg.median());
    TEST_ASSERT_EQUAL(104, avg.avg());

    // Add a large outlier "x". Since the elements will now be [101, 103, x], the new median is 103.
    // To be considered an outlier, x must be greater than (103 + 10% ~= 113).
    // Let's choose 114.
    // The average should be (101 + 103) / 2 = 102.
    avg.add(114);
    TEST_ASSERT_EQUAL(103, avg.median());
    TEST_ASSERT_EQUAL(102, avg.avg());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_CircularBufferLogger);

    RUN_TEST(test_Uint16MovingAverage_add);
    RUN_TEST(test_Uint16MovingAverage_outliers_low);
    RUN_TEST(test_Uint16MovingAverage_outliers_high);
    
    UNITY_END();

    return 0;
}
