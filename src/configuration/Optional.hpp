#ifndef OPTIONAL_HPP
# define OPTIONAL_HPP

// 템플릿 클래스 Optional 정의
template<typename T>
class Optional {
public:
    // 기본 생성자, isSet_을 false로 초기화
    Optional() : isSet_(false) {}

    // 값이 있는 생성자, value_와 isSet_ 초기화
    Optional(const T& value) : value_(value), isSet_(true) {}

    // 값이 설정되었는지 확인하는 함수
    bool isSet() const { return isSet_; }

    // 값을 반환하는 함수
    const T& value() const { return value_; }

    // 값을 설정하는 함수
    void setValue(const T& value) { value_ = value; isSet_ = true; }

    // 대입 연산자 오버로딩
    Optional<T>& operator=(const Optional<T>& other) {
        if (this != &other) {
            value_ = other.value_;
            isSet_ = other.isSet_;
        }
        return *this;
    }
private:
    T       value_;  // 값 저장 변수
    bool    isSet_;  // 값이 설정되었는지 여부를 저장하는 변수
};

#endif
