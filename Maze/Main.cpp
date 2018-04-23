// ���� ¬��� 201500302013
#define _CRT_SECURE_NO_DEPRECATE
#define BMP_Header_Length 54
#include <GL/glut.h>
#include <ctime>
#include <math.h>
#include <stdlib.h>
#include <atlimage.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <olectl.h>              // OLE���ƿ�ͷ�ļ�
#include <math.h>              // ��ѧ����ͷ�ļ�   
#include<atlimage.h>		//LoadTexture ����
#define _CRT_SECURE_NO_WARNINGS
#define MAX_LEVEL 1800
#define GAME_TIME 20
#define MAX_CHAR 128
using namespace std;

//Maze��SysMaze��UserMaze ��1��·��0��ǽ
//visited[] ��0��ǽ,���ɷ���  1�ǻ�û�б����ʵ�· 2�Ǳ����ʹ���· 3���߲�ͨ���ݻ�����·

unsigned short SysMaze[29][29];		//����ǽ
unsigned short UserMaze[29][29];	//����ǽ
unsigned short visited[841];
short playerx = 0, playery = 0;
float restTime = 0;
short windowState = 0;  //windowState : 0 ѡ�����  1 ��ͨ��Ϸ���� 
                        //2��ͨ��Ϸʤ�� 3 ��ͨ��Ϸʧ�� 4�༭�Թ� 5 ��ȡ�Թ� 
float TwoPI = 2.0 * 3.141592653;
bool showPath = false;
bool firstFind = true;
bool EditMode = 1;
bool Save_Read = 1;	//��ʶ�滹��ȡ,1��д,0�Ƕ�
string fileNames[10] = { ".//01.txt",".//02.txt" ,".//03.txt" ,".//04.txt" ,".//05.txt" ,".//06.txt",
".//07.txt" ,".//08.txt" ,".//09.txt" ,".//10.txt" };
typedef pair<unsigned short, unsigned short> wallInfo;		//��һ����pos,�ڶ�����source	first == pos,second == source
vector<wallInfo> Wall;
vector<wallInfo>::iterator it;
GLuint backGround;
GLuint texWall;
GLuint texGrass;
GLuint texMouse;
GLuint texFood;


GLuint LoadTexture(LPCTSTR fileName)
{
	BITMAP bm;
	GLuint idTexture = 0;
	CImage img;
	HRESULT hr = img.Load(fileName);
	if (!SUCCEEDED(hr))	//�ļ�����ʧ��
	{

		return NULL;
	}
	HBITMAP hbmp = img;
	if (!GetObject(hbmp, sizeof(bm), &bm))
		return 0;

	glGenTextures(1, &idTexture);
	if (idTexture)
	{
		glBindTexture(GL_TEXTURE_2D, idTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glPixelStoref(GL_PACK_ALIGNMENT, 1);
#define GL_BGR_EXT 0x80E0
		glTexImage2D(GL_TEXTURE_2D, 0, 3, bm.bmWidth, bm.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, bm.bmBits);
	}
	return idTexture;
}


void drawString(const char* str, int size) {
	static int isFirstCall = 1;
	static GLuint listSmall,listBig;
	HFONT hFont, hOldFont;
	if (isFirstCall) { // ����ǵ�һ�ε��ã�ִ�г�ʼ��
					   // Ϊÿһ��ASCII�ַ�����һ����ʾ�б�
	// ѡ������
		hFont = CreateFontA(32, 0, 0, 0, FW_MEDIUM, 0, 0, 0,
			ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Comic Sans MS");
		hOldFont = (HFONT)SelectObject(wglGetCurrentDC(), hFont);
		DeleteObject(hOldFont);
		isFirstCall = 0;
		// ����MAX_CHAR����������ʾ�б���
		listSmall = glGenLists(MAX_CHAR);
		// ��ÿ���ַ��Ļ������װ����Ӧ����ʾ�б���
		wglUseFontBitmaps(wglGetCurrentDC(), 0, MAX_CHAR, listSmall);

		hFont = CreateFontA(128, 0, 0, 0, FW_MEDIUM, 0, 0, 0,
			ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Comic Sans MS");
		hOldFont = (HFONT)SelectObject(wglGetCurrentDC(), hFont);
		DeleteObject(hOldFont);
		isFirstCall = 0;
		// ����MAX_CHAR����������ʾ�б���
		listBig = glGenLists(MAX_CHAR);
		// ��ÿ���ַ��Ļ������װ����Ӧ����ʾ�б���
		wglUseFontBitmaps(wglGetCurrentDC(), 0, MAX_CHAR, listBig);
	}
	// ����ÿ���ַ���Ӧ����ʾ�б�����ÿ���ַ�
	GLuint use;
	if (size == 32) use = listSmall;
	else if (size == 128) use = listBig;
	for (; *str != '\0'; ++str)
		glCallList(use + *str);
}

void printMaze() {
	cout << "---------------------Maze---------------------" << endl;
	for (int i = 0; i < 29; i++) {
		for (int j = 0; j < 29; j++) {
			cout << SysMaze[i][j] << " ";
		}
		cout << endl;
	}
	cout << "-------------------Maze End-------------------" << endl;
}

unsigned short random(unsigned short start, unsigned short end) {
	if (end == start) return start;
	return start + rand() % (end - start);
}

bool isOuter(unsigned short i) {
	if (i % 29 == 0) return true;
	if (i % 29 == 28) return true;
	if (0 < i && i < 29) return true;
	if (811 < i && i < 841) return true;
	return false;
}
void PutAdjWall(unsigned short i) {		//��һ��λ�õ�ǽ����ǽ�����б�
	if (i < 0 || i>840) return;
	if (i == 0) {
		Wall.push_back(wallInfo(1,i));
		Wall.push_back(wallInfo(29,i));
		return;
	}
	else if (i == 28) {
		Wall.push_back(wallInfo(27,i));
		Wall.push_back(wallInfo(57,i));
		return;
	}
	else if (i == 812) {
		Wall.push_back(wallInfo(783, i));
		Wall.push_back(wallInfo(813, i));
		return;
	}
	else if (i == 840) {
		Wall.push_back(wallInfo(839, i));
		Wall.push_back(wallInfo(811, i));
		return;
	}
	else if (0 < i && i < 28) {
		Wall.push_back(wallInfo(i + 29,i));
		Wall.push_back(wallInfo(i - 1,i));
		Wall.push_back(wallInfo(i + 1,i));
		return;
	}
	else if (812 < i && i < 840) {
		Wall.push_back(wallInfo(i - 1, i));
		Wall.push_back(wallInfo(i + 1, i));
		Wall.push_back(wallInfo(i - 29, i));
		return;
	}
	else if (i % 29 == 0) {
		Wall.push_back(wallInfo(i - 29, i));
		Wall.push_back(wallInfo(i + 1, i));
		Wall.push_back(wallInfo(i + 29, i));
		return;
	}
	else if (i % 29 == 28) {
		Wall.push_back(wallInfo(i - 29, i));
		Wall.push_back(wallInfo(i - 1, i));
		Wall.push_back(wallInfo(i + 29, i));
		return;
	}
	Wall.push_back(wallInfo(i - 29, i));
	Wall.push_back(wallInfo(i + 29, i));
	Wall.push_back(wallInfo(i - 1, i));
	Wall.push_back(wallInfo(i + 1, i));
	return;
}
void MazeCreate() {
	showPath = false;
	firstFind = true;
	int times = 0;
	unsigned short Maze[841];
	unsigned short temp;
	unsigned short pos,source,opposite;
	for (int i = 0; i < 29; i++) {
		for (int j = 0; j < 29; j++) {
			SysMaze[i][j] = 0;
		}
	}
	for (int i = 0; i < 841; i++) {
		Maze[i] = 0;
	}
	//���ѡһ��������Ϊ��ʼ���ĵ�·
	Maze[72] = 1;
	PutAdjWall(72);

	while (!Wall.empty()) {
		times++;
		if (times % 4 == 0) { temp = random(0, Wall.size() / 4); }
		else if (times % 4 == 1) { temp = random(Wall.size() / 4, Wall.size() / 2); }
		else if (times % 4 == 2) { temp = random(Wall.size() / 2, Wall.size() * 3 / 4); }
		else { temp = random(Wall.size()*3 / 4, Wall.size() ); }

		pos = Wall[temp].first;
		source = Wall[temp].second;
		opposite = pos - (source - pos);
		if (Maze[pos] == 1) {		//���������·������Ϊ��ǽ�Ž�ȥ�� 
			Wall.erase(Wall.begin() + temp);
			continue;
		}
		if (isOuter(pos)) {		//���pos�����ǽ���Ͳ�����Maze[opposite],��Ϊ��Խ��,ֻ��pos��Ϊͨ·���ɡ�
			if (random(0, 5) == 1) 
				Maze[pos] = 1;
			Wall.erase(Wall.begin() + temp);
			continue;
		}
		if (Maze[opposite] == 1) {		//�������ĸ������Թ���ͨ·
			Wall.erase(Wall.begin() + temp);
			continue;
		}
		Maze[pos] = 1;
		Wall.erase(Wall.begin() + temp);
		Maze[opposite] = 1;
		PutAdjWall(opposite);
	}
	for (int i = 0; i < 29; i++) {
		for (int j = 0; j < 29; j++) {
			SysMaze[i][j] = Maze[29 * i + j];
		}
	}
	SysMaze[0][0] = 1;
	SysMaze[0][1] = 1;
	SysMaze[28][28] = 1;
	SysMaze[28][27] = 1;
	SysMaze[0][27] = 1;
	SysMaze[0][28] = 1;
	SysMaze[28][0] = 1;
	SysMaze[27][0] = 1;
//	printMaze();
}

void init() {	//OpenGL init + Maze init
	//openGL init
	glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE);
	glutInitWindowPosition(280, 100);
	glutInitWindowSize(820, 620);
	glutCreateWindow("Maze");
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-410, 410, -310, 310, -1, 1);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	//Maze init
	srand(unsigned(time(NULL)));
	MazeCreate();
	for (int i = 0; i < 29; i++) {
		for (int j = 0; j < 29; j++) {
			UserMaze[i][j] = 0;
		}
	}
	UserMaze[0][0] = 1;
	UserMaze[28][28] = 1;
	//Texture init
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	backGround = LoadTexture(".\\Background.bmp");
	texWall = LoadTexture(".\\wallsmall.bmp");
	texGrass = LoadTexture(".\\grasssmall.bmp");
	texMouse = LoadTexture(".\\mouse.bmp");
	texFood = LoadTexture(".\\rice.bmp");
}

void drawMainPage() {
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0, 1.0, 1.0);
	glBindTexture(GL_TEXTURE_2D, backGround);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);	glVertex2f(-360, -210);
	glTexCoord2f(1.0, 0.0); glVertex2f(360, -210);
	glTexCoord2f(1.0, 1.0); glVertex2f(360, 310);
	glTexCoord2f(0.0, 1.0);	glVertex2f(-360, 310);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_LINES);
	//��һ����ť����ʼ��Ϸ
	glVertex2f(-100, -20);	glVertex2f(100, -20); //��	
	glVertex2f(100, -20);	glVertex2f(100, -70); //��
	glVertex2f(100, -70);	glVertex2f(-100, -70);//��
	glVertex2f(-100, -70);	glVertex2f(-100, -20);//��
	//�ڶ�����ť���༭�Թ�
	glVertex2f(-100, -130);	glVertex2f(100, -130);
	glVertex2f(100, -130);	glVertex2f(100, -180);
	glVertex2f(100, -180);	glVertex2f(-100, -180);
	glVertex2f(-100, -180);	glVertex2f(-100, -130);
	//��������ť��������Ϣ
	glVertex2f(-100, -240);	glVertex2f(100, -240);
	glVertex2f(100, -240);	glVertex2f(100, -290);
	glVertex2f(100, -290);	glVertex2f(-100, -290);
	glVertex2f(-100, -290);	glVertex2f(-100, -240);
	glEnd();
	glRasterPos2f(-80, -50);
	drawString("Start Game",32);
	glRasterPos2f(-80, -160);
	drawString("Edit Maze", 32);
	glRasterPos2f(-80, -270);
	drawString("Author info", 32);
}

void drawSolidTriangle(int row, int col) {
	glColor3f(1.0, 0.5, 0.5);
	glBegin(GL_TRIANGLES);
	glVertex2f(20 * col - 280, 287 - 20 * row);//��
	glVertex2f(20 * col - 287, 273 - 20 * row);//����
	glVertex2f(20 * col - 273, 273 - 20 * row);//����
	glEnd();
}
void drawSolidSquare(int row, int col,int state) {
	if (state == 1) {
		glColor3f(1.0, 1.0, 1.0);
	}
	else if (state == 0) {
		glColor3f(0.0, 0.0, 0.0);
	}
	glBegin(GL_QUADS);
	glVertex2f(20 * col - 290, 290 - 20 * row);//����
	glVertex2f(20 * col - 270, 290 - 20 * row);//����
	glVertex2f(20 * col - 270, 270 - 20 * row);//����
	glVertex2f(20 * col - 290, 270 - 20 * row);//����
	glEnd();
}
void drawTexSquare(int row, int col, int state) {

	if (state == 0) {
		glBindTexture(GL_TEXTURE_2D, texWall);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, texGrass);
	}

	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 1.0f);	glVertex2f(20 * col - 290, 290 - 20 * row);//����
	glTexCoord2f(1.0f, 1.0f);	glVertex2f(20 * col - 270, 290 - 20 * row);//����
	glTexCoord2f(1.0f, 0.0f);	glVertex2f(20 * col - 270, 270 - 20 * row);//����
	glTexCoord2f(0.0f, 0.0f);	glVertex2f(20 * col - 290, 270 - 20 * row);//����
	glEnd();
}
void drawMaze() {
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	unsigned short state;
	for (int i = 0; i < 29; i++) {
		for (int j = 0; j < 29; j++) {
			state = SysMaze[i][j];
			drawTexSquare(i, j,state);
		}
	}
	// �����ıߵ�ǽ 
	for (int i = 0; i < 29; i++) {
		drawTexSquare(-1, i, 0);
		drawTexSquare(29, i, 0);
	}
	for (int i = -1; i <= 29; i++) {
		drawTexSquare(i, -1, 0);
		drawTexSquare(i, 29, 0);
	}
	glDisable(GL_TEXTURE_2D);
}
void gameTimer(int id) {
	if (windowState == 1 && restTime > 0) {
		restTime -= 0.1;
	}
	glutPostRedisplay();
	glutTimerFunc(100, gameTimer, 1);
}
void drawTime() {
	glColor3f(0.0, 0.0, 0.0);
	glRasterPos2f(330, 280);
	drawString("Time", 32);
	float timeH = restTime  / GAME_TIME * 500;	//restTime / 20 * 500;
	glColor3f(0.3, 0.3, 0.9);
	glBegin(GL_QUADS);
	glVertex2f(340, -250);
	glVertex2f(380, -250);
	glVertex2f(380, timeH - 250);
	glVertex2f(340, timeH - 250);
	glEnd();
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_LINES);
	glVertex2f(340, 250);	glVertex2f(380, 250);
	glVertex2f(380, 250);	glVertex2f(380, -250);
	glVertex2f(380, -250);	glVertex2f(340, -250);
	glVertex2f(340, -250);	glVertex2f(340, 250);
	glEnd();
}
void drawPlayButton() {	//����Read Quit Restart Find_Path �ĸ���ť
	glColor3f(0.0, 0.0, 0.0);
	glRasterPos2f(-400, -40);
	drawString(" Read", 32);
	glRasterPos2f(-400, -120);
	drawString("  Cue", 32);
	glRasterPos2f(-400, -200);
	drawString("Restart", 32);
	glRasterPos2f(-400, -280);
	drawString(" Quit", 32);
	glBegin(GL_LINES);
	//Read Maze
	glVertex2f(-400, -10);	glVertex2f(-320, -10);
	glVertex2f(-320, -10);	glVertex2f(-320, -50);
	glVertex2f(-320, -50);	glVertex2f(-400, -50);
	glVertex2f(-400, -50);	glVertex2f(-400, -10);
	//Find_Path
	glVertex2f(-400,-90);	glVertex2f(-320,-90);
	glVertex2f(-320,-90);	glVertex2f(-320,-130);
	glVertex2f(-320,-130);	glVertex2f(-400,-130);
	glVertex2f(-400,-130);	glVertex2f(-400,-90);
	//Restart
	glVertex2f(-400,-170);	glVertex2f(-320,-170);
	glVertex2f(-320,-170);	glVertex2f(-320,-210);
	glVertex2f(-320,-210);	glVertex2f(-400,-210);
	glVertex2f(-400,-210);	glVertex2f(-400,-170);
	//Quit
	glVertex2f(-400,-250);	glVertex2f(-320,-250);
	glVertex2f(-320,-250);	glVertex2f(-320,-290);
	glVertex2f(-320,-290);	glVertex2f(-400,-290);
	glVertex2f(-400,-290);	glVertex2f(-400,-250);
	glEnd();
}

//Ѱ·�㷨��Ҫ�õ���һЩ����
int dx[4] = { 0,1,0,-1};		int dy[4] = { -1,0,1,0 };
int maxAct = 4;//ÿһλ�õ�����ƶ�����
int table[29][29];	//��¼·���ı� ·����¼Ϊ1
int block[29][29];	//��¼�Թ���Ŀ�ı�
int level;	//�ڼ���
int levelComplete;	//��һ���������Ƿ��Ѿ����
int allComplete;	//ȫ�������Ƿ��Ѿ����
int Act[MAX_LEVEL];	//ÿһ�����ƶ�����
int x, y;	//��ǰѰ·������
int targetX = 28, targetY = 28;	//�յ������

void test() {
	if (x == targetX && y == targetY) {
		levelComplete = 1;
		allComplete = 1;
	}
}
int actOK() {		//�ж��ƶ��Ƿ����
	int nextX = x + dx[Act[level] - 1];
	int nextY = y + dy[Act[level] - 1];
	if (Act[level] > maxAct)	return 0;	//���з���̽��
	if (nextX < 0 || nextX > 28 || nextY < 0 || nextY > 28)	return 0;	//Խ��
	if (table[nextY][nextX] == 1) return 0; //�Ѿ������
	if (block[nextY][nextX] == 0) return 0; //ײǽ
	//�ƶ�
	x = nextX;
	y = nextY;
	table[nextY][nextX] = 1;
	return 1;
}
void back() {
	table[y][x] = 0;
	if (level) {
		x -= dx[Act[level - 1] - 1];
		y -= dy[Act[level - 1] - 1];
	}
	Act[level] = 0;
	level--;
}
void drawPath() {
	if (showPath == true) {
		for (int i = 0; i < 29; i++) {
			for (int j = 0; j < 29; j++) {
				if (table[i][j] == 1)
					drawSolidTriangle(i, j);
			}
		}
	}
}
void printPath()
{
	cout << "The path is " << endl;
	for (int i = 0; i < 29; ++i)
	{
		for (int j = 0; j < 29; ++j)
		{
			/*cout << table[i][j] << " ";*/
			if (table[i][j])
				cout << "1 ";
			else cout << "  ";
		}
		cout << endl;
	}
}
bool findPath() {
	cout << "I am finding a path!" << endl;
	//��һ���������ĳ�ʼ������
	x = 0; y = 0;
	level = -1;
	levelComplete = 0;
	allComplete = 0;
	for (int i = 0; i < MAX_LEVEL; i++) {
		Act[i] = 0;
	}
	for (int i = 0; i < 29; i++) {
		for (int j = 0; j < 29; j++) {
			table[i][j] = 0;
		}
	}
	//�ڶ������õ���ǰ��ҪѰ��·����ͼ
	if (windowState == 4) //�Թ��༭ģʽ
	{
		for (int i = 0; i < 29; i++) {
			for (int j = 0; j < 29; j++) {
				block[i][j] = UserMaze[i][j];
			}
		}
	}
	else if (windowState == 1 || windowState == 2 || windowState == 3) {	//��ϵͳ�ṩ���Թ�
		for (int i = 0; i < 29; i++) {
			for (int j = 0; j < 29; j++) {
				block[i][j] = SysMaze[i][j];
			}
		}
	}
	//����������ʼѰ·
	table[y][x] = 1;
	while (!allComplete) {
		level++;	//next level
		levelComplete = 0; //��һ������δ���
		while (!levelComplete) {
			Act[level]++;	//�ı��ƶ�����
			if (actOK()) {
				test();
				levelComplete = 1;
			}
			else {
				if (Act[level] > maxAct) { back(); }
				if (level < 0)	//���еط����ѹ��ˣ�û��������Ŀ��
				{
					levelComplete = allComplete = 1;//�˳�
				}
			}
		}
	}
	showPath = true;
	printPath();
	for (int i = 0; i < 29; ++i)
	{
		for (int j = 0; j < 29; ++j)
		{
			if (table[i][j] == 1)
				return true;
		}
	}
	return false;
	
}


void drawSaveRead(){
	char buffer[256];
	ifstream in;
	string files[10] = {"File 01: ","File 02: " ,"File 03: " ,"File 04: " ,"File 05: " ,
		"File 06: " ,"File 07: " ,"File 08: " ,"File 09: " ,"File 10: " };
	for (int i = 0; i < 10; i++) {
		in.open(fileNames[i]);
		in.getline(buffer, 100);	// Read First Line
		files[i] += buffer;
		in.close();
	}
	glColor3f(0.0,0.0,0.0);
	for (int j = 0; j < 2; j ++) {
		for (int i = 0; i < 5; i++) {
			glRasterPos2f(390 * j - 340, 215 - 110 * i);
			const char *p = files[j * 5 + i].c_str();
			drawString(p , 32);
		}
	}
	glRasterPos2f(320, -280);
	drawString("back", 32);
	glBegin(GL_LINES);
	for (int i = 0; i < 5; i++) {
		glVertex2f(40, 245 - 110 * i);	glVertex2f(350, 245 - 110 * i);
		glVertex2f(-40, 245 - 110 * i);	glVertex2f(-350, 245 - 110 * i);
		glVertex2f(40, 195 - 110 * i);  glVertex2f(350, 195 - 110 * i);
		glVertex2f(-40, 195 - 110 * i);  glVertex2f(-350, 195 - 110 * i);
		glVertex2f(40, 245 - 110 * i);	glVertex2f(40, 195 - 110 * i);
		glVertex2f(-40, 245 - 110 * i);	glVertex2f(-40, 195 - 110 * i);
		glVertex2f(350, 245 - 110 * i);	glVertex2f(350, 195 - 110 * i);
		glVertex2f(-350, 245 - 110 * i);	glVertex2f(-350, 195 - 110 * i);
	}
	glVertex2f(310, -250);	glVertex2f(390, -250);
	glVertex2f(310, -300);	glVertex2f(390, -300);
	glVertex2f(310, -250);	glVertex2f(310, -300);
	glVertex2f(390, -250);	glVertex2f(390, -300);
	glEnd();
}


void drawPlayer(short playerx,short playery) {
	glColor3f(1.0, 1.0, 1.0);
	float x = 20 * playerx - 280;
	float y = 280 - 20 * playery;
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texMouse);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);	glVertex3f(x - 10, y - 10, 1.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(x - 10, y + 10, 1.0);
	glTexCoord2f(1.0, 1.0);	glVertex3f(x + 10, y + 10, 1.0);
	glTexCoord2f(1.0, 0.0);	glVertex3f(x + 10, y - 10, 1.0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	if (playerx == 28 && playery == 28) {	//���ʤ��
		windowState = 2;
	}
/*
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(x, y);
	for (int count = 0; count <= 100 ; count++) {
		glVertex2f(x + 9*cos(count * TwoPI / 100), y + 9*sin(count * TwoPI / 100));
	}
	glEnd();
	if (playerx == 28 && playery == 28) {	//���ʤ��
		windowState = 2;
	}
*/
}
void drawDest() {
	glColor3f(1.0, 1.0, 1.0);
	float x = 20 * 28 - 280;
	float y = 280 - 20 * 28;
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texFood);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);	glVertex3f(x - 10, y - 10, 1.0);
	glTexCoord2f(0.0, 1.0);	glVertex3f(x - 10, y + 10, 1.0);
	glTexCoord2f(1.0, 1.0);	glVertex3f(x + 10, y + 10, 1.0);
	glTexCoord2f(1.0, 0.0);	glVertex3f(x + 10, y - 10, 1.0);
	glEnd();
	glDisable(GL_TEXTURE_2D);

/*
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(x, y);
	for (int count = 0; count <= 100; count++) {
		glVertex2f(x + 9 * cos(count * TwoPI / 100), y + 9 * sin(count * TwoPI / 100));
	}
	glEnd();
*/
}


void drawUserEdit() {
	unsigned short state;
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0, 1.0, 1.0);
	for (int i = 0; i < 29; i++) {
		drawTexSquare(-1, i, 0);
		drawTexSquare(29, i, 0);
	}
	for (int i = -1; i <= 29; i++) {
		drawTexSquare(i, -1, 0);
		drawTexSquare(i, 29, 0);
	}
	for (int i = 0; i < 29; i++) {
		for (int j = 0; j < 29; j++) {
			state = UserMaze[i][j];
			drawTexSquare(i, j, state);
		}
	}
	glDisable(GL_TEXTURE_2D);
	drawDest();
	drawPlayer(0, 0);
}
void drawEditButton() {
	glColor3f(0.0, 0.0, 0.0);
	glRasterPos2f(-400, -120);
	drawString(" Save", 32);
	glRasterPos2f(-400, -200);
	if (EditMode == 0)
	{
		drawString(" Wall", 32);
	}
	else if (EditMode == 1)
	{
		drawString(" Path", 32);
	}
	glRasterPos2f(-400, -280);
	drawString(" Quit", 32);
	glBegin(GL_LINES);
	//Find_Path
	glVertex2f(-400, -90);	glVertex2f(-320, -90);
	glVertex2f(-320, -90);	glVertex2f(-320, -130);
	glVertex2f(-320, -130);	glVertex2f(-400, -130);
	glVertex2f(-400, -130);	glVertex2f(-400, -90);
	//Restart
	glVertex2f(-400, -170);	glVertex2f(-320, -170);
	glVertex2f(-320, -170);	glVertex2f(-320, -210);
	glVertex2f(-320, -210);	glVertex2f(-400, -210);
	glVertex2f(-400, -210);	glVertex2f(-400, -170);
	//Quit
	glVertex2f(-400, -250);	glVertex2f(-320, -250);
	glVertex2f(-320, -250);	glVertex2f(-320, -290);
	glVertex2f(-320, -290);	glVertex2f(-400, -290);
	glVertex2f(-400, -290);	glVertex2f(-400, -250);
	glEnd();
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	switch (windowState)
	{
	case 0:	//Choice Page
		drawMainPage();
		break;
	case 1:	//Play game
		drawMaze();
		drawPlayer(playerx, playery);
		drawDest();
		drawTime();
		drawPlayButton();
		drawPath();
		if (restTime <= 0) {
			windowState = 3;
		}
		break;
	case 2://Player win
		drawMaze();
		drawDest();
		drawTime();
		drawPlayButton();
		drawPath();
		glColor3f(1.0, 0.0, 0.0);
		glRasterPos2f(-170, 0);
		drawString("You Win", 128);
		break;
	case 3://Player lose
		drawMaze();
		drawPlayer(playerx,playery);
		drawDest();
		drawTime();
		drawPlayButton();
		drawPath();
		glColor3f(1.0, 0.0, 0.0);
		glRasterPos2f(-170, 0);
		drawString("You Lose", 128);
		break;
	case 4://Edit Maze
		drawUserEdit();
		drawEditButton();
		break;
	case 5: // Maze I/O
		drawSaveRead();
		break;
	default:
		break;
	}
	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
	if(key == 27)
		exit(0);
	if (windowState == 1) {
		switch (key) {
		case 'a':
			playerx--;
			if (playerx < 0 || SysMaze[playery][playerx] == 0) playerx++;
			break;
		case 'd':
			playerx++;
			if (playerx > 28 || SysMaze[playery][playerx] == 0) playerx--;
			break;
		case 'w':
			playery--;
			if (playery < 0 || SysMaze[playery][playerx] == 0) playery++;
			break;
		case 's':
			playery++;
			if (playery > 28 || SysMaze[playery][playerx] == 0) playery--;
			break;
		}
	}
	glutPostRedisplay();
}

void special(int key, int x, int y) {
	if (windowState == 1) {
		switch (key) {
		case GLUT_KEY_LEFT:
			playerx--;
			if (playerx < 0 || SysMaze[playery][playerx] == 0) playerx++;
			break;
		case GLUT_KEY_RIGHT:
			playerx++;
			if (playerx > 28 || SysMaze[playery][playerx] == 0) playerx--;
			break;
		case GLUT_KEY_UP:
			playery--;
			if (playery < 0 || SysMaze[playery][playerx] == 0) playery++;
			break;
		case GLUT_KEY_DOWN:
			playery++;
			if (playery > 28 || SysMaze[playery][playerx] == 0) playery--;
			break;
		}
	}
	glutPostRedisplay();
}

void mouse(int btn, int state, int x, int y) {
	x = x - 410;
	y = 310 - y;
	int row, col;
	ifstream in;
	ofstream out;
	char buffer[256];
	switch (windowState)
	{
	case 0:
		if (x > -100 && x<100 && y>-70 && y < -20) {//Play
			windowState = 1;
			restTime = GAME_TIME;
			MazeCreate();
			playerx = 0;
			playery = 0;
			Save_Read = 0;
			glutTimerFunc(100, gameTimer, 1);
		}
		else if (x > -100 && x<100 && y>-180 && y<-130) {//Edit
			Save_Read = 1;
			for (int i = 0; i < 29; i++) {
				for (int j = 0; j < 29; j++) {
					UserMaze[i][j] = 0;
				}
			}
			UserMaze[0][0] = 1;
			UserMaze[28][28] = 1;
			windowState = 4;
		}
		else if (x > -100 && x<100 && y>-290 && y < -240 && state == GLUT_DOWN) {
			MessageBoxA(0, "  ����������Ϊ\n15����ý�� ¬���\n  201500302013", "������Ϣ", 0);
		}
		break;
	case 1:
	case 2:
	case 3:
		if (x > -390 && x < -310) {
			if (y<-90 && y>-130) {	//Cue
				if (firstFind) {
					findPath();
					firstFind = false;
				}
				if(windowState == 1)
					windowState = 3;
			}
			else if (y<-170 && y>-210 && state == GLUT_DOWN) {	//Restart
				MazeCreate();
				playerx = 0;
				playery = 0;
				windowState = 1;
				restTime = GAME_TIME;
				if (windowState == 2) glutTimerFunc(100, gameTimer, 1);
			}
			else if (y>-290 && y<-250) {	//Quit
				showPath = false;
				windowState = 0;
				restTime = 0;
			}
			else if (y > -50 && y < -10) {	//Read
				showPath = false;
				windowState = 5;
			}
		}
		break;
	case 4:
		row = (290 - y) / 20;
		col = (x + 290) / 20;
		if (row > -1 && row < 29 && col > -1 && col < 29 ) {
			if (row == 0 && col == 0) break;
			if (row == 28 && col == 28)break;
			if(EditMode == 1)
			UserMaze[row][col] = 1;
			else UserMaze[row][col] = 0;
		}
		if (x > -390 && x < -310) {
			if (y<-90 && y>-130 && state == GLUT_DOWN) {	//Save Maze
				//MessageBoxA(0(ϵͳ����),��Ϣ����,����,�Ի�������);
				if (findPath()) {
					MessageBoxA(0, "�����Թ����Դ洢��", "��֤ͨ·�ɹ�", 0);
					windowState = 5;
				}
				else {
					MessageBoxA(0, "�����Թ�û��ͨ·��", "��֤ͨ·ʧ��", 1);
				}
			}
			else if (y<-170 && y>-210 && state == GLUT_UP) {	//Change Mode
				if (EditMode == 1) EditMode = 0;
				else EditMode = 1;
			}
			else if (y>-290 && y<-250) {	//Quit
				for (int i = 0; i < 29; i++) {
					for (int j = 0; j < 29; j++) {
						UserMaze[i][j] = 0;
					}
				}
				windowState = 0;
			}
		}
		break;
	case 5:
		if (x > 310 && x<390 && y>-300 && y < -250 && state == GLUT_DOWN) {
			if (Save_Read == 0) {
				MazeCreate();
				playerx = 0;
				playery = 0;
				windowState = 1;
				restTime = GAME_TIME;
			}
			else {
				for (int i = 0; i < 29; i++) {
					for (int j = 0; j < 29; j++) {
						UserMaze[i][j] = 0;
					}
				}
				UserMaze[0][0] = 1;
				UserMaze[28][28] = 1;
				windowState = 4;
				break;
			}
		}
		if (Save_Read == 0)//read 
		{	
//			char cc;
			if (x > 40 && x < 350) {
				for (int i = 0; i < 5; i++) {
					if (y < 245 - 110 * i && y>195 - 110 * i && state == GLUT_DOWN) {
						in.open(fileNames[5+i]);
						in.getline(buffer, 100);
						if (buffer[0] == 'E') {
							MessageBoxA(0, "���ڶ�ȡһ���մ浵��", "��ȡʧ��", 0);
						}
						else {
							in.getline(buffer, 100);
							int result = MessageBox(NULL, TEXT("�Ƿ�ȷ�϶�ȡ�浵��"), TEXT("ȷ��"), MB_ICONINFORMATION | MB_YESNO);
							switch (result)/*ע�⣡ʹ��UnicodeӦ��TEXT��Χ�ִ�*/
							{
							case IDYES:
								for (int i = 0; i < 29; i++) {
									in.getline(buffer, 100);
									for (int j = 0; j < 29; j++) {
										char cc = buffer[j];
										if (cc == '0') { SysMaze[i][j] = 0; }
										else if (cc == '1') { SysMaze[i][j] = 1; }
									}
								}
								playerx = 0;
								playery = 0;
								windowState = 1;
								firstFind = true;
								restTime = GAME_TIME;
								break;
							case IDNO:
								break;
							}
						}
					}
				}
			}
			else if (x > -350 && x < -40) {
				for (int i = 0; i < 5; i++) {
					if (y < 245 - 110 * i && y>195 - 110 * i && state == GLUT_DOWN) {
						in.open(fileNames[i]);
						in.getline(buffer, 100);
						if (buffer[0] == 'E') {
							MessageBoxA(0, "���ڶ�ȡһ���մ浵��", "��ȡʧ��", 0);
						}
						else {
							in.getline(buffer, 100);
							int result = MessageBox(NULL, TEXT("�Ƿ�ȷ�϶�ȡ�浵��"), TEXT("ȷ��"), MB_ICONINFORMATION | MB_YESNO);
							switch (result)/*ע�⣡ʹ��UnicodeӦ��TEXT��Χ�ִ�*/
							{
							case IDYES:
								for (int i = 0; i < 29; i++) {
									in.getline(buffer, 100);
									
									for (int j = 0; j < 29; j++) {
										char cc = buffer[j];
										cout << cc << " ";
										if (cc == '0') { SysMaze[i][j] = 0; }
										else if (cc == '1') { SysMaze[i][j] = 1; }
									}
									cout <<buffer << endl;
								}
								playerx = 0;
								playery = 0;
								windowState = 1;
								restTime = GAME_TIME;
								firstFind = true;
								break;
							case IDNO:
								break;
							}
						}
					}
				}
			}
		}
		else //write
		{
			if (x > 40 && x < 350) {
				for (int i = 0; i < 5; i++) {
					if (y < 245 - 110 * i && y>195 - 110 * i && state == GLUT_DOWN) {
						int result = MessageBox(NULL, TEXT("�Ƿ�ȷ��д��浵��"), TEXT("ȷ��"), MB_ICONINFORMATION | MB_YESNO);
						time_t t;
						switch (result)/*ע�⣡ʹ��UnicodeӦ��TEXT��Χ�ִ�*/
						{
						case IDYES:
							out.open(fileNames[5 + i]);
							t = time(0);
							char tmp[64];
							strftime(tmp, sizeof(tmp), "%Y/%m/%d %X \n", localtime(&t));
							out << tmp << endl;
							for (int i = 0; i < 29; i++) {
								for (int j = 0; j < 29; j++) {
									out << UserMaze[i][j];
								}
								out << endl;
							}
							MessageBoxA(0, "д��ɹ���", "OK", 0);
							for (int i = 0; i < 29; i++) {
								for (int j = 0; j < 29; j++) {
									UserMaze[i][j] = 0;
								}
							}
							UserMaze[0][0] = 1;
							UserMaze[28][28] = 1;
							break;
						case IDNO:
							break;
						}
					}
				}
			}
			else if (x > -350 && x < -40) {
				for (int i = 0; i < 5; i++) {
					if (y < 245 - 110 * i && y>195 - 110 * i && state == GLUT_DOWN) {
						int result = MessageBox(NULL, TEXT("�Ƿ�ȷ��д��浵��"), TEXT("ȷ��"), MB_ICONINFORMATION | MB_YESNO);
						time_t t;
						switch (result)/*ע�⣡ʹ��UnicodeӦ��TEXT��Χ�ִ�*/
						{
						case IDYES:
							out.open(fileNames[i]);
							t = time(0);
							char tmp[64];
							strftime(tmp, sizeof(tmp), "%Y/%m/%d %X \n", localtime(&t));
							out << tmp << endl;
							for (int i = 0; i < 29; i++) {
								for (int j = 0; j < 29; j++) {
									out << UserMaze[i][j];
								}
								out << endl;
							}
							MessageBoxA(0, "д��ɹ���", "OK", 0);
							for (int i = 0; i < 29; i++) {
								for (int j = 0; j < 29; j++) {
									UserMaze[i][j] = 0;
								}
							}
							break;
						case IDNO:
							break;
						}
					}
				}
			}
		}
		break;
	default:
		break;
	}
	glutPostRedisplay();
}




int main(int argv, char** argc) {
	glutInit(&argv, argc);
	init();
	glutDisplayFunc(display);
	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMainLoop();
}

