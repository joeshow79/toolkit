/*
 * Author:jason jiao
 * 
 * History: 2018-01-31: initial version
 * 
 * Reference:
 * 
 * 
 */

#include <iostream>
#include <random>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <stdio.h>
#include <time.h>

using namespace	std;
using namespace	cv;

static int	BUF_LENGTH = 1024;

//Parameters
static const double MAXSCALE=2.0;
static const double MINSCALE=0.2;
static const int	OFFSETSTRIDE = 200;

static double dCoef = 0.1;
int		nRepeatTimes = 1;
double		dRatio = 1.0;
static int nGoodWidth=600;

bool bLeftButtonDown;
bool bRightButtonDown;
bool bDragLeft;
bool bDragRight;
bool bDragUp;
bool bDragDown;
double dDragScale;
int nMouseWheelScale;


//Resource
string		strImgOne = "./1.png";
string		strImgTwo = "./2.png";
string		strImgQuestion = "./qm2.png";
string		strImgError = "./error.png";
string strWindowName="PairwiseComparison";
string strIndicator="Indicator";

//Debug
//#define DEBUG 

int 
usage(int argc)
{
	cout << "usage: paircomp -f fileList -t [Repeat Times] -r [Random Ratio]\n" << endl;
	cout << "-f: List file." << endl;
	cout << "-t: Repeat times. Must be integer and greater than 0" << endl;
	cout << "-r: Random ratio. Must be the value between (0.0,1.0]" << endl;
	cout << endl;
	if (argc < 2) {
		return -1;
	}
	return 0;
}

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

int
getPid(){
{
    int nPid = (int)getpid();
    return nPid;
}
}
void 
mergeImg(Mat & dst, Mat & src1, Mat & src2)
{
	//int		rows = src1.rows + 5 + src2.rows;
	int		rows = max(src1.rows, src2.rows);
	int		cols = src1.cols + 5 + src2.cols;
	CV_Assert(src1.type() == src2.type());
	dst.create(rows, cols, src1.type());
	src1.copyTo(dst(Rect(0, 0, src1.cols, src1.rows)));
	src2.copyTo(dst(Rect(src1.cols + 5, 0, src2.cols, src2.rows)));
}

void mouseCallback(int event, int x, int y, int flags, void* param){

	//cout<<"Mouse event: "<<event<<",flags"<<flags<<endl;
	static int nLastPointX=0;
	static int nLastPointY=0;

    switch(event)
    {
		//TODO:Seem the mouse wheel event not support (at least) on OSX
		case EVENT_MOUSEWHEEL:
			if (getMouseWheelDelta(flags)>0)
				nMouseWheelScale=1;
			else
				nMouseWheelScale=-1;
			break;
        case CV_EVENT_LBUTTONDOWN: 
			bLeftButtonDown=true;
			break;
        case CV_EVENT_LBUTTONUP: 
			bLeftButtonDown=false;
			break;
        case CV_EVENT_MOUSEMOVE: 
			if(true == bLeftButtonDown){
				if(abs(x - nLastPointX) > abs( y - nLastPointY)){ //x axis
					if(( x - nLastPointX) < 0 ){
						bDragRight=true;
					}
					if(( x - nLastPointX) > 0 ){
						bDragLeft=true;
					}
					dDragScale= abs(x-nLastPointX);
				}
				if(abs(x - nLastPointX) < abs( y - nLastPointY)){ //y axis
					if(( y - nLastPointY) < 0 ){
						bDragDown=true;
					}
					if(( y - nLastPointY) > 0 ){
						bDragUp=true;
					}
					dDragScale= abs(nLastPointY-y);
				}
			}

			break;
		default:
			break;
	}
	nLastPointX=x;
	nLastPointY=y;
}

int 
main(int argc, char **argv)
{
	char		ch;
	string strFileName;

	while ((ch = getopt(argc, argv, "hk:r:f:")) != -1) {

		switch (ch) {
		case 'h':
			usage(argc);
			return 0;
			break;
		case 'f':
			strFileName=string(optarg);
			break;
		case 'k':
			nRepeatTimes = atoi(optarg);
			if (nRepeatTimes <= 0) {
				cerr << "FATAL: Illegal value for repeat times!" << endl;
				usage(argc);
				return -1;
			}
			break;
		case 'r':
			dRatio = atof(optarg);
			if (dRatio <= 0.0 || dRatio > 1.0) {
				cerr << "FATAL: Illegal ratio value!" << endl;
				usage(argc);
				return -1;
			}
			break;
		default:
			cerr << "ERROR: Invalid  option: " << ch << endl;;
			break;
		}
	}

	int		result = usage(argc);
	cout<<"Repeat Times: "<<nRepeatTimes<<endl;
	cout<<"Ratio : "<<dRatio<<endl;

	if (result != 0) {
		return result;
	}

	if (strFileName.empty() ) {
		cerr << "<list file> is not specified! " << endl;
		return -1;
	}

	//Resources
	cv:: Mat matOne = cv::imread(strImgOne.c_str());
	cv:: Mat matTwo = cv::imread(strImgTwo.c_str());
	cv:: Mat matQuestion = cv::imread(strImgQuestion.c_str());
	cv:: Mat matError = cv::imread(strImgError.c_str());

	//Result file
	ofstream resultList("result"+to_string(getPid())+".txt");

	ifstream	fileList(strFileName);

	vector<string> vecList;
	char		buf       [BUF_LENGTH];

	if(fileList.good()){
		while(!fileList.eof()){
			if(fileList.getline(buf, BUF_LENGTH )){
				vecList.push_back(buf);	
			}
		}
	}
	else{
		cerr << "Unable to read list file, exit!" << endl;
		return -1;
	}
	/* Iterate through the files list */
	for (int iter = 0; iter < nRepeatTimes; ++iter) {
		cout<<"Iteration #"<<iter<<endl;
		cout<<"-------------------------------"<<endl;

		ifstream	fileList(strFileName);

		char		bufA      [BUF_LENGTH];
		char		bufB      [BUF_LENGTH];
		char		category  [BUF_LENGTH];

		vector<string>::iterator it;
		for(it=vecList.begin();it!=vecList.end();it++){
			string strValue=(*it);

			//Ratio Check
			static default_random_engine e(time(0));
			static uniform_real_distribution < double >u(0.0, 1.0);
			double dValue=u(e);

			if ( dRatio != 1.0){
				cout<<"Random value: "<<dValue<<endl;

				if (dValue > dRatio) {
					cout<<"Skip one due to the random ratio setting."<<endl;
					continue;
				}
			}

			sscanf(strValue.c_str(), "%s %s %s", category, bufA, bufB);
			string		strCompA = string(bufA);
			string		strCompB = string(bufB);
			string		strCat = string(category);
			/*cout << strCompA << endl;
			cout << strCompB << endl;*/

			cv:: Mat matAOrig = cv::imread(strCompA.c_str());
			cv:: Mat matBOrig = cv::imread(strCompB.c_str());

			cv::		Mat bgrd;
			cv::		Mat matA;
			cv::		Mat matB;
			cv::		Mat matAProcessed;
			cv::		Mat matBProcessed;

			cvNamedWindow(strWindowName.c_str());
			cvSetMouseCallback(strWindowName.c_str(), mouseCallback, NULL);

			int		nSelection = 0;
			double   dScale4DisplayA = std::min(1.0,((double)nGoodWidth/matAOrig.cols));
			double   dScale4DisplayB = std::min(1.0,((double)nGoodWidth/matBOrig.cols));
			double   dScaleA = dScale4DisplayA;
			double   dScaleB = dScale4DisplayB;
			double		dOffsetAX = 0;
			double		dOffsetAY = 0;
			double		dOffsetBX = 0;
			double		dOffsetBY = 0;
			double dMaxOffsetAX=0;
			double dMaxOffsetAY=0;
			double dMaxOffsetBX=0;
			double dMaxOffsetBY=0;

			//Resize original image for appropriate display 
			resize(matAOrig, matA, Size(0, 0), dScale4DisplayA, dScale4DisplayA, INTER_LINEAR);
			resize(matBOrig, matB, Size(0, 0), dScale4DisplayB, dScale4DisplayB, INTER_LINEAR);

			while (1) {
				/*cout << "ScaleA: " << dScaleA << endl;
				cout << "ScaleB: " << dScaleB << endl;
				cout << "dOffsetAX: "<< dOffsetAX <<endl;
				cout << "dOffsetAY: "<< dOffsetAY <<endl;
				cout << "dOffsetBX: "<< dOffsetBX <<endl;
				cout << "dOffsetBY: "<< dOffsetBY <<endl;*/

				resize(matAOrig, matAProcessed, Size(0, 0), dScaleA, dScaleA, INTER_LINEAR);
				resize(matBOrig, matBProcessed, Size(0, 0), dScaleB, dScaleB, INTER_LINEAR);
				if ((dScaleA > dScale4DisplayA) && (dScaleB > dScale4DisplayB))
				{
					dMaxOffsetAX=abs((double)matAProcessed.cols - (double)matA.cols)/2;
					dMaxOffsetAY=abs((double)matAProcessed.rows - (double)matA.rows)/2;
					dMaxOffsetBX=abs((double)matBProcessed.cols - (double)matB.cols)/2;
					dMaxOffsetBY=abs((double)matBProcessed.rows -(double)matB.rows)/2;
					//If the current offset beyond the maxim value of the offset step due to the zoomin & zoomout,
					//Adjust the value accordingly
					if( dOffsetAX > dMaxOffsetAX ){
						dOffsetAX = dMaxOffsetAX ;
					}
					if( dOffsetAY > dMaxOffsetAY ){
						dOffsetAY = dMaxOffsetAY ;
					}
					if( dOffsetAX < -dMaxOffsetAX ){
						dOffsetAX = -dMaxOffsetAX ;
					}
					if( dOffsetAY < -dMaxOffsetAY ){
						dOffsetAY = -dMaxOffsetAY ;
					}
					if( dOffsetBX > dMaxOffsetBX ){
						dOffsetBX = dMaxOffsetBX ;
					}
					if( dOffsetBY > dMaxOffsetBY ){
						dOffsetBY = dMaxOffsetBY ;
					}
					if( dOffsetBX < -dMaxOffsetBX ){
						dOffsetBX = -dMaxOffsetBX ;
					}
					if( dOffsetBY < -dMaxOffsetBY ){
						dOffsetBY = -dMaxOffsetBY ;
					}

#ifdef DEBUG
					cout<<"A Max step: "<<dMaxOffsetAX<<","<<dMaxOffsetAY<<endl;
					cout<<"B Max step: "<<dMaxOffsetBX<<","<<dMaxOffsetBY<<endl;
					cout<<"A Offset: "<<dOffsetAX<<","<<dOffsetAY<<endl;
					cout<<"B Offset: "<<dOffsetBX<<","<<dOffsetBY<<endl;
#endif

					/*				
					cout << "Original imageA size: ("<<matA.cols << "," << matA.rows <<")"<< endl;
					cout << "Original imageB size: ("<<matB.cols << "," << matB.rows <<")"<< endl;
					cout << "Zoomed imageA size: ("<<matAProcessed.cols << "," << matAProcessed.rows <<")"<< endl;
					cout << "Zoomed imageB size: ("<<matBProcessed.cols << "," << matBProcessed.rows <<")"<< endl;
					cout << "Max X/Y offset step: "<<dMaxOffsetAX<<"/"<<dMaxOffsetAY<<endl;
					*/

					int nPlanOffsetAX= dOffsetAX ;
					int nPlanOffsetAY= dOffsetAY ;
					int nPlanOffsetBX= dOffsetBX ;
					int nPlanOffsetBY= dOffsetBY ;

					int		nDiffAX = matAProcessed.cols - matA.cols;
					int		nDiffAY = matAProcessed.rows - matA.rows;
					int		nMarginAX = nDiffAX / 2;
					int		nMarginAY = nDiffAY / 2;

					if ((nMarginAX + matA.cols + nPlanOffsetAX) >= matAProcessed.cols) {
						nMarginAX = matAProcessed.cols - matA.cols;
					}
					else{
						if ((nMarginAX + nPlanOffsetAX) <= 0) {
							nMarginAX = 0;
						}
						else{
							nMarginAX += nPlanOffsetAX;
						}
					}

					if ((nMarginAY + matA.rows + nPlanOffsetAY) >= matAProcessed.rows) {
						nMarginAY = matAProcessed.rows - matA.rows;
					}
					else{
						if ((nMarginAY + nPlanOffsetAY) <= 0) {
							nMarginAY = 0;
						}
						else{
							nMarginAY += nPlanOffsetAY;
						}
					}

					int		nDiffBX = matBProcessed.cols - matB.cols;
					int		nDiffBY = matBProcessed.rows - matB.rows;
					int		nMarginBX = nDiffBX / 2;
					int		nMarginBY = nDiffBY / 2;

					if ((nMarginBX + matB.cols + nPlanOffsetBX) >= matBProcessed.cols) {
						nMarginBX = matBProcessed.cols - matB.cols;
					}
					else{
						if ((nMarginBX + nPlanOffsetBX) <= 0) {
							nMarginBX = 0;
						}
						else{
							nMarginBX += nPlanOffsetBX;
						}
					}

					if ((nMarginBY + matB.rows + nPlanOffsetBY) >= matBProcessed.rows) {
						nMarginBY = matBProcessed.rows - matB.rows;
					}
					else{
						if ((nMarginBY + nPlanOffsetBY) <= 0) {
							nMarginBY = 0;
						}
						else{
							nMarginBY += nPlanOffsetBY;
						}
					}

					//ROI
					Rect		rectRoiA  (nMarginAX, nMarginAY, matA.cols, matA.rows);
					matAProcessed = matAProcessed(rectRoiA);
					Rect		rectRoiB  (nMarginBX, nMarginBY, matB.cols, matB.rows);
					matBProcessed = matBProcessed(rectRoiB);
					//resize is optional here
					//resize(matAProcessed, matAProcessed, Size(matA.cols, matA.rows), 0, 0, INTER_LINEAR);
					//resize(matBProcessed, matBProcessed, Size(matB.cols, matB.rows), 0, 0, INTER_LINEAR);

				}

				mergeImg(bgrd, matAProcessed, matBProcessed);

				cv::		imshow(strWindowName, bgrd);
				static bool bFirstRun=true;
				if(bFirstRun){
					cv::		moveWindow(strWindowName, 512, 0);
					bFirstRun=false;
				}

				cv::Mat matIndicator;

				switch (nSelection) {
				case 1:
					matIndicator=matOne.clone();
					break;
				case 2:
					matIndicator=matTwo.clone();
					break;
				case -1:
					cv::imshow(strIndicator,matError);
					waitKey(100);
					nSelection=0;
				default:
					matIndicator=matQuestion.clone();
					break;
				}

				//Draw indicator text
				Size szText=getTextSize(strCat,FONT_HERSHEY_COMPLEX,3,5,0);
				Point ptOrg((matIndicator.cols - szText.width)/2,(matIndicator.rows)/4);
				putText(matIndicator,strCat,ptOrg,FONT_HERSHEY_COMPLEX,3,Scalar(0,255,0));
				cv::imshow(strIndicator, matIndicator);

				int		input_key = cvWaitKey(1);
				//int		input_key = cvWaitKey();
				
				//For non timeout event
				if(-1 != input_key){
					cout <<"Input key: "<< input_key << endl;
				}

				bool		bEnter = false;

				switch (input_key) {
				case 49: //1
					nSelection = 1;
					cout << "1st picture selected as better." << endl;
					break;
				case 50: //2
					nSelection = 2;
					cout << "2nd picture selected as better." << endl;
					break;
				case 61: //+
					if ((dScaleA < MAXSCALE) && (dScaleB < MAXSCALE)) {
						dScaleA += dCoef;
						dScaleB += dCoef;
					}
					break;
				case 45: //-
					if ((dScaleA > MINSCALE) && (dScaleB > MINSCALE)) {
						dScaleA -= dCoef;
						dScaleB -= dCoef;
					}
					else{
						dOffsetAX=0;
						dOffsetAY=0;
					}
					break;
				case 63233: //Down   #TODO:Check the key on different platform
					if(dOffsetAY < dMaxOffsetAY && dOffsetAY >= (-dMaxOffsetAY)){
						dOffsetAY += 1 * OFFSETSTRIDE;
					}
					if(dOffsetBY < dMaxOffsetBY && dOffsetBY >= (-dMaxOffsetBY)){
						dOffsetBY += 1 * OFFSETSTRIDE;
					}
					break;
				case 63232: //Up
					if(dOffsetAY <= dMaxOffsetAY && dOffsetAY > (-dMaxOffsetAY)){
						dOffsetAY -= 1 * OFFSETSTRIDE;
					}
					if(dOffsetBY <= dMaxOffsetBY && dOffsetBY > (-dMaxOffsetBY)){
						dOffsetBY -= 1 * OFFSETSTRIDE;
					}
					break;
				case 63234: //Left
					if(dOffsetAX <= dMaxOffsetAX && dOffsetAX > (-dMaxOffsetAX)){
						dOffsetAX -= 1 * OFFSETSTRIDE;
					}
					if(dOffsetBX <= dMaxOffsetBX && dOffsetBX > (-dMaxOffsetBX)){
						dOffsetBX -= 1 * OFFSETSTRIDE;
					}
					break;
				case 63235: //Right
					if(dOffsetAX < dMaxOffsetAX && dOffsetAX >= (-dMaxOffsetAX)){
						dOffsetAX += 1 * OFFSETSTRIDE;
					}
					if(dOffsetBX < dMaxOffsetBX && dOffsetBX >= (-dMaxOffsetBX)){
						dOffsetBX += 1 * OFFSETSTRIDE;
					}
					break;
				case 114: //R key
					dScaleA = dScale4DisplayA ;
					dScaleB = dScale4DisplayB ;
					dOffsetAX = 0;
					dOffsetAY = 0;
					dOffsetBX = 0;
					dOffsetBY = 0;
					break;
				case 13: //Enter
					cout << "Iterate to next one." << endl;
					bEnter = true;
					break;
				case 27: //ESC
					cout << "Cancel the work." << endl;
					return 0;
				case -1://timeout
					break;
				default:
					nSelection = -1;
					cerr << "Invalid selection." << endl;
					break;
				}

				//Check the mouse event
				//TODO:somethine wrong with the following snippet
				if((true == bLeftButtonDown ) ){
#ifdef DEBUG
					cout<<"------------------------------"<<endl;
					cout<<"dDragScale: "<<dDragScale<<endl;
					cout<<"dScaleA: "<<dScaleA<<endl;
					cout<<"dScaleB: "<<dScaleB<<endl;
					cout<<"dScale4DisplayA: "<<dScale4DisplayA<<endl;
					cout<<"dScale4DisplayB: "<<dScale4DisplayB<<endl;
#endif
					if(true == bDragLeft){
						if(dOffsetAX <= dMaxOffsetAX && dOffsetAX > (-dMaxOffsetAX)){
							dOffsetAX -= dDragScale;
							//dOffsetAX -= dDragScale/dScale4DisplayA ;
						}
						if(dOffsetBX <= dMaxOffsetBX && dOffsetBX > (-dMaxOffsetBX)){
							dOffsetBX -= dDragScale;
							//dOffsetBX -= dDragScale/dScale4DisplayB ;
						}
					}
					if(true == bDragRight){
						if(dOffsetAX < dMaxOffsetAX && dOffsetAX >= (-dMaxOffsetAX)){
							dOffsetAX += dDragScale;
						//	dOffsetAX += dDragScale/dScale4DisplayA ;
						}
						if(dOffsetBX < dMaxOffsetBX && dOffsetBX >= (-dMaxOffsetBX)){
							dOffsetBX += dDragScale;
							//dOffsetBX += dDragScale/dScale4DisplayB ;
						}
					}
					if(true == bDragUp){
						if(dOffsetAY <= dMaxOffsetAY && dOffsetAY > (-dMaxOffsetAY)){
							dOffsetAY -= dDragScale;
							//dOffsetAY -= dDragScale/dScale4DisplayA ;
						}
						if(dOffsetBY <= dMaxOffsetBY && dOffsetBY > (-dMaxOffsetBY)){
							dOffsetBY -= dDragScale;
							//dOffsetBY -= dDragScale/dScale4DisplayB ;
						}
					}
					if(true == bDragDown){
						if(dOffsetAY < dMaxOffsetAY && dOffsetAY >= (-dMaxOffsetAY)){
							dOffsetAY += dDragScale;
							//dOffsetAY += dDragScale/dScale4DisplayA ;
						}
						if(dOffsetBY < dMaxOffsetBY && dOffsetBY >= (-dMaxOffsetBY)){
							dOffsetBY += dDragScale;
							//dOffsetBY += dDragScale/dScale4DisplayB ;
						}
					}
					bDragLeft=false;
					bDragRight=false;
					bDragUp=false;
					bDragDown=false;
				}

				if(-1 == nMouseWheelScale)
				{
					if ((dScaleA > MINSCALE) && (dScaleB > MINSCALE)) {
						dScaleA -= dCoef;
						dScaleB -= dCoef;
					}
					else{
						dOffsetAX=0;
						dOffsetAY=0;
					}
					nMouseWheelScale=0;
				}
				else{
					if(1 == nMouseWheelScale){
						if ((dScaleA < MAXSCALE) && (dScaleB < MAXSCALE)) {
							dScaleA += dCoef;
							dScaleB += dCoef;
						}
						nMouseWheelScale=0;
					}
				}

				if (bEnter) {
					cout << "Your selection is: " << nSelection << endl;
					if (nSelection == 1 || nSelection == 2) {
						resultList << strCat << " "<< strCompA << " " << strCompB << " " << nSelection << endl;
						nSelection = 0;
						dScaleA = dScale4DisplayA ;
						dScaleB = dScale4DisplayB ;
						dOffsetAX = 0;
						dOffsetAY = 0;
						dOffsetBX = 0;
						dOffsetBY = 0;
						break;
					} else {
						cv::imshow(strIndicator,matError);
						waitKey(100);
						cerr << "Pleaes slelect 1 or 2 for the better one before iterate to next comparison." << endl;
					}
				}
			}
			cvDestroyWindow(category);
		}
	}

	return (int)0;
}
