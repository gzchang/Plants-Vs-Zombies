#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <graphics.h> //ͼ�ο�
#include "tools.h"
/**
* 1. �����ز�
* 2. ʵ���ʼ����Ϸ����
* 3. ʵ����Ϸ�����Ĺ�����
* 4. ʵ�ֹ������еĿ���
* 5. ʵ����ֲֲ��
* 5. ʵ��ֲ�ﶯ̬
*/

#define WIN_WIDTH 900
#define WIN_HEIGHT 600

enum{WAN_DOU_SHE_SHOU, XIANG_RI_KUI, ZHI_WU_COUNT};//���˼· ö���������һ���±����ô��� ö�������ܸ���

IMAGE imgBg;// ����ͼƬ
IMAGE imgBar;// ��Ƭ��
IMAGE imgCards[ZHI_WU_COUNT];// ֲ�￨Ƭ
IMAGE* imgUnit[ZHI_WU_COUNT][20];// ֲ������֡

int curX, curY;// ��ǰѡ�е�ֲ�����꣬���ƶ������е�λ�ã����浽ȫ�ֱ�����
int curUnit;// ��ǰ��ֲ�� 0��δѡ�� 1����һ��ֲ�� 2���ڶ���ֲ��

struct UnitFrame
{
    int type;       // 0��û��ֲ�� 1����һ��ֲ�� 2���ڶ���ֲ��
    int frameIndex;//����֡�����
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
    // �����б���ͼƬ
    // ���ַ�����Ϊ���ֽ��ַ���
    loadimage(&imgBg, "res/bg.jpg");
    loadimage(&imgBar, "res/bar.png");
    
    memset(imgUnit, 0, sizeof(imgUnit));
    memset(map, 0, sizeof(map));

    //��ʼ��ֲ�￨��
    char name[32];
    for (int i = 0; i < ZHI_WU_COUNT; i++) {
        // ����ֲ�￨�Ƶ��ļ���
        sprintf_s(name, sizeof(name), "res/Cards/card_%d.png", i + 1);// �����ݸ�ʽ��������ַ���
        loadimage(&imgCards[i], name);
        for (int j = 0; j < 20; j++) {
            sprintf_s(name, sizeof(name), "res/zhiwu/%d/%d.png", i, j + 1);// �����ݸ�ʽ��������ַ���
            // ���ж�����ļ��治����
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

    // ������Ϸ����
    initgraph(WIN_WIDTH, WIN_HEIGHT, 1);//����������Ϊ1 ���Դ򿪿���̨ ���е���
}

// ��ȾͼƬ����(ȷ������)
void updateWindow() {
    //����Ⱦ����������һ����ӡ��ȥ ��Ҫһ��һ����Ⱦ
    BeginBatchDraw();

    putimagePNG(0,0,&imgBg);// �ѱ���ͼƬ��Ⱦ����
    putimagePNG(250,0,&imgBar);// �ѿ�Ƭ�ۺͿ�Ƭ��Ⱦ����
    for (int i = 0; i < ZHI_WU_COUNT; i++) {
        int x = 338 + i * 65;
        int y = 6;
        putimage(x, y, &imgCards[i]);
    }

    // ��Ⱦ ��ֲ��ɵ�ֲ��Ķ�̬֡
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
    // ����˳�� ʹ���϶������е�ֲ��������ֲ��ֲ��Ķ�̬֡����
    // ��Ⱦ�϶������е�ֲ��
    if (curUnit > 0) {
        IMAGE* curUnitImgP = imgUnit[curUnit - 1][0];
        putimagePNG(curX - curUnitImgP->getwidth() / 2, curY - curUnitImgP->getheight() / 2, curUnitImgP);
    }
    EndBatchDraw();// ����˫����
}

void userClick() {
    ExMessage msg;
    static int status = 0;// ����״̬�ж� 0:Ϊδѡ��ֲ�� 1��ѡ��ֲ��
    if (peekmessage(&msg)) {// peekmessage() �������Ϣ������ ����Ϣ���ݵ�&msg
        if (msg.message == WM_LBUTTONDOWN) {
            if (msg.x > 338 && msg.x < 338 + 65 * ZHI_WU_COUNT && msg.y < 96) {
                int index = (msg.x - 338) / 65;
                /*printf("%d\n", index);*/
                status = 1;
                curUnit = index + 1;// ����ѡ�е�ֲ��
                curX = msg.x;
                curY = msg.y;
            }
        }
        else if (msg.message == WM_MOUSEMOVE && status == 1) {
            // �ѵ�ǰ��λ�ü�¼����
            curX = msg.x;
            curY = msg.y;

        }
        else if (msg.message == WM_LBUTTONUP) {
            if (msg.x > 254 && msg.y > 180 && msg.y < 489 ) {
                //�ɿ���ֲ��
                int row = (msg.y - 180) / 102;
                int col = (msg.x - 254) / 80;

                /*printf("%d %d ", row, col);*/
                if (map[row][col].type == 0) {
                    map[row][col].type = curUnit;
                    map[row][col].frameIndex = 0;//����֡��ʼ��Ϊ0
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
                    map[i][j].frameIndex = 0;//��ָ��֡Ϊ��ָ�� ���������� ����ָ
                }
            }
        }
    }
}

// ���������˵�
void startUI() {
    IMAGE imgBg, imgMenu1, imgMenu2;
    loadimage(&imgBg, "res/menu.png");
    loadimage(&imgMenu1, "res/menu1.png");
    loadimage(&imgMenu2, "res/menu2.png");
    int flag = 0;//�Ƿ�ѡ�в˵�
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
    int timer = 0;// ��ʱ��
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
        //Sleep(10);// ֡�ȴ� ms
    }

    system("pause");// �����ͣ ��ⴰ�ڻ���

    return 0;
}