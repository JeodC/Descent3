#pragma once

#include <cstdint>

int32_t CVarGetInteger(const char *name, int32_t defaultValue);
void CVarSetInteger(const char *name, int32_t value);
