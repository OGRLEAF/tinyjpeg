# Tiny jpeg

简易的JPEG解码实现，实现了1x1格式的JPEG解码，实现了输出到YUV444格式。

## 使用方法

编译：

```shell
make
```

运行

```shell
./build/Main <Jpeg path> <YUV output Path>
```


使用FFplay打开生成的YUV文件：

```
ffplay -s <Width>x<Height> -pixel_format yuv444p -i <YUV output Path>
```
