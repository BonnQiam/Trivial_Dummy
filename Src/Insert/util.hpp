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

#include "Legal_fill.hpp"

#define Num_grid 160
#define x_grid_num 8
#define y_grid_num 20

#define Layer_1  10001
#define Layer_2  10002
#define Layer_3  10003

void parse_Overlay(const std::string& filename, std::vector<double> &Overlay){
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

void parse_No_Fill_Density(const std::string& filename, std::vector<double> &Density_Mental)
{
    struct Data {
        int x;
        int y;
        double val1;
        double val2;
    };
    
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Could not open file: " << filename << '\n';
        exit(1);
    }

    std::string line;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        Data d;
        if (!(iss >> d.x >> d.y >> d.val1 >> d.val2)) { 
            break; // Error
        }
        Density_Mental.push_back(d.val1);
    }

    file.close();
}


int LoadFillObj(const std::string& Filename, std::vector<std::vector<double>> &Obj, int size){
    std::regex re;
    if(size == 2){
        //"0.15084,0.15084" 
        re = R"((\d+\.\d+),(\d+\.\d+))";
        Obj.push_back({});
        Obj.push_back({});
    }
    else if(size == 4){
        //"0.15084,0.15084,0.15084,0.15084" 
        re = R"((\d+\.\d+),(\d+\.\d+),(\d+\.\d+),(\d+\.\d+))";
        Obj.push_back({});
        Obj.push_back({});
        Obj.push_back({});
        Obj.push_back({});
    }
    else{
        return -1;
    }

    std::ifstream file(Filename);

    std::string line;

    while(std::getline(file, line)){
        double x1, x2, x3, x4;

        if(size == 2){
            std::smatch match;

            if(std::regex_search(line, match, re)){
                x1 = std::stod(match[1]);
                x2 = std::stod(match[2]);
            }
            Obj.end()[-2].push_back(x1);
            Obj.end()[-1].push_back(x2);
        }
        else if(size == 4){
            std::smatch match;
            if(std::regex_search(line, match, re)){
                x1 = std::stod(match[1]);
                x2 = std::stod(match[2]);
                x3 = std::stod(match[3]);
                x4 = std::stod(match[4]);
            }

            Obj.end()[-4].push_back(x1);
            Obj.end()[-3].push_back(x2);
            Obj.end()[-2].push_back(x3);
            Obj.end()[-1].push_back(x4);
        }
    }

    file.close();

    return 1;
}

void LoadRectangle(const std::string& Filename, std::vector<std::vector<Rect<int>>> &List_Rectangles){
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
            List_Rectangles.push_back(std::vector<Rect<int>>());
        }
        else{
            if(line == "</grid>"){
                //sort the rectangles in the grid by area
                std::sort(List_Rectangles.back().begin(), List_Rectangles.back().end(), [](const Rect<int> &a, const Rect<int> &b){
                    return a.Area() > b.Area();
                });
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
            List_Rectangles.back().push_back(tmp);
        }
    }

    file.close();
}

void Layer_Rectangle_Generate(std::vector<double> Density_Obj, int offset,
    std::vector<std::vector<Rect<int>>> &List_Rectangles,
    std::vector<std::vector<Rect<int>>> &List_Result,
    std::vector<double> &Density_Fillable){
    
    for(int i = 0; i < List_Rectangles.size(); i++){
        List_Result.push_back(std::vector<Rect<int>>());

        if(Density_Obj[i+offset] == 0.0){
            Density_Fillable.push_back(0.0);
            continue;
        }

        double density_fill = Legal_Fill(List_Rectangles[i], List_Result[i], Density_Obj[i+offset]);

        Density_Fillable.push_back(density_fill);
        
        //std::cout << "Handling Grid " << i << " done !" << std::endl;
        //std::cout << "======================================" << std::endl;
    }
}
    

#endif