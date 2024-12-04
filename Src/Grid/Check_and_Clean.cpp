#include <iostream>
#include <fstream>
#include <sstream>

#include <vector>
#include <algorithm>
#include <regex>

struct Rect
{
    int x1, y1, x2, y2;
};

void Debug_in_Grid(std::ifstream& file, int& count, std::ofstream& out);

int main(int argc, char** argv)
{
    char* filename = argv[1];

    std::ifstream file(filename);
    std::ofstream out("output.txt", std::ios::app);

    int count = 1;

    // If file is not found, report error
    if (!file)
    {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return 1;
    }

    while(file)
    {
        std::string line;
        std::getline(file, line);

        //std::cout << "Grid count: " << count << std::endl;

        if(line == "<grid>"){
            std::getline(file, line);//skip the 'density' line
            // debug in grid
            Debug_in_Grid(file, count, out);
        }
    }

    file.close();
    out.close();

    return 0;
}

void Debug_in_Grid(std::ifstream& file, int& count, std::ofstream& out)
{   
    std::vector<Rect> rectangles;

    //construct the Rect based '(15705, 18196),(16117, 17652)'    
    std::regex r("\\((\\d+), (\\d+)\\),\\((\\d+), (\\d+)\\)");
    std::smatch m;

    std::string line;

    while(std::getline(file, line)){
        if(line == "</grid>"){
            count++;
            break;
        }

        Rect rect;
        if(std::regex_search(line, m, r)){
            rect.x1 = std::stoi(m[1]);
            rect.y1 = std::stoi(m[2]);
            rect.x2 = std::stoi(m[3]);
            rect.y2 = std::stoi(m[4]);
        }
        rectangles.push_back(rect);
    }

    //check repeated rectangles based std::unique
    std::sort(rectangles.begin(), rectangles.end(), [](const Rect& a, const Rect& b){
        if(a.x1 != b.x1) 
            return a.x1 < b.x1;
        else{
            if(a.y1 != b.y1)
                return a.y1 < b.y1;
            else{
                if(a.x2 != b.x2)
                    return a.x2 < b.x2;
                else
                    return a.y2 < b.y2;
            }
        }
    });

    auto last = std::unique(rectangles.begin(), rectangles.end(), [](const Rect& a, const Rect& b){
        return a.x1 == b.x1 && a.y1 == b.y1 && a.x2 == b.x2 && a.y2 == b.y2;
    });

    if(last != rectangles.end()){
        std::cout << "In grid " << count << ", repeated rectangles found" << std::endl;
    }
    else{
        std::cout << "In grid " << count << ", no repeated rectangles found" << std::endl;
    }

    rectangles.erase(last, rectangles.end());

#if 1
    //check illegal rectangles based on x1, y1, x2, y2
    double sum_area = 0;
    int flag = 0;
    for(auto rect = rectangles.begin(); rect != rectangles.end();){
        if(rect->x1 == rect->x2 || rect->y1 == rect->y2){
            flag = 1;

            std::cout << "In grid " << count << ", illegal rectangles found" << std::endl;

            std::cout << "x1: " << rect->x1 << std::endl;
            std::cout << "y1: " << rect->y1 << std::endl;
            std::cout << "x2: " << rect->x2 << std::endl;
            std::cout << "y2: " << rect->y2 << std::endl;

            rect = rectangles.erase(rect);
        }
        else{
            sum_area += (rect->x2 - rect->x1) * (rect->y1 - rect->y2);
            rect++;
        }
    }

    if(flag == 0){
        std::cout << "In grid " << count << ", no illegal rectangles found" << std::endl;
    }
    else{
        std::cout << "In grid " << count << ", illegal rectangles removed" << std::endl;
    }
#endif

#if 1
    //output the valid rectangles
    out << "<grid>" << std::endl;
    out << sum_area/(20000*20000) << std::endl;

    for(auto rect = rectangles.begin(); rect != rectangles.end(); rect++){
        out << "(" << rect->x1 << ", " << rect->y1 << "),(" << rect->x2 << ", " << rect->y2 << ")" << std::endl;
    }
    out << "</grid>" << std::endl;
#endif
}