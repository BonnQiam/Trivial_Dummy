#ifndef UTIL
#define UTIL

#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>
#include <cmath>
#include <utility>

#include <vector>

#include "../Overlay/Interval_Tree.hpp"
//! Attention ---- When use the "Interval_Tree.hpp", adjust the struct "Interval"

#define Num_grid 160
#define x_grid_num 8
#define y_grid_num 20

#define grid_size       20000
//! Design rule
#define minimum_w       32    // unit: nm
#define minimum_s       32    // unit: nm
#define minimum_area    4800  // unit: nm

#define Layer_1      10001
#define Layer_2      10002
#define Layer_3      10003

// For evaluation
struct Final_Grid{
    double density_metal;
    int layer;
    std::vector<double> density_fill;
};

struct Grid{
    double              density_metal;
    int                 size; // 2 or 4
    int                 layer;
    std::vector<double> density_obj;
    /**
     * if size == 2
     *  density_obj[0] corresponds to Dummy_Dummy
     *  density_obj[1] corresponds to Dummy_Wire
     * if size == 4
     *  density_obj[0] corresponds to Dummy_Dummy_Dummy
     *  density_obj[1] corresponds to Dummy_Dummy_Wire
     *  density_obj[2] corresponds to Wire_Dummy_Dummy
     *  density_obj[3] corresponds to Wire_Dummy_Wire
     * ----
     * Same set for density_fill and Fillable_rect
     */
    std::vector<double> density_fill;
    
    std::vector<std::vector<Rect<int>>> Fillable_rect;
    std::vector<std::vector<Rect<int>>> Fill_rect;


    void density_report(){
        std::cout << "---------------------------" << std::endl;
        for(int i = 0; i < density_obj.size(); i++){
            if(abs(density_obj[i] - density_fill[i])/density_obj[i] > 0.001){
                std::cout << "Error: fill density is not equal to the target density" << std::endl;
                std::cout << "Density_obj[" << i << "] = " << density_obj[i] << std::endl;
                std::cout << "Density_fill[" << i << "] = " << density_fill[i] << std::endl;
            }
        }
    }

    void Fill_rect_output(std::string filename){
        std::fstream output(filename, std::ios::out|std::ios::app);

        output<<"<grid>"<<std::endl;
        
        double density = 0.0;

        for(int i = 0; i < density_fill.size(); i++){
            density += density_fill[i]; 
        }

        output << density << std::endl;

        for(int i = 0; i < Fill_rect.size(); i++){
            for(int j = 0; j < Fill_rect[i].size(); j++){
                output << "(" << Fill_rect[i][j].getBL().getX() << ", " << Fill_rect[i][j].getBL().getY() << "),";
                output << "(" << Fill_rect[i][j].getTR().getX() << ", " << Fill_rect[i][j].getTR().getY() << ")" << std::endl;
            }
        }

        output<<"</grid>"<<std::endl;
    }
};

int Rectangle_shrink(Rect<int> rect, double ratio)
{
    int x1 = rect.getBL().getX(), y1 = rect.getBL().getY();
    int x2 = rect.getTR().getX(), y2 = rect.getTR().getY();

    int x1_new = x1 + (x2 - x1) * (1 - ratio) / 2;
    int x2_new = x2 - (x2 - x1) * (1 - ratio) / 2;

    int y1_new = y1 + (y2 - y1) * (1 - ratio) / 2;
    int y2_new = y2 - (y2 - y1) * (1 - ratio) / 2;

    if(x1_new >= x2_new || y1_new >= y2_new)
        return 0;

    Coor<int> bl(x1_new, y1_new);
    Coor<int> tr(x2_new, y2_new);

    rect.setBL(bl);
    rect.setTR(tr);

    return 1;
}


/*
* ******************************************* Parser
*/

int parse_No_Fill_Density(std::ifstream &file, Grid &grid, int layer){
    struct Data {
        int x;
        int y;
        double val1;
        double val2;
    };

    // check if the file is at the end
    if (file.eof())
        return 1;
    
    std::string line;
    std::getline(file, line);
    std::istringstream iss(line);
    Data d;
    if (!(iss >> d.x >> d.y >> d.val1 >> d.val2)) { 
        std::cout << "Wrong format in file" << std::endl;
        exit(1);// Error
    }

    grid.density_metal = d.val1;
    if(layer == Layer_2){
        grid.size = 4;
    }
    else{
        grid.size = 2;
    }
    grid.layer = layer;
    grid.density_obj = {};
    grid.density_fill = {};
    grid.Fillable_rect = {};

    return 0;
}

int LoadFillObj(std::ifstream &file, Grid &grid){
    // check if the file is at the end
    if (file.eof())
        return 1;

    std::regex re;
    if(grid.size == 2){
        //"0.15084,0.15084" 
        re = R"((\d+\.\d+),(\d+\.\d+))";
        grid.density_obj.push_back(0);
        grid.density_obj.push_back(0);

        grid.density_fill.push_back(0);
        grid.density_fill.push_back(0);
    }
    else if(grid.size == 4){
        //"0.15084,0.15084,0.15084,0.15084" 
        re = R"((\d+\.\d+),(\d+\.\d+),(\d+\.\d+),(\d+\.\d+))";
        grid.density_obj.push_back(0);
        grid.density_obj.push_back(0);
        grid.density_obj.push_back(0);
        grid.density_obj.push_back(0);

        grid.density_fill.push_back(0);
        grid.density_fill.push_back(0);
        grid.density_fill.push_back(0);
        grid.density_fill.push_back(0);
    }
    else{
        return -1;
    }

    std::string line;
    std::getline(file, line);
    double x1, x2, x3, x4;

    if(grid.size == 2){
        std::smatch match;

        if(std::regex_search(line, match, re)){
            x1 = std::stod(match[1]);
            x2 = std::stod(match[2]);
        }
        grid.density_obj[0] = x1;
        grid.density_obj[1] = x2;

        return 0;
    }
    else if(grid.size == 4){
        std::smatch match;
        if(std::regex_search(line, match, re)){
            x1 = std::stod(match[1]);
            x2 = std::stod(match[2]);
            x3 = std::stod(match[3]);
            x4 = std::stod(match[4]);
        }

        grid.density_obj[0] = x1;
        grid.density_obj[1] = x2;
        grid.density_obj[2] = x3;
        grid.density_obj[3] = x4;

        return 0;
    }
    else{
        return -1;
    }
}

int LoadRectangle(std::ifstream &file, Grid &grid){
    std::string line;
    std::regex re(R"(\((\d+), (\d+)\),\((\d+), (\d+)\))");

    while(std::getline(file, line)){
        if(file.eof()){
            return 1;
        }
        if(line == "<grid>"){
            //skip one more line
            std::getline(file, line);
            grid.Fillable_rect.push_back(std::vector<Rect<int>>());
            //maybe we can load fillable density here
            grid.Fill_rect.push_back(std::vector<Rect<int>>());
        }
        else{
            if(line == "</grid>"){
                //sort the rectangles in the grid by area
                std::sort(grid.Fillable_rect.back().begin(), grid.Fillable_rect.back().end(), [](const Rect<int> &a, const Rect<int> &b){
                    return a.Area() > b.Area();
                });
                return 0;
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

            Rect<int> tmp(Coor<int>(x1, y1), Coor<int>(x2, y2));
            //List_Rectangles.push_back(tmp);
            grid.Fillable_rect.back().push_back(tmp);
        }
    }

    return 1;
}

void Layer_Rectangle_Generate(Grid &grid){
    std::vector<std::vector<Interval>>  X_intervals;
    std::vector<double>                 Fill_targets;

    if(grid.size == 2){
        X_intervals.push_back(std::vector<Interval>());
        X_intervals.push_back(std::vector<Interval>());

        Fill_targets.push_back(grid.density_obj[0]*grid_size*grid_size);
        Fill_targets.push_back(grid.density_obj[1]*grid_size*grid_size);
    }
    else if(grid.size == 4){
        X_intervals.push_back(std::vector<Interval>());
        X_intervals.push_back(std::vector<Interval>());
        X_intervals.push_back(std::vector<Interval>());
        X_intervals.push_back(std::vector<Interval>());

        Fill_targets.push_back(grid.density_obj[0]*grid_size*grid_size);
        Fill_targets.push_back(grid.density_obj[1]*grid_size*grid_size);
        Fill_targets.push_back(grid.density_obj[2]*grid_size*grid_size);
        Fill_targets.push_back(grid.density_obj[3]*grid_size*grid_size);
    }

    std::vector<int>                    X_points;

    /*
     *-----------------------------------------   Initialize the X Interval Tree
     */

    for(int i = 0; i < grid.Fillable_rect.size(); i++){
        for(int j = 0; j < grid.Fillable_rect[i].size(); j++){
            X_intervals[i].push_back({
                grid.Fillable_rect[i][j].getBL().getX(), grid.Fillable_rect[i][j].getTR().getX(), 
                grid.Fillable_rect[i][j].getBL().getY(),
                0,
                &(grid.Fillable_rect[i][j])});

            X_points.push_back(grid.Fillable_rect[i][j].getBL().getX());
            X_points.push_back(grid.Fillable_rect[i][j].getTR().getX());
        }
    }
    
    std::sort(X_points.begin(), X_points.end());
    X_points.erase(std::unique(X_points.begin(), X_points.end()), X_points.end());

    // sort the intervals by area, large 2 small
    for(int i = 0; i < X_intervals.size(); i++){
        std::sort(X_intervals[i].begin(), X_intervals[i].end(), [](const Interval &a, const Interval &b){
            return a.r->Area() > b.r->Area();
        });
    }

    IntervalTree X_Tree(X_points);
    /*
     *-----------------------------------------   Legal fill
     */
    //std::vector<int> fill_flag = (grid.size == 2) ? std::vector<int>(2, 1) : std::vector<int>(4, 1);
    std::vector<double> zero_flag = (grid.size == 2) ? std::vector<double>(2, 0.0) : std::vector<double>(4, 0.0);

    std::vector<int> index = (grid.size == 2) ? std::vector<int>(2, 0) : std::vector<int>(4, 0);

    while(1){
        for(int i = 0; i < grid.size; i++){
            if(Fill_targets[i] == 0.0){ 
                continue;
            }

            if(index[i] == X_intervals[i].size()){
                Fill_targets[i] = 0.0;
                continue;
            }

            for(int j=index[i]; j < X_intervals[i].size(); j++){
                //Window setting
                Coor<int> bl = X_intervals[i][j].r->getBL();
                Coor<int> tr = X_intervals[i][j].r->getTR();

                bl.addToX(-minimum_s);
                bl.addToY(-minimum_s);
                tr.addToX(minimum_s);
                tr.addToY(minimum_s);

                Rect<int> window = {bl, tr};

                std::vector<Rect<int>> overlap_rects;

                //check the overlap rectangle by window query

                std::vector<Interval> X_overlap_intervals = X_Tree.Overlap_Query(bl.getX(), tr.getX(), 0);

                for(auto &interval: X_overlap_intervals){
                    if(interval.x_start == X_intervals[i][j].r->getBL().getX() &&
                        interval.x_end == X_intervals[i][j].r->getTR().getX() &&
                        interval.y == X_intervals[i][j].r->getBL().getY()){
                        continue;
                    }

                    Rect<int> R = Rectangle_intersection(*interval.r, window);

                    //Rect<int> R = Rectangle_intersection(*interval.r, *X_intervals[i].r);

                    if(R.Area() > 0){
                        overlap_rects.push_back(R);
                    }
                }

                //distance between the rectangle and the overlap rectangles
                int up=minimum_s, down=minimum_s, left=minimum_s, right=minimum_s;

                //get the original rectangle
                bl.addToX(minimum_s);
                bl.addToY(minimum_s);
                tr.addToX(-minimum_s);
                tr.addToY(-minimum_s);

                for(auto &rect: overlap_rects){
                    Coor<int> rect_tr = rect.getTR();
                    Coor<int> rect_bl = rect.getBL();

                    if(rect_tr.getX() < bl.getX()){
                        // rect is on the left
                        if(rect_bl.getY() > tr.getY() || rect_tr.getY() < bl.getY()){
                            continue;
                        }
                        else{
                            left = std::min(left, bl.getX() - rect_tr.getX());
                        }
                    }
                    else if(rect_bl.getX() > tr.getX()){
                        // rect is on the right
                        if(rect_bl.getY() > tr.getY() || rect_tr.getY() < bl.getY()){
                            continue;
                        }
                        else{
                            right = std::min(right, rect_bl.getX() - tr.getX());
                        }
                    }
                    else{
                        if(rect_bl.getY() > tr.getY()){
                            // rect is on the top
                            up = std::min(up, rect_bl.getY() - tr.getY());
                        }
                        else if(rect_tr.getY() < bl.getY()){
                            // rect is on the bottom
                            down = std::min(down, bl.getY() - rect_tr.getY());
                        }
                        else{
                            // rect is inside the window
                            continue;
                        }
                    }
                }

#if 0
                std::cout << "===================================" << std::endl;
                std::cout << "up: " << up << std::endl;
                std::cout << "down: " << down << std::endl;
                std::cout << "left: " << left << std::endl;
                std::cout << "right: " << right << std::endl;
                std::cout << "overlap_rects.size(): " << overlap_rects.size() << std::endl;
                std::cout << "===================================" << std::endl;
#endif
                //adjust the rectangle
                if(overlap_rects.size()){
                    // handling the special case
                    up = (up==0) ? minimum_s : up;
                    down = (down==0) ? minimum_s : down;
                    left = (left==0) ? minimum_s : left;
                    right = (right==0) ? minimum_s : right;

                    X_intervals[i][j].r->setBL(
                        X_intervals[i][j].r->getBL().getX() + minimum_s - left, 
                        X_intervals[i][j].r->getBL().getY() + minimum_s - down);
                    X_intervals[i][j].r->setTR(
                        X_intervals[i][j].r->getTR().getX() - minimum_s + right, 
                        X_intervals[i][j].r->getTR().getY() - minimum_s + up);
                }

                //check if the rectangle is legal
                int width = X_intervals[i][j].r->getTR().getX() - X_intervals[i][j].r->getBL().getX();
                int height = X_intervals[i][j].r->getTR().getY() - X_intervals[i][j].r->getBL().getY();
                if(X_intervals[i][j].r->Area() < minimum_area || width < minimum_w || height < minimum_w){
                    continue;
                }

                if(X_intervals[i][j].r->Area() <= Fill_targets[i]){
                    //insert the rectangle
                    X_Tree.Insert(X_intervals[i][j]);
                    
                    grid.Fill_rect[i].push_back(*X_intervals[i][j].r);
                    grid.density_fill[i] += double(X_intervals[i][j].r->Area())/grid_size/grid_size;

                    Fill_targets[i] -= X_intervals[i][j].r->Area();
                    index[i] = j + 1;
                    
                    break;
                }
                else{
                    //shrink the rectangle
                    double ratio = sqrt(Fill_targets[i] / X_intervals[i][j].r->Area());
                    Rectangle_shrink(*X_intervals[i][j].r, ratio);

                    int width = X_intervals[i][j].r->getTR().getX() - X_intervals[i][j].r->getBL().getX();
                    int height = X_intervals[i][j].r->getTR().getY() - X_intervals[i][j].r->getBL().getY();

                    if(X_intervals[i][j].r->Area() < minimum_area || width < minimum_w || height < minimum_w){
                        continue;
                    }

                    //insert the rectangle
                    X_Tree.Insert(X_intervals[i][j]);

                    grid.Fill_rect[i].push_back(*X_intervals[i][j].r);
                    grid.density_fill[i] += double(X_intervals[i][j].r->Area())/grid_size/grid_size;

                    Fill_targets[i] = 0;
                    index[i] = j + 1;

                    break;
                }
            }
        }

        if(Fill_targets == zero_flag){
            break;
        }
    }
}


void parse_Overlay(const std::string& filename, std::vector<double> &Overlay)
{
    std::ifstream file(filename);

    if(!file){
        std::cerr << "Could not open file: " << filename << '\n';
        exit(1);
    }

    std::string line;

    while(std::getline(file, line)){
        double val;
        if(!(std::istringstream(line) >> val)){
            break;
        }
        Overlay.push_back(val);
    }

    file.close();
}

#endif