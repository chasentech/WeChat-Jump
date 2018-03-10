//#define Python
#define MYC





#include <math.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <direct.h>


#ifdef Python
#include <Python.h>
#endif

using namespace std;
using namespace cv;

float param_k = 0;
float param_b = 0;
char path[200];		//Pyton路径

//读入配置信息
void yml_read()
{
	FileStorage fs("configure.yml", FileStorage::READ);

	fs["param_k"] >> param_k;			//线性关系k
	fs["param_b"] >> param_b;			//线性关系b

	cout << "param_k = " << param_k << endl;
	cout << "param_b = " << param_b << endl;
}

//设定Python路径
void set_Python_path()
{
	//获取当前路径
	char buffer[200];
	_getcwd(buffer, 200);
	for (int i = 0; i < 200; i++)
	{
		if (buffer[i] == '\0')
			break;
		if (buffer[i] == '\\')	// '\'的ASCII码 92
		{
			buffer[i] = '/';	// '/'的ASCII码 47
		}
	}

	//连接字符串
	strcpy_s(path, "sys.path.append('");
	strcat_s(path, buffer);
	strcat_s(path, "')");

	////string 转化为 char*
	//char path[100];
	//strcpy_s(path, project_path.c_str());
}

//获取截图
void get_screen()
{
#ifdef Python
	Py_Initialize();	//初始化Python

	// 检查初始化是否成功  
	if (!Py_IsInitialized())
		cout << "python init failed!" << endl;

	int flag;
	PyRun_SimpleString("import os");

	//手机截图，再上传到电脑，比较麻烦
	//PyRun_SimpleString("os.system('adb shell screencap -p //sdcard//src.png')");
	//flag = PyRun_SimpleString("os.system('adb pull //sdcard//src.png')");

	//不用手机截图，直接上传到电脑
	flag = PyRun_SimpleString("os.system('adb exec-out screencap -p > src.png')");

	if (flag != -1)
		cout << "获取截图成功" << endl;
	else cout << "获取截图失败" << endl;

	Py_Finalize();	//关闭Python
#endif

#ifdef MYC
	system("adb exec-out screencap -p > src.png");
	cout << "获取截图" << endl;
#endif
}

//按压屏幕
void press(int x1, int y1, int x2, int y2, int dist)
{
#ifdef Python
	Py_Initialize();	//初始化Python

	// 检查初始化是否成功  
	if (!Py_IsInitialized())
		cout << "python init failed!" << endl;

	PyRun_SimpleString("import os");
	PyRun_SimpleString("import sys");
	PyRun_SimpleString(path);	//设定Python路径
	PyObject *pModule = NULL;
	PyObject *pFunc = NULL;
	PyObject *pArg = NULL;
	PyObject *pResult = NULL;

	pModule = PyImport_ImportModule("py_called");	//找到Python文件
	if (!pModule)
		cout << "pModule erro!" << endl;

	//pFunc = PyObject_GetAttrString(pModule, "add_func");	//找到add_func函数
	//pArg = Py_BuildValue("(i,i)", 10, 25);	//传入参数 两个整形的参数
	//pResult = PyEval_CallObject(pFunc, pArg);	//执行函数

	pFunc = PyObject_GetAttrString(pModule, "press_value");
	pArg = Py_BuildValue("(i, i, i, i, i)", x1, y1, x2, y2, dist);
	PyEval_CallObject(pFunc, pArg);

	Py_Finalize();	//关闭Python  
#endif

#ifdef MYC
	char mypath[200];
	char xx1[10], yy1[10], xx2[10], yy2[10];
	char ddist[10];
	sprintf_s(xx1, "%d", x1);
	sprintf_s(yy1, "%d", y1);
	sprintf_s(xx2, "%d", x2);
	sprintf_s(yy2, "%d", y2);
	sprintf_s(ddist, "%d", dist);
	//连接字符串
	strcpy_s(mypath, "adb shell input swipe ");
	strcat_s(mypath, xx1);
	strcat_s(mypath, " ");
	strcat_s(mypath, yy1);
	strcat_s(mypath, " ");
	strcat_s(mypath, xx2);
	strcat_s(mypath, " ");
	strcat_s(mypath, yy2);
	strcat_s(mypath, " ");
	strcat_s(mypath, ddist);

	system(mypath);
	cout << mypath << endl;

#endif

}

//获取棋子位置
void loca_start(Mat img_src, Mat img_model, Point &point)
{
	/************************************模板匹配****************************************/

	//创建输出结果的矩阵
	int result_cols = img_src.cols - img_model.cols + 1;
	int result_rows = img_src.rows - img_model.rows + 1;
	Mat result(Size(result_cols, result_rows), CV_8UC1, Scalar(0));

	//进行匹配和标准化
	int match_method = 3;//选择 匹配的方式
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

	// 通过函数 minMaxLoc 定位最匹配的位置
	double minVal; double maxVal; Point minLoc; Point maxLoc;
	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

	// 对于方法 SQDIFF 和 SQDIFF_NORMED, 越小的数值代表更高的匹配结果. 而对于其他方法, 数值越大匹配越好

	Point matchLoc;
	if (match_method == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED)
	{
		matchLoc = minLoc;
		//cout << "匹配相似度为：" << minVal * 100 << endl;
	}
	else
	{
		matchLoc = maxLoc;
		//cout << "匹配相似度为：" << maxVal * 100 << endl;
	}
		

	//画出棋子位置
	//rectangle(img_src, matchLoc, Point(matchLoc.x + img_model.cols, matchLoc.y + img_model.rows), Scalar(0, 255, 0), 2);
	
	//画出中心位置
	Point point_cir = Point(matchLoc.x + (img_model.cols >> 1), matchLoc.y + img_model.rows - 8);
	//circle(img_src, point_cir, 5, Scalar(0, 0, 255), -1);
	point = point_cir;

	//imshow("result", result);
	/************************************模板匹配****************************************/
}

//获取目标位置
void loca_next(Mat img_gray, Point point_start, Point &point_next)
{
	Rect area_0(0, 0, (img_gray.cols >> 3) * 4, (img_gray.rows >> 2));
	if (point_start.x <= (img_gray.cols >> 1))		//小人在左边
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
	////画出下一个目标大体位置
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

		p_node.x += 1;

		point_next = p_node;
	}

	imshow("img_scan", img_scan);
}

//计算比例
void dist(Point start, Point next, float &dist_val)
{
	float xx = abs((float)next.x - (float)start.x);
	float distance = xx / cos(3.1415926 / 6);

	next.y = start.y - (distance / 2);

	//float yy = abs((float)start.y - (float)next.y);
	//float distance = xx / sin(3.1415926 / 6);

	//int distance = sqrt(pow(next.x - start.x, 2) + pow(start.y - next.y, 2));
	dist_val = (float)distance * param_k + param_b;	//dist_val = (float)distance * 2.8419 + 8.4488;
	cout << "距离：" << distance << "  按压时间：" << dist_val << endl;
}

int main()
{
	yml_read();		//读入配置信息
#ifdef Python
	set_Python_path();	//设置Python路径为当前路径
#endif
	get_screen();	//提前得到手机截图

	int count = 0;		//跳的次数进行计数
	while (1)
	{
		//得到手机截图
		get_screen();

		//载入截图
		Mat img_src = imread("src.png");
		if (img_src.empty())
			continue;
		resize(img_src, img_src, Size(img_src.cols >> 1, img_src.rows >> 1));	//540, 960

		//创建图像用来处理
		Mat img_temp;
		img_src.copyTo(img_temp);

		//载入棋子模板
		Mat img_model = imread("img_model.jpg");
		resize(img_model, img_model, Size(img_model.cols >> 1, img_model.rows >> 1));

		//模板匹配得到棋子位置
		Point point_start;
		loca_start(img_src, img_model, point_start);
		//circle(img_src, point_start, 5, Scalar(0, 0, 255), -1);
		line(img_src, Point(0, point_start.y), Point(img_src.cols, point_start.y), Scalar(0, 255, 0), 2);
		line(img_src, Point(point_start.x, 0), Point(point_start.x, img_src.rows), Scalar(0, 255, 0), 2);

		//边缘检测得到目标位置
		Point point_next;
		loca_next(img_temp, point_start, point_next);
		//circle(img_src, point_next, 5, Scalar(0, 255, 0), -1);//画出下一个目标点
		float xx = abs((float)point_next.x - (float)point_start.x);
		float distance = xx / cos(3.1415926 / 6);
		point_next.y = point_start.y - (distance / 2);
		line(img_src, Point(0, point_next.y), Point(img_src.cols, point_next.y), Scalar(0, 0, 255), 2);
		line(img_src, Point(point_next.x, 0), Point(point_next.x, img_src.rows), Scalar(0, 0, 255), 2);

		line(img_src, point_start, point_next, Scalar(255, 0, 0), 2);


		imshow("img_src", img_src);
		waitKey(10);

		//起始点和目标点都存在，则发送指令
		if (point_start.x != 0 && point_start.y != 0 )
		{
			//输出起始终止点坐标
			cout << "start: " << point_start << "  next: " << point_next << endl;

			//算出下一个位置坐标
			float dist_val = 0;
			dist(point_start, point_next, dist_val);



			//产生一定范围内的随机数，模拟手指进行按压，否则得分会被清除
			srand((int)time(NULL));
			int myrnd_x = (rand() % (800 - 700 + 1)) + 700 - 1;
			int myrnd_y = (rand() % (1500 - 1400 + 1)) + 1400 - 1;

			//发送指令按压屏幕
			press(myrnd_x, myrnd_y, myrnd_x, myrnd_y, (int)dist_val);
			count++;
			cout << "跳的次数：" << count << endl;

			//间隔时间设定为1500-900之间的随机数
			int mytime = (rand() % (5000 - 1000 + 1)) + 900 - 1;
			cout << "间隔：" << mytime << endl;
			cout << endl;
			char key = waitKey(mytime);//一定的延时
		}
		
	}

	return 0;
}