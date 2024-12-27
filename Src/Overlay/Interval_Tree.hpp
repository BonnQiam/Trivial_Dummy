#ifndef UTIL
#define UTIL

#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>
#include <cmath>
#include <vector>

#include "../GDS2_Read_Decomposition/ScanLine_Edge_Decomposition.hpp"

#define FILEEND  1

#define ACTIVE   1
#define INACTIVE 0

struct Rectangle_with_complement{
    Rect<int> R;
    std::vector<Rect<int>> Complement;
    int area;
};

struct Interval {
    int x_start, x_end;
    int y;
    int layer;
    Rectangle_with_complement* rect_with_complement;
};

struct Edge {
    Interval* I;
    int y;
    int type;
};

void Rectanlges_2_Edges(std::vector<Rectangle_with_complement> &List_Rectangles, std::vector<Edge> &List_Edges, std::vector<Interval> &List_Intervals, int layer){
    for(int i = 0; i < List_Rectangles.size(); i++){
        Edge edge1, edge2;

        edge1.I = &List_Intervals[i];
        edge1.y = List_Rectangles[i].R.getBL().getY();
        edge1.type = ACTIVE;

        edge2.I = &List_Intervals[i];
        edge2.y = List_Rectangles[i].R.getTR().getY();
        edge2.type = INACTIVE;

        List_Edges.push_back(edge1);
        List_Edges.push_back(edge2);
    }
}

int LoadWindowData(std::ifstream &file, int layer,std::vector<Rectangle_with_complement> &List_Rectangles, double &Overall_area){
    // check if the file is at the end
    if (file.eof())
        return FILEEND;

    std::string line;
    std::regex re(R"(\((\d+), (\d+)\),\((\d+), (\d+)\))");

    while(std::getline(file, line)){
        // check if line = '<grid>'
        if(line == "<grid>"){
            // read next line
            std::getline(file, line);
            Overall_area = std::stod(line);
        }
        else{
            // check if line = '</grid>'
            if(line == "</grid>")
                break;

            // extract x1,y1,x2,y2 from the format like "(15705, 18196),(16117, 17652)"
            int x1, y1, x2, y2;
            std::smatch match;
            if(std::regex_search(line, match, re)){
                x1 = std::stoi(match[1]);
                y1 = std::stoi(match[2]);
                x2 = std::stoi(match[3]);
                y2 = std::stoi(match[4]);
            }

            if(x1 == x2 || y1 == y2){
                //std::cout << "Test" << std::endl;
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

            Rect<int> R(Coor<int>(x1, y1), Coor<int>(x2, y2));
            List_Rectangles.push_back({R, {}, R.Area()});
        }
    }

    return file.eof();
}

struct IntervalTreeNode {
    double value;
    double max, min;
    std::vector<Interval> Left_Endpoints;// sort by increasing order
    std::vector<Interval> Right_Endpoints;// sort by increasing order
};

class IntervalTree {

private:
    std::vector<IntervalTreeNode> CBT;// Using the Complete Binary Tree to store the Node, the leaf nodes are the endpoints of all segments

public:
    IntervalTree(std::vector<int> Seg_Endpoints) {
        int TreeSize = 2 * Seg_Endpoints.size()-1;
        CBT.resize( TreeSize + 1);
        CBT[0].value = 0;// Do not use the 0th index

        // Calculate the depth of the tree
        int Depth = log2(Seg_Endpoints.size()) + 1;
        int Index_1 = pow(2, Depth);// The left-most index of the last level
        int Index_2 = Index_1 - (Seg_Endpoints.size() - (TreeSize - Index_1))+1; // The left-most index of the leaf node in the last full level

        
        // Assign the leaf nodes
        for(int i = Index_2; i < CBT.size(); i++) {
            if(i < Index_1) {
                CBT[i].value = Seg_Endpoints[ (TreeSize - Index_1) + 1 +  i - Index_2];
                CBT[i].max = CBT[i].value;
                CBT[i].min = CBT[i].value;
            }
            else {
                CBT[i].value = Seg_Endpoints[i - Index_1];
                CBT[i].max = CBT[i].value;
                CBT[i].min = CBT[i].value;
            }
        }

        // Assign the non-leaf nodes
        for(int i = Index_2 - 1; i > 0; i--) {
            CBT[i].value = (CBT[2*i].max + CBT[2*i + 1].min) / 2;
            CBT[i].max = CBT[2*i].max > CBT[2*i + 1].max ? CBT[2*i].max : CBT[2*i + 1].max;
            CBT[i].min = CBT[2*i].min < CBT[2*i + 1].min ? CBT[2*i].min : CBT[2*i + 1].min;
        }
    }

    void Traverse_Display(int index){
        if(index >= CBT.size()) {
            return;
        }

        std::cout << "CBT[" << index << "]: " << CBT[index].value << std::endl;

        for(auto &Interval : CBT[index].Left_Endpoints) {
            std::cout << "Left_Endpoints: " << Interval.x_start << std::endl;
        }

        for(auto &Interval : CBT[index].Right_Endpoints) {
            std::cout << "Right_Endpoints: " << " " << Interval.x_end << std::endl;
        }

        std::cout << "----------------------" << std::endl;

        Traverse_Display(2 * index);
        Traverse_Display(2 * index + 1);
    }

    std::vector<Interval> Traverse_Collect_Interval(int index){
        std::vector<Interval> Result;

        if(index >= CBT.size()) {
            return Result;
        }

        for(auto &Interval : CBT[index].Left_Endpoints) {
            Result.push_back(Interval);
        }

        std::vector<Interval> Left_Result = Traverse_Collect_Interval(2 * index);
        std::vector<Interval> Right_Result = Traverse_Collect_Interval(2 * index + 1);

        Result.insert(Result.end(), Left_Result.begin(), Left_Result.end());
        Result.insert(Result.end(), Right_Result.begin(), Right_Result.end());

        return Result;
    }

    int Insert(Interval I) {
        int index = 1;

        while( index < CBT.size() && (CBT[index].value > I.x_end || CBT[index].value < I.x_start)) {
            if(CBT[index].value > I.x_end) {
                index = 2 * index;
            }
            else if(CBT[index].value < I.x_start) {
                index = 2 * index + 1;
            }
        }

        //std::cout << "Insert Test-1" << std::endl;

        if(index >= CBT.size()) {
            //std::cout << "The interval is out of range" << std::endl;
            return 0;
        }

        // Insert I.start into the left endpoint list by increasing order
        auto itr = CBT[index].Left_Endpoints.begin();
        while(itr != CBT[index].Left_Endpoints.end() && itr->x_start < I.x_start) {
            itr++;
        }
        CBT[index].Left_Endpoints.insert(itr, I);

        // Insert I.end into the right endpoint list by decreasing order
        itr = CBT[index].Right_Endpoints.begin();
        while(itr != CBT[index].Right_Endpoints.end() && itr->x_end > I.x_end) {
            itr++;
        }
        CBT[index].Right_Endpoints.insert(itr, I);

        return index;
        //return 1;
    }

    int Delete(Interval I) {
        int index = 1;

        while( index < CBT.size() && (CBT[index].value > I.x_end || CBT[index].value < I.x_start)) {
            if(CBT[index].value > I.x_end) {
                index = 2 * index;
            }
            else if(CBT[index].value < I.x_start) {
                index = 2 * index + 1;
            }
        }

        if(index >= CBT.size()) {
            //std::cout << "The interval is out of range" << std::endl;
            return 0;
        }
        // Delete I.start from the left endpoint list
        auto itr = CBT[index].Left_Endpoints.begin();
        //while(itr != CBT[index].Left_Endpoints.end() && (itr->x_start != I.x_start || itr->x_end != I.x_end)) {
        while(itr != CBT[index].Left_Endpoints.end()) {
            if(itr->x_start == I.x_start && itr->x_end == I.x_end &&
                itr->y == I.y && itr->layer == I.layer
            ){
                break;
            }
            itr++;
        }
        if(itr != CBT[index].Left_Endpoints.end()) {
            //std::cout << "Delete operation" << std::endl;
            CBT[index].Left_Endpoints.erase(itr);
        }

        // Delete I.end from the right endpoint list
        itr = CBT[index].Right_Endpoints.begin();
        //while(itr != CBT[index].Right_Endpoints.end() && (itr->x_end != I.x_end || itr->x_start != I.x_start)) {
        while(itr != CBT[index].Right_Endpoints.end()) {
            if(itr->x_start == I.x_start && itr->x_end == I.x_end &&
                itr->y == I.y && itr->layer == I.layer
            ){
                break;
            }
            itr++;
        }
        if(itr != CBT[index].Right_Endpoints.end()) {
            //std::cout << "Delete operation" << std::endl;
            CBT[index].Right_Endpoints.erase(itr);
        }

        return 1;
    }

#if 1
    std::vector<Interval> Overlap_locate_ST(int index, int ST){
        std::vector<Interval> Result;
        
        if(index >= CBT.size()) {
            return Result;
        }

        if(CBT[index].value == ST){
            return Result;
        }

        int Flag = 0;

        if(CBT[index].value > ST) {
            for(auto &Interval : CBT[index].Left_Endpoints){ 

#if 0       
                if(Interval.x_start == 3305 && Interval.x_end == 3837){
                    std::cout << "Find the interval is [" << Interval.x_start << ", " << Interval.x_end << "] at CBT[" << index << "]" << std::endl;
                }
                else if(Interval.x_start == 3305 && Interval.x_end == 3819){
                    std::cout << "Find the interval is [" << Interval.x_start << ", " << Interval.x_end << "] at CBT[" << index << "]" << std::endl;
                }
#endif           
                Result.push_back(Interval);
            }

            std::vector<Interval> Right_Result = Traverse_Collect_Interval(2 * index + 1);
            Result.insert(Result.end(), Right_Result.begin(), Right_Result.end());
            Flag = 0; // check in left child
        }
        else {
            for(auto &Interval : CBT[index].Right_Endpoints) {
                if(Interval.x_end >= ST) {
                    Result.push_back(Interval);
                }
            }

            Flag = 1; // check in right child
        }

        std::vector<Interval> Temp_Result;
        if(Flag == 0){
            Temp_Result = Overlap_locate_ST(2 * index, ST);
        }
        else {
            Temp_Result = Overlap_locate_ST(2 * index + 1, ST);
        }
        
        Result.insert(Result.end(), Temp_Result.begin(), Temp_Result.end());

        return Result;
    }

    std::vector<Interval> Overlap_locate_ED(int index, int ED){
        std::vector<Interval> Result;

        if(index >= CBT.size()) {
            return Result;
        }

        if(CBT[index].value == ED){
            return Result;
        }

        int Flag = 0;

        if(CBT[index].value > ED) {
            for(auto &Interval : CBT[index].Left_Endpoints) {
                if(Interval.x_start <= ED) {
                    Result.push_back(Interval);
                }
            }

            Flag = 0; // check in left child
        }
        else {
            for(auto &Interval : CBT[index].Right_Endpoints) 
                Result.push_back(Interval);

            std::vector<Interval> Left_Result = Traverse_Collect_Interval(2 * index);
            Result.insert(Result.end(), Left_Result.begin(), Left_Result.end());
            Flag = 1; // check in right child
        }

        std::vector<Interval> Temp_Result;

        if(Flag == 0){
            Temp_Result = Overlap_locate_ED(2 * index, ED);
        }
        else {
            Temp_Result = Overlap_locate_ED(2 * index + 1, ED);
        }

        Result.insert(Result.end(), Temp_Result.begin(), Temp_Result.end());

        return Result;
    }

    //! Requirement: [ST, ED] is the interval that has been inserted into the tree
    //* Return the intervals that overlap with [ST, ED]
    //  std::vector<Interval> Overlap_Query(int ST, int ED){
    std::vector<Interval> Overlap_Query(int ST, int ED, int Debug){
        std::vector<Interval> Result;

        int index = 1;

#if 0
        if(Debug){
            std::vector<Interval> Temp = Traverse_Collect_Interval(1);
            for(auto &Interval : Temp){
                if(Interval.x_start == 10604 && Interval.x_end == 16458){
                    std::cout << "Find the interval is [" << Interval.x_start << ", " << Interval.x_end << "]" << std::endl;
                }
            }
        }
#endif

        /*
        * Find the node that satisfies the condition: CBT[index].value >= ST && CBT[index].value <= ED
        * Names the node as the target node
        */
        while((CBT[index].value > ED || CBT[index].value < ST) ) {
            if(Debug){
                std::cout << "CBT[" << index << "]: " << CBT[index].value << std::endl;
                for(auto &Interval : CBT[index].Left_Endpoints) {
                    std::cout << "The interval is [" << Interval.x_start << ", " << Interval.x_end << "]" << std::endl;
                } 
            }

            if(CBT[index].value > ED) {
                for(auto &Interval : CBT[index].Left_Endpoints) {
                    if(Interval.x_start <= ED) {
                    //if(Interval.x_start <= ST) {
                        Result.push_back(Interval);
                        if(Debug)
                            std::cout << "Overlap Interval is [" << Interval.x_start << ", " << Interval.x_end << "] in Upper check" << std::endl;
                    }
                }

                index = 2 * index;
            }
            else if(CBT[index].value < ST) {
                for(auto &Interval : CBT[index].Right_Endpoints) {
                    if(Interval.x_end >= ST) {
                    //if(Interval.x_end >= ED) {
                        Result.push_back(Interval);
                        if(Debug)
                            std::cout << "Overlap Interval is [" << Interval.x_start << ", " << Interval.x_end << "] in Upper check" << std::endl;
                    }
                }
                index = 2 * index + 1;
            }

            if(index >= CBT.size()) {
                return Result;
            }
        }

        if(Debug == 1){
            std::cout << "The target node is " << index << std::endl;
        }

        for(auto &Interval : CBT[index].Left_Endpoints) {
            if( 
                (Interval.x_start >= ST && Interval.x_start <= ED) ||
                (Interval.x_end >= ST && Interval.x_end <= ED) ||
                (Interval.x_start <= ST && Interval.x_end >= ED)
            ){
                Result.push_back(Interval);

                if(Debug == 1)
                    std::cout << "Overlap Interval is [" << Interval.x_start << ", " << Interval.x_end << "] in target" << std::endl;
            }
        }

        std::vector<Interval> Left_Result = Overlap_locate_ST(2*index, ST);

        if(Debug==1){
            for(auto &Interval : Left_Result){
                std::cout << "Overlap Interval is [" << Interval.x_start << ", " << Interval.x_end << "] in left" << std::endl;
            }
        }

        Result.insert(Result.end(), Left_Result.begin(), Left_Result.end());
//        std::cout << "Test-2" << std::endl;


        std::vector<Interval> Right_Result = Overlap_locate_ED(2*index+1, ED);

        if(Debug==1){
            for(auto &Interval : Right_Result){
                std::cout << "Overlap Interval is [" << Interval.x_start << ", " << Interval.x_end << "] in right" << std::endl;
            }
        }

        Result.insert(Result.end(), Right_Result.begin(), Right_Result.end());
//        std::cout << "Test-3" << std::endl;

        return Result;
    }
    
#endif

};


#endif