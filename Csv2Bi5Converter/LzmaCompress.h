#pragma once

#include "lzma/easylzma/compress.h"  

int
simpleCompress(elzma_file_format format, const char * inPath, const char * outPath);