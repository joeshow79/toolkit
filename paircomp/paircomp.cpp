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
static float	fCoef = 0.1;
static int	nOffsetStride = 200;
int		nRepeatTimes = 1;
float		dRatio = 1.0;
static int nGoodWidth=600;
static float MAXSCALE=2.0;
static float MINSCALE=0.2;
static int DRAGSTEPSCALE=100;

bool bLeftButtonDown;
bool bRightButtonDown;
bool bDragLeft;
bool bDragRight;
bool bDragUp;
bool bDragDown;
int nDragScale;

//Resource
string		strImgOne = "./1.png";
string		strImgTwo = "./2.png";
string		strImgQuestion = "./qm.png";
string strWindowName="PairwiseComparison";
string strIndicator="Indicator";

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
        case CV_EVENT_LBUTTONDOWN: 
			nLastPointX=x;
			nLastPointY=y;
			bLeftButtonDown=true;
			break;
        case CV_EVENT_LBUTTONUP: 
			if(abs(x - nLastPointX) > abs( y - nLastPointY)){ //x axis
				if(( x - nLastPointX) < 0 ){
					bDragRight=true;
				}
				if(( x - nLastPointX) > 0 ){
					bDragLeft=true;
				}
				nDragScale= abs(x-nLastPointX)/DRAGSTEPSCALE;
			}
			if(abs(x - nLastPointX) < abs( y - nLastPointY)){ //y axis
				if(( y - nLastPointY) < 0 ){
					bDragDown=true;
				}
				if(( y - nLastPointY) > 0 ){
					bDragUp=true;
				}
				nDragScale= abs(y-nLastPointY)/DRAGSTEPSCALE;
			}
			cout<<"Drag scale:"<<nDragScale<<endl;

			bLeftButtonDown=false;
			break;
        case CV_EVENT_MOUSEMOVE: 
			/*if(true == bLeftButtonDown){
				if(abs(x - nLastPointX) > abs( y - nLastPointY)){ //x axis
					if(( x - nLastPointX) < 0 ){
						bDragRight=true;
					}
					if(( x - nLastPointX) > 0 ){
						bDragLeft=true;
					}
					nDragScale= abs(x-nLastPointX)/DRAGSTEPSCALE;
				}
				if(abs(x - nLastPointX) < abs( y - nLastPointY)){ //y axis
					if(( y - nLastPointY) < 0 ){
						bDragUp=true;
					}
					if(( y - nLastPointY) > 0 ){
						bDragDown=true;
					}
					nDragScale= abs(y-nLastPointY)/DRAGSTEPSCALE;
				}
				cout<<"Drag scale:"<<nDragScale<<endl;
			}*/
			break;
		default:
			break;
	}
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
	/* Iterator through the files list */
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

			cout<<"dRatio: "<<dRatio<<endl;
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
			cout << strCompA << endl;
			cout << strCompB << endl;

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
			double   fScale4DisplayA = std::min(1.0,((double)nGoodWidth/matAOrig.cols));
			double   fScale4DisplayB = std::min(1.0,((double)nGoodWidth/matBOrig.cols));
			double   fScaleA = fScale4DisplayA;
			double   fScaleB = fScale4DisplayB;
			double		dOffsetAX = 0;
			double		dOffsetAY = 0;
			double		dOffsetBX = 0;
			double		dOffsetBY = 0;
			double dMaxOffsetStepAX=0;
			double dMaxOffsetStepAY=0;
			double dMaxOffsetStepBX=0;
			double dMaxOffsetStepBY=0;

			//Resize original image for appropriate display 
			resize(matAOrig, matA, Size(0, 0), fScale4DisplayA, fScale4DisplayA, INTER_LINEAR);
			resize(matBOrig, matB, Size(0, 0), fScale4DisplayB, fScale4DisplayB, INTER_LINEAR);

			while (1) {
				cout << "ScaleA: " << fScaleA << endl;
				cout << "ScaleB: " << fScaleB << endl;
				cout << "dOffsetAX: "<< dOffsetAX <<endl;
				cout << "dOffsetAY: "<< dOffsetAY <<endl;
				cout << "dOffsetBX: "<< dOffsetBX <<endl;
				cout << "dOffsetBY: "<< dOffsetBY <<endl;

				resize(matAOrig, matAProcessed, Size(0, 0), fScaleA, fScaleA, INTER_LINEAR);
				resize(matBOrig, matBProcessed, Size(0, 0), fScaleB, fScaleB, INTER_LINEAR);
				if ((fScaleA > fScale4DisplayA) && (fScaleB > fScale4DisplayB))
				{
					dMaxOffsetStepAX=abs((double)matAProcessed.cols - (double)matA.cols)/nOffsetStride/2;
					dMaxOffsetStepAY=abs((double)matAProcessed.rows - (double)matA.rows)/nOffsetStride/2;
					dMaxOffsetStepBX=abs((double)matBProcessed.cols - (double)matB.cols)/nOffsetStride/2;
					dMaxOffsetStepBY=abs((double)matBProcessed.rows -(double)matB.rows)/nOffsetStride/2;

					//If the current offset beyond the maxim value of the offset step due to the zoomin & zoomout,
					//Adjust the value accordingly
					if( dOffsetAX > dMaxOffsetStepAX ){
						dOffsetAX = dMaxOffsetStepAX ;
					}
					if( dOffsetAY > dMaxOffsetStepAY ){
						dOffsetAY = dMaxOffsetStepAY ;
					}
					if( dOffsetAX < -dMaxOffsetStepAX ){
						dOffsetAX = -dMaxOffsetStepAX ;
					}
					if( dOffsetAY < -dMaxOffsetStepAY ){
						dOffsetAY = -dMaxOffsetStepAY ;
					}
					if( dOffsetBX > dMaxOffsetStepBX ){
						dOffsetBX = dMaxOffsetStepBX ;
					}
					if( dOffsetBY > dMaxOffsetStepBY ){
						dOffsetBY = dMaxOffsetStepBY ;
					}
					if( dOffsetBX < -dMaxOffsetStepBX ){
						dOffsetBX = -dMaxOffsetStepBX ;
					}
					if( dOffsetBY < -dMaxOffsetStepBY ){
						dOffsetBY = -dMaxOffsetStepBY ;
					}

					cout<<"A Max step: "<<dMaxOffsetStepAX<<","<<dMaxOffsetStepAY<<endl;
					cout<<"B Max step: "<<dMaxOffsetStepBX<<","<<dMaxOffsetStepBY<<endl;
					cout<<"A Offset: "<<dOffsetAX<<","<<dOffsetAY<<endl;
					cout<<"B Offset: "<<dOffsetBX<<","<<dOffsetBY<<endl;

					/*				
					cout << "Original imageA size: ("<<matA.cols << "," << matA.rows <<")"<< endl;
					cout << "Original imageB size: ("<<matB.cols << "," << matB.rows <<")"<< endl;
					cout << "Zoomed imageA size: ("<<matAProcessed.cols << "," << matAProcessed.rows <<")"<< endl;
					cout << "Zoomed imageB size: ("<<matBProcessed.cols << "," << matBProcessed.rows <<")"<< endl;
					cout << "Max X/Y offset step: "<<dMaxOffsetStepAX<<"/"<<dMaxOffsetStepAY<<endl;
					*/

					//TODO: display is not complete if the fScaleA & fScaleB applied
					//The magnitude of the pan action should be adjust as per the scale value
					/*int nPlanOffsetAX= dOffsetAX * nOffsetStride * fScaleA ;
					int nPlanOffsetAY= dOffsetAY * nOffsetStride * fScaleA ;
					int nPlanOffsetBX= dOffsetBX * nOffsetStride * fScaleB ;
					int nPlanOffsetBY= dOffsetBY * nOffsetStride * fScaleB ;*/
					int nPlanOffsetAX= dOffsetAX * nOffsetStride ;
					int nPlanOffsetAY= dOffsetAY * nOffsetStride ;
					int nPlanOffsetBX= dOffsetBX * nOffsetStride ;
					int nPlanOffsetBY= dOffsetBY * nOffsetStride ;

					int		nDiffAX = matAProcessed.cols - matA.cols;
					int		nDiffAY = matAProcessed.rows - matA.rows;
					int		nNewAX = nDiffAX / 2;
					int		nNewAY = nDiffAY / 2;

					if ((nNewAX + matA.cols + nPlanOffsetAX) >= matAProcessed.cols) {
						nNewAX = matAProcessed.cols - matA.cols;
					}
					else{
						if ((nNewAX + nPlanOffsetAX) <= 0) {
							nNewAX = 0;
						}
						else{
							nNewAX += nPlanOffsetAX;
						}
					}

					if ((nNewAY + matA.rows + nPlanOffsetAY) >= matAProcessed.rows) {
						nNewAY = matAProcessed.rows - matA.rows;
					}
					else{
						if ((nNewAY + nPlanOffsetAY) <= 0) {
							nNewAY = 0;
						}
						else{
							nNewAY += nPlanOffsetAY;
						}
					}

					int		nDiffBX = matBProcessed.cols - matB.cols;
					int		nDiffBY = matBProcessed.rows - matB.rows;
					int		nNewBX = nDiffBX / 2;
					int		nNewBY = nDiffBY / 2;

					if ((nNewBX + matB.cols + nPlanOffsetBX) >= matBProcessed.cols) {
						nNewBX = matBProcessed.cols - matB.cols;
					}
					else{
						if ((nNewBX + nPlanOffsetBX) <= 0) {
							nNewBX = 0;
						}
						else{
							nNewBX += nPlanOffsetBX;
						}
					}

					if ((nNewBY + matB.rows + nPlanOffsetBY) >= matBProcessed.rows) {
						nNewBY = matBProcessed.rows - matB.rows;
					}
					else{
						if ((nNewBY + nPlanOffsetBY) <= 0) {
							nNewBY = 0;
						}
						else{
							nNewBY += nPlanOffsetBY;
						}
					}

					//ROI
					Rect		rectRoiA  (nNewAX, nNewAY, matA.cols, matA.rows);
					matAProcessed = matAProcessed(rectRoiA);
					Rect		rectRoiB  (nNewBX, nNewBY, matB.cols, matB.rows);
					matBProcessed = matBProcessed(rectRoiB);
					resize(matAProcessed, matAProcessed, Size(matA.cols, matA.rows), 0, 0, INTER_LINEAR);
					resize(matBProcessed, matBProcessed, Size(matB.cols, matB.rows), 0, 0, INTER_LINEAR);

				}

				mergeImg(bgrd, matAProcessed, matBProcessed);

				cv::		imshow(strWindowName, bgrd);
				cv::		moveWindow(strWindowName, 512, 0);

				cv::Mat matIndicator;

				switch (nSelection) {
				case 1:
					matIndicator=matOne.clone();
					break;
				case 2:
					matIndicator=matTwo.clone();
					break;
				default:
					matIndicator=matQuestion.clone();
					break;
				}

				//Draw indicator text
				Size szText=getTextSize(strCat,FONT_HERSHEY_COMPLEX,3,5,0);
				Point ptOrg((matIndicator.cols - szText.width)/2,(matIndicator.rows)/2);
				putText(matIndicator,strCat,ptOrg,FONT_HERSHEY_COMPLEX,3,Scalar(0,255,0));
				cv::imshow(strIndicator, matIndicator);

				int		input_key = cvWaitKey(50);
				//int		input_key = cvWaitKey();
				cout <<"Input key: "<< input_key << endl;

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
					if ((fScaleA < MAXSCALE) && (fScaleB < MAXSCALE)) {
						fScaleA += fCoef;
						fScaleB += fCoef;
					}
					break;
				case 45: //-
					if ((fScaleA > MINSCALE) && (fScaleB > MINSCALE)) {
						fScaleA -= fCoef;
						fScaleB -= fCoef;
					}
					else{
						dOffsetAX=0;
						dOffsetAY=0;
					}
					break;
				case 63233: //Down   #TODO:Check the key on different platform
					if(dOffsetAY < dMaxOffsetStepAY && dOffsetAY >= (-dMaxOffsetStepAY)){
						dOffsetAY += 1;
					}
					if(dOffsetBY < dMaxOffsetStepBY && dOffsetBY >= (-dMaxOffsetStepBY)){
						dOffsetBY += 1;
					}
					break;
				case 63232: //Up
					if(dOffsetAY <= dMaxOffsetStepAY && dOffsetAY > (-dMaxOffsetStepAY)){
						dOffsetAY -= 1;
					}
					if(dOffsetBY <= dMaxOffsetStepBY && dOffsetBY > (-dMaxOffsetStepBY)){
						dOffsetBY -= 1;
					}
					break;
				case 63234: //Left
					if(dOffsetAX <= dMaxOffsetStepAX && dOffsetAX > (-dMaxOffsetStepAX)){
						dOffsetAX -= 1;
					}
					if(dOffsetBX <= dMaxOffsetStepBX && dOffsetBX > (-dMaxOffsetStepBX)){
						dOffsetBX -= 1;
					}
					break;
				case 63235: //Right
					if(dOffsetAX < dMaxOffsetStepAX && dOffsetAX >= (-dMaxOffsetStepAX)){
						dOffsetAX += 1;
					}
					if(dOffsetBX < dMaxOffsetStepBX && dOffsetBX >= (-dMaxOffsetStepBX)){
						dOffsetBX += 1;
					}
					break;
				case 114: //R key
					fScaleA = fScale4DisplayA ;
					fScaleB = fScale4DisplayB ;
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
				case -1: //timeout
					break;
				default:
					nSelection = 0;
					cerr << "Invalid selection." << endl;
					break;
				}

				//Check the mouse event
				//TODO:somethine wrong with the following snippet
				if((false == bLeftButtonDown ) ){
					if(true == bDragLeft){
						cout<<"DragLeft.................."<<endl;
						if(dOffsetAX <= dMaxOffsetStepAX && dOffsetAX > (-dMaxOffsetStepAX)){
							//dOffsetAX -= 1;
							dOffsetAX -= nDragScale;
						}
						if(dOffsetBX <= dMaxOffsetStepBX && dOffsetBX > (-dMaxOffsetStepBX)){
							//dOffsetBX -= 1;
							dOffsetBX -= nDragScale;
						}
					}
					if(true == bDragRight){
						cout<<"DragRight.................."<<endl;
						if(dOffsetAX < dMaxOffsetStepAX && dOffsetAX >= (-dMaxOffsetStepAX)){
							//dOffsetAX += 1;
							dOffsetAX += nDragScale;
						}
						if(dOffsetBX < dMaxOffsetStepBX && dOffsetBX >= (-dMaxOffsetStepBX)){
							//dOffsetBX += 1;
							dOffsetBX += nDragScale;
						}
					}
					if(true == bDragUp){
						if(dOffsetAY <= dMaxOffsetStepAY && dOffsetAY > (-dMaxOffsetStepAY)){
							//dOffsetAY -= 1;
							dOffsetAY -= nDragScale;
						}
						if(dOffsetBY <= dMaxOffsetStepBY && dOffsetBY > (-dMaxOffsetStepBY)){
							//dOffsetBY -= 1;
							dOffsetBY -= nDragScale;
						}
					}
					if(true == bDragDown){
						if(dOffsetAY < dMaxOffsetStepAY && dOffsetAY >= (-dMaxOffsetStepAY)){
							//dOffsetAY += 1;
							dOffsetAY += nDragScale;
						}
						if(dOffsetBY < dMaxOffsetStepBY && dOffsetBY >= (-dMaxOffsetStepBY)){
							//dOffsetBY += 1;
							dOffsetBY += nDragScale;
						}
					}
					bDragLeft=false;
					bDragRight=false;
					bDragUp=false;
					bDragDown=false;
				}

				if (bEnter) {
					cout << "Your selection is: " << nSelection << endl;
					if (nSelection == 1 || nSelection == 2) {
						resultList << strCat << " "<< strCompA << " " << strCompB << " " << nSelection << endl;
						nSelection = 0;
						fScaleA = fScale4DisplayA ;
						fScaleB = fScale4DisplayB ;
						dOffsetAX = 0;
						dOffsetAY = 0;
						dOffsetBX = 0;
						dOffsetBY = 0;
						break;
					} else {
						cerr << "Pleaes slelect 1 or 2 for the better one before iterate to next comparison." << endl;
					}
				}
			}
			cvDestroyWindow(category);
		}
	}

	return (int)0;
}
