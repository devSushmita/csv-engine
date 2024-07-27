#define PARSE_CSV_IN_DEBUG_MODE
#include <stdio.h>
#include "csv.h"

int main() {
    const char *filePath = "./tests/sample1.csv";
    Csv* result = parseCsv(filePath);
    return 0;
}