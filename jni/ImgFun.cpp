#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <opencv2/opencv.hpp>
using namespace cv;

//Step size of orientation map blocks
//It means the StepSize*StepSize block will have same orientation
#define StepSize 17

//nBlockSize the higher the more accurate the orientation is
#define nBlockSize 19

//filter kernel size
#define kernel_size 15

#define CropOut (nBlockSize + (kernel_size-1)/2)


/******* F u n c t i o n   D e c l e r a t i o n ******/
IplImage * change4channelTo3InIplImage(IplImage * src);
cv::Mat mkKernel(int ks, double sig, double th, double lm, double ps);
cv::Mat calculateOrientations(cv::Mat gradientX, cv::Mat gradientY);
void smoothAngles(cv::Mat smooth_angles, cv::Mat angles);


extern "C" {
JNIEXPORT jintArray JNICALL Java_com_example_haveimgfun_LibImgFun_ImgFun(
        JNIEnv* env, jobject obj, jintArray buf, int w, int h);
JNIEXPORT jintArray JNICALL Java_com_example_haveimgfun_LibImgFun_ImgFun(
        JNIEnv* env, jobject obj, jintArray buf, int w, int h) {

    jint *cbuf;
    cbuf = env->GetIntArrayElements(buf, false);
    if (cbuf == NULL) {
        return 0;
    }

    Mat myimg(h, w, CV_8UC4, (unsigned char*) cbuf);
    IplImage image=IplImage(myimg);

    /***** Change 4 channels image (ARGB8888) into to 3 channels (RGB564)  ****/
    IplImage* image3channel = change4channelTo3InIplImage(&image);

    /***** RGB to Grayscale *****/
    IplImage* grayImage=cvCreateImage(cvGetSize(image3channel),IPL_DEPTH_8U,1);
    cvCvtColor(image3channel, grayImage, CV_RGB2GRAY);

    /***** Convert Iplimage to Mat ******/
    cv::Mat myMat(grayImage);

    /***** Normalize Image  ************/
    cv::Mat norm_image;
    cv::normalize(myMat, norm_image, 0, 255, NORM_MINMAX, CV_8UC1);

    /*** Convert to Floating-point gray image ***/
    cv::Mat float_gray ;
    norm_image.convertTo(float_gray, CV_32F, 1.0/255.0);

    /******************  O r i e n t a t i o n   C a l c u l a t i o n  *********************/
    /// Generate grad_x and grad_y
    int scale = 1;
    int delta = 0;
    int ddepth = CV_16S;

    cv::Mat grad_x, grad_y;

    cv::Mat abs_grad_x, abs_grad_y;

    /// Gradient X
    //Scharr( src_gray, grad_x, ddepth, 1, 0, scale, delta, BORDER_DEFAULT );
    cv::Sobel(norm_image , grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );
    //cv::convertScaleAbs( grad_x, abs_grad_x );

    /// Gradient Y
    //Scharr( src_gray, grad_y, ddepth, 0, 1, scale, delta, BORDER_DEFAULT );
    cv::Sobel(norm_image, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT );
    //cv::convertScaleAbs( grad_y, abs_grad_y );

    grad_x.convertTo(grad_x, CV_32F, 1.0/255.0);
    grad_y.convertTo(grad_y, CV_32F, 1.0/255.0);
    cv::Mat orient = cv::Mat(grad_x.rows,grad_x.cols,CV_32F);
    orient = calculateOrientations(grad_x, grad_y);


  /******************  O r i e n t a t i o n   S m o o t h i n g  *********************/
    cv::Mat orient_smooth(grad_x.rows,grad_x.cols,CV_32F);

    smoothAngles( orient_smooth, orient);


  /**********************    G a b o r   F i l t e r i n g   **************************/
    //gabor filter parameter
  	int kwin = (kernel_size -1)/2;
  	int pos_sigma= 15;
  	int pos_lm = 95;
  	int pos_psi = 90;
  	float max =0.0;
  	float min =0.0;

  	double sig = pos_sigma;
    double lm = 0.5+pos_lm/100.0;
    double ps = pos_psi;
  	cv::Mat kernel;
  	cv::Mat roi(kernel_size,kernel_size,CV_32F);
  	cv::Mat gabor_dest(kernel_size,kernel_size,CV_32F);
  	cv::Mat result(grad_x.rows,grad_x.cols,CV_32F);
  	cv::Mat binary(grad_x.rows,grad_x.cols,CV_8UC1);
  	int i,j,u,v;
  	float angle;
  	float sum;

  	for (j = kwin; j <= grad_y.cols-kwin-1; j=j+1)
  	for (i = kwin; i <= grad_x.rows-kwin-1; i=i+1)
  	{
  		angle = orient_smooth.cv::Mat::at<float>(i,j);

  		for (v = 0; v < kernel_size; v++)
          for (u = 0; u < kernel_size; u++)
          {
  			roi.cv::Mat::at<float>(u,v) = float_gray.cv::Mat::at<float>(i-kwin+u,j-kwin+v);
  		}
  		kernel = mkKernel(kernel_size, sig, angle, lm, ps);

		sum = 0.0;
		for (v = 0; v < kernel_size; v++)
        for (u = 0; u < kernel_size; u++)
        {
			sum += kernel.cv::Mat::at<float>(u,v) * roi.cv::Mat::at<float>(u,v);
		}
		result.cv::Mat::at<float>(i,j) = sum;

  		if (result.cv::Mat::at<float>(i,j) >max)
  			max = result.cv::Mat::at<float>(i,j);
  		else if (result.cv::Mat::at<float>(i,j) < min)
  			min = result.cv::Mat::at<float>(i,j);

  	}


  	//cv::Rect result_roi = cv::Rect(nBlockSize+kwin,nBlockSize+kwin,grad_x.cols-2*nBlockSize-2*kwin, grad_x.rows-2*nBlockSize-2*kwin );
  	//cv::Mat result_rect;
  	//result_rect = result(result_roi);


  	//double lapmin, lapmax;
    //minMaxLoc(result_rect, &lapmin, &lapmax);
    double scale_factor = 127/ cv::max(-min, max);
    result.convertTo(result, CV_8U, scale_factor, 128);


  	cv::Mat final(grad_x.rows, grad_x.cols, CV_8U);
  	//result_rect.convertTo(result_rect, CV_8U);
  	result.convertTo(result,CV_8U);
  	cv::adaptiveThreshold(result, final, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 11, 0);//13 9.7

  	cv::Rect final_roi = cv::Rect(nBlockSize+kwin,nBlockSize+kwin,grad_x.cols-2*nBlockSize-2*kwin, grad_x.rows-2*nBlockSize-2*kwin );
  	cv::Mat final_rect = final(final_roi);


  	cv::Mat finalfinal = cv::Mat::zeros(grad_x.rows, grad_x.cols, CV_8U);
  	final_rect.copyTo(finalfinal(final_roi));

    IplImage output_dst = IplImage(finalfinal);
    IplImage *resultImage = &output_dst;
    //resultImage = cvCreateImage(cvGetSize(mask_Ipl), IPL_DEPTH_8U, 1);
    //resultImage->imageData = (char *)mask.data;

    /**************** Output Image ****************/
    int* outImage=new int[w*h];
    for(int i=0;i<w*h;i++)
    {
        outImage[i]=(int)resultImage->imageData[i];
    }

    int size = w * h;
    jintArray output_img = env->NewIntArray(size);
    env->SetIntArrayRegion(output_img, 0, size, outImage);
    env->ReleaseIntArrayElements(buf, cbuf, 0);
    return output_img;
}
}

IplImage * change4channelTo3InIplImage(IplImage * src) {
    if (src->nChannels != 4) {
        return NULL;
    }

    IplImage * destImg = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 3);
    for (int row = 0; row < src->height; row++) {
        for (int col = 0; col < src->width; col++) {
            CvScalar s = cvGet2D(src, row, col);
            cvSet2D(destImg, row, col, s);
        }
    }

    return destImg;
}


cv::Mat mkKernel(int ks, double sig, double th, double lm, double ps)
{
    int hks = (ks-1)/2;
    double theta = th*CV_PI/180;
    double psi = ps*CV_PI/180;
    double del = 2.0/(ks-1);
    double lmbd = lm;
    double sigma = sig/ks;
    double x_theta;
    double y_theta;
    cv::Mat kernel(ks,ks, CV_32F);
    for (int y=-hks; y<=hks; y++)
    {
        for (int x=-hks; x<=hks; x++)
        {
            x_theta = x*del*cos(theta)+y*del*sin(theta);
            y_theta = -x*del*sin(theta)+y*del*cos(theta);
            kernel.at<float>(hks+y,hks+x) = (float)exp(-0.5*(pow(x_theta,2)+pow(y_theta,2))/pow(sigma,2))* cos(2*CV_PI*x_theta/lmbd + psi);
        }
    }
    return kernel;
}


cv::Mat calculateOrientations(cv::Mat gradientX, cv::Mat gradientY)
{
	/* The width and the height of the image */
    int32_t h       = (gradientX).rows;
    int32_t w       = (gradientX).cols;

    // Create container element
	cv::Mat orientation = cv::Mat(h,w,CV_32F);

	int32_t i,j,x,y;
	float angle;
	float nx, ny;
	cv::Mat dx = cv::Mat(2*nBlockSize+1, 2*nBlockSize+1, CV_32F);
	cv::Mat dy = cv::Mat(2*nBlockSize+1, 2*nBlockSize+1, CV_32F);

    // Calculate orientations of gradients --> in degrees
    // Loop over all matrix values and calculate the accompagnied orientation

	/* 1 - Image block-wise */
	// For image w*h the range should be [0...w-1, 0...h-1]
    for (y = nBlockSize; y <= w-nBlockSize-1; y=y+StepSize)
    for (x = nBlockSize; x <= h-nBlockSize-1; x=x+StepSize)
    {
		/* 2 - Get the gradient of current block */
        for (j = 0; j <= (nBlockSize*2); j++)
        for (i = 0; i <= (nBlockSize*2); i++)
        {
			dx.cv::Mat::at<float>(i,j)=gradientX.cv::Mat::at<float>(x-nBlockSize+i,y-nBlockSize+j);
			dy.cv::Mat::at<float>(i,j)=gradientY.cv::Mat::at<float>(x-nBlockSize+i,y-nBlockSize+j);
		}

		nx = 0.0;
		ny = 0.0;
        for (j = 0; j <= (nBlockSize*2); j++)
        for (i = 0; i <= (nBlockSize*2); i++)
        {
			nx += 2 * dx.cv::Mat::at<float>(i,j) * dy.cv::Mat::at<float>(i,j);
            ny += dx.cv::Mat::at<float>(i,j)*dx.cv::Mat::at<float>(i,j)
				- dy.cv::Mat::at<float>(i,j)*dy.cv::Mat::at<float>(i,j);
			//printf("(%d, %d): nx = %f, ny = %f\n",i,j,nx, ny);
		}

		/* 3 - compute angle (0 degree ~ 180 degree) */
		angle = (float)( fastAtan2(nx, ny) * 0.50);
        for (j = 0; j < StepSize; j++)
        for (i = 0; i < StepSize; i++)
        {
			orientation.cv::Mat::at<float>(x+i,y+j) = angle;
		}
	}
    return orientation;
}

/** Smooth the orientation **/
void smoothAngles(cv::Mat smooth_angles, cv::Mat angles)
{
    cv::Mat sin_angles;
	cv::Mat cos_angles;

    cos_angles.create(angles.size(), CV_32FC1);
    sin_angles.create(angles.size(), CV_32FC1);

	for(int j = 0; j < angles.cols; j++){
		for(int i = 0; i < angles.rows; i++){
			cos_angles.at<float>(i, j) = std::cos(2*angles.at<float>(i, j) * CV_PI / 180);
			sin_angles.at<float>(i, j) = std::sin(2*angles.at<float>(i, j) * CV_PI / 180);
		}
	}

    //cv::Mat gaussKernel = cv::getGaussianKernel(21, 7.0, CV_32FC1);
	cv::Mat cos_angles_lp;
	cv::Mat sin_angles_lp;
    // apply gaussian low-pass filtering
    //cv::filter2D(cos_angles, cos_angles_lp, cos_angles.depth(), gaussKernel);
    //cv::filter2D(sin_angles, sin_angles_lp, sin_angles.depth(), gaussKernel);
	cv::GaussianBlur(cos_angles, cos_angles_lp, Size(15,15),10);
	cv::GaussianBlur(sin_angles, sin_angles_lp, Size(15,15),10);
    for (int j = 0; j < angles.cols; j++) {
        for (int i = 0; i < angles.rows; i++) {
            smooth_angles.at<float>(i, j) = 0.5 * fastAtan2(sin_angles_lp.at<float>(i, j), cos_angles_lp.at<float>(i, j));
        }
    }
}


