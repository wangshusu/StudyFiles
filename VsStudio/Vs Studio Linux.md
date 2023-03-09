## Vs Studio Linux

## Linux工程编辑

1.ssh地址为：172.16.1.93  build build



2.文件的排列方式如下  .c 文件为引用文件

![image-20230209142648053](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/image-20230209142648053.png)



2.进入项目管理 MakeFile settings 

![image-20230210095251205](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/image-20230210095251205.png)



4.接下里修改Project settings

![image-20230210095530779](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/image-20230210095530779.png)



6.连接到虚拟机后 文件的输出目录在 ：

![image-20230209195403260](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/image-20230209195403260.png)





## windows 工程包含其他路径的目录

![image-20230210135051391](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/image-20230210135051391.png)

![image-20230210135129512](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/image-20230210135129512.png)



## 使用动态库

1. 将需要使用的动态库copy到项目的源文件中

![image-20230223185520803](C:\Users\Administrator\AppData\Roaming\Typora\typora-user-images\image-20230223185520803.png)

2.接下来修改

![image-20230223185910930](C:\Users\Administrator\AppData\Roaming\Typora\typora-user-images\image-20230223185910930.png)

3. 包含当前*.so

![image-20230223192052357](C:\Users\Administrator\AppData\Roaming\Typora\typora-user-images\image-20230223192052357.png)



4.到linux的工程文件中  使用ln -s链接库文件

![image-20230223190106440](C:\Users\Administrator\AppData\Roaming\Typora\typora-user-images\image-20230223190106440.png)

