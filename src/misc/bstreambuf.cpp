#include "bstreambuf.hpp"

bstreambuf& operator<<(bstreambuf &out, char val) { out.write(val); }
bstreambuf& operator<<(bstreambuf &out, unsigned char val) { out.write(val); }
bstreambuf& operator<<(bstreambuf &out, short val) { out.write(val); }
bstreambuf& operator<<(bstreambuf &out, unsigned short val) { out.write(val); }
bstreambuf& operator<<(bstreambuf &out, int val) { out.write(val); }
bstreambuf& operator<<(bstreambuf &out, unsigned int val) { out.write(val); }
bstreambuf& operator<<(bstreambuf &out, long val) { out.write(val); }
bstreambuf& operator<<(bstreambuf &out, unsigned long val) { out.write(val); }
