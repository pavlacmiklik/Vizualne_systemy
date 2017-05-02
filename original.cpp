#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"

#include <iostream>
#include <ctype.h>

#define OBS_RADIUS_CM 6
#define OBS_DISTANCE 68
#define OBS_RADIUS_PX 91

#define FOCAL (OBS_RADIUS_PX * OBS_DISTANCE)/OBS_RADIUS_CM


using namespace cv;
using namespace std;


Point2f point;
bool addRemovePt = false;

static void onMouse( int event, int x, int y, int /*flags*/, void* /*param*/ )
{
    if( event == EVENT_LBUTTONDOWN )
    {
        point = Point2f((float)x, (float)y);
        addRemovePt = true;
    }
}

int realDistance (int DIST_PX) {
    return (int) ((OBS_RADIUS_CM*FOCAL)/DIST_PX);
}

void brightnessContrast(Mat image, double alpha, int beta) {
    for( int y = 0; y < image.rows; y++ ) {
        for( int x = 0; x < image.cols; x++ ) {
            for( int c = 0; c < 3; c++ ) {
                image.at<Vec3b>(y,x)[c] = saturate_cast<uchar>( alpha*( image.at<Vec3b>(y,x)[c] ) + beta);
            }
        }
    }
}

Mat maskImg(Mat image) {
    Mat mask;
    inRange(image, Scalar(0,0,0), Scalar(50,50,50),mask);
    Mat masked;
    bitwise_or(image, mask, masked);
    return masked;
}


int main( int argc, char** argv )
{
    VideoCapture cap(0);
    TermCriteria termcrit(TermCriteria::COUNT|TermCriteria::EPS,20,0.03);
    Size subPixWinSize(10,10), winSize(31,31);
    
    const int MAX_COUNT = 5000;
    bool needToInit = false;
    bool nightMode = false;
    
    
    cap.open(0);
    
    if( !cap.isOpened() )
    {
        cout << "Could not initialize capturing...\n";
        return 0;
    }
    
    namedWindow( "Optical Flow", 1 );
    setMouseCallback( "Optical Flow", onMouse, 0 );
    
    Mat gray, prevGray, image, frame;
    vector<Point2f> points[2];
    
    while(1)
    {
        cap >> frame;
        if( frame.empty() )
            break;
        
        frame.copyTo(image);
        cvtColor(image, gray, COLOR_BGR2GRAY);
        
        if( nightMode )
            image = Scalar::all(0);
        
        if( needToInit )
        {
            // automatic initialization
            goodFeaturesToTrack(gray, points[1], MAX_COUNT, 0.01, 10, Mat(), 3, 0, 0.04);
            cornerSubPix(gray, points[1], subPixWinSize, Size(-1,-1), termcrit);
            addRemovePt = false;
        }
        else if( !points[0].empty() )
        {
            vector<uchar> status;
            vector<float> err;
            if(prevGray.empty())
                gray.copyTo(prevGray);
            calcOpticalFlowPyrLK(prevGray, gray, points[0], points[1], status, err, winSize,
                                 3, termcrit, 0, 0.001);
            size_t i, k;
            for( i = k = 0; i < points[1].size(); i++ )
            {
                if( addRemovePt )
                {
                    if( norm(point - points[1][i]) <= 5 )
                    {
                        addRemovePt = false;
                        continue;
                    }
                }
                
                if( !status[i] )
                    continue;
                
                points[1][k++] = points[1][i];
                circle( image, points[1][i], 3, Scalar(0,255,0), -1, 8);
            }
            points[1].resize(k);
        }
        
        if( addRemovePt && points[1].size() < (size_t)MAX_COUNT )
        {
            vector<Point2f> tmp;
            tmp.push_back(point);
            cornerSubPix( gray, tmp, winSize, Size(-1,-1), termcrit);
            points[1].push_back(tmp[0]);
            addRemovePt = false;
        }
        
        medianBlur(gray, gray, 5);
        //brightnessContrast(gray, 1, 20);
        //gray = maskImg(gray);
        
        vector<Vec3f> circles;
        HoughCircles(gray, circles, HOUGH_GRADIENT, 5,
                     500, // change this value to detect circles with different distances to each other
                     100, 20, 20, 100 // change the last two parameters
                     // (min_radius & max_radius) to detect larger circles
                     );
        int radius = 0;
        
        for( size_t j = 0; j < circles.size(); j++ )
        {
            Vec3i c = circles[j];
            radius = cvRound(c[2]);
            circle( image, Point(c[0], c[1]), c[2], Scalar(0,0,255), 3, LINE_AA);
            circle( image, Point(c[0], c[1]), 2, Scalar(0,255,0), 3, LINE_AA);
            
        }
        
        char text[255];
        sprintf(text, "Vzdialenost: %d cm", realDistance(radius));
        int baseline = 0;
        Size textSize = getTextSize(text, FONT_ITALIC, 1.5, 4, &baseline);
        Point textOrg(gray.cols - textSize.width, 0 + textSize.height);
        
        
        putText (image, text,textOrg, FONT_ITALIC,1.5, cvScalar(255,255,255),4);
        printf("Real distance: %d cm\n",realDistance(radius));
        
        needToInit = false;
        imshow("Optical Flow", image);
        
        char c = (char)waitKey(10);
        if( c == 27 )
            break;
        switch( c )
        {
            case 'r':
                needToInit = true;
                break;
            case 'c':
                points[0].clear();
                points[1].clear();
                break;
            case 'n':
                nightMode = !nightMode;
                break;
        }
        
        std::swap(points[1], points[0]);
        cv::swap(prevGray, gray);
    }
    
    return 0;
}
