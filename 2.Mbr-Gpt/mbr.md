## mbr

![image-20230207090639521](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/image-20230207090639521.png)

### mbr分区表

![image-20230212210608380](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/20230212210608.png)

```C++
typedef unsigned char		uint8_t ;
typedef unsigned short		uint16_t;
typedef unsigned int		uint32_t;
typedef unsigned long long	uint64_t;

struct MbrPartitionTable
{
	uint8_t partion_boot_id;					// 引导标志
	uint8_t partion_start_head;					// 开始磁头
	uint8_t partion_start_sector_cylinder[2];	// 起始扇区 柱面
	uint8_t partion_type;						// 分区类型 05H||0FH 拓展分区
	uint8_t partion_end_head;					// 结束磁头
	uint8_t partion_end_sector_cylinder[2];		// 结束扇面 柱面
	uint8_t partion_used_sector_num[4];			// 本分区起始扇区
	uint8_t partion_use_sector_num[4];			// 本分区使用扇区数
};
struct Mbr
{
	uint8_t boot_code[446];								// 引导代码
	struct MbrPartitionTable mbr_partition_table[4];	// 四个mbr分区表
	uint8_t end_flag[2];								// 结束标志 55AA
};

void showPMBR(struct PMBR* the_pmbr)
{
	printf("引导标志为%X\n", the_pmbr->pation_table_entry[0].partion_boot_id);
	printf("磁头号为%X\n", the_pmbr->pation_table_entry[0].partion_start_head);
	printf("扇区号为%X\n", the_pmbr->pation_table_entry[0].partion_start_sector_cylinder[0]);
	printf("柱面号为%X\n", the_pmbr->pation_table_entry[0].partion_start_sector_cylinder[1]);
	printf("分区类型标志为 %X\n", the_pmbr->pation_table_entry[0].partion_type);
	printf("第一个扇区为 %u\n", uint8to32(the_pmbr->pation_table_entry[0].partion_used_sector_num));
	printf("扇区数为 %u\n", uint8to32(the_pmbr->pation_table_entry[0].partion_use_sector_num));
}
```

### mbr拓展分区

![image-20230208093320409](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/image-20230208093320409.png)

>  （60 08 00） （16）= （6293504）（10） 指向了分区间隙
>
> 接下来进入分区间隙

![image-20230208094500596](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/image-20230208094500596.png)

> 磁盘的位置地址为 ： （600800 + 800）（16）
>
> ​									（601000）（16） =  （6295522）（10）也就是磁盘5的扇区地址
>
> a下一个分区间隙的地址为：（600800 + 200800）（16）
>
> ​											  （801000）（16） = （8392704）（10）也就是分区间隙的地址
>
> 即可知 磁盘5 的扇区数量为 ： （200800 - 800）（16） = （2097152）（10）个扇区





## GPT

![image-20230207091852225](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/image-20230207091852225.png)

### pmbr

![image-20230207092042671](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/image-20230207092042671.png)

```C++
//LBA1 占据1扇区 512字节
struct Pmbr
{
	uint8_t boot_code[446];								// 引导代码
	struct MbrPartitionTable mbr_partition_table[4];	// 第一个分区表 partion_type == 0XEE 表示gpt
	uint8_t end_flag[2];								// 结束标志 55AA
};
```



### gpt header

![image-20230207092151408](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/image-20230207092151408.png)

```C++
struct GptHeader
{
	uint8_t header_signature[8];			// 签名
	uint8_t header_version[4];				// 版本号
	uint8_t header_size[4];					// GPT表头大小
	uint8_t header_crc32[4];				// GPT表头的CRC-32校验
	uint8_t header_reserve[4];				// 保留 为0
	uint8_t header_start_lba[8];			// 表头的扇区号
	uint8_t header_backup_lba[8];			// 备份表头的扇区号
	uint8_t header_first_partition_lba[8];	// GPT分区起始扇区号
	uint8_t header_last_partition_lba[8];	// GPT分区结束扇区号
	uint8_t header_guid[16];				// 磁盘的GUID
	uint8_t header_table_first[8];			// 分区表起始扇区号
	uint8_t header_table_entries[4];		// 分区表总项数
	uint8_t header_table_size[4];			// 单个分区表占用字节数
	uint8_t header_table_crc[4];			// 分区表的CRC校验
	uint8_t header_notuse[420];				// 保留的420字节
};

void show_gpt_header(struct gpt_header* the_gpt_header) {
	printf("GPT头签名为:");
	for (int i = 0; i < 8; i++)
		printf("%c", the_gpt_header->signature[i]);
	printf("\n");

	printf("版本号为:");
	for (int i = 0; i < 4; i++)
		printf("%0X", the_gpt_header->version[i]);
	printf("\n");

	printf("GPT头大小为 %u 字节\n", uint8to32(the_gpt_header->headersize));

	printf("GPT头CRC校验值为:");
	for (int i = 0; i < 4; i++)
		printf("%0X", the_gpt_header->headercrc32[i]);
	printf("\n");

	printf("GPT表头起始扇区号为 %I64X\n", uint8to64(the_gpt_header->header_lba));
	//备份表头在最后一个EFI扇区，可以得知整个磁盘的大小，扇区数*512/1024/1024/1024
	printf("GPT备份表头扇区号为 %I64X\n", uint8to64(the_gpt_header->backup_lba));

	printf("GPT分区区域的起始扇区号为 %I64X\n", uint8to64(the_gpt_header->pation_first_lba));
    
    printf("GPT分区表起始扇区号为 %I64X\n", uint8to64(the_gpt_header->pation_table_first));

	printf("GPT分区表总项数为 %I32X\n", uint8to32(the_gpt_header->pation_table_entries));

	printf("每个分区表占用字节数为 %I32X\n", uint8to32(the_gpt_header->pation_table_size));

	printf("分区表CRC校验值为 %I32X\n", uint8to32(the_gpt_header->pation_table_crc));
}
```



### gpt partion

![image-20230207092208773](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/image-20230207092208773.png)

```C++
struct GptPartitionTable // 分区表 128字节
{
	uint8_t partion_type[16];	// 分区类型，全0是未使用
	uint8_t partion_id[16];		// 分区唯一标识符
	uint8_t partion_start[8];	// 分区起始扇区号
	uint8_t partion_end[8];		// 分区结束扇区号
	uint8_t partion_attr[8];	// 分区属性标志,区分分区是什么类型的
	uint8_t partion_name[72]; 	// 分区名
};

void show_partion_name(uint8_t* beginchar, int length) {
	int j = 0;
	for (int i = 0; i < length; i++) {
		if (beginchar[i] == 0)
			j++;
		else
			j = 0;

		if (j > 2)
			return;//后面都是0
		else if (j == 0)
			printf("%c", beginchar[i]);
	}
}


```






## 编码问题

### 1.0 SetFilePointer

```C++
SetFilePointer函数的作用将 hFile（对应文件）里的文件指针从dwMoveMethod 所表示的文件位置开始移动(lpDistanceToMoveHigh<<32+lDistanceToMove)个字节偏移量的距离到一个新的文件位置，即修改 hFile（对应文件）里的文件指针为一个新值。
 
可以通过 GetLastError() 来获取错误的信息码 若无错误 GetLastError() 返回为 0；
如果为较小的字节，直接填写即可
    SetFilePointer(i_hDevice, 512, NULL, FILE_BEGIN);// 读取分区间隙
    
    
如果移动较为大的字节数  参数的设置将会改变如下
    LARGE_INTEGER offset;//long long signed
	offset.QuadPart = (ULONGLONG)(*(uint32_t *)(pMbr->pation_table_entry[3].relative)) * (ULONGLONG)512;// 记录分区间隙指向
	SetFilePointer(i_hDevice, offset.LowPart, &offset.HighPart, FILE_BEGIN);// 读取分区间隙
	if (GetLastError())
	{
		printf("SetFilePointer Error!\n");
	}
```



### 2.0 将 4位 uint8_t 转换为 uint32_t

```C++
uint32_t uint8to32(uint8_t fouruint8[4]) {
	return *(uint32_t*)fouruint8;
	// or
    return ((uint32_t)fouruint8[3] << 24) | 
        	((uint32_t)fouruint8[2] << 16) | 
        	((uint32_t)fouruint8[1] << 8) | 
        	((uint32_t)fouruint8[0]);
}
```



### 3.0 将 8位 uint8_t 转换为 uint64_t

```C++
typedef unsigned char uint8_t;
typedef unsigned long uint64_t;
uint64_t uint8to64(uint8_t fouruint8[8]) {
	return *(uint64_t*)fouruint8;
    // or
	return ((uint64_t)fouruint8[7] << 56) | 
    		((uint64_t)fouruint8[6] << 48) | 
        	((uint64_t)fouruint8[5] << 40) | 
        	((uint64_t)fouruint8[4] << 32) |
			((uint64_t)fouruint8[3] << 24) | 
        	((uint64_t)fouruint8[2] << 16) | 
        	((uint64_t)fouruint8[1] << 8) | 
        	((uint64_t)fouruint8[0]);
}
```



### 4.0 sprintf 不安全问题

![image-20230208193950912](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/image-20230208193950912.png)

![image-20230208194028111](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/image-20230208194028111.png)



### 5.打印读出的数据

```C++
struct PMBR *pMbr;
for (i = 0; i < ret; i++) {
		printf("%02X ", ((uint8_t *)pMbr)[i]);
		if (((i + 1) % 16 == 0)) {
			printf("\r\n");
		}
	}
	printf("\r\n");
```

```C++
这里的 %x 是以16字节来展示数据
02输出两位  建议字节类型使用 uint_8 类型
%d 十进制有符号整数
%u 十进制无符号整数
%f 浮点数
%s 字符串
%c 单个字符
%p 指针的值
%e 指数形式的浮点数
%x, %X 无符号以十六进制表示的整数
%o 无符号以八进制表示的整数
%g 把输出的值按照 %e 或者 %f 类型中输出长度较小的方式输出
%p 输出地址符
%lu 32位无符号整数
%llu 64位无符号整数
```

