#include <cv.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include "opencv2/imgcodecs.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/ml.hpp>
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <typeinfo>
#include <vector>

using namespace cv;
using namespace cv::ml;
using namespace std;

#define HORIZONTAL 1
#define VERTICAL 2

typedef struct node {
	int a; 
	int b;
} point;

void mserExtractor (const Mat& image, Mat& mserOutMask);
vector<int> seperationHistogram(Mat *image, int dir);
vector<Mat> seperateByVerticalLine(Mat *image,vector<int> hist);
vector<Mat> seperateByHorizontalLine(Mat *image,vector<int> hist);
void showCharImages(vector<Mat> char_images);
void resizeVector(vector<Mat>& imgs);
Mat characterLBP(Mat I);
unsigned char lbpMask(Mat image, int i, int j);
vector<Mat> lbpVector(vector<Mat> params);
void myImageShow(Mat image);


uchar values[3][3] = {{128, 64, 32}, {1, 0, 16}, {2, 4, 8}};
Mat mask( 3, 3, CV_8UC1, values);

//training data
//tupple count = 5
String file_name[] = {"img_1.png","img_2.png", "img_3.png", "img_4.png", "img_5.png", "img_6.png", "img_7.png", "img_8.png", "img_9.png", "img_10.png", "img_11.png", "img_12.png", "img_13.png", "img_14.png", "img_15.png"};
uchar labels[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'x', 'y', '+', '-', '='};



int main() {

	int num_files = 15; //change this value whenevr you add new file
	int descriptor_cells = 8*8*256;
	
	vector<Mat> training_images_vector;

	Mat training_data = Mat::zeros(num_files, descriptor_cells, CV_8UC1);

	for (int i = 0; i < num_files; i++) {
		//for reading colored images
		Mat img = imread("./res/" + file_name[i], IMREAD_GRAYSCALE);
		training_images_vector.push_back(img);
	}

	for (int i = 0; i < num_files; i++) {
		Mat image = training_images_vector[i];
		if (image.empty()) break;
		//binarization
		threshold(image, image, 0, 255, CV_THRESH_BINARY_INV + CV_THRESH_OTSU);
		
		//separating horizontally
		vector<int> hHistogram = seperationHistogram(&image, HORIZONTAL);
		image = (seperateByHorizontalLine(&image, hHistogram))[0];
		//myImageShow(image);
		
		//seperating vertically
		vector<int> vHistogram = seperationHistogram(&image, VERTICAL);
		image = (seperateByVerticalLine(&image, vHistogram))[0];
		//myImageShow(image);

		//resizing image to 32*32
		Mat sampled;
		resize(image, sampled, Size(32,32));
		
		cout << sampled.size().height << " * " << sampled.size().width << endl;
		image = characterLBP(sampled);
		image.copyTo(training_data.row(i));
	}

	

	myImageShow(training_data);


	Mat labels_mat(num_files, 1, CV_8UC1, labels);


	// Set up SVM's parameters
    	SVM::Params params;
    	params.svmType    = SVM::C_SVC;
    	params.kernelType = SVM::LINEAR;
    	params.termCrit   = TermCriteria(TermCriteria::MAX_ITER, 100, 1e-6);


	// Train the SVM
	Mat trainingDataMat, labelsMat;
	training_data.convertTo(trainingDataMat, CV_32FC1);
	labels_mat.convertTo(labelsMat, CV_32SC1);
    	Ptr<SVM> svm = StatModel::train<SVM>(trainingDataMat, ROW_SAMPLE, labelsMat, params);

	
	

	svm->save("./res/classifier");
	
	

	return 0;
}


vector<int> seperationHistogram(Mat *image, int dir)
{
	int height = (*image).size().height;
	int width = (*image).size().width;

	int len = (dir == HORIZONTAL) ? height : width;
	vector<int> hist(len);

	//initialization
	for (int i = 0; i < len; i++)
		hist[i] = 0;

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int intensity = (*image).at<uchar>(i,j);
			if (dir == HORIZONTAL) {
				if (intensity == 255) hist[i] += 1;
			} else if (dir == VERTICAL) {
				if (intensity == 255) hist[j] += 1;
			}
		}
	}
	return hist;	
}


vector<Mat> seperateByHorizontalLine(Mat *image,vector<int> hist) {

	vector<Mat> char_images;
	bool state = true;
	
	int y1 = 0;
	int y2 = 0;

	int width = (*image).size().width;

	for (int i = 0; i < hist.size()-1; i++) {
		
		if (state) {
			if (hist[i] == 0 && hist[i+1] > 0) {
                cout << hist[i] << " " << hist[i+1] << endl;
				y1 = i;
				state = false;
			}
		} else if (!state) {
			if (hist[i] > 0 && hist[i+1] == 0) {
				y2 = i;
                cout << hist[i] << " " << hist[i+1] << endl;
				Mat char_image(*image, Rect(0,y1, width, y2-y1+1));
				char_images.push_back(char_image);
				state = true;
			}
		}
	}

	return char_images;
}


vector<Mat> seperateByVerticalLine(Mat *image,vector<int> hist){
	
	vector<Mat> char_images;
	bool state = true;
	
	int x1 = 0;
	int x2 = 0;

	int height = (*image).size().height;

	for (int i = 0; i < hist.size()-1; i++) {
		
		if (state) {
			if (hist[i] == 0 && hist[i+1] > 0) {
                cout << hist[i] << " " << hist[i+1] << endl;
				x1 = i;
				state = false;
			}
		} else if (!state) {
			if (hist[i] > 0 && hist[i+1] == 0) {
				x2 = i;
                cout << hist[i] << " " << hist[i+1] << endl;
				Mat char_image(*image, Rect(x1,0,x2-x1+1, height));
				char_images.push_back(char_image);
				state = true;
			}
		}
	}

	return char_images;
}


//lbp of 32*32 image divided into regions of 8*8 cells
Mat characterLBP(Mat Image) {
	Mat I;
	copyMakeBorder(Image, I, 1,1,1,1, BORDER_CONSTANT, Scalar(0));
	Size size = I.size();
	cout << size.height << " * " << size.width << endl;

	Mat lbpVector = Mat::zeros(1, 64*256, CV_8UC1);
	
	int cell = 0;

    for (int i = 1; i <= 32; i += 4) {
        for (int j = 1; j <= 32; j += 4) {
            for (int k = 0; k < 4; k++) {
                for (int l = 0; l < 4; l++){
                    unsigned char decimalValue = lbpMask(I, i + k, j + l);
                    lbpVector.at<uchar>(0,cell*256+decimalValue) = lbpVector.at<uchar>(0,cell*256+decimalValue) + 1;  
                }
            }
            cell++;
       }
    }

	return lbpVector;
}

unsigned char lbpMask(Mat image, int i, int j) {
	

	unsigned char center = image.at<uchar>(i, j);
	unsigned char result = 0;

	int p = i-1;
	for (; p < i+2; p++){
		int q = j-1;
		for (; q < j+2; q++)
		{
			int neighbour_value = image.at<uchar>(p,q);
			if (center <= neighbour_value)
				result += mask.at<uchar>(p-i+1, q-j+1);			
		}			
	}

	return result;
}

void myImageShow(Mat image) {
    String window_name = "my_image_window";
    namedWindow( window_name, WINDOW_AUTOSIZE );
    imshow(window_name, image);
    cout << image.size().height << " " << image.size().width << endl;
    waitKey(0);
    destroyWindow(window_name);
}
