#include <math.h>
#include <iostream>
#include <Python.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

//��ֵ��
void mythreshold(Mat &img, uchar T)
{
	int n1 = img.rows;
	int nc = img.cols * img.channels();
	if (img.isContinuous())//�ж�ͼ���Ƿ�����
	{
		nc = nc * n1;
		n1 = 1;
	}
	for (int i = 0; i < n1; i++)
	{
		uchar *p = img.ptr<uchar>(i);
		for (int j = 0; j < nc; j++)
		{
			if (p[j] < T)
				p[j] = 0;
			else p[j] = 255;
		}
	}
}

//��ȡ��ͼ
void get_screen()
{
	Py_Initialize();	//��ʼ��Python

	// ����ʼ���Ƿ�ɹ�  
	if (!Py_IsInitialized())
		cout << "python init failed!" << endl;

	int flag;
	PyRun_SimpleString("import os");
	//PyRun_SimpleString("os.system('adb shell screencap -p //sdcard//src.png')");
	//flag = PyRun_SimpleString("os.system('adb pull //sdcard//src.png')");


	//�����ֻ���ͼ��ֱ���ϴ�������
	flag = PyRun_SimpleString("os.system('adb exec-out screencap -p > src.png')");

	if (flag != -1)
		cout << "��ȡ��ͼ�ɹ�" << endl;
	else cout << "��ȡ��ͼʧ��" << endl;

	Py_Finalize();	//�ر�Python  
}

//��ѹ��Ļ
void press(int x1, int y1, int x2, int y2, int dist)
{
	Py_Initialize();	//��ʼ��Python

	// ����ʼ���Ƿ�ɹ�  
	if (!Py_IsInitialized())
		cout << "python init failed!" << endl;

	PyRun_SimpleString("import os");
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path.append('E:/mygithub/WeChat-Jump')");	//�趨Python·��
	PyObject *pModule = NULL;
	PyObject *pFunc = NULL;
	PyObject *pArg = NULL;
	PyObject *pResult = NULL;

	pModule = PyImport_ImportModule("py_called");	//�ҵ�Python�ļ�
	if (!pModule)
		cout << "pModule erro!" << endl;

	//pFunc = PyObject_GetAttrString(pModule, "add_func");	//�ҵ�add_func����
	//pArg = Py_BuildValue("(i,i)", 10, 25);	//������� �������εĲ���
	//pResult = PyEval_CallObject(pFunc, pArg);	//ִ�к���

	pFunc = PyObject_GetAttrString(pModule, "press_value");
	pArg = Py_BuildValue("(i, i, i, i, i)", x1, y1, x2, y2, dist);
	PyEval_CallObject(pFunc, pArg);

	Py_Finalize();	//�ر�Python  
}

//��ȡ����λ��
void loca_start(Mat img_src, Mat img_model, Point &point)
{
	/************************************ģ��ƥ��****************************************/

	//�����������ľ���
	int result_cols = img_src.cols - img_model.cols + 1;
	int result_rows = img_src.rows - img_model.rows + 1;
	Mat result(Size(result_cols, result_rows), CV_8UC1, Scalar(0));

	//����ƥ��ͱ�׼��
	int match_method = 3;//ѡ�� ƥ��ķ�ʽ
	/*	
		CV_TM_SQDIFF = 0,
		CV_TM_SQDIFF_NORMED = 1,
		CV_TM_CCORR = 2,
		CV_TM_CCORR_NORMED = 3,
		CV_TM_CCOEFF = 4,
		CV_TM_CCOEFF_NORMED = 5
	*/
	matchTemplate(img_src, img_model, result, match_method);
	normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());

	// ͨ������ minMaxLoc ��λ��ƥ���λ��
	double minVal; double maxVal; Point minLoc; Point maxLoc;
	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

	// ���ڷ��� SQDIFF �� SQDIFF_NORMED, ԽС����ֵ������ߵ�ƥ����. ��������������, ��ֵԽ��ƥ��Խ��

	Point matchLoc;
	if (match_method == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED)
	{
		matchLoc = minLoc;
		//cout << "ƥ�����ƶ�Ϊ��" << minVal * 100 << endl;
	}
	else
	{
		matchLoc = maxLoc;
		//cout << "ƥ�����ƶ�Ϊ��" << maxVal * 100 << endl;
	}
		

	//��������λ��
	//rectangle(img_src, matchLoc, Point(matchLoc.x + img_model.cols, matchLoc.y + img_model.rows), Scalar(0, 255, 0), 2);
	
	//��������λ��
	Point point_cir = Point(matchLoc.x + (img_model.cols >> 1), matchLoc.y + img_model.rows - 8);
	//circle(img_src, point_cir, 5, Scalar(0, 0, 255), -1);
	point = point_cir;

	//imshow("result", result);
	/************************************ģ��ƥ��****************************************/
}

//��ȡĿ��λ��
void loca_next(Mat img_gray, Point point_start, Point &point_next)
{
	Rect area_0(0, 0, (img_gray.cols >> 3) * 4, (img_gray.rows >> 2));
	if (point_start.x <= (img_gray.cols >> 1))		//С�������
	{
		//area_0.x = (img_gray.cols >> 3) * 3;
		area_0.x = img_gray.cols >> 1;
		area_0.y = (img_gray.cols >> 1) + 50;
	}
	else
	{
		area_0.x = 10;
		area_0.y = (img_gray.cols >> 1) + 50;
		area_0.width = img_gray.cols >> 1;
	}
	////������һ��Ŀ�����λ��
	//rectangle(img_src, area_0, Scalar(0, 255, 0), 2);

	Mat img_scan = img_gray(area_0).clone();

	Point p_node1 = Point(0, 0);
	Point p_node2 = Point(0, 0);
	Point p_node = Point(0, 0);
	bool flag1 = false;
	for (int i = 5; i < img_scan.rows; i++)
	{
		bool flag = false;

		Vec3b *p = img_scan.ptr<Vec3b>(i);
		for (int j = 0; j < img_scan.cols - 1; j++)
		{
			int val_l = p[j + 1][0] + p[j + 1][1] + p[j + 1][2] - p[j][0] - p[j][1] - p[j][2];
			int val_r = p[j][0] + p[j][1] + p[j][2] - p[j + 1][0] - p[j + 1][1] - p[j + 1][2];
			if (val_l > 10 && flag1 == false)
			{
				//cout << "1:" << val_l << endl;
				p_node1.x = j;
				p_node1.y = i;
				flag1 = true;
			}
			if (val_r > 10 && flag1 == true)
			{
				//cout << "2:" << val_r << endl;
				p_node2.x = j;
				p_node2.y = i;
				flag = true;
				break;
			}
		}
		if (flag == true)
			break;
	}
	if (p_node1.x != 0 && p_node2.y != 0)
	{
		//cout << "already" << endl;
		p_node.x = (p_node1.x + p_node2.x) >> 1;
		p_node.y = (p_node1.y + p_node2.y) >> 1;
		circle(img_scan, p_node1, 10, Scalar(255, 0, 0), -1);
		circle(img_scan, p_node2, 10, Scalar(255, 0, 0), -1);
		circle(img_scan, p_node, 10, Scalar(0), 2);
		p_node.x += area_0.x;
		p_node.y += area_0.y;
		point_next = p_node;
	}

	imshow("img_scan", img_scan);
}

//�������
void dist(Point start, Point next, float &dist_val)
{
	float xx = abs((float)next.x - (float)start.x);
	float distance = xx / cos(3.1415926 / 6);

	//float yy = abs((float)start.y - (float)next.y);
	//float distance = xx / sin(3.1415926 / 6);

	//int distance = sqrt(pow(next.x - start.x, 2) + pow(start.y - next.y, 2));
	dist_val = (float)distance * 2.8019 + 14.4488;	//dist_val = (float)distance * 2.8419 + 8.4488;
	cout << "���룺" << distance << "  ��ѹʱ�䣺" << dist_val << endl;
}

Point point_mouse = Point(0, 0);
void on_mouse(int event, int x, int y, int flags, void *ustc)//event����¼����ţ�x,y������꣬flags��ק�ͼ��̲����Ĵ���  
{
	if (event == CV_EVENT_LBUTTONDOWN)//������£���ȡ��ʼ����
	{
		point_mouse.x = x;
		point_mouse.y = y;
		//cout << "  ���ֵ�Ѹ���" << point_mouse << endl;
	}
}

int main()
{
	////ͨ�����ȷ����һλ��(���Զ�)
	//namedWindow("img_src");
	//setMouseCallback("img_src", on_mouse, 0);//���ûص�����


	int count = 0;		//���Ĵ������м���
	while (1)
	{
		//����һ����Χ�ڵ��������ģ����ָ���а�ѹ������÷ֻᱻ���
		srand((int)time(NULL));
		int myrnd_x = (rand() % (800 - 700 + 1)) + 700 - 1;
		int myrnd_y = (rand() % (1500 - 1400 + 1)) + 1400 - 1;

		//�õ��ֻ���ͼ
		get_screen();

		//�����ͼ
		Mat img_src = imread("src.png");
		resize(img_src, img_src, Size(img_src.cols >> 1, img_src.rows >> 1));	//540, 960

		//��������ģ��
		Mat img_model = imread("E:\\mygithub\\WeChat-Jump\\img_model.jpg");
		resize(img_model, img_model, Size(img_model.cols >> 1, img_model.rows >> 1));

		////������Ϸ����ģ��
		//Mat img_end = imread("E:\\vs2013test\\3.py-test\\py-test\\img_end.jpg");
		//resize(img_end, img_end, Size(img_end.cols >> 1, img_end.rows >> 1));

		//ģ��ƥ��õ�����λ��
		Point point_start;
		loca_start(img_src, img_model, point_start);
		circle(img_src, point_start, 5, Scalar(0, 0, 255), -1);

		Mat img_gray;
		img_src.copyTo(img_gray);
		//cvtColor(img_src, img_gray, COLOR_BGR2GRAY);


		//��Ե���õ�Ŀ��λ��
		Point point_next;
		loca_next(img_gray, point_start, point_next);
		circle(img_src, point_next, 5, Scalar(0, 255, 0), -1);//������һ��Ŀ���


		/*ͨ������ȡλ��*/
		//Point point_next;
		//point_next = point_mouse;
		//circle(img_src, point_next, 5, Scalar(0, 255, 0), -1);//������һ��Ŀ���


		imshow("img_src", img_src);
		waitKey(10);


		//��ʼ���Ŀ��㶼���ڣ�����ָ��
		if (point_start.x != 0 && point_start.y != 0 /*&& point_next.x != 0*/)
		{
			//�����ʼ��ֹ������
			cout << "start: " << point_start << "  next: " << point_next << endl;

			//�����һ��λ������
			float dist_val = 0;
			dist(point_start, point_next, dist_val);

			//����ָ�ѹ��Ļ
			press(myrnd_x, myrnd_y, myrnd_x, myrnd_y, (int)dist_val);
			count++;
			cout << "���Ĵ�����" << count << endl;
			cout << endl;

			
			////���λ������
			//point_mouse.x = 0;
			//point_mouse.y = 0;

			char key = waitKey(1000);//һ������ʱ
		}
		//waitKey(0);




		//if (key == 27)
		//	break;
	}

	//system("pause");
	return 0;
}