#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

typedef long long int bigint;

enum CsvHeaderType { 
    TEXT,
    INT,
    FLOAT,
    DATE,
    TIME,
    DATETIME,
    BOOLEAN
} typedef CsvHeaderType;

enum Encoding {  
    ASCII,
    UTF8,
    ISO8859_1,
    WINDOWS1252
} typedef Encoding;

struct CsvHeader {
    char *name;
    CsvHeaderType type;
} typedef CsvHeader;

struct CsvColumn {
    CsvHeader header;
    char **values;
} typedef CsvColumn;

struct Csv {    
    CsvColumn *columns;
    char *fileName;
    char *filePath;
    Encoding encoding;
    bigint totalRows;
    bigint totalColumns;
} typedef Csv;

bool endsWith(char *template, char *suffix) {    
    int templateLength = strlen(template);
    int suffixLength = strlen(suffix);
    return templateLength >= suffixLength && strcmp(template + templateLength - suffixLength, suffix) == 0;
}

Csv *parse(const char *filePath) {
    if(filePath) {
        if (endsWith(filePath, ".csv")) {
            FILE *fp = fopen(filePath, "r");
            if (fp) {
                Csv *csv = (Csv *) malloc(sizeof(Csv));
                if (csv)
                {   
                    while(getc(fp) != ','){    
                        
                    }
                }
                else {   
                    // error handling
                }
            }
            else {  
                // error handling
            }
        }
        else {  
            // error handling
        }
    }
    else {
        // error handling
    }
}
    