#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <unistd.h>
#endif

#define NEWLINE_CHARACTER 10
#define COMMA_CHARACTER 44
#define ASCII_MAX 127

typedef long long int bigint;

enum CsvHeaderType {
  CSV_TEXT,
  CSV_NUMBER,
  CSV_DATE,
  CSV_TIME,
  CSV_DATETIME,
  CSV_BOOLEAN
}
typedef CsvHeaderType;

enum {
  NEW,  
  PROCESSING,
  SUCCESS,
  FAILED
} typedef CsvProcessingStatus;

enum {
  ERROR_NONE,
  ERROR_NON_ASCII_ENCODING,
  ERROR_OUT_OF_MEMORY,
  ERROR_NON_CSV_FILE,
  ERROR_FILEPATH_NOT_SPECIFIED,
  ERROR_FILEPATH_LENGTH_EXCEEDED,
  ERROR_FILE_DIRECTORY_NOT_EXISTS,
  ERROR_FILE_READ_PERMISSION,
  ERROR_FILE_LOCKED,
  ERROR_FILE_OPEN_FAILED,
  ERROR_UNKNOWN_PLATFORM
} typedef ErrorCode;

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
  bigint totalRows;
  bigint totalColumns;
}
typedef Csv;

struct CsvContextError {
  ErrorCode code;
  char *message;
} typedef CsvContextError;

struct CsvContext {
    Csv *csv;
    FILE *csvStream;
    CsvProcessingStatus status;
    CsvContextError error;
} typedef CsvContext;

CsvContext _csvContext;

bool endsWith(const char * template, const char * suffix) {
  bool result = false;
  if (template && suffix) {
    int templateLength = strlen(template);
    int suffixLength = strlen(suffix);
    result = templateLength >= suffixLength && strcmp(template + templateLength - suffixLength, suffix) == 0;
  }
  return result;
}

char *trimQuote(char *text) {
  if (text) {
    int n = strlen(text);
    if (text[0] == '"' && text[n - 1] == '"') {
      for(int i = 0; i < n; i++) {
        if (i == n - 2) {
          text[i] = '\0';
          break;
        }
        text[i] = text[i + 1];
      }
    }
  }
  return text; 
}

void _resetContext() {
  #pragma region CloseFileStream
  if (_csvContext.csvStream) {
    fclose(_csvContext.csvStream);
    _csvContext.csvStream = NULL;
  }
  #pragma endregion CloseFileStream
  if(_csvContext.csv) {
    if(_csvContext.csv->columns){
      for(int colIndex = 0; colIndex < _csvContext.csv->totalColumns; colIndex++) {
        #pragma region DellocateColumnHeader
        if(_csvContext.csv->columns[colIndex].header.name) {
          if (_csvContext.status == FAILED) { 
            free(_csvContext.csv->columns[colIndex].header.name);
          }
          _csvContext.csv->columns[colIndex].header.name = NULL;
        }
        #pragma endregion DellocateColumnHeader
        #pragma region DeallocateCellsUnderColumn
        if (_csvContext.csv->columns[colIndex].values)
        {
          for (int rowIndex = 0; rowIndex < _csvContext.csv->totalRows; rowIndex++) {
            if (_csvContext.csv->columns[colIndex].values[rowIndex]) {
              if (_csvContext.status == FAILED) {
                free(_csvContext.csv->columns[colIndex].values[rowIndex]);
              }
              _csvContext.csv->columns[colIndex].values[rowIndex] = NULL;
            }
          }
          if (_csvContext.status == FAILED) {
            free(_csvContext.csv->columns[colIndex].values);
          }
          _csvContext.csv->columns[colIndex].values = NULL;
        }
        #pragma endregion DeallocateCellsUnderColumn
      }
      if (_csvContext.status == FAILED) {
        free(_csvContext.csv->columns);
      }
      _csvContext.csv->columns = NULL;
    }
    if (_csvContext.status == FAILED) {
      free(_csvContext.csv);
    }
    _csvContext.csv = NULL;
  }
  _csvContext.status = NEW;
  _csvContext.error.code = ERROR_NONE;
  _csvContext.error.message = NULL;
}

void _setContextError(int errorCode) {
  _csvContext.error.code = errorCode;
  switch (_csvContext.error.code)
  {
    case ERROR_NONE:
      _csvContext.error.message = NULL;
      break;
    case ERROR_NON_ASCII_ENCODING:
      _csvContext.error.message = "Error: Only ASCII character encoding is supported.";
      break;
    case ERROR_OUT_OF_MEMORY:
      _csvContext.error.message = "Error: Memory allocation failed.";
      break;
    case ERROR_NON_CSV_FILE:
      _csvContext.error.message = "Error: Only files with extension of '.csv' is supported.";
      break;
    case ERROR_FILEPATH_NOT_SPECIFIED:
      _csvContext.error.message = "Error: File path is not specified.";
      break;
    case ERROR_FILEPATH_LENGTH_EXCEEDED:
      _csvContext.error.message = "Error: File path length exceeded.";
      break;
    case ERROR_FILE_DIRECTORY_NOT_EXISTS:
      _csvContext.error.message = "Error: File or directory does not exist.";
      break;
    case ERROR_FILE_READ_PERMISSION:
      _csvContext.error.message = "Error: File does not have read permission.";
      break;
    case ERROR_FILE_LOCKED:
      _csvContext.error.message = "Error: File is locked by another process.";
      break;
    case ERROR_FILE_OPEN_FAILED:
      _csvContext.error.message = "Error: File could not be opened.";
      break;
    case ERROR_UNKNOWN_PLATFORM:
      _csvContext.error.message = "Error: Only Windows, Linux, macOS platforms are supported.";
      break;
  }
  if (_csvContext.error.code != ERROR_NONE) {
    printf("%s", _csvContext.error.message);
    _csvContext.status = FAILED;
  }
}

bool _contextHasError() {
  return _csvContext.error.code != ERROR_NONE;
}

bool _parseHeaders() {
  int headerLength = 0;
  char *data = NULL;         
  while (true) {
    int code = fgetc(_csvContext.csvStream);
    if (code > ASCII_MAX)
    {
      _csvContext.status = FAILED;
    }
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
      _csvContext.csv -> columns[_csvContext.csv -> totalColumns].header.type = CSV_TEXT;
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
            } 
            else {
              _csvContext.status = FAILED;
              }
          }
          headerLength++;
      }
  }
}

bool _parseCells() {
  int code, valueCount = 0, valueLength = 0, index;
  char * value = NULL, *data = NULL;
  while (true) {
    code = fgetc(_csvContext.csvStream);
      bool hasStartedWithDoubleQuote = data && data[0] == '"';
      bool enclosedWithinDoubleQuote = hasStartedWithDoubleQuote && data[valueLength - 1] == '"' && valueLength > 1;
      if (enclosedWithinDoubleQuote || (!hasStartedWithDoubleQuote && (code == COMMA_CHARACTER || code == NEWLINE_CHARACTER || feof(_csvContext.csvStream)))) {
        data = (char *) realloc(data, (valueLength + 1) * sizeof(char));
        data[valueLength] = '\0';
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
          strcpy(values[rowIndex], trimQuote(data));
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
}

Csv *parseCsv(const char * filePath) {
  Csv *result = NULL;
  if (filePath) {
    if (endsWith(filePath, ".csv")) {
      #if defined(_WIN32)
      if (strlen(filePath) > MAX_PATH) {
        _setContextError(ERROR_FILEPATH_LENGTH_EXCEEDED);
      }
      else {
        DWORD fileAttributes = GetFileAttributes(filePath);
        if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
          _setContextError(ERROR_FILE_DIRECTORY_NOT_EXISTS);
        }
        else {
          HANDLE fileHandler = CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
          if (fileHandler == INVALID_HANDLE_VALUE) {
            _csvContext.status = FAILED;
            DWORD winError = GetLastError();
            switch (winError) {
              case ERROR_SHARING_VIOLATION:
              case ERROR_LOCK_VIOLATION:
                _setContextError(ERROR_FILE_LOCKED);
                break;
              default:
                _setContextError(ERROR_FILE_READ_PERMISSION);
            }
          }
          CloseHandle(fileHandler);
        }
      }
      #elif defined(__linux__) || defined(__APPLE__)
      long maxPathLength = sysconf(_PC_PATH_MAX);
      if ((maxPathLength == -1 && strlen(filePath) > MAX_PATH) || (strlen(filePath) > maxPathLength)) {
        _setContextError(ERROR_FILEPATH_LENGTH_EXCEEDED);
      }
      else if (access(filePath, F_OK) == -1) {
        _setContextError(ERROR_FILE_DIRECTORY_NOT_EXISTS);
      }
      else if (!access(filePath, R_OK)) {
        _setContextError(ERROR_FILE_READ_ACCESS_DENIED);
      }
      #else
      _setContextError(ERROR_UNKNOWN_PLATFORM);
      #endif

      if (!_contextHasError()) {
        _csvContext.csvStream = fopen(filePath, "r");
        if (_csvContext.csvStream) {
          _csvContext.csv = (Csv *) malloc(sizeof(Csv));
          if (_csvContext.csv) {
            _csvContext.csv->totalRows = 0;
            _csvContext.csv->totalColumns = 0;
            _csvContext.status = PROCESSING;
            if (_parseHeaders() && _parseCells()) {
              result = _csvContext.csv;
              _csvContext.status = SUCCESS;
            }
          }
          else {
            _setContextError(ERROR_OUT_OF_MEMORY);
          }
        }
        else {
          _setContextError(ERROR_FILE_OPEN_FAILED);
        }
      }
    }
    else {
      _setContextError(ERROR_NON_CSV_FILE);
    }
  }
  else {
    _setContextError(ERROR_FILEPATH_NOT_SPECIFIED);
  }
  _resetContext();
  return result;
}