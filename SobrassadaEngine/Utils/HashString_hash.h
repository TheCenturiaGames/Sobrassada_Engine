#pragma once

#include <string>
#include "HashString.h"

namespace std {
   template <>
   struct hash<HashString> {
       size_t operator()(const HashString& h) const {
           return std::hash<std::string>{}(h.original);
       }
   };
}