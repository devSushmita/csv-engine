#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define NEWLINE_CHARACTER 10
#define COMMA_CHARACTER 44

typedef long long int bigint;

enum CsvHeaderType {
  TEXT,
  INT,
  FLOAT,
  DATE,
  TIME,
  DATETIME,
  BOOLEAN
}
typedef CsvHeaderType;

enum Encoding {
  ASCII,
  UTF8,
  ISO8859_1,
  WINDOWS1252
}
typedef Encoding;

struct CsvHeader {
  char * name;
  CsvHeaderType type;
}
typedef CsvHeader;

struct CsvColumn {
  CsvHeader header;
  char ** values;
}
typedef CsvColumn;

struct Csv {
  CsvColumn * columns;
  char * fileName;
  char * filePath;
  Encoding encoding;
  bigint totalRows;
  bigint totalColumns;
}
typedef Csv;

bool endsWith(const char * template, const char * suffix) {
  int templateLength = strlen(template);
  int suffixLength = strlen(suffix);
  return templateLength >= suffixLength && strcmp(template + templateLength - suffixLength, suffix) == 0;
}

Csv * parse(const char * filePath) {
  if (filePath) {
    if (endsWith(filePath, ".csv")) {
      FILE * fp = fopen(filePath, "r");
      if (fp) {
        Csv * csv = (Csv * ) malloc(sizeof(Csv));
        if (csv) {
          int headerLength = 0;
          char * data = NULL;
          csv -> totalRows = 0;
          csv -> totalColumns = 0;

          // abc,def,ghi\n
          // header parsing logic
          while (true) {
            int code = fgetc(fp);
            if (code == COMMA_CHARACTER || code == NEWLINE_CHARACTER) {
              data = (char * ) realloc(data, (headerLength + 1) * sizeof(char));
              data[headerLength] = '\0';
              if (csv -> totalColumns == 0) {
                csv -> columns = (CsvColumn * ) malloc(sizeof(CsvColumn));
                if (!csv -> columns) {
                  // handle error
                }
              } else {
                csv -> columns = (CsvColumn * ) realloc(data, (csv -> totalColumns + 1) * sizeof(CsvColumn));
                if (!csv -> columns) {
                  // handle error
                }
              }
              csv -> columns[csv -> totalColumns].header.name = data;
              csv -> columns[csv -> totalColumns].header.type = TEXT;
              csv -> columns[csv -> totalColumns].values = NULL;
              csv -> totalColumns++;
              headerLength = 0;
              if (code == NEWLINE_CHARACTER) {
                break;
              }
            } else {
              if (headerLength == 0) {
                data = (char * ) malloc(sizeof(char));
                if (data) {
                  data[headerLength] = (char) code;
                } else {
                  // handle error
                }
              } else {
                data = (char * ) realloc(data, (headerLength + 1) * sizeof(char));
                if (data) {
                  data[headerLength] = (char) code;
                } else {
                  // handle error
                }
              }
              headerLength++;
            }
          }

          // value parsing logic
          int code, valueCount = 0, valueLength = 0, index;
          char * value = NULL;
          while ((code = fgetc(fp)) != EOF) {
            {
              if (code == COMMA_CHARACTER || code == NEWLINE_CHARACTER) {
                value = (char * ) realloc(value, (valueLength + 1) * sizeof(char));
                if (value) {
                  value[valueLength] = '\0';
                  int colIndex = valueCount % csv -> totalColumns;
                  // store inside column's values property (csv->columns[colindex].values)
                  if (csv->totalRows == 0) {
                    char **values = (char **) malloc(sizeof(char *));
                    if (values) {
                        csv->columns[colIndex].values = values;
                    }
                    else {
                        // error handle
                    }
                  }  
                  else {
                    char **values = csv->columns[colIndex].values;
                    values = (char **) realloc(values, (csv->totalRows + 1) * sizeof(char *));
                    if (values) {
                        csv->columns[colIndex].values = values;
                    }
                    else {
                        // error handle
                    }
                  }
                  char *cellValue = (char *) malloc((valueLength + 1) * sizeof(char));
                  if (cellValue) {
                    strcpy(cellValue, value);
                    csv->columns[colIndex].values[csv->totalRows] = cellValue;
                  }
                  else {
                    // error handle
                  }
                  if (code == NEWLINE_CHARACTER) {
                    csv -> totalRows++;
                  }
                  value = NULL;
                  valueLength = 0;
                  valueCount++;
                } else {
                  // error handle
                }
              } else {
                if (valueLength == 0) {
                  value = (char * ) malloc(sizeof(char));
                  if (value) {
                    value[valueLength] = (char) code;
                  } else {
                    // handle error
                  }
                } else {
                  value = (char * ) realloc(value, (valueLength + 1) * sizeof(char));
                  if (value) {
                    value[valueLength] = (char) code;
                  } else {
                    // handle error
                  }
                }
              }
            }
          }
        } else {
          // error handling
        }
      } else {
        // error handling
      }
    } else {
      // error handling
    }
  } else {
    // error handling
  }
}