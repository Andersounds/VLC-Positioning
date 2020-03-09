//#include <boost/lambda/lambda.hpp>
#include <iostream>
#include <fstream> //Input stream from file
//#include <iterator>
//#include <algorithm>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
//#include <iostream>
//#include <fstream> //Input stream from file
#include <opencv2/opencv.hpp>



/*
    How its is to be used.
        - Every program defines their own program options.
        - This is done in a separate function defined in the main file
        - The function defines options, their default values
        - Reads argc argc
        - For each option, one conversion is done to a correct format, and then added to a map with the same keys
        - This map is passed back to main, which accesses values using the same keys as settings.


        - How to write config file?
        - Order options in categories
            -multiple sources
                https://www.boost.org/doc/libs/1_72_0/doc/html/program_options/tutorial.html
        - Write parameter description as comment in config file
            https://www.boost.org/doc/libs/1_55_0/doc/html/program_options/overview.html#idp163379208
        - config file multitoken
            - in ini-file, one line can only be one value.
            - How can multitoken be handled properly? Ideas below.
                1. Do not use multitoken. Instead specify as single string and write parser for it.
                    - Simple parser that
                        -Goes through whole string
                            -if char is numeric
                            -read all numbers in a row, allowing a single point '.' somewhere
                            -Convert this string to either int or float depending on option
                        - This will allow to write pretty much however. [1,2,3;4,5,6] | 1 2 3 4 5 6 etc.


    Additional options:
        - Write default settings file
            - Possibly at some specified path
        - Settings file must be given as argument.
            - Can give just settings file, and its directory will be parsed, and all paths in file are relative to this path
            - Option to specifically specify base path, i.e path to some other dataset. Other paths will then be relative to this instead
        - ARguments given om command line should override arguments in ini file.
*/



/*
    Method that takes a string, and converts it to a opencv mat_<float>
    Possibly overload this with int versions
*/

//int string2CVMat(std::string str0,cv::Mat_<float>& cvMat){
int string2CVMat(std::string str0){
        std::cout << "raw:  " << str0 << std::endl;
    boost::trim_if(str0,boost::is_any_of("[]"));//Trim brackets
        std::cout << "trim: " << str0 << std::endl;
    std::vector<std::string> SplitVec;
    boost::split(SplitVec, str0, boost::is_any_of(";"));//Split into rows
    int rows = SplitVec.size();
    std::cout << "Rows: " << rows << std::endl;
        std::cout << "split: ";
        for(std::string i:SplitVec){
            std::cout << i << std::endl;
        }
        /*int cols = -1;

    for(std::string i:SplitVec){
        std::vector<std::string> rowStr;
        boost::split(rowStr, i, boost::is_any_of(","));//Must have ',' as column delimiter
        cols = row.size();




}
cv::Mat_<float> A = cv::Mat(rows, cols, CV_32FC1,cv::Scalar::all(0));*/

std::cout << "This is NICE mthod! define as vector with push back and then reshape to matrix" << std::endl;
    std::vector<float> V = {1,2,3,4,5,6};
    cv::Mat V2 = cv::Mat(V).reshape(3);
    std::cout << V2 << std::endl;



    // split first row into elements and create cv mat
    std::vector<std::string> row;
    boost::split(row, SplitVec[0], boost::is_any_of(","));//Must have ',' as column delimiter
    int cols = row.size();
    std::cout << "columns: " << cols << std::endl;
    cv::Mat_<float> A = cv::Mat(rows, cols, CV_32FC1,cv::Scalar::all(0));
    for(int i=0;i<cols;i++){
        std::string elementStr = boost::trim_copy(row[i]);//remove whitespaces
        float element = std::stof(elementStr);
        A(0,i) = element;
    }
    std::cout << "Matrix: " << A << std::endl;
    for(int i=1;i<rows;i++){//Continue with additional rows if there are any

        std::cout << "" << std::endl;
    }
    for(std::string i:row){
        std::cout << i << std::endl;
    }
    //boost::trim(str0); //Remove any leading and trailing whitespaces (unnecessary?)
    //boost::trim_right_if()
    //iends_with(filename, ".exe")
//boost::trim_left_if(str0,boost::is_any_of("["));
/*
    - Remove brackets [] throw error if not there boost::trimming för whitespace
    - Split into rows using ';' as delimiter
    - Split into elements using either whitespace or ',' as delimiter
     - If ',' are used, remove leading and trailing whitespaces as well - boost::trimming
    - Add elements to matrix.
    - copy over to inputoutput matrix
*/

    //std::vector<std::string> rows;
return 1;
}



/*
INI file of following syntax


ROLL_COLUMN=4 #Works
OUT=testi 1 2 4     #Works
ROLL_INIT=1.2   #Works



Will add string2Mat which will allow:
K_MAT = [1,2,3;1,2,4;1,2.5,3]
*/


//https://www.pyimagesearch.com/2015/04/27/installing-boost-and-boost-python-on-osx-with-homebrew/
int main(int argc, char** argv)
{
    namespace po = boost::program_options;
    po::options_description parameters("Options");
    parameters.add_options()
        ("help,h",  "Print help messages")
        ("OUT,o",   po::value<std::string>(), "Write output data to specified file. No output is not set")// Single string argument
        //Parameters
        ("RES_XY",  po::value<std::vector<int> >()->multitoken(), "Camera resolution in X and Y direction")
        ("K_MAT",   po::value<std::vector<float> >()->multitoken(), "Camera K matrix specified as float numbers row by row separated by whitespace") //Tänk om man kan definiera denna direkt som en opencv mat och ge 9 argument på rad?
        ("T_MAT",   po::value<std::vector<float> >()->multitoken(), "UAV - camera T matrix specified as float numbers row by row separated by whitespace")
        ("CAMERA_BARREL_DISTORTION",    po::value<std::vector<float> >()->multitoken(), "Barrel distortion coefficients K1, K2, K3 as floats")
        ("OPTICAL_FLOW_GRID",           po::value<int>(),"Sqrt of number of optical flow vectors")//Single int
        ("XYZ_INIT",                    po::value<std::vector<float> >()->multitoken(), "Initial position expressed as X Y Z coordinate floats")
        ("ROLL_INIT", po::value<float>(),"Initial roll of UAV, radians")
//        ("PITCH_INIT", po::value<float>(),"Initial pitch of UAV, radians")
//        ("YAW_INIT", po::value<float>(),"Initial yaw of UAV, radians")
        ("ROLL_COLUMN", po::value<int>(),"Specifies which column of csv file that contains roll data (0-indexed)")
//        ("PITCH_COLUMN", po::value<int>(),"Specifies which column of csv file that contains roll data (0-indexed)")
//        ("YAW_COLUMN", po::value<int>(),"Specifies which column of csv file that contains roll data (0-indexed)")
        ;







    // Parse command line
/*    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, parameters), vm);
    po::notify(vm);
*/

//Parse config file
    std::ifstream ini_file("config.ini");
    po::variables_map vm;
    //What are arguments to parse config file?
    po::store(po::parse_config_file(ini_file, parameters, true), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << parameters << "\n";
        std::cout << "HELP" << std::endl;
        return 1;
    }

    if (vm.count("ROLL_COLUMN")) { //Funkar
        std::cout << "Compression level was set to "
     << vm["ROLL_COLUMN"].as<int>() << ".\n";
    } else {
        std::cout << "CRES_XY was not set.\n";
    }
    if (vm.count("OUT")) { //Funkar
        std::cout << "output file: " << vm["OUT"].as<std::string>() << std::endl;

        string2CVMat(vm["OUT"].as<std::string>());
    }else{
        std::cout << "OUT was not set.\n";
    }
    if (vm.count("ROLL_INIT")) { //Funkar
        std::cout << "Float:" << vm["ROLL_INIT"].as<float>() << std::endl;
    }else{
        std::cout << "float not set.\n";
    }

    if (vm.count("RES_XY")) { //funkar
        std::vector<int> resolution = vm["RES_XY"].as<std::vector<int> >();
        for(int i:resolution){
            std::cout << i << ", ";
        }
        std::cout << std::endl;
    }else{
        std::cout << "res int vector not set.\n";
    }


}

//g++ -std=c++11 -fvisibility=hidden /usr/local/lib/libboost_program_options.a examples/boost_compile_test.cpp -o bin/example