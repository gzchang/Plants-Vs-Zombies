#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <time.h>
#include <graphics.h> //图形库
#include "tools.h"
//音效
#include <mmsystem.h> 
#pragma comment(lib, "winmm.lib")

/**
* 1. 导入素材
* 2. 实现最开始的游戏场景
* 3. 实现游戏顶部的工具栏
* 4. 实现工具栏中的卡牌
* 5. 实现种植植物
* 6. 实现植物动态
* 7. 实现启动菜单
* 8. 实现随机阳光
* 9. 实现收集阳光
* 10.创建僵尸
*/

#define WIN_WIDTH 900
#define WIN_HEIGHT 600

enum{WAN_DOU_SHE_SHOU, XIANG_RI_KUI, ZHI_WU_COUNT};//设计思路 枚举类型最后一个下标正好代表 枚举类型总个数

IMAGE imgBg;// 背景图片
IMAGE imgBar;// 卡片栏
IMAGE imgCards[ZHI_WU_COUNT];// 植物卡片
IMAGE* imgUnit[ZHI_WU_COUNT][20];// 植物个体各帧
int sunshine;// 阳光值

int curX, curY;// 当前选中的植物坐标，在移动过程中的位置，保存到全局变量中
int curUnit;// 当前的植物 0：未选中 1：第一个植物 2：第二个植物

struct UnitFrame
{
    int type;       // 0：没有植物 1：第一个植物 2：第二个植物
    int frameIndex;//序列帧的序号
};

struct UnitFrame map[3][9];

typedef struct SunshineBall
{
    int x, y;// 阳光的飘落过程中的坐标位置(x坐标不变)
    int frameIndex;// 当前显示的图片帧的序号
    int destY;//飘落的目标位置的y坐标
    bool used;// 是否在使用
    int timer; // 阳光生命周期
}SunshineBall ;

SunshineBall Balls[10];// 阳光球池
IMAGE imgSunshineBall[29];

typedef struct zm
{
    int x, y;
    int frameIndex;
    bool used;
    int speed;
}zm;
zm zms[10];//10个僵尸
IMAGE imgZM[22];

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
    // 初始化阳光值
    sunshine = 50;


    //创建阳光池
    memset(Balls, 0, sizeof(Balls));
    for (int i = 0; i < 29; i++) {
        sprintf_s(name, sizeof(name), "res/sunshine/%d.png", i + 1);
        loadimage(&imgSunshineBall[i], name);
    }
    //配置随机种子
    srand(time(NULL));

    // 创建游戏窗口
    initgraph(WIN_WIDTH, WIN_HEIGHT, 1);//第三个参数为1 可以打开控制台 进行调试

    // 设置字体
    LOGFONT font;
    gettextstyle(&font);
    font.lfHeight = 30;
    font.lfWeight = 15;
    strcpy(font.lfFaceName, "Segoe UI Black");
    font.lfQuality = ANTIALIASED_QUALITY; // 抗锯齿效果
    settextstyle(&font);
    setbkmode(TRANSPARENT);// 字体背景透明
    setcolor(BLACK);

    // 初始化僵尸
    memset(zms, 0, sizeof(zms));
    for (int i = 0; i < 22; i++) {// 初始化图片帧
        sprintf_s(name, sizeof(name), "res/zm/%d.png", i + 1);
        loadimage(&imgZM[i], name);
    }

}

void drawZm() {
    int zmCount = sizeof(zms) / sizeof(zms[0]);
    for (int i = 0; i < zmCount; i++) {
        if (zms[i].used) {
            IMAGE* img = &imgZM[zms[i].frameIndex];
            putimagePNG(zms[i].x, zms[i].y - img->getheight(), img);
        }
    }
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
    //渲染阳光
    int ballMax = sizeof(Balls) / sizeof(Balls[0]);
    for (int i = 0; i < ballMax; i++) {
        if (Balls[i].used) {
            IMAGE* img = &imgSunshineBall[Balls[i].frameIndex];
            putimagePNG(Balls[i].x, Balls[i].y, img);
        }
    }
    //渲染阳光值
    char scoreText[8];
    sprintf_s(scoreText, sizeof(scoreText), "%d", sunshine);
    outtextxy(278, 67, scoreText);//在指定位置输出文本

    // 渲染僵尸
    drawZm();

    EndBatchDraw();// 结束双缓冲
}

void collectSunshine(ExMessage* msg) {
    int count = sizeof(Balls) / sizeof(Balls[0]);
    int width = imgSunshineBall[0].getwidth();
    int height = imgSunshineBall[0].getheight();
    for (int i = 0; i < count; i++) {
        if (Balls[i].used) {
            // 出现的阳光球坐标
            int x = Balls[i].x;
            int y = Balls[i].y;
            if (msg->x > x && msg->x < x + width && msg->y > y && msg->y < y + height) {
                Balls[i].used = false;
                sunshine += 25;
                mciSendString("play res/sunshine.mp3", 0, 0, 0);//播放收集阳光音乐
            }
        }
    }
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
            else {
                // 是否在收集阳光
                collectSunshine(&msg);
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

void createSunshine() {

    static int sunsineSum = 0;// 阳光计数器
    static int frequent = 4;// 阳光频率 

    sunsineSum++;
    if (sunsineSum >= frequent) {
        frequent = 2 + rand() % 200;
        sunsineSum = 0;
        // 从阳光池中取一个可用的
        int ballMax = sizeof(Balls) / sizeof(Balls[0]);
        int i = 0;
        for (i = 0; i < ballMax && Balls[i].used; i++);//空for循环 找到一个符合条件的i
        if (i >= ballMax)return;
        // 找到
        Balls[i].used = true;//设为在使用
        Balls[i].frameIndex = 0;
        Balls[i].x = 260 + rand() % (900 - 260); //260->900
        Balls[i].y = 60;
        Balls[i].destY = 200 + (rand() % 4) * 90;
        Balls[i].timer = 0;
    }
}

void updateSunshine() {
    int ballsMax = sizeof(Balls) / sizeof(Balls[0]);
    for (int i = 0; i < ballsMax; i++) {
        if (Balls[i].used) {
            Balls[i].frameIndex = (Balls[i].frameIndex + 1) % 29;
            if (Balls[i].timer == 0) {
                Balls[i].y += 2;
            }
            if (Balls[i].y >= Balls[i].destY) {
                Balls[i].timer++;
                if (Balls[i].timer > 100) {
                    Balls[i].used = false;
                }
            }
        }
    }
}

void createZm() {
    static int zmFrequent = 50;
    static int count = 0;
    count++;
    if (count > zmFrequent) {
        count = 0;
        zmFrequent = rand() % 20 + 30;
        int i = 0;
        int zmMax = sizeof(zms) / sizeof(zms[0]);
        for (i = 0; i < zmMax && zms[i].used; i++);//找到能用的僵尸
        if (i < zmMax) {
            zms[i].used = true;
            zms[i].x = WIN_WIDTH;
            zms[i].y = 172 + (rand() % 3 +1 ) * 100;
            zms[i].speed = 1;
        }
    }
}

void updateZm() {
    int zmMax = sizeof(zms) / sizeof(zms[0]);
    static int count = 0;
    count++;
    if (count > 1) {
        // 更新僵尸的位置
        for (int i = 0; i < zmMax; i++) {
            if (zms[i].used) {
                zms[i].x -= zms[i].speed;
                if (zms[i].x < 170) {
                    printf("GAME OVER\n");
                    MessageBox(NULL, "over", "over", 0);//
                    exit(0);//
                }
            }
        }

    }
    static int count2 = 0;
    count2++;
    if (count2 > 256) {
        // 更新僵尸的帧数
        for (int i = 0; i < zmMax; i++) {
            if (zms[i].used) {
                zms[i].frameIndex = (zms[i].frameIndex + 1) % 22;
            }
        }
    }

}

// 修改动态帧
void updateGame() {
    // 修改植物动态帧
    for (int i = 0; i < 3; i++){
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

    createSunshine();// 创建阳光
    updateSunshine();// 更新阳光动态帧
    createZm();// 创建僵尸
    updateZm();// 更新僵尸动态帧
}

// 制作启动菜单
void startUI() {
    IMAGE imgBg, imgMenu1, imgMenu2;
    loadimage(&imgBg, "res/menu.png");
    loadimage(&imgMenu1, "res/menu1.png");
    loadimage(&imgMenu2, "res/menu2.png");
    int flag = 0;//是否选中菜单
    while (1) {
        BeginBatchDraw();
        putimage(0, 0, &imgBg);
        putimagePNG(474, 75, flag ? &imgMenu2 : &imgMenu1);
        ExMessage msg;
        if (peekmessage(&msg)) {
            if (msg.message == WM_LBUTTONDOWN && msg.x >474 && msg.x <474+331 && msg.y>75 && msg.y<75+140) {
                flag = 1;
            }
            else if (msg.message == WM_LBUTTONUP && flag == 1) {
                return;
            }
        }
        EndBatchDraw();

    }
}

int main(void){
    gameInit();
    startUI();
    int timer = 0;// 计时器
    bool flag = true;

    while(1) {
        userClick();
        timer += getDelay();
        if (timer > 30) {
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