#ifndef UTIL
#define UTIL

#include <fstream>
#include <sstream>
#include <iostream>

#include <cmath>

#include <vector>

//contest configure: the window size is 20 microns, the gdsii unit si 1 nm, i.e., 0.001 micron
//#define grid_size  20000

struct Grid_Coor {
    int x;
    int y;
};

void parse_No_Fill_Density(const std::string& filename, std::vector<Grid_Coor> &Grid, std::vector<double> &Density_Mental);
void parse_Fillable_Density(const std::string& filename, std::vector<double> &Density_Fillable);
void parse_Overlay(const std::string& filename, std::vector<double> &Overaly);

void parse_No_Fill_Density(const std::string& filename, std::vector<Grid_Coor> &Grid, std::vector<double> &Density_Mental){
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
        Grid.push_back(Grid_Coor{d.x, d.y});
        Density_Mental.push_back(d.val1);
    }

    file.close();
}

#if 0
void parse_Fillable_Density(const std::string& filename, std::vector<double> &Density_Fillable){
    std::ifstream file(filename);
    std::string line;

    if (file.is_open()) {
        while (getline(file, line)) {
            if (line == "<grid>") {
                getline(file, line);
                Density_Fillable.push_back(std::stod(line));
            }
        }
        file.close();
    } else {
        std::cout << "Unable to open file";
    }
}
#endif

void parse_Fillable_Density(const std::string& filename, std::vector<double> &Density_Fillable){
    std::ifstream file(filename);

    if(!file){
        std::cerr << "Could not open file: " << filename << '\n';
        exit(1);
    }

    std::string line;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        double d;
        if (!(iss >> d)) { 
            break; // Error
        }
        Density_Fillable.push_back(d);
    }

    file.close();
}

#endif