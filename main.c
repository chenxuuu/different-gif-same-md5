#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "lzw.h"

//# define DECODE
//#define ENCODE
//#define CREATE_GIF
#define COMMON


//碰撞用的小数组
const unsigned char md5_s[] =
{
0x14,0xba,0x1f,0x3a,0x94,0x14,0xc7,0x5b,0x6e,0xb1,0x32,0xc3,0xbc,0x6f,0xb8,0x13,
0x58,0x9b,0xa0,0x44,0x73,0x42,0x09,0x4f,0x44,0x9e,0x78,0xe9,0xb7,0xbb,0x2f,0xc2,
0x5a,0xfd,0xee,0xc0,0xf2,0x62,0x2a,0x98,0x0b,0x5e,0xd3,0xf9,0xd3,0xf9,0x05,0x90,
0x94,0x22,0x1f,0xf9,0x56,0xaa,0x1e,0xd9,0xe3,0x20,0x74,0x19,0xca,0xa1,0x2a,0x0e,
0x29,0xaa,0xe8,0x7e,0x5c,0x4b,0xf0,0xec,0x4d,0xd0,0x7c,0x20,0xd9,0x1c,0xcc,0x9e,
0x2c,0xf6,0xb4,0x9a,0x18,0xbb,0xc0,0x20,0xc5,0xb7,0xf4,0xc0,0xd1,0x61,0x7d,0x1e,
0x71,0x59,0x96,0xda,0x26,0x84,0x73,0x22,0x77,0x53,0xbd,0xcf,0x50,0x51,0x53,0xc8,
0x7d,0x3c,0x17,0x72,0x7d,0x95,0x00,0xa0,0xcc,0x21,0x6d,0x25,0xcd,0x1b,0xd3,0x12,
// ------------------------------------------------------^^-----差异点
};
//碰撞用的大数组
const unsigned char md5_h[] =
{
0x14,0xba,0x1f,0x3a,0x94,0x14,0xc7,0x5b,0x6e,0xb1,0x32,0xc3,0xbc,0x6f,0xb8,0x13,
0x58,0x9b,0xa0,0xc4,0x73,0x42,0x09,0x4f,0x44,0x9e,0x78,0xe9,0xb7,0xbb,0x2f,0xc2,
0x5a,0xfd,0xee,0xc0,0xf2,0x62,0x2a,0x98,0x0b,0x5e,0xd3,0xf9,0xd3,0x79,0x05,0x90,
0x94,0x22,0x1f,0xf9,0x56,0xaa,0x1e,0xd9,0xe3,0x20,0x74,0x99,0xca,0xa1,0x2a,0x0e,
0x29,0xaa,0xe8,0x7e,0x5c,0x4b,0xf0,0xec,0x4d,0xd0,0x7c,0x20,0xd9,0x1c,0xcc,0x9e,
0x2c,0xf6,0xb4,0x1a,0x18,0xbb,0xc0,0x20,0xc5,0xb7,0xf4,0xc0,0xd1,0x61,0x7d,0x1e,
0x71,0x59,0x96,0xda,0x26,0x84,0x73,0x22,0x77,0x53,0xbd,0xcf,0x50,0xd1,0x53,0xc8,
0x7d,0x3c,0x17,0x72,0x7d,0x95,0x00,0xa0,0xcc,0x21,0x6d,0xa5,0xcd,0x1b,0xd3,0x12,
// ------------------------------------------------------^^-----差异点
};

const char hex_table[] = {
'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
};
void to_hex(unsigned char *s, int l, unsigned char *d)
{
    while(l--)
    {
        *(d++) = hex_table[*s >> 4];
        *(d++) = hex_table[*(s++) & 0x0f];
    }
}
void printHex(unsigned char * d, unsigned long l)
{
    unsigned char * rd = (unsigned char *)malloc(l*2+1);
    to_hex(d,l,rd);
    rd[l*2] = '\0';
    printf("size: %ld\ndata:\n%s",l,rd);
    free(rd);
}

int make(
    unsigned char* compressed_data,
    unsigned char compressed_len,
    int width,
    int height,
    int x,
    int y,
    unsigned char** r,
    const unsigned char* t)
{
    // 0x21,0xef,0x08,1,2,3,4,0,5,6,7,   2,1,2,0,
    // 0x2c, 5,0, 5,0, 2,0,2,0, 0,//Image Descriptor
    // 0x08,4,0x01,0x04,0x06,0x00,  0x00,
    *r = malloc(3+40+123+1+md5_h[123]+1);
    unsigned char * p = *r;//临时指针变量，防止把*r给改了
    p[0]=0x21;p[1]=0xef;p[2]=123+40;    p+=3;//注释头
    memset(p,0x00,40);                  p+=40;//40bit空数据
    memcpy(p,t,128);                    p+=128;//128字节碰撞数据
    memset(p,0,md5_s[123]-4+1);         p+=md5_s[123]-4+1;//空注释
    //图片位置数据
    p[0]=0x2c;
    p[1]=x%256; p[2]=x/256;
    p[3]=y%256; p[4]=y/256;
    p[5]=width%256; p[6]=width/256;
    p[7]=height%256; p[8]=height/256;
    p[9]=0;                             p+=10;
    //图片实际数据
    p[0]=0x08;
    p[1]=compressed_len;
    memcpy(p+2,compressed_data,compressed_len);
    p[2+compressed_len]=0x00;         p+=2+compressed_len+1;
    //剩下的注释
    p[0]=0x21;p[1]=0xef;p[2]=md5_h[123]-md5_s[123]-1-12-compressed_len-1-3;
    memset(p+3,0x00,p[2]+1);

    return 3+40+123+1+md5_h[123]+1;

    // (*r)[0]=0x21;(*r)[1]=0xef;(*r)[2]=123+40;
    // memcpy(*r+3,t,sizeof(md5_s));
    // memset(*r+3+sizeof(md5_s),0,md5_s[123]-4+1);
    // (*r)[3+sizeof(md5_s)-4+md5_s[123]+1] = 0x2c;
    // (*r)[3+sizeof(md5_s)-4+md5_s[123]+2] = x%256;
    // (*r)[3+sizeof(md5_s)-4+md5_s[123]+3] = x/256;
    // (*r)[3+sizeof(md5_s)-4+md5_s[123]+4] = y%256;
    // (*r)[3+sizeof(md5_s)-4+md5_s[123]+5] = y/256;
    // (*r)[3+sizeof(md5_s)-4+md5_s[123]+6] = width%256;
    // (*r)[3+sizeof(md5_s)-4+md5_s[123]+7] = width/256;
    // (*r)[3+sizeof(md5_s)-4+md5_s[123]+8] = height%256;
    // (*r)[3+sizeof(md5_s)-4+md5_s[123]+9] = height/256;
    // (*r)[3+sizeof(md5_s)-4+md5_s[123]+10] = 0;
    // (*r)[3+sizeof(md5_s)-4+md5_s[123]+11] = 0x08;
    // (*r)[3+sizeof(md5_s)-4+md5_s[123]+12] = compressed_len;
    // memcpy(*r+3+sizeof(md5_s)-4+md5_s[123]+13,compressed_data,compressed_len);
    // (*r)[3+sizeof(md5_s)-4+md5_s[123]+13+compressed_len] = 0x00;
    // (*r)[3+sizeof(md5_s)-4+md5_s[123]+13+compressed_len+1] = 0x21;
    // (*r)[3+sizeof(md5_s)-4+md5_s[123]+13+compressed_len+2] = 0xef;
    // (*r)[3+sizeof(md5_s)-4+md5_s[123]+13+compressed_len+3] = md5_h[123]-md5_s[123]-1-12-compressed_len-1-3;
    // memset(*r+3+sizeof(md5_s)-4+md5_s[123]+13+compressed_len+4,0,md5_h[123]-md5_s[123]-1-12-compressed_len-1-3+1);
    // return 3+123+1+md5_h[123]+1;
}

//生成贴上去的数据
int append_data(
    unsigned char* data,
    unsigned long data_len,
    int width,
    int height,
    int x,
    int y,
    unsigned char** s,
    unsigned char** h)
{
    unsigned char *r;
    unsigned long len;
    lzw_compress(
        8,
        data_len,
        data,
        &len,
        &r
    );
    if(len > md5_h[123]-md5_s[123]-17)
    {
        printf("data too long~ %d\n",len);
        return 0;//太长
    }
    make(r,len,width,height,x,y,s,md5_s);
    return make(r,len,width,height,x,y,h,md5_h);
}

int main () {

#ifdef DECODE
    // LZW 编码初始表大小的位数：3
    unsigned char code = 8;
    //  GIF 一帧图像的数据压缩文件（rainbow-compressed.gif.frame）大小
    long dl = 0x11;
    // GIF 一帧图像的数据压缩数据
    unsigned char d[] = {0x00,0xF7,0x09,0x1C,0x48,0xB0,0xA0,0xC1,0x83,0x08,0x13,0x2A,0x5C,0xC8,0x50,0x60,0x40,0x00};
    // GIF 一帧图像的数据解压后的数据
    unsigned char *raw;
    //  GIF 一帧图像的数据解压后大小
    unsigned long len;

    // 进行 LZW 解压
    lzw_decompress(
        code,
        dl,
        d,
        &len,
        &raw
    );

    printHex(raw,len);
#endif

#ifdef ENCODE
    //原始数据
    unsigned char raw[] = {1,1,1,1};
    //压缩结果
    unsigned char *r;
    //结果大小
    unsigned long len;

    lzw_compress(
        8,
        sizeof(raw),
        raw,
        &len,
        &r
    );

    printHex(r,len);

#endif

#ifdef CREATE_GIF
    int w = 100;
    int h = 20;
    unsigned char gif[] =
    {
        'G','I','F','8','9','a',//Header Block
        w%256,w/256,h%256,h/256,//width & height
        0xf0,0xff,0x00,//logical screen descriptor
        0x00,0x00,0x00,  0x00,0xff,0x00,//Global Color Table
        0x21,0xF9,0x04,  0x01, 0x00,0x00,  0x00,  0x00,//Graphics Control Extension
        0x21,0xef,0x08,1,2,3,4,0,5,6,7,   2,1,2,0,
        0x2c, 5,0, 5,0, 2,0,2,0, 0,//Image Descriptor
        0x08,4,0x01,0x04,0x06,0x00,  0x00,
        0x3b
    };
    FILE *giffile = fopen("test.gif", "wb+");
    fwrite(gif, sizeof(gif), 1, giffile);
    fflush(giffile);
    fclose(giffile);
#endif

#ifdef COMMON
    int width = 5;
    int height = 5;

    //固定的头参数
    unsigned char gh[] =
    {
        'G','I','F','8','9','a',//Header Block
        width%256,width/256,height%256,height/256,//width & height
        0xf7,0xff,0x00,//logical screen descriptor
        #include "colors.h"//Global Color Table, from https://jonasjacek.github.io/colors/
        0x21,0xF9,0x04,  0x01, 0x00,0x00,  0x00,  0x00,//Graphics Control Extension
    };
    //图像数据存储处
    unsigned char* img1 = malloc(sizeof(gh));
    unsigned char* img2 = malloc(sizeof(gh));
    //图像大小
    size_t img_size = sizeof(gh);
    //头数据搬进去
    memcpy(img1,gh,sizeof(gh));
    memcpy(img2,gh,sizeof(gh));

    //不同的图片
    unsigned char *s,*h;
    unsigned char d[] = {
        9,9,9,9,9,
        10,10,10,10,10,
        11,11,11,11,11,
        12,12,12,12,12,
        13,13,13,13,13,
    };
    img_size += append_data(d,sizeof(d),5,5,0,0,&s,&h);
    img1 = realloc(img1,img_size);
    img2 = realloc(img2,img_size);
    memcpy(img1+sizeof(gh),s,img_size-sizeof(gh)-1);
    memcpy(img2+sizeof(gh),h,img_size-sizeof(gh)-1);

    //相同的图片区域
    // 0x2c, 5,0, 5,0, 2,0,2,0, 0,//Image Descriptor
    // 0x08,4,0x01,0x04,0x06,0x00,  0x00,
    unsigned char same[] = {10,10,10,10,10,10,10,10,10,10,10,10};
    unsigned char *r;
    unsigned long len;
    lzw_compress(
        8,
        sizeof(same),
        same,
        &len,
        &r
    );
    size_t last_size = img_size;//记下之前的大小
    img_size += len + 13;
    img1 = realloc(img1,img_size);
    img2 = realloc(img2,img_size);
    unsigned char same_i[] = {0x2c, 0,0,0,0,4,0,3,0,0,0x08,len};
    memcpy(img1+last_size,same_i,sizeof(same_i));
    memcpy(img1+last_size+sizeof(same_i),r,len);
    img1[img_size-1] = 0;
    memcpy(img2+last_size,same_i,sizeof(same_i));
    memcpy(img2+last_size+sizeof(same_i),r,len);
    img2[img_size-1] = 0;

    //最后1字节
    img1 = realloc(img1,img_size+1);
    img2 = realloc(img2,img_size+1);

    FILE *giffile;
    img1[img_size-1] = 0x3b;
    printf("gif1 size: %d\n",img_size);
    giffile = fopen("test1.gif", "wb+");
    fwrite(img1, img_size, 1, giffile);
    fflush(giffile);
    fclose(giffile);


    img2[img_size-1] = 0x3b;
    printf("gif2 size: %d\n",img_size);
    giffile = fopen("test2.gif", "wb+");
    fwrite(img2, img_size, 1, giffile);
    fflush(giffile);
    fclose(giffile);

    free(s);
    free(h);
    free(img1);
    free(img2);

#endif

    return 0;
}
