#include <opencv2/opencv.hpp>

using namespace cv;

//blur effect
IplImage* blur_effect();                                                                                                                
//motion effect
IplImage* motion_effect(IplImage* input_img, Point2f quad1, Point2f quad2, Point2f quad3, Point2f quad4);   
//down sampling
IplImage* down_sampling(IplImage* output_img);                                                                              

//high->low resolution
void low_resolution();

void main()
{
	low_resolution();
}

IplImage* blur_effect()
{
	IplImage* input_img = cvLoadImage("lena.jpg", -1);
	IplImage* output_img = cvCreateImage(cvSize(input_img->width, input_img->height), IPL_DEPTH_8U, 1);

	cvSmooth(input_img, output_img, CV_BLUR, 10, 10);

	return output_img;
}

IplImage* motion_effect(IplImage* input_img, Point2f quad0, Point2f quad1, Point2f quad2, Point2f quad3)
{
	int H = input_img->height;
	int W = input_img->width;

	Mat input_mat, output_mat;
	input_mat = cvarrToMat(input_img);   //Iplimage->Mat Conversion

	Mat lambda(2, 4, CV_32FC1);      //Lambda Matrix
	lambda = Mat::zeros(input_mat.rows, input_mat.cols, input_mat.type());     //Set the lambda matrix the same type and size as input

	Point2f input_quad[4], output_quad[4];
	input_quad[0] = Point2f(0, 0);
	input_quad[1] = Point2f(input_mat.cols - 1, 0);
	input_quad[2] = Point2f(input_mat.cols - 1, input_mat.rows - 1);
	input_quad[3] = Point2f(0, input_mat.rows - 1);

	output_quad[0] = quad0;
	output_quad[1] = quad1;
	output_quad[2] = quad2;
	output_quad[3] = quad3;

	lambda = getPerspectiveTransform(input_quad, output_quad);   // Get the Perspective Transform Matrix i.e. lambda 
	warpPerspective(input_mat, output_mat, lambda, output_mat.size());   // Apply the Perspective Transform just found to the src image

	//Mat->IplImage
	IplImage* output_img = cvCreateImage(cvSize(W, H), IPL_DEPTH_8U, 1);
	for (int i = 0;i < H;i++)
	{
		for (int j = 0;j < W;j++)
		{
			output_img->imageData[i*W + j] = output_mat.data[i*W + j];
		}
	}

	return output_img;
}

IplImage* down_sampling(IplImage* input_img)
{
	int H = input_img->height;
	int W = input_img->width;

	unsigned char** input_arr2D = (unsigned char**)malloc(sizeof(unsigned char*) * H);             
	unsigned char** output_arr2D = (unsigned char**)malloc(sizeof(unsigned char*) * H / 2);      
	for (int i = 0; i < H; i++)
	{
		input_arr2D[i] = (unsigned char*)malloc(sizeof(unsigned char)*W);
	}
	for (int i = 0; i < H / 2; i++)
	{
		output_arr2D[i] = (unsigned char*)malloc(sizeof(unsigned char)*W / 2);
	}

	IplImage* output_img = cvCreateImage(cvSize(W / 2, H / 2), IPL_DEPTH_8U, 1);      

	int avg = 0;     //평균값

	//down sampling 전 
	for (int i = 0; i < H; i++)
	{
		for (int j = 0; j < W; j++)
		{
			input_arr2D[i][j] = input_img->imageData[i * W + j];
		}
	}

	//down sampling
	for (int i = 0; i < H / 2; i++)
	{
		for (int j = 0; j < W / 2; j++)
		{
			avg = (input_arr2D[2 * i][2 * j] + input_arr2D[2 * i][2 * j + 1] + input_arr2D[2 * i + 1][2 * j] + input_arr2D[2 * i + 1][2 * j + 1]) / 4;
			output_arr2D[i][j] = avg;
		}
	}

	//down sampling 후
	for (int i = 0; i < H / 2; i++)
	{
		for (int j = 0; j < W / 2; j++)
		{
			output_img->imageData[i * H / 2 + j] = output_arr2D[i][j];
		}
	}

	return output_img;
}

void low_resolution()
{
	// 1.Blur Effect
	IplImage* blur = blur_effect();

	// 2. Motion Effect
	int W = blur->width;
	int H = blur->height;

	Point2f quad0, quad1, quad2, quad3;

	// 2.(1) 원근감
	quad0 = Point2f(10, 10);
	quad1 = Point2f(W - 10, 10);
	quad2 = Point2f(W + 5, H);
	quad3 = Point2f(-5, H);
	IplImage* motion1 = motion_effect(blur, quad0, quad1, quad2, quad3);

	// 2.(2) 원근감
	quad0 = Point2f(-5, 5);
	quad1 = Point2f(W + 5, 5);
	quad2 = Point2f(W - 5, H + 5);
	quad3 = Point2f(5, H + 5);
	IplImage* motion2 = motion_effect(blur, quad0, quad1, quad2, quad3);

	// 2.(3) 이동
	quad0 = Point2f(5, 5);
	quad1 = Point2f(W + 5, 5);
	quad2 = Point2f(W + 5, H + 5);
	quad3 = Point2f(5, H + 5);
	IplImage* motion3 = motion_effect(blur, quad0, quad1, quad2, quad3);

	// 2.(4) 회전
	quad0 = Point2f(-5, 5);
	quad1 = Point2f(W - 5, -5);
	quad2 = Point2f(W + 5, H - 5);
	quad3 = Point2f(5, H + 5);
	IplImage* motion4 = motion_effect(blur, quad0, quad1, quad2, quad3);

	// 3. Down Sampling
	IplImage* down1 = down_sampling(motion1);
	IplImage* down2 = down_sampling(motion2);
	IplImage* down3 = down_sampling(motion3);
	IplImage* down4 = down_sampling(motion4);

	// display
	cvNamedWindow("1.원근감", 1);
	cvShowImage("1.원근감", down1);

	cvNamedWindow("2.원근감", 1);
	cvShowImage("2.원근감", down2);

	cvNamedWindow("3.이동", 1);
	cvShowImage("3.이동", down3);

	cvNamedWindow("4.회전", 1);
	cvShowImage("4.회전", down4);

	cvWaitKey(0);

	cvReleaseImage(&blur);

	cvReleaseImage(&motion1);
	cvReleaseImage(&motion2);
	cvReleaseImage(&motion3);
	cvReleaseImage(&motion4);

	cvReleaseImage(&down1);
	cvReleaseImage(&down2);
	cvReleaseImage(&down3);
	cvReleaseImage(&down4);
}