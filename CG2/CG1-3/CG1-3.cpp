#include<glut.h>
#include<stdio.h>
#include<iostream>

int* pointXYArray;
int pointNumber;

struct Edge{
	int xmin;//y最小点的x值，光栅坐标用于显示
	int ymax;//y最大点的y值
	float m;//斜率的倒数
	float xmath;//数学坐标用于计算
	int ymin;//y最小点的y值
};

struct EdgeNode{
	Edge edge;
	EdgeNode* next;
};

int Round(float x)//四舍五入
{
	if (x - (int)x >= 0.5){
		return (int)x + 1;
	}
	else{
		return (int)x;
	}
}

void FillPolygon(const int* PointXYArray, int PointNumber)
{
	Edge *E = new Edge[PointNumber];
	EdgeNode *AEL = 0;
	EdgeNode **ET = 0;
	int AELcount = 0;
	int Ecount = 0;
	int yMin = PointXYArray[1];//扫描的下界
	int yMax = PointXYArray[1];//扫描的上界
	//储存边的信息
	for (int i = 0; i < PointNumber; i++){//第i个点坐标在2*i,2*i+1,对应第i条边的起点
		//忽略水平线
		if (PointXYArray[2 * i + 1] != PointXYArray[(2 * i + 3) % (2 * PointNumber)]){//第i个点和第i+1个点如果不是水平线
			if (PointXYArray[2 * i + 1] <= PointXYArray[(2 * i + 3) % (2 * PointNumber)]){//如果第i个点的y值小于等于第i+1个点的y值
				E[Ecount].xmin = PointXYArray[2 * i];
				E[Ecount].ymin = PointXYArray[2 * i + 1];
				E[Ecount].ymax = PointXYArray[(2 * i + 3) % (2 * PointNumber)];
			}
			else{
				E[Ecount].xmin = PointXYArray[(2 * i + 2) % (2 * PointNumber)];
				E[Ecount].ymin = PointXYArray[(2 * i + 3) % (2 * PointNumber)];
				E[Ecount].ymax = PointXYArray[2 * i + 1];
			}
			E[Ecount].m = ((float)PointXYArray[2 * i] - (float)PointXYArray[(2 * i + 2) % (2 * PointNumber)]) / ((float)PointXYArray[2 * i + 1] - (float)PointXYArray[(2 * i + 3) % (2 * PointNumber)]);
			E[Ecount].xmath = E[Ecount].xmin;
			Ecount++;
		}
		//确定所有顶点中最大的y值和最小的y值
		if (yMax < PointXYArray[2 * i + 1]){
			yMax = PointXYArray[2 * i + 1];
		}
		if (yMin > PointXYArray[2 * i + 1]){
			yMin = PointXYArray[2 * i + 1];
		}
	}
	//预处理共享顶点且分布两侧的两条边
	for (int c = 0; c < Ecount; c++){
		if (E[c].ymin == E[(c + 1) % Ecount].ymax){
			E[c].xmath += E[c].m;
			E[c].xmin = Round(E[c].xmath);
			E[c].ymin++;
		}
		else if (E[c].ymax == E[(c + 1) % Ecount].ymin){
			E[c].ymax--;
		}
	}
	//printf("yMin,yMax = %d,%d\n", yMin, yMax);
	/*for (int h = 0; h < PointNumber; h++){
	printf("n,xmin,ymin,ymax,m = %d,%d,%d,%d,%f\n", h, E[h].xmin, E[h].ymin, E[h].ymax, E[h].m);
	}*/
	//初始化ET
	ET = new EdgeNode*[yMax - yMin + 1];//把ET从0到yMax - yMin映射从yMin到yMax
	for (int i = 0; i < yMax - yMin + 1; i++){
		ET[i] = 0;
	}
	for (int i = 0; i < Ecount; i++){
		if (ET[E[i].ymin - yMin] == 0){
			ET[E[i].ymin - yMin] = new EdgeNode();
			ET[E[i].ymin - yMin]->edge = E[i];
			ET[E[i].ymin - yMin]->next = 0;
		}
		else{
			EdgeNode*p = ET[E[i].ymin - yMin];
			while (p->next != 0){ p = p->next; }
			p->next = new EdgeNode();
			p->next->edge = E[i];
			p->next->next = 0;
		}
	}
	/*//打印ET以便debug
	EdgeNode **ptemp = ET;
	EdgeNode *qtemp =0;
	for (int v = 0; v < yMax - yMin + 1; v++){
	qtemp = ptemp[v];
	while (qtemp){
	printf("y,xmin,ymin,ymax,m:%d,%d,%d,%d,%f\n", v, qtemp->edge.xmin, qtemp->edge.ymin, qtemp->edge.ymax, qtemp->edge.m);
	qtemp = qtemp->next;
	}
	}
	*/
	//扫描线循环
	for (int i = yMin; i <= yMax; i++){
		//加边
		EdgeNode*p = ET[i - yMin];
		while (p){
			if (AEL){
				EdgeNode*q = AEL;
				while (q->next){ q = q->next; }
				q->next = new EdgeNode;
				q->next->edge = p->edge;
				q->next->next = 0;
			}
			else{
				AEL = new EdgeNode;
				AEL->edge = p->edge;
				AEL->next = 0;
			}
			AELcount++;
			p = p->next;
		}
		//排序
		int* xPixel = new int[AELcount];
		EdgeNode* m = AEL;
		int n = 0;
		while (m != 0)//收回xmin
		{
			xPixel[n] = m->edge.xmin;
			m = m->next;
			n++;
		}
		for (int a = 0; a < AELcount; a++){//对xmin排序
			for (int b = a + 1; b < AELcount; b++){
				if (xPixel[a]>xPixel[b]){
					int temp = xPixel[a];
					xPixel[a] = xPixel[b];
					xPixel[b] = temp;
				}
			}
		}
		//printf("y,AELcount:%d,%d\n", i, AELcount);
		/*
		for (int xt = 0; xt < AELcount; xt++){
		printf("xPixel[%d]:%d\n", xt, xPixel[xt]);
		}*/
		//画边
		if (AEL){
			int start = 0;
			int end = 1;
			while (start < AELcount&&end < AELcount){
				int temp = xPixel[start];
				while (temp <= xPixel[end]){
					glBegin(GL_POINTS);
					glVertex3f(temp, i, 0);
					glEnd();
					temp++;
				}
				start += 2;
				end += 2;
			}
		}
		else{
			printf("AEL is empty\n");
		}
		//删边
		EdgeNode* k = AEL;
		EdgeNode* l = 0;
		while (k){
			l = k->next;
			if (l){
				if (l->edge.ymax <= i){
					k->next = l->next;
					delete l;//先删头结点之后的
					AELcount--;
				}
			}
			k = k->next;
		}
		k = AEL;
		if (k){
			if (k->edge.ymax <= i){
				AEL = k->next;
				delete k;//再删头结点
				AELcount--;
			}
		}
		//改xin
		EdgeNode* c = AEL;
		while (c){
			c->edge.xmath += c->edge.m;
			c->edge.xmin = Round(c->edge.xmath);
			c = c->next;
		}
	}
	//删除ET
	EdgeNode*r = 0;
	EdgeNode*o = 0;
	for (int i = 0; i < yMax - yMin + 1; i++){
		r = ET[i];
		while (r){
			o = r;
			r = r->next;
			delete o;
		}
	}
	delete[] ET;
	//删除E
	delete[] E;
}

void display()
{
	//设置摄像机位置和二维平面位置
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT);
	gluLookAt(0, 0, 0, 0, 0, -1, 0, 1, 0);
	glTranslatef(0, 0, -5);//二维平面为z=-5

	FillPolygon(pointXYArray,pointNumber);

	glFlush();
}

void reshape(int w, int h)
{
	if (0 == h)
		h = 1;
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 100, 0, 100, 1, 100);//二维平面范围是0<=x<=100,0<=y<=100
	glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'q':
		std::cout << "how many points?(>=3)\n";
		std::cin.sync();
		std::cin >> pointNumber;
		if (pointNumber < 3)
		{
			return;
		}
		if (pointXYArray != 0)
		{
			delete[] pointXYArray;
		}
		pointXYArray = new int[2 * pointNumber];
		for (int i = 0; i < pointNumber; i++)
		{
			std::cout << "Enter X and Y coordinates for point " << i << std::endl;
			std::cin >> pointXYArray[2*i];
			std::cin >> pointXYArray[2*i+1];
		}
		std::cout << "Press q to change polygon" << std::endl;
		glutPostRedisplay();
		break;
	default:
		break;
	}
}

int main(int argc, char** argv)
{
	pointNumber = 5;
	pointXYArray = new int[2*pointNumber];
	pointXYArray[0] = 30;
	pointXYArray[1] = 0;
	pointXYArray[2] = 10;
	pointXYArray[3] = 40;
	pointXYArray[4] = 20;
	pointXYArray[5] = 60;
	pointXYArray[6] = 90;
	pointXYArray[7] = 70;
	pointXYArray[8] = 60;
	pointXYArray[9] = 10;
	std::cout << "Press q to change polygon" << std::endl;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("CG1-3");
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutMainLoop();

	delete[] pointXYArray;
	return 0;
}