file(READ ${input} content HEX)
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," bytes ${content})
file(WRITE ${output} "#pragma once
inline constexpr unsigned char ${name}[] = {${bytes}};
")
