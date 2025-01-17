#include <iostream>
#include <iomanip>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv/cv.h"
#include "opencv/highgui.h"

using namespace cv;
using namespace std;

void rgb2gray(const Mat src, Mat &result)
{
    CV_Assert(src.depth() != sizeof(uchar)); //harus 8 bit

    result = Mat::zeros(src.rows, src.cols, CV_8UC1); //buat matrik 1 chanel
    uchar data;

    if(src.channels() == 3){

        for( int i = 0; i < src.rows; ++i)
            for( int j = 0; j < src.cols; ++j )
            {
                data = (uchar)(((Mat_<Vec3b>) src)(i,j)[0] * 0.0722 + ((Mat_<Vec3b>) src)(i,j)[1] * 0.7152 + ((Mat_<Vec3b>) src)(i,j)[2] * 0.2126);

                result.at<uchar>(i,j) = data;
            }


    }else{

        result = src;
    }

}

void invariantMomentHu(const Mat src, vector<double>& result)
{
    //momen dua dimensi
    double m00=0, m10=0, m01=0;
    int x,y;

    for( int i = 0; i < src.rows; ++i){
        for( int j = 0; j < src.cols; ++j ){
            x = j + 1;
            y = i + 1;
            m00 += src.at<uchar>(i,j);
            m10 += x * src.at<uchar>(i,j);
            m01 += y * src.at<uchar>(i,j);
        }
    }

    //central moment lainnya & kemudian normalized central moments
    double xbar = m10 / m00;
    double ybar = m01 / m00;

    double m11 = 0, m20 = 0, m02 = 0, m30 = 0,
           m03 = 0, m12 = 0, m21 = 0;

    for( int i = 0; i < src.rows; ++i){
        for( int j = 0; j < src.cols; ++j ){
            x = j + 1;
            y = i + 1;
            m11 += ( x - xbar ) * ( y - ybar ) * src.at<uchar>(i,j) / pow( m00, 2 );
            m20 += pow( ( x - xbar ), 2 ) * src.at<uchar>(i,j) / pow( m00, 2 );
            m02 += pow( ( y - ybar ), 2 ) * src.at<uchar>(i,j) / pow( m00, 2 );
            m30 += pow( ( x - xbar ), 3 )  * src.at<uchar>(i,j) / pow( m00, 2.5 );
            m03 += pow( ( y - ybar ), 3 ) * src.at<uchar>(i,j) / pow( m00, 2.5 );
            m12 += ( x - xbar ) * pow( ( y - ybar ), 2 ) * src.at<uchar>(i,j) / pow( m00, 2.5 );
            m21 += pow( ( x - xbar ), 2 ) * ( y - ybar ) * src.at<uchar>(i,j) / pow( m00, 2.5 );
        }
    }

    //ketujuh nilai invariant moments
    double moment;
    //1
    moment = m20 + m02;
    result.push_back(moment);

    //2
    moment = pow( m20 - m02, 2 ) + 4 * pow( m11, 2);
    result.push_back(moment);

    //3
    moment = pow( m30 - 3 * m12, 2 ) + pow( 3 * m21 - m03, 2);
    result.push_back(moment);

    //4
    moment = pow( m30 + m12, 2 ) + pow( m21 + m03, 2);
    result.push_back(moment);

    //5
    moment = ( m30 - 3 * m12 ) * ( m30 + m12 ) * ( pow( m30 + m12, 2 ) - 3* pow( m21 + m03, 2) )
             + ( 3 * m21 - m03 ) * ( m21 + m03 ) * ( 3 * pow( m30 + m12, 2 ) - pow( m21 + m03, 2) );
    result.push_back(moment);

    //6
    moment = ( m20 - m02 )  * ( pow( m30 + m12, 2 ) - pow( m21 + m03, 2) )
             + 4 * m11 * ( m30 + m12 ) * ( m21 + m03 );
    result.push_back(moment);

	/*opencv hu implementing*/
	{
	double t0 = m30 + m12;
    double t1 = m21 + m03;

    double q0 = t0 * t0, q1 = t1 * t1;

    //double n4 = 4 * m.nu11;
    //double s = m20 + m02;
    //double d = m20 - m02;

    t0 *= q0 - 3 * q1;
    t1 *= 3 * q0 - q1;

    q0 = m30 - 3 * m12;
    q1 = 3 * m21 - m03;

    moment = q1 * t0 - q0 * t1;
	}
    //7
    //moment = ( 3 * m21 - m03 )  * ( m21 + m30 )  * ( pow( m30 + m12, 2 ) - 3* pow( m21 + m03, 2) )
     //        - ( m30 + 3 * m12 )  * ( m21 + m03 )  * ( 3 * pow( m30 + m12, 2 ) - pow( m21 + m03, 2) );
    result.push_back(moment);
}

void printMoments(char label[] ,const vector<double> huMoments)
{
    cout<<label<<endl<<setprecision(50);;
    for(unsigned int i = 0; i < huMoments.size(); ++i){
        cout<<i+1<<" = "<<huMoments[i]<<endl;
    }
}

CvHuMoments cvHu(char * filename)
{
	IplImage*img = cvLoadImage(filename,0);
	cout << "hu of " << filename << endl;
	 CvMoments moments;
     CvHuMoments hu;     
	 cvMoments(img,&moments,0); 
     cvGetHuMoments(&moments, &hu);
     cout<<hu.hu1<<endl<<hu.hu2<<endl<<hu.hu3<<endl<<hu.hu4<<endl<<hu.hu5<<endl<<hu.hu6<<endl<<hu.hu7<<endl<<endl<<endl;
	 return hu;
}

double getScore(vector<double> Sa, vector<double> Ta) {
#if 0
	int i, sma, smb;
	double eps = 1.e-5;
    double mmm;
    double result = 0;
	for( i = 0; i < 7; i++ ) {
                double ama = fabs( ma[i] );
                double amb = fabs( mb[i] );

                if( ma[i] > 0 )
                    sma = 1;
                else if( ma[i] < 0 )
                    sma = -1;
                else
                    sma = 0;
                if( mb[i] > 0 )
                    smb = 1;
                else if( mb[i] < 0 )
                    smb = -1;
                else
                    smb = 0;

                if( ama > eps && amb > eps )
                {
                    ama = 1. / (sma * log10( ama ));
                    amb = 1. / (smb * log10( amb ));
                    result += fabs( -ama + amb );
                }
            
	}
	return result;
#endif

	//  计算相似度1
double dbR =0; //相似度
double dSigmaST =0;
    double dSigmaS =0;
    double dSigmaT =0;
    double temp =0;  
    {for(int i=0;i<7;i++)
    {
        temp = fabs(Sa[i]*Ta[i]);
        dSigmaST+=temp;
        dSigmaS+=pow(Sa[i],2);
        dSigmaT+=pow(Ta[i],2);
    }}
    dbR = dSigmaST/(sqrt(dSigmaS)*sqrt(dSigmaT));
	return dbR;
}

#if 0
int main(int /*argc*/, char /*argv*/)
{
    vector<double> huMoments;


    Mat g1 = imread("gpt.bmp");
    rgb2gray(g1, g1);
    invariantMomentHu(g1, huMoments);
    printMoments("gambar 1 : " ,huMoments);

    namedWindow("gambar1");
    imshow("gambar1", g1);

    vector<double> huMoments2;
    Mat g2 = imread("gp.bmp");
    rgb2gray(g2, g2);
    invariantMomentHu(g2, huMoments2);
    printMoments("gambar 2 : " ,huMoments2);

	cout << "Score: " << getScore(huMoments, huMoments2) << endl;

    namedWindow("gambar2");
    imshow("gambar2", g2);

    vector<double> huMoments3;
    Mat g3 = imread("gp3.bmp");
    rgb2gray(g3, g3);
    invariantMomentHu(g3, huMoments3);
    printMoments("gambar 3 : ", huMoments3);

	cout << "Score: " << getScore(huMoments, huMoments3) << endl;
    namedWindow("gambar3");
    imshow("gambar3", g3);

    vector<double> huMoments4;
    Mat g4 = imread("gp4.bmp");
    rgb2gray(g4, g4);
    invariantMomentHu(g4, huMoments4);
    printMoments("gambar 4 : ", huMoments4);

	cout << "Score: " << getScore(huMoments, huMoments4) << endl;
    namedWindow("gambar4");
    imshow("gambar4", g4);

	//cvHu("gpt.bmp");
	//cvHu("gp.bmp");

	//cvHu("gp3.bmp");
	//cvHu("gp4.bmp");


    waitKey(0);
	return 0;
}
#endif