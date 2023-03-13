#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <graphics.h> //图形库
#include "tools.h"
/**
* 1. 导入素材
* 2. 实现最开始的游戏场景
* 3. 实现游戏顶部的工具栏
* 4. 实现工具栏中的卡牌
* 5. 实现种植植物
* 5. 实现植物动态
*/

#define WIN_WIDTH 900
#define WIN_HEIGHT 600

enum{WAN_DOU_SHE_SHOU, XIANG_RI_KUI, ZHI_WU_COUNT};//设计思路 枚举类型最后一个下标正好代表 枚举类型总个数

IMAGE imgBg;// 背景图片
IMAGE imgBar;// 卡片栏
IMAGE imgCards[ZHI_WU_COUNT];// 植物卡片
IMAGE* imgUnit[ZHI_WU_COUNT][20];// 植物个体各帧

int curX, curY;// 当前选中的植物坐标，在移动过程中的位置，保存到全局变量中
int curUnit;// 当前的植物 0：未选中 1：第一个植物 2：第二个植物

struct UnitFrame
{
    int type;       // 0：没有植物 1：第一个植物 2：第二个植物
    int frameIndex;//序列帧的序号
};

struct UnitFrame map[3][9];

bool fileExist(const char* name) {
    FILE* fp = fopen(name, "r");
    if (fp == NULL) {
        return false;
    }
    fclose(fp);
    fp = NULL;
    return true;
}

void gameInit() {
    // 加载有背景图片
    // 把字符集改为多字节字符集
    loadimage(&imgBg, "res/bg.jpg");
    loadimage(&imgBar, "res/bar.png");
    
    memset(imgUnit, 0, sizeof(imgUnit));
    memset(map, 0, sizeof(map));

    //初始化植物卡牌
    char name[32];
    for (int i = 0; i < ZHI_WU_COUNT; i++) {
        // 生成植物卡牌的文件名
        sprintf_s(name, sizeof(name), "res/Cards/card_%d.png", i + 1);// 将数据格式化输出到字符串
        loadimage(&imgCards[i], name);
        for (int j = 0; j < 20; j++) {
            sprintf_s(name, sizeof(name), "res/zhiwu/%d/%d.png", i, j + 1);// 将数据格式化输出到字符串
            // 先判断这个文件存不存在
            if(fileExist(name)) {
                imgUnit[i][j] = new IMAGE;
                loadimage(imgUnit[i][j], name);
            }
            else {
                break;
            }
        }
    }

    curUnit = 0;//

    // 创建游戏窗口
    initgraph(WIN_WIDTH, WIN_HEIGHT, 1);//第三个参数为1 可以打开控制台 进行调试
}

// 渲染图片函数(确定坐标)
void updateWindow() {
    //先渲染到缓冲区再一步打印出去 不要一个一个渲染
    BeginBatchDraw();

    putimagePNG(0,0,&imgBg);// 把背景图片渲染出来
    putimagePNG(250,0,&imgBar);// 把卡片槽和卡片渲染出来
    for (int i = 0; i < ZHI_WU_COUNT; i++) {
        int x = 338 + i * 65;
        int y = 6;
        putimage(x, y, &imgCards[i]);
    }

    // 渲染 种植完成的植物的动态帧
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 9; j++) {
            if (map[i][j].type > 0) {
                int x = 254 + 80 * j;
                int y = 187 + 102 * i;
                int unitType = map[i][j].type-1;
                int frameIndex = map[i][j].frameIndex;
                putimagePNG(x, y, imgUnit[unitType][frameIndex]);
            }
        }
    }
    // 调整顺序 使得拖动过程中的植物在已种植的植物的动态帧上面
    // 渲染拖动过程中的植物
    if (curUnit > 0) {
        IMAGE* curUnitImgP = imgUnit[curUnit - 1][0];
        putimagePNG(curX - curUnitImgP->getwidth() / 2, curY - curUnitImgP->getheight() / 2, curUnitImgP);
    }
    EndBatchDraw();// 结束双缓冲
}

void userClick() {
    ExMessage msg;
    static int status = 0;// 加入状态判断 0:为未选中植物 1：选中植物
    if (peekmessage(&msg)) {// peekmessage() 如果有消息返回真 把消息传递到&msg
        if (msg.message == WM_LBUTTONDOWN) {
            if (msg.x > 338 && msg.x < 338 + 65 * ZHI_WU_COUNT && msg.y < 96) {
                int index = (msg.x - 338) / 65;
                /*printf("%d\n", index);*/
                status = 1;
                curUnit = index + 1;// 保存选中的植物
                curX = msg.x;
                curY = msg.y;
            }
        }
        else if (msg.message == WM_MOUSEMOVE && status == 1) {
            // 把当前的位置记录下来
            curX = msg.x;
            curY = msg.y;

        }
        else if (msg.message == WM_LBUTTONUP) {
            if (msg.x > 254 && msg.y > 180 && msg.y < 489 ) {
                //松开种植物
                int row = (msg.y - 180) / 102;
                int col = (msg.x - 254) / 80;

                /*printf("%d %d ", row, col);*/
                if (map[row][col].type == 0) {
                    map[row][col].type = curUnit;
                    map[row][col].frameIndex = 0;//序列帧初始化为0
                }
            }

            curUnit = 0;
            status = 0;
        }
    }
}

void updateGame() {
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 9; j++) {
            if (map[i][j].type > 0) {
                map[i][j].frameIndex++;
                int unitType = map[i][j].type - 1;
                int unitFrame = map[i][j].frameIndex;
                if (imgUnit[unitType][unitFrame] == NULL) {
                    map[i][j].frameIndex = 0;//若指向帧为空指针 让索引归零 重新指
                }
            }
        }
    }
}

int main(void){
    gameInit();
    int timer = 0;// 计时器
    bool flag = true;

    while(1) {
        userClick();
        timer += getDelay();
        if (timer > 40) {
            flag = true;
            timer = 0;
        }

        if (flag) {
            flag = false;
            updateWindow();
            updateGame();
        }
        //Sleep(10);// 帧等待 ms
    }

    system("pause");// 添加暂停 检测窗口环境

    return 0;
}