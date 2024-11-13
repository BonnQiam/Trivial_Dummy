//! When needed to compile, rename the file to main.cpp
#include <iostream>
#include <vector>
#include <iterator>
#include <set>

#include <time.h>

#include <limbo/parsers/gdsii/gdsdb/GdsIO.h>
#include <limbo/preprocessor/Msg.h>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/adapted/boost_polygon.hpp>
#include <boost/geometry/geometries/geometries.hpp>

#include "ScanLine_Edge_Decomposition.hpp"

typedef std::vector<GdsParser::GdsDB::GdsCell>                                                  Structure_s;
typedef std::vector<std::pair<GdsParser::GdsRecords::EnumType, GdsParser::GdsDB::GdsObject*> >  Element_s;

typedef GdsParser::GdsDB::GdsShape::coordinate_type                                             Coordinate_type;
typedef std::vector< boost::polygon::point_data<GdsParser::GdsDB::GdsShape::coordinate_type> >  Coordinate_s;

typedef GdsParser::GdsDB::GdsPolygon                                                            Polygon;

struct layer{
    int layer;
    std::vector<Polygon> polygon_s;
};

int main(int argc, char** argv)
{
    GdsParser::GdsDB::GdsDB db;
    GdsParser::GdsDB::GdsReader reader (db);
    limboAssert(reader(argv[1]));

    std::vector<layer> Layer_s;

/// read the gdsii and get ploygon vector

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

#if 0
    int length = Layer_s[0].polygon_s.size();
    std::cout << "length: " << length << std::endl;

    // exaim the ploygon vertex duplicate
    for(int i=0; i < length; i++){
        GdsParser::GdsDB::GdsPolygon test = Layer_s[0].polygon_s[i];
        std::set< std::pair<Coordinate_type, Coordinate_type> > seenPairs;
        for(Coordinate_s::const_iterator coor = test.begin(); coor != test.end() -1; coor++){// polygon is closed, so the last point is the same as the first point   
            std::pair <Coordinate_type, Coordinate_type> temp;
            temp.first = coor->x();
            temp.second = coor->y();
            if (!seenPairs.insert(temp).second) {
                std::cout << "In ploygon " << i << " Duplicate pair: (" << temp.first << ", " << temp.second << ")\n";
            }
        }
    }
#endif

// access the coordiante    
//    GdsParser::GdsDB::GdsPolygon* test = ploygon_s[1];

#if 0
    // Test showing coordinate of test
    GdsParser::GdsDB::GdsPolygon test = Layer_s[0].polygon_s[0];
    for(Coordinate_s::const_iterator coor = test.begin(); coor != test.end(); coor++){
        std::cout << "x: " << coor->x() << ", y: " << coor->y() << std::endl;     
    }
#endif

#if 1
    // Test the single polygon decomposition

    GdsParser::GdsDB::GdsPolygon test = Layer_s[0].polygon_s[0];
    //GdsParser::GdsDB::GdsPolygon test = Layer_s[2].polygon_s[300];
    //GdsParser::GdsDB::GdsPolygon test = Layer_s[0].polygon_s[max_vertex_index];
    

    std::vector< Coor<int> > polygon;
    std::vector< Rect<int> > result;

    for(Coordinate_s::const_iterator coor = test.begin(); coor != test.end(); coor++){
        //std::cout << "x: " << coor->x() << ", y: " << coor->y() << std::endl;
        std::cout << coor->x() << "," << coor->y() << std::endl;

        if(coor == std::prev(test.end()) &&
            coor->x() == test.begin()->x() &&
            coor->y() == test.begin()->y()){
            continue;
        }
        else{
            polygon.push_back(Coor<int>(coor->x(), coor->y()));
        }
    }

    Edge_based_decomposition(polygon.begin(), polygon.end(), result);
    
    std::cout << "Polygon is decomposed successfully" << std::endl;
#endif


#if 0

    //find the polygon with the most vertex
    for(int i=0; i < Layer_s.size(); i++){
        int max_vertex = 0;
        int max_vertex_index = 0;
        int length = Layer_s[i].polygon_s.size();
        for(int j=0; j < length; j++){
            if(Layer_s[i].polygon_s[j].size() > max_vertex){
                max_vertex = Layer_s[i].polygon_s[j].size();
                max_vertex_index = j;
            }
        }
        std::cout << "In " << i << " layer, the polygon with the most vertex is " << max_vertex_index << " and the vertex number is " << max_vertex << std::endl;
    }

    // Test the all polygons decomposition
    for(int i=0; i < Layer_s.size(); i++){

#if 0
        if(i != 0){
            continue;
        }
#endif

        int length = Layer_s[i].polygon_s.size();
        std::cout << "length: " << length << std::endl;

        clock_t start, end;
        start = clock();

        for(int j=0; j < length; j++){
            //std::cout << "Test " << j << "-th polygon" << " in " << i << " layer" << std::endl;

            GdsParser::GdsDB::GdsPolygon test = Layer_s[i].polygon_s[j];

            std::vector< Coor<int> > polygon;
            std::vector< Rect<int> > result;

            for(Coordinate_s::const_iterator coor = test.begin(); coor != test.end(); coor++){
                if(coor == std::prev(test.end()) &&
                    coor->x() == test.begin()->x() &&
                    coor->y() == test.begin()->y()){
                    continue;
                }
                else{
                    polygon.push_back(Coor<int>(coor->x(), coor->y()));
                }
            }

            Edge_based_decomposition(polygon.begin(), polygon.end(), result);

    /*
            std::cout << "result: " << std::endl;
            for(auto poly : result){
                for(auto v : poly.vertexes){
                    std::cout << "(" << v.getX() << ", " << v.getY() << ")" << std::endl;
                }
                std::cout << std::endl;
            }
    */      
            //std::cout << j << "-th polygon is decomposed successfully" << std::endl;
        }

        end = clock();

        std::cout << "Time: " << (double)(end - start) / CLOCKS_PER_SEC << " (s)" << std::endl;

        std::cout << "Decomposition of " << i << " Layer is completed successfully" << std::endl;
    }
#endif

    return 0;
} 