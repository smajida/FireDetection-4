#include <iostream>
#include <fstream>
#include <vector>
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <math.h>


using namespace cv;
using namespace std;
Mat frame;
RNG rng(12345);
int thresh = 100;
int max_thresh = 255;
typedef struct{
	int red, blue, green;
}RGB;


typedef struct{
	RGB rgb;
}Pixel;

typedef struct{
	vector<Pixel> pixel;
	bool isFire;
	int rows, cols;
	float meanR;
	float meanG;
	float meanB;
	float varianceR;
	float varianceG;
	float varianceB;
	float Hu1;
	float Hu2;
	float Hu3;
	float Hu4;
	float Hu5;
	float Hu6;
	float Hu7;
}Images;



RGB rgbThreshold;
vector<Images> image;
vector<Mat> processedImage;
vector<Mat> trainingImage;
Mat processedImages;

RGB readFile(){
	RGB rgb;
	ifstream input_file("rgb.data", ios::binary);
	input_file.read((char*)&rgb, sizeof(rgb));
	return rgb;
}

void writeFile(RGB rgb){
	ofstream out("rgb.data", ios::binary);
	if (!out)
	{
		cout << "Cannot load file" << endl;
	}
	else
	{
		out.write((char*)&rgb, sizeof(rgb));
	}
	out.close();
}



void YCbCrThreshold()
{
	for (int x = 0; x < processedImage.size(); x++){
		for (int i = 0; i < processedImage[x].rows; i++)
		{
			for (int j = 0; j < processedImage[x].cols; j++)
			{
				bool isFire = false;
				double bT = processedImage[x].at<cv::Vec3b>(i, j)[0]; // B		
				double gT = processedImage[x].at<cv::Vec3b>(i, j)[1]; // G
				double rT = processedImage[x].at<cv::Vec3b>(i, j)[2]; // R

				double b = bT, g = gT, r = rT;

				double y = 16 + r *  65.481 + g * 128.553 + b *24.996;
				double cB = 128 + r * -37.797 - g * 74.203 + b * 112.0;
				double cR = 128 + r * 112.00 + g * -93.7864 + b * -18.214;

				isFire = (y >= cR >= cB);


				if (isFire)
				{
					processedImage[x].at<cv::Vec3b>(i, j)[0] = bT;
					processedImage[x].at<cv::Vec3b>(i, j)[1] = gT;
					processedImage[x].at<cv::Vec3b>(i, j)[2] = rT;
				}
				else
				{
					processedImage[x].at<cv::Vec3b>(i, j)[0] = 0;
					processedImage[x].at<cv::Vec3b>(i, j)[1] = 0;
					processedImage[x].at<cv::Vec3b>(i, j)[2] = 0;
				}
			}
		}
	}
	cout << "nahuman na ycbcr threshold" << endl;
}

void redThreshold(){
	for (int x = 0; x < processedImage.size(); x++){
		//cout << x << endl;
		for (int i = 0; i < trainingImage[x].rows; i++)
		{
			for (int j = 0; j<trainingImage[x].cols; j++)
			{
				bool isFire = false;

				int b = trainingImage[x].at<cv::Vec3b>(i, j)[0];
				int g = trainingImage[x].at<cv::Vec3b>(i, j)[1];
				int r = trainingImage[x].at<cv::Vec3b>(i, j)[2];

				if ((r > g && g > b))
				{
					//check if red is over a threshold
					if (r > rgbThreshold.red && g > rgbThreshold.green && b < rgbThreshold.blue)
					{
						isFire = true;
					}
				}

				if (isFire)
				{
					processedImage[x].at<cv::Vec3b>(i, j)[0] = b;
					processedImage[x].at<cv::Vec3b>(i, j)[1] = g;
					processedImage[x].at<cv::Vec3b>(i, j)[2] = r;

				}
				
			}
		}
	}
	cout << "nahuman na red threshold" << endl;
}

void storeTrainingImage(){
	for (int x = 0; x < processedImage.size(); x++){
		int ctr = 0;
		image.push_back(Images());
		for (int i = 0; i < processedImage[x].rows; i++)
		{
			for (int j = 0; j < processedImage[x].cols; j++)
			{
				int b = processedImage[x].at<cv::Vec3b>(i, j)[0];
				int g = processedImage[x].at<cv::Vec3b>(i, j)[1];
				int r = processedImage[x].at<cv::Vec3b>(i, j)[2];
				if (b != 0 && g != 0 && r != 0){
					image[x].pixel.push_back(Pixel());
					image[x].pixel[ctr].rgb.blue = b;
					image[x].pixel[ctr].rgb.green = g;
					image[x].pixel[ctr].rgb.red = r;
					ctr++;
				}
			}
		}
	}
}


// subtract the background to get only the desired pixel for training
void processTrainingImages(){
	cout << "proccesed Image: " << processedImage.size() << endl;
	cout << "training IMage:" << trainingImage.size() << endl;
	redThreshold();
	YCbCrThreshold();
	storeTrainingImage();
	cout << "mana" << endl;	
}


void openTrainingImages(){
	vector<cv::String> fn;
	int i;
	cv::String  folder = "newNegative\\";
	//cv::String folder = "dataPics\\";
	glob(folder, fn, false);

	for (i = 0; i<fn.size(); i++)
	{
		Mat m = imread(fn[i]);
		trainingImage.push_back(m);
		processedImage.push_back(Mat());
		processedImage[i] = cv::Mat(trainingImage[i].size().height, trainingImage[i].size().width, CV_8UC3);
	}
	folder = "newPositive\\";
	glob(folder, fn, false);
	int ctr = i;
	for ( i = 0; i<fn.size(); i++)
	{
		Mat m = imread(fn[i]);		
		trainingImage.push_back(m);
		processedImage.push_back(Mat());
		processedImage[ctr] = cv::Mat(trainingImage[ctr].size().height, trainingImage[ctr].size().width, CV_8UC3);
		ctr++;
	}
}


void writeCSV(){
	cout << "sud sa hu writeCSV" << endl;
	ofstream myfile;
	myfile.open("newtrain4.csv");
	int ctr = 1;
	int isFire = 0;
	myfile << "RedMean,GreenMean,BlueMean,RedVariance,GreenVariance,BlueVariance,Hu1,Hu2,Hu3,Hu4,Hu5,Hu6,Hu7,Fire" << endl;
	for (int x = 0; x < image.size(); x++){
		if (x >= 286){
			isFire = 1;
		}			
		myfile << image[x].meanR << "," << image[x].meanG << "," << image[x].meanB << "," << image[x].varianceR << "," << image[x].varianceG << "," << image[x].varianceB << "," << image[x].Hu1 << "," << image[x].Hu2 << "," << image[x].Hu3 << "," << image[x].Hu4 << "," << image[x].Hu5 << "," << image[x].Hu6 << "," << image[x].Hu7 << "," << isFire << endl;
					ctr++;					
	}
	myfile.close();
}

void trial(int, void*){
	Mat canny_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	for (int x = 0; x < trainingImage.size(); x++){
		/// Detect edges using canny
		Canny(trainingImage[x], canny_output, thresh, thresh * 2, 3);
		/// Find contours
		findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

		/// Get the moments
		vector<Moments> mu(contours.size());
		for (int i = 0; i < contours.size(); i++)
		{
			mu[i] = moments(contours[i], false);
		}

		///  Get the mass centers:
		vector<Point2f> mc(contours.size());
		for (int i = 0; i < contours.size(); i++)
		{
			mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
		}

		/// Draw contours
		Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
		for (int i = 0; i < contours.size(); i++)
		{
			Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
			drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());
			circle(drawing, mc[i], 4, color, -1, 8, 0);
		}

		/// Show in a window
		namedWindow("Contours", CV_WINDOW_AUTOSIZE);
		imshow("Contours", drawing);
		imshow("training Image", trainingImage[x]);
		cout << contours.size();
		cv::Moments mom = cv::moments(contours[0]);
		double hu[7];
		cv::HuMoments(mom, hu); // now in hu are your 7 Hu-Moments
		image[x].Hu1 = hu[0];
		image[x].Hu2 = hu[1];
		image[x].Hu3 = hu[2];
		image[x].Hu4 = hu[3];
		image[x].Hu5 = hu[4];
		image[x].Hu6 = hu[5];
		image[x].Hu7 = hu[6];
		//waitKey(0); //Note: wait for user input for every image
	}
}

void thresh_callback(int, void*)
{
	cout << "sud sa hu moments" << endl;
	cout << "image size: " << image.size() << endl;
	cout << "proc image size: " << processedImage.size() << endl;
	
	Mat canny_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	for (int x = 0; x < processedImage.size(); x++){

		cvtColor(processedImage[x], processedImages, CV_BGR2GRAY);
		blur(processedImages, processedImages, Size(3, 3));
		/// Detect edges using canny
		Canny(processedImages, canny_output, thresh, thresh * 2, 3);
		/// Find contours
		findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

		/// Get the moments
		double hu[7];
		if (contours.size() > 0){
			cv::Moments mom = cv::moments(contours[0]);
			cv::HuMoments(mom, hu); // now in hu are your 7 Hu-Moments
		}
		else {
			hu[0] = hu[1] = hu[2] = hu[3] = hu[4] = hu[5] = hu[6] = 0;
		}
		image[x].Hu1 = hu[0];
		image[x].Hu2 = hu[1];
		image[x].Hu3 = hu[2];
		image[x].Hu4 = hu[3];
		image[x].Hu5 = hu[4];
		image[x].Hu6 = hu[5];
		image[x].Hu7 = hu[6];
	}
	cout << "nahuman hu" << endl;
	
}



void mean(){
	int x, y;
	cout << "sud sa mean" << endl;
	for (y = 0; y < image.size();y++){
		float retval[3] = { 0, 0, 0 };
		for (x = 0; x < image[y].pixel.size(); x++){
			retval[0] = retval[0] + image[y].pixel[x].rgb.blue;
			retval[1] = retval[1] + image[y].pixel[x].rgb.green;
			retval[2] = retval[2] + image[y].pixel[x].rgb.red;
		}
		image[y].meanB = retval[0] / image[y].pixel.size();
		image[y].meanG = retval[1] / image[y].pixel.size();
		image[y].meanR = retval[2] / image[y].pixel.size();

	}
	cout << "nahuman mean" << endl;
}
void variance(){
	cout << "sud sa variance" << endl;
	float varianceR, varianceG, varianceB;
	int x, y;
	for (x = 0; x < image.size(); x++){
		varianceR = varianceG = varianceB = 0;
		for (y = 0; y < image[x].pixel.size(); y++){
			varianceR = varianceR + pow(image[x].pixel[y].rgb.red - image[x].meanR,2.0);
			varianceG = varianceG + pow(image[x].pixel[y].rgb.green - image[x].meanG,2.0);
			varianceB = varianceB + pow(image[x].pixel[y].rgb.blue - image[x].meanB,2.0);
			
		}
		image[x].varianceR = sqrt(varianceR / image[x].pixel.size());
		image[x].varianceG = sqrt(varianceG / image[x].pixel.size());
		image[x].varianceB = sqrt(varianceB / image[x].pixel.size());
	}
	cout << "nahuman variance" << endl;
}


void saveImage(){
	Size size(640, 480);//the dst image size,e.g.100x100
	vector<cv::String> fn;
	int i;
	cv::String  folder = "Positive\\";
	cv::String  dst = "dst\\";
	glob(folder, fn, false);
	cout << fn.size() << endl;
	for (i = 0; i < fn.size(); i++)
	{
		Mat image = imread(fn[i]);
		ostringstream convert;

		resize(image, image, size);//resize image
		imshow("aw", image);
		convert << "newPositive\\" << i << ".jpg";
		imwrite(convert.str().c_str(), image);
		//do your logging... 
	}

	//cvtColor(frame, YCbCr, CV_BGR2YCrCb);

}
int main()
{
	rgbThreshold = readFile();
	cout << "Initial thresholds:" << endl;
	cout << "red: " << rgbThreshold.red << endl;
	cout << "blue: " << rgbThreshold.green << endl;
	cout << "greens: " << rgbThreshold.blue << endl;

	openTrainingImages();
	processTrainingImages();
	mean();
	variance();
	thresh_callback(0, 0);
	//trial(0, 0);
	writeCSV();

	for (int i = 0; i<trainingImage.size(); i++){
		imshow("training Image", trainingImage[i]);
		imshow("processed Image", processedImage[i]);
		//createTrackbar(" Canny thresh:", "Source", &thresh, max_thresh, thresh_callback);


		waitKey(0); //Note: wait for user input for every image
	}
	return 0;

}  

