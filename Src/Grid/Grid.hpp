#ifndef Grid_hpp
#define Grid_hpp

#include "../GDS2_Read_Decomposition/Rectangle.hpp"

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

/*******************************************************************
 * Gird Definition
*/
template <typename T>
struct Grid
{
    Coor<T> BasePoint;
    double slack_density;
    double size;

    std::vector< Rect<T> > rectangles;

    void calculate_slack_density(){
        double fillable_area = 0;
        for(auto rect = rectangles.begin(); rect != rectangles.end(); rect++){
            fillable_area += rect->Area();
        }
        slack_density = fillable_area / (size*size);
    }

    void output_file(string filename){
        // open filename in std::ios::app
        std::ofstream file;
        file.open(filename, std::ios::app);

        file << "<grid>" << std::endl;
        file << slack_density << std::endl;
        for(auto rect = rectangles.begin(); rect != rectangles.end(); rect++){
            file << rect->getTL() << "," << rect->getBR() << std::endl;
        }
        file << "</grid>" << std::endl;

        file.close();
    }
};

#endif