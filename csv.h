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

enum {
    NEW,  
    PROCESSING,
    SUCCESS,
    FAILED
} typedef CsvProcessingStatus;

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

struct CsvContext {
    Csv *csv;
    FILE *csvStream;
    CsvProcessingStatus status;
} typedef CsvContext;

CsvContext _csvContext;

bool endsWith(const char * template, const char * suffix) {
  int templateLength = strlen(template);
  int suffixLength = strlen(suffix);
  return templateLength >= suffixLength && strcmp(template + templateLength - suffixLength, suffix) == 0;
}

Csv * parse(const char * filePath) {
  if (filePath) {
    if (endsWith(filePath, ".csv")) {
      _csvContext.csvStream = fopen(filePath, "r");
      if (_csvContext.csvStream) {
        _csvContext.csv = (Csv * ) malloc(sizeof(Csv));
        if (_csvContext.csv) {
          int headerLength = 0;
          char * data = NULL;
          _csvContext.csv-> totalRows = 0;
          _csvContext.csv-> totalColumns = 0;
          _csvContext.status = PROCESSING;

          // abc,def,ghi\n
          // header parsing logic
          while (true) {
            int code = fgetc(_csvContext.csvStream);
            if (code == COMMA_CHARACTER || code == NEWLINE_CHARACTER) {
              data = (char * ) realloc(data, (headerLength + 1) * sizeof(char));
              data[headerLength] = '\0';
              if (_csvContext.csv -> totalColumns == 0) {
                _csvContext.csv-> columns = (CsvColumn * ) malloc(sizeof(CsvColumn));
                if (_csvContext.csv-> columns) {
                _csvContext.status = FAILED;
                }
              } else {
                _csvContext.csv -> columns = (CsvColumn * ) realloc(data, (_csvContext.csv -> totalColumns + 1) * sizeof(CsvColumn));
                if (_csvContext.csv -> columns) {
                _csvContext.status = FAILED;
                }
              }
              _csvContext.csv -> columns[_csvContext.csv -> totalColumns].header.name = data;
#ifdef PARSE_CSV_IN_DEBUG_MODE
              printf("%s\n", _csvContext.csv -> columns[_csvContext.csv -> totalColumns].header.name);
#endif
              _csvContext.csv -> columns[_csvContext.csv -> totalColumns].header.type = TEXT;
              _csvContext.csv -> columns[_csvContext.csv -> totalColumns].values = NULL;
              _csvContext.csv -> totalColumns++;
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
                  _csvContext.status = FAILED;
                }
              } else {
                data = (char * ) realloc(data, (headerLength + 1) * sizeof(char));
                if (data) {
                  data[headerLength] = (char) code;
                } else {
                  _csvContext.status = FAILED;
                }
              }
              headerLength++;
            }
          }

          // value parsing logic
          int code, valueCount = 0, valueLength = 0, index;
          char * value = NULL;
          while (true) {
            code = fgetc(_csvContext.csvStream);
            if (code == COMMA_CHARACTER || feof(_csvContext.csvStream)) {
              data = (char *) realloc(data, (valueLength + 1) * sizeof(char));
              data[valueLength] = '\0';
              // printf("%s\n", data);
              int colIndex = valueCount % _csvContext.csv->totalColumns;
              int rowIndex = valueCount / _csvContext.csv->totalColumns;
              char **values = _csvContext.csv->columns[colIndex].values;
              if (rowIndex > 0) {
                values = (char **) realloc(values, (valueCount + 1) * sizeof(char *));
                if (!values) {
                  _csvContext.status = FAILED;
                }
              }
              else {
                values = (char **) malloc(sizeof(char *));
                if (!values) {
                  _csvContext.status = FAILED;
                }
              }
              values[rowIndex] = (char *) malloc(strlen(data) * sizeof(char));
              if (values[rowIndex]) {
                strcpy(values[rowIndex], data);
                _csvContext.csv->columns[colIndex].values = values;
#ifdef PARSE_CSV_IN_DEBUG_MODE
                printf("%s\n", _csvContext.csv->columns[colIndex].values[rowIndex]);
#endif
                valueCount++;
                valueLength = 0;
                data = NULL;
                if (feof(_csvContext.csvStream)) {
                  break;
                }
              }
              else {
                _csvContext.status = FAILED;
              }
            }
            else {
              if (valueLength == 0) {
                data = (char *) malloc(sizeof(char));
                if (data) {
                  data[valueLength] = (char) code;
                  valueLength++;
                }
                else {
                  _csvContext.status = FAILED;
                }
              }
              else {
                data = (char *) realloc(data, (valueLength + 1) * sizeof(char));
                if (data) {
                  data[valueLength] = (char) code;
                  valueLength++;
                }
                else {
                  _csvContext.status = FAILED;
                }
              }
            }
          }
        } else {
          _csvContext.status = FAILED;
        }
      } else {
        _csvContext.status = FAILED;
      }
    } else {
      _csvContext.status = FAILED;
    }
  } else {
    _csvContext.status = FAILED;
  }
}