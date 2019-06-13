#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include "vopos.hpp"


/*
Master positioning class constructor. Is called after the inherited classes are constructed
Inherited class constructors are called with relevant arguments after the ":" in the initialization list.
*/
pos::positioning::positioning(int opticalFlow_mode,
                                int visualOdometry_mode,
                                int arucoDictionary,                            //Example: cv::aruco::DICT_4X4_50
                                int maxID,
                                std::string anchorPath,
                                int flowGrid,
                                cv::Rect2f roi_rect,
                                cv::Mat_<float> K,
                                cv::Mat_<float> T):
        ang::angulation(maxID,anchorPath),                          //Angulation constructor
        of::opticalFlow(opticalFlow_mode,flowGrid,roi_rect.width),  //Optical flow constructor
        vo::planarHomographyVO(visualOdometry_mode),                //Homography constructor
        roi(roi_rect)                                               //Assign argument to positioning attribute
{
    //Set some settings for angulation object
    ang::angulation::setKmat(K);
    ang::angulation::setTmat(T);
    minAnchors = 2;//Descider wether to try angulation or not
    //Initialize the aruco dictionary
    dictionary = cv::aruco::getPredefinedDictionary(arucoDictionary);
    //Set some settings for Optical Flow object
    of::opticalFlow::setDefaultSettings();
    //Set some settings for Visual Odometry object
    vo::planarHomographyVO::setKmat(K,roi_rect);
    vo::planarHomographyVO::setTmat(T);
    vo::planarHomographyVO::setDefaultSettings();
}
/*
Process the given data and update the position and yaw
*/
int pos::positioning::process(int mode,cv::Mat& frame, float dist,float roll, float pitch,float& yaw, cv::Mat_<float>& pos){
    //Aruco detect and draw
    std::vector<int> ids;
    std::vector<std::vector<cv::Point2f> > corners;
    cv::aruco::detectMarkers(frame, dictionary, corners, ids);



    return 1;
}
/*
Process the given data and update position and yaw. Also illustrate by drawing on the outputFrame mat
dist - Distance from camera to the flow field plane.
*/
int pos::positioning::processAndIllustrate(int mode,cv::Mat& frame, cv::Mat& outputFrame,int illustrate_flag,float dist,float roll, float pitch,float& yaw, cv::Mat_<float>& pos){
    static cv::Mat subPrevFrame; //Static init of prev subframe for optical flow field
    //Aruco detect and draw
    std::vector<int> ids;
    std::vector<std::vector<cv::Point2f> > corners;
    cv::aruco::detectMarkers(frame, dictionary, corners, ids);
    //Draw markers
    cv::aruco::drawDetectedMarkers(outputFrame, corners, ids, CV_RGB(0,250,0));
    //Only do angulation if at least two known anchors are visible
    bool success = false;
    int returnMode = pos::RETURN_MODE_AZIPE;
    if(ids.size()>=minAnchors){
        std::vector<bool> mask;
        success = ang::angulation::calculate(corners,ids,mask,pos,yaw,roll,pitch);
    }
    if(!success){
        //Get flow field
        std::vector<cv::Point2f> features;
        std::vector<cv::Point2f> updatedFeatures; //The new positions estimated from KLT
        of::opticalFlow::getFlow(subPrevFrame,frame(roi),features,updatedFeatures);
        //Draw flow field arrows
        float scale = 10;
        cv::Point2f focusOffset(roi.x,roi.y);
        drawArrows(outputFrame,features,updatedFeatures,scale,focusOffset);
        cv::rectangle(outputFrame,roi,CV_RGB(255,0,0),2,cv::LINE_8,0);
        success = vo::planarHomographyVO::process(features,updatedFeatures,roll,pitch,dist,pos,yaw);
        if(success){returnMode = pos::RETURN_MODE_VO;}
        else{returnMode = pos::RETURN_MODE_INERTIA;}
    }


    frame(roi).copyTo(subPrevFrame);//Copy the newest subframe to subPrevFrame for use in next function call
    return returnMode;
}
/* Draws a closed loop between all given points
 */
void pos::positioning::drawLines(cv::Mat& img,std::vector<cv::Point2f> points,cv::Point2f offset){
    int size = points.size();
    int i = 0;
    while(i<(size-1)){
        cv::line(img,offset+points[i],offset+points[i+1],CV_RGB(255,0,0),2,cv::LINE_8,0);
        i++;
    }
    cv::line(img,offset+points[i],offset+points[0],CV_RGB(255,0,0),2,cv::LINE_8,0);//close loop
}

/* Fuses the VO position estimation with incomplete angulation measurement (Angular measurement to single anchor)
 * This is done by obtaining an expression for a possible 3D line where the vehicle can be positioned, assuming known pose, anchor position and angular measurement to anchor
 * (Pose obtained from IMU (Roll, Pitch), VO-algorithm (Yaw))
 * The initial 3d coordinate estimation from VO is projected onto the 3D line.
 * Theory wise will this prevent drift. But is sensitive of roll, pitch, yaw errors. Yaw may be pretty good estimated with VO. roll, pitch
 * is obtained from Gyro + Accelerometer to improve quality
 *

    cv::Mat_<float>& pos  : Inputoutputarray: Contains VO pos estimation. Will be updated with improved estimation
    std::vector<cv::Mat_<float>> q: Input array containing coordinate of known anchor.
    std::vector<cv::Mat_<float>> v; Input array containging uLOS vectors from vehicle to known anchor(s)
    std::vector<uchar> mask       : Input array containing a mask to choose which element of q to use. (Will choose first 1)
    float yaw, roll, pitch: Input floats :Pose info
 */
void projectionFusing(cv::Mat_<float>& pos,std::vector<cv::Mat_<float>> q, std::vector<cv::Mat_<float>> v, std::vector<uchar> mask,
                        float yaw, float roll,float pitch){
    //Create R mat describing the pose of the vehicle (Instead directly create inverted R-mat)
    //cv::Mat_<float> R = getXRot(roll)*getYRot(pitch)*getZRot(yaw);
    cv::Mat_<float> R_t = getZRot(-yaw)*getYRot(-pitch)*getXRot(-roll);
    //Find first element with known anchor
    std::vector<uchar>::iterator it = std::find(mask.begin(), mask.end(), 1);
    if(it!=mask.end()){
        int index = std::distance(mask.begin(), it);
        //Calculate v_tilde, which is the uLos vector expressed in global frame in negative direction. I.e vector pointing from anchor to vehicle
        cv::Mat_<float> v_tilde = -R_t*v[index];

    }

}
