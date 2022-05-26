#define MAX_CHARS 1000

#define TRUE  1
#define FALSE 0

#define TRACE_EN TRUE
#define TRACE_ERROR_EN  TRACE_EN&&  TRUE
#define TRACE_INFO_EN   TRACE_EN&&  TRUE
#define TRACE_DEBUG_EN  TRACE_EN&&  TRUE


#define GET_TRACE_MACRO(_1, _2, _3, NAME, ...) NAME

#define TRACE_START(TAG)  trace_log_header(TAG, __LINE__)

#define TRACE_INFO_START() TRACE_START("INFO")
#define TRACE_DEBUG_START() TRACE_START("DEBUG")
#define TRACE_ERROR_START() TRACE_START("ERROR")

#define TRACE_CONTENT(...) trace_log_content(__VA_ARGS__)
#define TRACE_END() trace_log_tail()

#if TRACE_INFO_EN
#define TRACE_INFO(...) TRACE_INFO_START();TRACE_CONTENT(__VA_ARGS__);TRACE_END();
//#define TRACE_INFO(msg) TRACE_INFO("%s", msg)
#else
#define TRACE_INFO(...) (0)
#endif

#if TRACE_DEBUG_EN
#define TRACE_DEBUG(...) TRACE_DEBUG_START();TRACE_CONTENT(__VA_ARGS__);TRACE_END();
#else
#define TRACE_DEBUG(...) (0)
#endif

#if TRACE_ERROR_EN
#define TRACE_ERROR(...) TRACE_ERROR_START();TRACE_CONTENT(__VA_ARGS__);TRACE_END();
#else
#define TRACE_ERROR(...) (0)
#endif



#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')
#define BIN(byte) BYTE_TO_BINARY(byte)

void trace_log(char *tag, int line_number, char * fmt, ...);
void trace_log_tail();
void trace_log_content(char * fmt, ...);
void trace_log_header(char *tag, int line_number);
void output(char * msg);

