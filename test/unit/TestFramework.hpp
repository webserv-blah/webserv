#ifndef TEST_FRAMEWORK_HPP
#define TEST_FRAMEWORK_HPP

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

class TestCase {
private:
    std::string name_;
    void (*testFunction_)();
    
public:
    TestCase(const std::string& name, void (*testFunction)())
        : name_(name), testFunction_(testFunction) {}
    
    const std::string& getName() const { return name_; }
    
    bool run() {
        try {
            testFunction_();
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Exception in test [" << name_ << "]: " << e.what() << std::endl;
            return false;
        } catch (...) {
            std::cerr << "Unknown exception in test [" << name_ << "]" << std::endl;
            return false;
        }
    }
};

class TestSuite {
private:
    std::string name_;
    std::vector<TestCase> tests_;
    
public:
    TestSuite(const std::string& name) : name_(name) {}
    
    void addTest(const TestCase& test) {
        tests_.push_back(test);
    }
    
    int run() {
        std::cout << "\n=== Running Test Suite: " << name_ << " ===" << std::endl;
        
        int passed = 0;
        int failed = 0;
        
        for (size_t i = 0; i < tests_.size(); ++i) {
            std::cout << "Running test [" << tests_[i].getName() << "]... ";
            if (tests_[i].run()) {
                std::cout << "PASSED" << std::endl;
                ++passed;
            } else {
                std::cout << "FAILED" << std::endl;
                ++failed;
            }
        }
        
        std::cout << "\nResults: " << passed << " passed, " << failed << " failed" << std::endl;
        return failed;
    }
};

class TestRegistry {
private:
    std::vector<TestSuite*> suites_;
    
    TestRegistry() {}
    
public:
    static TestRegistry& getInstance() {
        static TestRegistry instance;
        return instance;
    }
    
    void addSuite(TestSuite& suite) {
        suites_.push_back(&suite);
    }
    
    int runAll() {
        int totalFailed = 0;
        
        for (size_t i = 0; i < suites_.size(); ++i) {
            totalFailed += suites_[i]->run();
        }
        
        std::cout << "\n=== Test Summary ===" << std::endl;
        if (totalFailed == 0) {
            std::cout << "All tests PASSED!" << std::endl;
        } else {
            std::cout << totalFailed << " tests FAILED!" << std::endl;
        }
        
        return totalFailed == 0 ? 0 : 1;
    }
};

// Assertion macros
#define ASSERT_TRUE(condition) \
    if (!(condition)) { \
        std::cerr << "Assertion failed: " << #condition << " is not true" << std::endl; \
        std::cerr << "  at " << __FILE__ << ":" << __LINE__ << std::endl; \
        throw std::runtime_error("Assertion failed"); \
    }

#define ASSERT_FALSE(condition) \
    if (condition) { \
        std::cerr << "Assertion failed: " << #condition << " is not false" << std::endl; \
        std::cerr << "  at " << __FILE__ << ":" << __LINE__ << std::endl; \
        throw std::runtime_error("Assertion failed"); \
    }

#define ASSERT_EQ(expected, actual) \
    if (!((expected) == (actual))) { \
        std::cerr << "Assertion failed: " << #expected << " == " << #actual << std::endl; \
        std::cerr << "  Expected: " << (expected) << std::endl; \
        std::cerr << "  Actual: " << (actual) << std::endl; \
        std::cerr << "  at " << __FILE__ << ":" << __LINE__ << std::endl; \
        throw std::runtime_error("Assertion failed"); \
    }

#define ASSERT_NE(expected, actual) \
    if ((expected) == (actual)) { \
        std::cerr << "Assertion failed: " << #expected << " != " << #actual << std::endl; \
        std::cerr << "  Value: " << (expected) << std::endl; \
        std::cerr << "  at " << __FILE__ << ":" << __LINE__ << std::endl; \
        throw std::runtime_error("Assertion failed"); \
    }

#define TEST_CASE(name, suite_name) \
    void test_##name(); \
    namespace { \
        class TestRegistration_##name { \
        public: \
            TestRegistration_##name() { \
                TestSuite& suite = suite_##suite_name(); \
                suite.addTest(TestCase(#name, test_##name)); \
            } \
        }; \
        TestRegistration_##name testRegistration_##name##_instance; \
    } \
    void test_##name()

#define TEST_SUITE(name) \
    TestSuite& suite_##name() { \
        static TestSuite suite(#name); \
        static bool registered = false; \
        if (!registered) { \
            TestRegistry::getInstance().addSuite(suite); \
            registered = true; \
        } \
        return suite; \
    } \
    namespace { \
        struct SuiteInitializer_##name { \
            SuiteInitializer_##name() { \
                suite_##name(); \
            } \
        }; \
        SuiteInitializer_##name g_suiteInitializer_##name; \
    }

#define RUN_ALL_TESTS() \
    TestRegistry::getInstance().runAll()

#endif // TEST_FRAMEWORK_HPP