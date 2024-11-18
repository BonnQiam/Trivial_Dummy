#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <string>

#include <vector>
#include <iterator>
#include <set>

#include <limbo/parsers/gdsii/gdsdb/GdsIO.h>
#include <limbo/preprocessor/Msg.h>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/adapted/boost_polygon.hpp>
#include <boost/geometry/geometries/geometries.hpp>

#include "../GDS2_Read_Decomposition/ScanLine_Edge_Decomposition.hpp"
#include "Grid.hpp"

typedef std::vector<GdsParser::GdsDB::GdsCell>                                                  Structure_s;
typedef std::vector<std::pair<GdsParser::GdsRecords::EnumType, GdsParser::GdsDB::GdsObject*> >  Element_s;

typedef GdsParser::GdsDB::GdsShape::coordinate_type                                             Coordinate_type;
typedef std::vector< boost::polygon::point_data<GdsParser::GdsDB::GdsShape::coordinate_type> >  Coordinate_s;

typedef GdsParser::GdsDB::GdsPolygon                                                    Polygon;

struct layer{
    int layer;
    std::vector<Polygon> polygon_s;
};

//contest configure: the window size is 20 microns, the gdsii unit si 1 nm, i.e., 0.001 micron
#define grid_size  20000

void GDSII_Decomposition(char* gdsii_file);

int main(int argc, char** argv)
{   
    /**********************************
     * 
     * Read GDSII, decompose the polygon into rectangles, write the rectangles to file
     * 
     * *******************************/
#if 0
    char* gdsii_file = argv[1];
    GDSII_Decomposition(gdsii_file);
#endif

#if 1
    /**********************************
     * 
     * Read the rectangles from file, establish the grid system, write the grid to file
     * 
     * *******************************/

    std::vector < Rect<int> > rectangles;

    // read from rectangle.txt
    //std::ifstream file("rectangle_test.txt");
    char* filename = argv[1]; 
    int x_grid_num = std::stoi(argv[2]);
    int y_grid_num = std::stoi(argv[3]);

    std::ifstream file(filename);

    std::regex r("\\((\\d+), (\\d+)\\)");
    std::smatch m;
    
    std::string line;
    while(std::getline(file, line)){
        std::regex_iterator<std::string::iterator> rit(line.begin(), line.end(), r);
        std::regex_iterator<std::string::iterator> rend;
        while (rit != rend) {
            Coor<int> Topleft(std::stoi((*rit)[1]), std::stoi((*rit)[2]));
            ++rit;
            Coor<int> Bottomright(std::stoi((*rit)[1]), std::stoi((*rit)[2]));
            ++rit;

            Coor<int> Bottomleft(Topleft.getX(), Bottomright.getY());
            Coor<int> Topright(Bottomright.getX(), Topleft.getY());
            Rect<int> rect(Bottomleft, Topright);

            rectangles.push_back(rect);
        }
    }

    std::cout << "The number of rectangles is " << rectangles.size() << std::endl;

    // establish the grid system
    string grid_file = "grid.txt";
    //clear the file
    std::ofstream ofs;
    ofs.open(grid_file, std::ofstream::out | std::ofstream::trunc);
    ofs.close();

    std::vector< Grid<int> > grids;
    for(int i=0; i<x_grid_num; i++){
        for(int j=0; j<y_grid_num; j++){
            Coor<int> BasePoint(i*grid_size, j*grid_size);
            Grid<int> grid;
            grid.BasePoint = BasePoint;
            grid.size = grid_size;
            grid.rectangles.clear();
            grids.push_back(grid);
        }
    }
    for(auto grid = grids.begin(); grid != grids.end(); grid++){
    //for(auto grid = grids.begin(); grid != grids.begin()+2; grid++){
        Coor<int> grid_Top_right = grid->BasePoint + Coor<int>(grid_size, grid_size);
        Rect<int> grid_rect(grid->BasePoint, grid_Top_right);

        for(auto rect = rectangles.begin(); rect != rectangles.end(); rect++){
            Rect<int> grid_intersection = Rectangle_intersection(grid_rect, *rect);
            // check the grid_intersection is not Rectangle<T>(Coor<T>(0, 0), Coor<T>(0, 0))
            if(grid_intersection.getTL() != grid_intersection.getBR()){
                grid->rectangles.push_back(grid_intersection);
            }
        }

        grid->calculate_slack_density();
        grid->output_file(grid_file);
        std::cout << "The grid at " << grid->BasePoint << " has " << grid->rectangles.size() << " rectangles" << std::endl;
    }
#endif

    return 0;
}

void GDSII_Decomposition(char* gdsii_file){
    GdsParser::GdsDB::GdsDB db;
    GdsParser::GdsDB::GdsReader reader (db);
    //limboAssert(reader(argv[1]));
    limboAssert(reader(gdsii_file));

#if 1
    // read the gdsii and get unit
    double unit = db.unit();

    std::cout << "unit: " << unit << std::endl;

#endif

#if 1
    std::vector<layer> Layer_s;
    // read the gdsii and get ploygon vector
    for (Structure_s::const_iterator str = db.cells().begin(); str != db.cells().end(); ++str){

        std::cout << "cell: " << str->name() << std::endl;

        for(Element_s::const_iterator ele = str->objects().begin(); ele != str->objects().end(); ++ele){
            if(ele->first == GdsParser::GdsRecords::BOUNDARY){
                int ele_layer = dynamic_cast<Polygon*>(ele->second)->layer();
                
                if(Layer_s.size() == 0){
                    layer temp;
                    temp.layer = ele_layer;
                    temp.polygon_s.push_back(*dynamic_cast<Polygon*>(ele->second));
                    Layer_s.push_back(temp);
                }
                else{
                    bool flag = false;
                    for(int i=0; i < Layer_s.size(); i++){
                        if(Layer_s[i].layer == ele_layer){
                            Layer_s[i].polygon_s.push_back(*dynamic_cast<Polygon*>(ele->second));
                            flag = true;
                            break;
                        }
                    }
                    if(!flag){
                        layer temp;
                        temp.layer = ele_layer;
                        temp.polygon_s.push_back(*dynamic_cast<Polygon*>(ele->second));
                        Layer_s.push_back(temp);
                    }
                }
            }
        }
    }

    for(int i=0; i < Layer_s.size(); i++){
        std::cout << "Layer: " << Layer_s[i].layer << std::endl;
        std::cout << "Polygon size: " << Layer_s[i].polygon_s.size() << std::endl;
        std::cout << "======================================================" << std::endl;
    }
#endif

#if 0
    // find the min/max coordinate
    Coordinate_type min_x = 0;
    Coordinate_type min_y = 0;
    Coordinate_type max_x = 0;
    Coordinate_type max_y = 0;

    int length = Layer_s.size();

    for(int i=0; i < length; i++){
        int length_polygon_s = Layer_s[i].polygon_s.size();

        for(int j=0; j < length_polygon_s; j++){
            GdsParser::GdsDB::GdsPolygon test = Layer_s[i].polygon_s[j];
            for(Coordinate_s::const_iterator coor = test.begin(); coor != test.end(); coor++){
                if(coor->x() < min_x){
                    min_x = coor->x();
                }
                if(coor->y() < min_y){
                    min_y = coor->y();
                }
                if(coor->x() > max_x){
                    max_x = coor->x();
                }
                if(coor->y() > max_y){
                    max_y = coor->y();
                }
            }
        }
    }

    std::cout << "min_x: " << min_x << std::endl;
    std::cout << "min_y: " << min_y << std::endl;
    std::cout << "max_x: " << max_x << std::endl;
    std::cout << "max_y: " << max_y << std::endl;

    int X_grid_num = ((max_x-min_x)%grid_size == 0) ? (max_x-min_x)/grid_size : (max_x-min_x)/grid_size+1;
    int Y_grid_num = ((max_y-min_y)%grid_size == 0) ? (max_y-min_y)/grid_size : (max_y-min_y)/grid_size+1;

    std::cout << "X_grid_num: " << X_grid_num << std::endl;
    std::cout << "Y_grid_num: " << Y_grid_num << std::endl;
    
#endif

#if 1
    // decompose the ploygon
    int length = Layer_s.size();
    for(int i = 0; i < length; i++){
        int length_polygon_s = Layer_s[i].polygon_s.size();
        std::string filename = "rectangle" + std::to_string(i) + ".txt";

        std::cout << "length: " << length_polygon_s << std::endl;

        for(int j=0; j < length_polygon_s; j++){
            std::cout << "Test " << j << "-th polygon" << std::endl;

            GdsParser::GdsDB::GdsPolygon test = Layer_s[i].polygon_s[j];

            std::vector< Coor<int> > polygon;
            std::vector< Rect<int> > result;

            for(Coordinate_s::const_iterator coor = test.begin(); coor != std::prev(test.end()); coor++){
                    if(coor == std::prev(test.end()) &&
                        coor->x() == test.begin()->x() &&
                        coor->y() == test.begin()->y()){
                        continue;
                    }
                    else{
                        polygon.push_back(Coor<int>(coor->x(), coor->y()));
                    }
            }

            Polygon_edge_collection<int> collection(polygon);
            Edge_based_decomposition(collection, result);

            std::cout << j << "-th polygon is decomposed successfully" << std::endl;

            //std::vector < Rectangle<int> > rectangle_result;
            std::ofstream file(filename, std::ios::app);
            // clear the file
            for(auto rect: result){
                // output Topleft and Bottomright to file
                file << rect.getTL() << "," << rect.getBR() << std::endl;
            }
            file.close();
        }
    }
#endif
}