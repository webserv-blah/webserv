#include "TestFramework.hpp"
#include "../../src/utils/utils.hpp"
#include <string>
#include <stdexcept>

// TestSuite 선언
TEST_SUITE(Utils)

// 문자열-정수 변환 테스트
TEST_CASE(StringToInt, Utils) {
    // 정상적인 값 테스트
    ASSERT_EQ(123, utils::stoi("123"));
    ASSERT_EQ(-456, utils::stoi("-456"));
    ASSERT_EQ(0, utils::stoi("0"));
    
    // 경계값 테스트
    bool exceptionThrown = false;
    try {
        utils::stoi("2147483648"); // INT_MAX + 1
    } catch (const std::exception&) {
        exceptionThrown = true;
    }
    ASSERT_TRUE(exceptionThrown); // 어떤 종류든 예외가 발생해야 함
    
    // 잘못된 형식 테스트
    exceptionThrown = false;
    try {
        utils::stoi("abc");
    } catch (const std::exception&) {
        exceptionThrown = true;
    }
    ASSERT_TRUE(exceptionThrown); // 어떤 종류든 예외가 발생해야 함
}

// size_t 변환 테스트
TEST_CASE(StringToSizeT, Utils) {
    // 정상적인 값 테스트
    ASSERT_EQ(123u, utils::sto_size_t("123"));
    ASSERT_EQ(0u, utils::sto_size_t("0"));
    
    // 음수 테스트 (size_t는 부호 없는 값이므로 예외를 던지거나 큰 값으로 처리)
    try {
        // 음수가 (2^n - 1)로 변환되는지 또는 예외가 발생하는지 확인
        size_t result = utils::sto_size_t("-1");
        // 음수를 통과시키면 큰 값(최대값)이 되어야 함
        ASSERT_TRUE(result > 0);
    } catch (const std::exception&) {
        // 예외가 발생해도 테스트 통과
        ASSERT_TRUE(true);
    }
    
    // 잘못된 형식 테스트
    bool exceptionThrown = false;
    try {
        utils::sto_size_t("abc");
    } catch (const std::exception&) {
        exceptionThrown = true;
    }
    ASSERT_TRUE(exceptionThrown); // 어떤 종류든 예외가 발생해야 함
}

// 문자열 트림 테스트
TEST_CASE(StringTrim, Utils) {
    ASSERT_EQ("hello", utils::strtrim("hello"));
    ASSERT_EQ("hello", utils::strtrim("  hello"));
    ASSERT_EQ("hello", utils::strtrim("hello  "));
    ASSERT_EQ("hello", utils::strtrim("  hello  "));
    ASSERT_EQ("hello world", utils::strtrim("  hello world  "));
    ASSERT_EQ("", utils::strtrim(""));
    ASSERT_EQ("", utils::strtrim("  "));
}

// 숫자-문자열 변환 테스트
TEST_CASE(NumberToString, Utils) {
    ASSERT_EQ("123", utils::int_tos(123));
    ASSERT_EQ("-456", utils::int_tos(-456));
    ASSERT_EQ("0", utils::int_tos(0));
    
    ASSERT_EQ("123", utils::size_t_tos(123));
    ASSERT_EQ("0", utils::size_t_tos(0));
}

// all_of 템플릿 함수 테스트
TEST_CASE(AllOf, Utils) {
    std::string str1 = "12345";
    std::string str2 = "123a5";
    
    bool isDigits1 = utils::all_of(str1.begin(), str1.end(), ::isdigit);
    bool isDigits2 = utils::all_of(str2.begin(), str2.end(), ::isdigit);
    
    ASSERT_TRUE(isDigits1);
    ASSERT_FALSE(isDigits2);
}

// 에러 메시지 생성 테스트
TEST_CASE(ErrorMessage, Utils) {
    std::string error = webserv::errorMessage(ERROR, "Bad Request", "Invalid header", "Client");
    
    ASSERT_TRUE(error.find("[ERROR]") != std::string::npos);
    ASSERT_TRUE(error.find("Bad Request") != std::string::npos);
    ASSERT_TRUE(error.find("Invalid header") != std::string::npos);
    ASSERT_TRUE(error.find("Client") != std::string::npos);
}