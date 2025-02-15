#ifndef OPTIONAL_HPP
# define OPTIONAL_HPP

template<typename T>
class Optional {
public:
    Optional() : isSet_(false) {}
    Optional(const T& value) : value_(value), isSet_(true) {}

    bool isSet() const { return isSet_; }
    const T& value() const { return value_; }
    void setValue(const T& value) { value_ = value; isSet_ = true; }

    // Allow assignment from another Optional<T>
    Optional<T>& operator=(const Optional<T>& other) {
        if (this != &other) {
            value_ = other.value_;
            isSet_ = other.isSet_;
        }
        return *this;
    }
private:
    T       value_;
    bool    isSet_;
};

#endif