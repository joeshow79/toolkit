/*
 *
 * Author:jason jiao
 *
 * History:
 * 2017-11-21: initial version
 * 
 * Reference:
 * http://blog.csdn.net/zhoufan900428/article/details/45890053
 *
 *
*/

#include <iostream>  
#include <iomanip>
#include <fstream>
#include <algorithm>
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

using namespace std;  
using namespace cv;

static int BUF_LENGTH=256;
static int LANDMARK_NUM=101;

CvScalar mask_color(0x0,0xff,0x0);

int usage(int argc){
	cout<<"usage: face_landmark_viewer <Face_Image_Dir_Path>\n"<<endl;
	if ( argc != 2 )
	{
		return -1;
	}
	return 0;
}

int main(int argc, char** argv)
{
	char ch;
	while((ch = getopt(argc, argv, "h")) != -1){  
  
        switch(ch){  
            case 'h':  
				usage(argc);
                break;  
            default:  
                cout<<"invalid  option: "<< ch<<endl;;  
                break;  
        }  
    }  

	int result=usage(argc);
	if(result!=0){
		return result;
	}

	//Iterator the data dir
	char* dir_name=argv[1];
    if( NULL == dir_name )  
    {  
        cout<< "<Image_Dir_Path> is not specified! "<<endl;  
        return -1;  
    }  
  
    // check if dir_name is a valid dir  
    struct stat s;  
    lstat( dir_name , &s );  
    if( ! S_ISDIR( s.st_mode ) )  
    {  
        cout<<"<Image_Dir_Path>: "<<string(dir_name)<<" is not a valid directory!"<<dir_name<<endl;  
        return -1;  
    }  
      
    struct dirent * file_name;    // return value for readdir()  
    DIR * dir;                   // return value for opendir()  
    dir = opendir( dir_name );  
    if( NULL == dir )  
    {  
        cout<<"Can not open dir"<<dir_name<<endl;  
        return -1;  
    }  
      
	cout<<"1"<<endl;
    /* read all the files in the dir ~ */  
    while( ( file_name = readdir(dir) ) != NULL )  
    {  
        // get rid of . and ..  
        if( strcmp( file_name->d_name , "." ) == 0 ||   
            strcmp( file_name->d_name , "..") == 0    )  
            continue;  

		string file_str(file_name->d_name);
		//TBD: hard code for the list of jpg files only
		if(file_str.compare(file_str.size()-3,3,"jpg")==0){

			string orig_image_name=string(dir_name)+"/"+file_str;
			string mark_file_name=string(dir_name)+"/"+file_str.replace(file_str.size()-3,3,"pts");

			cout<<"processing ..."<<endl;
			cout<<orig_image_name<<endl;
			cout<<mark_file_name<<endl;

			cv::Mat orig=cv::imread(orig_image_name.c_str());

			if (orig.empty())
			{
				printf("Can not open image, skip to next one \n");
				continue;
			}

			//read landmark data
			ifstream landmark_file(mark_file_name.c_str());
			if(!landmark_file){
				cerr<<"Unable to read landmark file, skip to next one!"<<endl;
				continue;
			}
			

			char buf[BUF_LENGTH];
			float x,y;
			int landmark[LANDMARK_NUM][2];
			int index=0;
			int pos=0;

			while(!landmark_file.eof()){
				landmark_file.getline(buf,256);
				cout<<buf<<endl;
				if(index++<3){
					continue;
				}
				sscanf(buf,"%f %f",&x,&y);
				cout<<x<<","<<y<<endl;
				landmark[pos][0]=x;
				landmark[pos][1]=y;

				if(++pos >= LANDMARK_NUM){
					break;
				}
			}

			int radius=max(orig.cols/720,1);

			for(int i=0;i<LANDMARK_NUM;++i){
				cv::circle(orig,cv::Point(landmark[i][0],landmark[i][1]),radius,mask_color,-1);
			}

			cvNamedWindow("Face Landmark Viewer");

			while(1)
			{
				cv::imshow("FaceLandmarkViewer",orig);
		
				int input_key=cvWaitKey(1);

				if(input_key == 27 || input_key==1048603) return 1;    //wait ESC key
				if(input_key == 13 || input_key==1048586) {    //wait Enter key
					break;
				}
			}
		
			cvDestroyWindow("MaskTuner");
		}
    }  

    return (int)0;
}

