#ifndef UTIL
#define UTIL

#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>
#include <cmath>

#include <vector>

#define grid_size       20000
//! Design rule
#define minimum_w       32    // unit: nm
#define minimum_s       32    // unit: nm
#define minimum_area    4800  // unit: nm

struct Rectangle {
    std::vector<int> x;
    std::vector<int> y;
    int area;
    int layer;

    Rectangle(){}

    Rectangle(int x1, int y1, int x2, int y2, int l){
        x.push_back(x1); y.push_back(y1);
        x.push_back(x1); y.push_back(y2);
        x.push_back(x2); y.push_back(y2);
        x.push_back(x2); y.push_back(y1);

        area = (x2 - x1) * (y2 - y1);

        layer = l;
    }
};

struct DensityObj{
    std::vector< std::vector<double> > grid_fill_density;
    int size;
};

void RectangleSort(std::vector<Rectangle> &List_Rectangles){
    //sort by area, large 2 small
    std::sort(List_Rectangles.begin(), List_Rectangles.end(), [](const Rectangle &a, const Rectangle &b){
        return a.area > b.area;
    });
}

// Adjust the rectangle avoiding the design rule violation
int Rectangle_adjust(const Rectangle &Rect, Rectangle &Result){
    int x1 = Rect.x[0], y1 = Rect.y[0];
    int x2 = Rect.x[2], y2 = Rect.y[2];
    int layer = Rect.layer;

    int x1_new = x1 + minimum_s;
    int x2_new = x2 - minimum_s;

    int y1_new = y1 + minimum_s;
    int y2_new = y2 - minimum_s;

    if(x1_new >= x2_new || y1_new >= y2_new){
        // design rule violation
        return -1;
    }
    else if(x2_new - x1_new < minimum_w || y2_new - y1_new < minimum_w){
        // design rule violation
        return -1;
    }
    else{
        Result = Rectangle(x1_new, y1_new, x2_new, y2_new, layer);

        if(Result.area < minimum_area){
            // design rule violation
            return -1;
        }
        else{
            return 1;
        }
    }
}

int Rectangle_shrink(const Rectangle &Rect, Rectangle &Result, double shrink_ratio){
    int x1 = Rect.x[0], y1 = Rect.y[0];
    int x2 = Rect.x[2], y2 = Rect.y[2];
    int layer = Rect.layer;

    int x1_new = x1 + int((x2 - x1) * shrink_ratio)*0.5;// x1 + shrink_ratio * width
    int x2_new = x2 - int((x2 - x1) * shrink_ratio)*0.5;// x2 - shrink_ratio * width

    int y1_new = y1 + int((y2 - y1) * shrink_ratio)*0.5;// y1 + shrink_ratio * height
    int y2_new = y2 - int((y2 - y1) * shrink_ratio)*0.5;// y2 - shrink_ratio * height

    if(x1_new >= x2_new || y1_new >= y2_new){
        // design rule violation
        return -1;
    }
    else if(x2_new - x1_new < minimum_w || y2_new - y1_new < minimum_w){
        // design rule violation
        return -1;
    }
    else{
        Result = Rectangle(x1_new, y1_new, x2_new, y2_new, layer);
        
        if(Result.area < minimum_area){
            // design rule violation
            return -1;
        }
        else{
            return 1;
        }
    }
}

int LoadFillObj(const std::string& Filename, DensityObj &Obj){
    std::regex re;

    if(Obj.size == 2){
        //"0.15084,0.15084" 
        re = R"((\d+\.\d+),(\d+\.\d+))";
        Obj.grid_fill_density.push_back({});
        Obj.grid_fill_density.push_back({});
    }
    else if(Obj.size == 4){
        //"0.15084,0.15084,0.15084,0.15084" 
        re = R"((\d+\.\d+),(\d+\.\d+),(\d+\.\d+),(\d+\.\d+))";
        Obj.grid_fill_density.push_back({});
        Obj.grid_fill_density.push_back({});
        Obj.grid_fill_density.push_back({});
        Obj.grid_fill_density.push_back({});
    }
    else{
        return -1;
    }

    std::ifstream file(Filename);

    std::string line;

    while(std::getline(file, line)){
        double x1, x2, x3, x4;

        if(Obj.size == 2){
            std::smatch match;
            if(std::regex_search(line, match, re)){
                //std::cout << "match[0] = " << match[0] << "  match[1] = " << match[1] << std::endl;

                x1 = std::stod(match[1]);
                x2 = std::stod(match[2]);
            }
            //else{
            //    std::cout << "regex_search failed !" << std::endl;
            //}
            Obj.grid_fill_density[0].push_back(x1);
            Obj.grid_fill_density[1].push_back(x2);
        }
        else if(Obj.size == 4){
            std::smatch match;
            if(std::regex_search(line, match, re)){
                x1 = std::stod(match[1]);
                x2 = std::stod(match[2]);
                x3 = std::stod(match[3]);
                x4 = std::stod(match[4]);
            }
            Obj.grid_fill_density[0].push_back(x1);
            Obj.grid_fill_density[1].push_back(x2);
            Obj.grid_fill_density[2].push_back(x3);
            Obj.grid_fill_density[3].push_back(x4);
        }
    }

    file.close();

    return 1;
}

void LoadRectangle(const std::string& Filename, std::vector<std::vector<Rectangle>> &List_Rectangles, int layer){
    std::ifstream file(Filename);
    std::string line;
    std::regex re(R"(\((\d+), (\d+)\),\((\d+), (\d+)\))");

    while(std::getline(file, line)){
        if(file.eof()){
            break;
        }
        if(line == "<grid>"){
            //skip one more line
            std::getline(file, line);
            List_Rectangles.push_back(std::vector<Rectangle>());
        }
        else{
            if(line == "</grid>"){
                //sort the rectangles in the grid by area
                RectangleSort(List_Rectangles.back());
            }

            //extract x1, y1, x2, y2 from the format like "(15705, 18196),(16117, 17652)"
            int x1, y1, x2, y2;
            std::smatch match;
            if(std::regex_search(line, match, re)){
                x1 = std::stoi(match[1]);
                y1 = std::stoi(match[2]);
                x2 = std::stoi(match[3]);
                y2 = std::stoi(match[4]);
            }

            if(x1 == x2 || y1 == y2){
                continue;
            }

            int temp;
            if(y1 > y2){
                temp = y1;
                y1 = y2;
                y2 = temp;
            }
            
            if(x1 > x2){
                temp = x1;
                x1 = x2;
                x2 = temp;
            }

            Rectangle tmp = Rectangle(x1, y1, x2, y2, layer);
            //List_Rectangles.push_back(tmp);
            List_Rectangles.back().push_back(tmp);
        }
    }

    file.close();
}

void Grid_Rectangle_Generate(double Density_Obj, 
    const std::vector<Rectangle> &List_Rectangles,
    std::vector<Rectangle> &List_Result){
    
    double fill_target = Density_Obj * grid_size * grid_size;
    auto itr = List_Rectangles.begin();

    while(fill_target > 0){
        if(fill_target <= itr->area){
            double ratio = sqrt((itr->area - fill_target) / itr->area);
            Rectangle tmp;
            if(Rectangle_shrink(*itr, tmp, ratio) == 1){
                //std::cout << "Rectangle generate successfully in case 1 !" << std::endl;
                List_Result.push_back(tmp);
                fill_target = 0;
            }
            else{
                //std::cout << "Rectangle generate failed in case 1 !" << std::endl;
            }
        }
        else{
            Rectangle tmp;
            Rectangle_adjust(*itr, tmp);
            if(Rectangle_adjust(*itr, tmp) == 1){
                //std::cout << "Rectangle generate successfully in case 2 !" << std::endl;
                List_Result.push_back(tmp);
                fill_target -= tmp.area;
            }
            else{
                //std::cout << "Rectangle generate failed in case 2 !" << std::endl;
            }
        }
        //fill_target -= itr->area;

        itr++;
        if(itr == List_Rectangles.end()){
            break;
        }
    }

    if(fill_target > 0){
        std::cout << "Remaining fill_target_density = " << fill_target/(grid_size*grid_size) << std::endl;
    }

}

void Layer_Rectangle_Generate(std::vector<double> Density_Obj, 
    const std::vector<std::vector<Rectangle>> &List_Rectangles,
    std::vector<std::vector<Rectangle>> &List_Result){

    std::cout << "Density_Obj.size() = " << Density_Obj.size() << std::endl;
    
    for(int i = 0; i < Density_Obj.size(); i++){
        List_Result.push_back(std::vector<Rectangle>());
        Grid_Rectangle_Generate(Density_Obj[i], List_Rectangles[i], List_Result.back());

        std::cout << "Handling Grid " << i << " done !" << std::endl;
        std::cout << "======================================" << std::endl;
    }
}
    

#endif