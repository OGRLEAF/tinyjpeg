# Tiny jpeg

简易的JPEG解码实现，实现了1x1格式的JPEG解码，实现了输出到YUV444格式。

## 使用方法

### 编译

```shell
make
```

### 运行

```shell
./build/Main <Jpeg path> <YUV output Path>
```


使用FFplay打开生成的YUV文件：

```
ffplay -s <Width>x<Height> -pixel_format yuv444p -i <YUV output Path>
```

### 日志

开关日志信息，在`lib/Inc/trace.h`中修改宏定义：

```c
#define MAX_CHARS 1000

#define TRACE_EN 1
#define TRUE  1
#define FALSE 0
#define TRACE_ERROR_EN  TRUE    &&TRACE_EN  // 开启ERROR级别日志
#define TRACE_INFO_EN   TRUE    &&TRACE_EN  // 开启INFO级别日志
#define TRACE_DEBUG_EN  FALSE   &&TRACE_EN  // 关闭DEBUG级别日志
```