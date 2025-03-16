#ifndef ENUM_TO_STR_HPP
#define ENUM_TO_STR_HPP

#include <string>
#include "commonEnums.hpp"

namespace enumToStr {
    std::string EnumStatusCodeToStr(EnumStatusCode HttpStatusCode);
    std::string EnumSesStatusToStr(EnumSesStatus status);
}

#endif
