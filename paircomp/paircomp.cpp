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
#include <vector>
#include <libgen.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

using namespace	std;
using namespace	cv;


static int	BUF_LENGTH = 1024;

//Parameters
static float	fCoef = 0.1;
static int	nOffset = 100;
int		nRepeatTimes = 1;
float		dRatio = 1.0;
static int nGoodWidth=600;
bool bLeftButtonDown;
bool bRightButtonDown;
bool bDragLeft;
bool bDragRight;
bool bDragUp;
bool bDragDown;

//Resource
string		strImgOne = "./1.png";
string		strImgTwo = "./2.png";
string		strImgQuestion = "./qm.png";
string strWindowName="PairwiseComparison";
string strIndicator="Indicator";

int 
usage(int argc)
{
	cout << "usage: paircomp -f fileList -k [Repeat Times] -r [Random Ratio]\n" << endl;
	cout << "-f: List file." << endl;
	cout << "-k: Repeat times. Must be integer and greater than 0" << endl;
	cout << "-r: Random ratio. Must be the value between (0.0,1.0]" << endl;
	cout << endl;
	if (argc < 2) {
		return -1;
	}
	return 0;
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
			bLeftButtonDown=true;
			break;
        case CV_EVENT_LBUTTONUP: 
			bLeftButtonDown=false;
			break;
        case CV_EVENT_MOUSEMOVE: 
			//if(((flags & CV_EVENT_FLAG_LBUTTON) == CV_EVENT_FLAG_LBUTTON)) {
			if(true == bLeftButtonDown){
				if(abs(x - nLastPointX) > abs( y - nLastPointY)){ //x axis
					if(( x - nLastPointX) < 0 ){
						bDragRight=true;
					}
					if(( x - nLastPointX) > 0 ){
						bDragLeft=true;
					}
				}
				if(abs(x - nLastPointX) < abs( y - nLastPointY)){ //y axis
					if(( y - nLastPointY) < 0 ){
						bDragUp=true;
					}
					if(( y - nLastPointY) > 0 ){
						bDragDown=true;
					}
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

	//Result file
	ofstream resultList("result.txt");

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
			default_random_engine e(time(0));
			uniform_real_distribution < double >u(0.0, 1.0);
			double dValue=u(e);

			cout<<"Random value: "<<dValue<<endl;

			if (dValue > dRatio) {
				cout<<"Skip one due to the random ratio setting."<<endl;
				continue;
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

			//cvNamedWindow(category);
			cvNamedWindow(strWindowName.c_str());
			cvSetMouseCallback(strWindowName.c_str(), mouseCallback, NULL);

			int		nSelection = 0;
			double   fScale4Display = std::min(1.0,((double)nGoodWidth/matAOrig.cols));
			double   fScale = fScale4Display;
			int		xOffset = 0;
			int		yOffset = 0;
			int nMaxOffsetStepX=0;
			int nMaxOffsetStepY=0;

			//cout<<"scale:"<<fScale4Display<<endl;

			resize(matAOrig, matA, Size(0, 0), fScale4Display, fScale4Display, INTER_LINEAR);
			resize(matBOrig, matB, Size(0, 0), fScale4Display, fScale4Display, INTER_LINEAR);

			while (1) {
				cout << "Scale: " << fScale << endl;
				cout << "xOffset: "<< xOffset <<endl;
				cout << "yOffset: "<< yOffset <<endl;

				resize(matAOrig, matAProcessed, Size(0, 0), fScale, fScale, INTER_LINEAR);
				resize(matBOrig, matBProcessed, Size(0, 0), fScale, fScale, INTER_LINEAR);
				//if (fScale > 1.0) {
				if (fScale > fScale4Display) {
					nMaxOffsetStepX=max(abs(matAProcessed.cols - matA.cols)/nOffset,abs(matBProcessed.cols-matB.cols)/nOffset)/2;
					nMaxOffsetStepY=max(abs(matAProcessed.rows - matA.rows)/nOffset,abs(matBProcessed.rows-matB.rows)/nOffset)/2;

					/*				
					cout << "Original imageA size: ("<<matA.cols << "," << matA.rows <<")"<< endl;
					cout << "Original imageB size: ("<<matB.cols << "," << matB.rows <<")"<< endl;
					cout << "Zoomed imageA size: ("<<matAProcessed.cols << "," << matAProcessed.rows <<")"<< endl;
					cout << "Zoomed imageB size: ("<<matBProcessed.cols << "," << matBProcessed.rows <<")"<< endl;
					cout << "Max X/Y offset step: "<<nMaxOffsetStepX<<"/"<<nMaxOffsetStepY<<endl;
					*/

					int nPlanOffsetX= xOffset * nOffset ;
					int nPlanOffsetY= yOffset * nOffset ;

					int		nDiffAX = matAProcessed.cols - matA.cols;
					int		nDiffAY = matAProcessed.rows - matA.rows;
					int		nNewAX = nDiffAX / 2;
					int		nNewAY = nDiffAY / 2;

					if ((nNewAX + matA.cols + nPlanOffsetX) >= matAProcessed.cols) {
						nNewAX = matAProcessed.cols - matA.cols;
					}
					else{
						if ((nNewAX + nPlanOffsetX) <= 0) {
							nNewAX = 0;
						}
						else{
							nNewAX += nPlanOffsetX;
						}
					}

					if ((nNewAY + matA.rows + nPlanOffsetY) >= matAProcessed.rows) {
						nNewAY = matAProcessed.rows - matA.rows;
					}
					else{
						if ((nNewAY + nPlanOffsetY) <= 0) {
							nNewAY = 0;
						}
						else{
							nNewAY += nPlanOffsetY;
						}
					}

					int		nDiffBX = matBProcessed.cols - matB.cols;
					int		nDiffBY = matBProcessed.rows - matB.rows;
					int		nNewBX = nDiffBX / 2;
					int		nNewBY = nDiffBY / 2;

					if ((nNewBX + matB.cols + nPlanOffsetX) >= matBProcessed.cols) {
						nNewBX = matBProcessed.cols - matB.cols;
					}
					else{
						if ((nNewBX + nPlanOffsetX) <= 0) {
							nNewBX = 0;
						}
						else{
							nNewBX += nPlanOffsetX;
						}
					}

					if ((nNewBY + matB.rows + nPlanOffsetY) >= matBProcessed.rows) {
						nNewBY = matBProcessed.rows - matB.rows;
					}
					else{
						if ((nNewBY + nPlanOffsetY) <= 0) {
							nNewBY = 0;
						}
						else{
							nNewBY += nPlanOffsetY;
						}
					}

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

				Size szText=getTextSize(strCat,FONT_HERSHEY_COMPLEX,3,5,0);
				Point ptOrg((matIndicator.cols - szText.width)/2,(matIndicator.rows)/2);
				putText(matIndicator,strCat,ptOrg,FONT_HERSHEY_COMPLEX,3,Scalar(0,255,0));
				cv::imshow(strIndicator, matIndicator);

				int		input_key = cvWaitKey(100);
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
					if (fScale < 2) {
						fScale += fCoef;
					}
					break;
				case 45: //-
					if (fScale > 0.2) {
						fScale -= fCoef;
					}
					break;
				case 63232: //Up   #TODO:Check the key on different platform
					if(yOffset < nMaxOffsetStepY && yOffset >= (-nMaxOffsetStepY)){
						yOffset += 1;
					}
					break;
				case 63233: //Down
					if(yOffset <= nMaxOffsetStepY && yOffset > (-nMaxOffsetStepY)){
						yOffset -= 1;
					}
					break;
				case 63234: //Left
					if(xOffset <= nMaxOffsetStepX && xOffset > (-nMaxOffsetStepX)){
						xOffset -= 1;
					}
					break;
				case 63235: //Right
					if(xOffset < nMaxOffsetStepX && xOffset >= (-nMaxOffsetStepX)){
						xOffset += 1;
					}
					break;
				case 114: //R key
					fScale = fScale4Display ;
					xOffset = 0;
					yOffset = 0;
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
				if((false == bLeftButtonDown ) ){
					if(true == bDragLeft){
						if(xOffset <= nMaxOffsetStepX && xOffset > (-nMaxOffsetStepX)){
							xOffset -= 1;
						}
					}
					if(true == bDragRight){
						if(xOffset < nMaxOffsetStepX && xOffset >= (-nMaxOffsetStepX)){
							xOffset += 1;
						}
					}
					if(true == bDragUp){
						if(yOffset < nMaxOffsetStepY && yOffset >= (-nMaxOffsetStepY)){
							yOffset += 1;
						}
					}
					if(true == bDragDown){
						if(yOffset <= nMaxOffsetStepY && yOffset > (-nMaxOffsetStepY)){
							yOffset -= 1;
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
						fScale = fScale4Display ;
						xOffset = 0;
						yOffset = 0;
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
