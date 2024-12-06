/*
* Input: two windows consist of rectangles
* Output 1: the density of the intersection area of the two windows
* Output 2: the overlay area of the two windows will be located by rectangles
* Algorithm: using the interval tree to the any two overlapped rectangles
* Speed: fast
*/


#include <algorithm>
#include <time.h>

#include "Interval_Tree.hpp"
#include "Rect_Complement.hpp"

#define grid_size 20000

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    std::ifstream file1(argv[1]); //* Input File of Layer 1
    std::ifstream file2(argv[2]); //* Input File of Layer 2

    std::ofstream file3(argv[3]); //* Output File for Overlay Area
    std::ofstream file5("layer1.txt"); //* Output File for Non-Overlay Area in Layer 1
    std::ofstream file6("layer2.txt"); //* Output File for Non-Overlay Area in Layer 2

    std::vector<double> density_list;
    std::ofstream file4("overlay_density.txt");
    std::ofstream file7("Non_overlay_density_layer_1.txt");
    std::ofstream file8("Non_overlay_density_layer_2.txt");

    int file_end1 = 0, file_end2 = 0;

    int Count_grid = 0;

    while (file_end1 == 0 || file_end2 == 0)
    {
        Count_grid++;

        clock_t start, end;
        start = clock();

        /*
        ******************************************* Read Data from File *******************************************
        */

        //std::vector<Edge> List_Edges;
        std::vector<Interval> List_Edges;
        std::vector<Rectangle_with_complement> List_Rectangles_Layer_1, List_Rectangles_Layer_2;
        double Overall_Area_Layer_1, Overall_Area_Layer_2;

        //file_end1 = LoadWindowData(file1, 1, List_Edges, List_Rectangles_Layer_1);
        //file_end2 = LoadWindowData(file2, 2, List_Edges, List_Rectangles_Layer_2);

        file_end1 = LoadWindowData(file1, 1, List_Rectangles_Layer_1, Overall_Area_Layer_1);
        file_end2 = LoadWindowData(file2, 2, List_Rectangles_Layer_2, Overall_Area_Layer_2);

        Rectanlges_2_Edges(List_Rectangles_Layer_1, List_Edges, 1);
        Rectanlges_2_Edges(List_Rectangles_Layer_2, List_Edges, 2);
        
        //std::cout << "File End 1 is " << file_end1 << std::endl;
        //std::cout << "File End 2 is " << file_end2 << std::endl;
        //std::cout << "Operate at the " << Count_grid << " grid" << std::endl;

        /*
        ******************************** Initialize the Interval Tree  ********************************
        */

        std::vector<int> Points;
        for (auto &edge : List_Edges)
        {
            Points.push_back(edge.x_start);
            Points.push_back(edge.x_end);
            //Points.push_back(edge.I.x_start);
            //Points.push_back(edge.I.x_end);
        }

        // sort and remove duplicates
        std::sort(Points.begin(), Points.end());
        Points.erase(std::unique(Points.begin(), Points.end()), Points.end());

        std::cout << "Size of Points is " << Points.size() << std::endl;

        if(Points.size() == 0)
            continue;

        IntervalTree Tree(Points);

        //Tree.Traverse_Display(1);

        /*
        ****************************** Operation
        ******************************
        */

        // sort the intervals by y
        //std::sort(List_Edges.begin(), List_Edges.end(), [](const Edge &a, const Edge &b){
        std::sort(List_Edges.begin(), List_Edges.end(), [](const Interval &a, const Interval &b){
//            return a.y < b.y;
#if 1
            if(a.y == b.y){
                //return (a.I.x_end - a.I.x_start) > (b.I.x_end - b.I.x_start);
                //return a.I.x_start < b.I.x_start;
                return a.x_start < b.x_start;
            }
            else{
                return a.y < b.y;
            }
#endif
        });
        
        std::vector<Rect<int>> Overlap_Rectangles;
        //std::vector<Rectangle> Overlap_Rectangles;
        int area = 0;

        std::cout << "The size of List_Edge is " << List_Edges.size() << std::endl;

        int count = 0;
        for(auto &edge : List_Edges)
        {
            count++;
            //std::cout << "The " << count << "th edge" << std::endl;

            if (edge.type == ACTIVE)
            {
                // std::cout << "Debug Test 1" << std::endl;
                Interval interval = edge;
                std::vector<Interval> Overlap_Intervals = Tree.Overlap_Query(interval.x_start, interval.x_end, 0);


                for (auto &overlap_interval : Overlap_Intervals)
                {
                    Rect<int> R = Rectangle_intersection(interval.rect_with_complement->R, overlap_interval.rect_with_complement->R);
                    
                    if (R.Area() > 0)
                    {
                        Overlap_Rectangles.push_back(R);
                        area += R.Area();

                        interval.rect_with_complement->Complement.push_back(R);
                        interval.rect_with_complement->area -= R.Area();

                        overlap_interval.rect_with_complement->Complement.push_back(R);
                        overlap_interval.rect_with_complement->area -= R.Area();
                    }
                }

                int signal = Tree.Insert(interval);

                if(signal == 0){
                    std::cout << "Interval is [" << interval.x_start << ", " << interval.x_end << "]" << std::endl;
                    std::cout << "Insert failed" << std::endl;
                    std::cout << "=============================" << std::endl;
                }
            }
            else if (edge.type == INACTIVE)
            {
                //std::cout << "Debug Test 2" << std::endl;
                Interval interval = edge;
                
                int signal = Tree.Delete(interval);
                if(signal == 0){
                    std::cout << "Interval is [" << interval.x_start << ", " << interval.x_end << "]" << std::endl;
                    std::cout << "Delete failed" << std::endl;
                    std::cout << "=============================" << std::endl;
                }
            }
        }

        double density = double(area) / (grid_size * grid_size);
        density_list.push_back(density);

        end = clock();
        //double density = double(area);

        end = clock();
        std::cout << "Overlap Operation time is " << (double)(end - start) / CLOCKS_PER_SEC / 60 << " min" << " for " << Count_grid << " grid" << std::endl;

        std::cout << "Density is " << density << " for " << Count_grid << " grid" << std::endl;
        
        // Output for the overlay area
        file3 << "<grid>" << std::endl;
        file3 << density << std::endl;

        for (auto &R : Overlap_Rectangles)
        {
            file3 << "(" << R.getBL().getX() << ", " << R.getBL().getY() << "),";
            file3 << "(" << R.getTR().getX() << ", " << R.getTR().getY() << ")" << std::endl;
        }

        file3 << "</grid>" << std::endl;

        file4 << density << std::endl;

        // Locate the non-overlay area
        file5 << "<grid>" << std::endl;
        double non_overlay_area = 0;
        for(auto &rect: List_Rectangles_Layer_1){
            if(rect.area == 0){
                continue;
            }
            else if(rect.area > 0){
                non_overlay_area += rect.area;

                std::vector<Rect<int>> result = Rectangle_Complement(rect.R, rect.Complement);
                for(auto &R: result){
                    file5 << "(" << R.getBL().getX() << ", " << R.getBL().getY() << "),";
                    file5 << "(" << R.getTR().getX() << ", " << R.getTR().getY() << ")" << std::endl;
                }
            }
            else{
                std::cout << "Illegal situation cause of the area is negative" << std::endl;
            }
        }

        double non_overlay_density = double(non_overlay_area) / (grid_size * grid_size * 1.0);
        
        file5 << non_overlay_density << std::endl;
        file5 << "</grid>" << std::endl;

        //check
        if(Overall_Area_Layer_1 - (non_overlay_density + density) > 1e-5){
            std::cout << "The area is not equal for layer 1 in grid " << Count_grid << std::endl;
            std::cout << "Overall Area is " << Overall_Area_Layer_1 << " in grid " << Count_grid << std::endl;
            std::cout << "Non-Overlay Area + Overlay Area is " << non_overlay_density << " + " << density << " = " << non_overlay_density + density << " in grid " << Count_grid << std::endl;
        }
        else{
            std::cout << "The area is equal for layer 1 in grid " << Count_grid << std::endl;
        }

        file6 << "<grid>" << std::endl;
        non_overlay_area = 0;
        for(auto &rect: List_Rectangles_Layer_2){
            if(rect.area == 0){
                continue;
            }
            else if(rect.area > 0){
                non_overlay_area += rect.area;

                std::vector<Rect<int>> result = Rectangle_Complement(rect.R, rect.Complement);
                for(auto &R: result){
                    file6 << "(" << R.getBL().getX() << ", " << R.getBL().getY() << "),";
                    file6 << "(" << R.getTR().getX() << ", " << R.getTR().getY() << ")" << std::endl;
                }
            }
            else{
                std::cout << "Illegal situation cause of the area is negative" << std::endl;
            }
        }
        non_overlay_density = double(non_overlay_area) / (grid_size * grid_size * 1.0);
        file6 << non_overlay_density << std::endl;
        file6 << "</grid>" << std::endl;

        file7 << non_overlay_density << std::endl;

        //check
        if(Overall_Area_Layer_2 - (non_overlay_density + density) > 1e-5){
            std::cout << "The area is not equal for layer 2 in grid " << Count_grid << std::endl;
            std::cout << "Overall Area is " << Overall_Area_Layer_2 << " in grid " << Count_grid << std::endl;
            std::cout << "Non-Overlay Area + Overlay Area is " << non_overlay_density << " + " << density << " = " << non_overlay_density + density << " in grid " << Count_grid << std::endl;
        }
        else{
            std::cout << "The area is equal for layer 2 in grid " << Count_grid << std::endl;
        }

        file8 << non_overlay_density << std::endl;

        std::cout << "===============================================================" << std::endl;
    }

    // close the file
    file1.close();
    file2.close();
    file3.close();
    file4.close();
    file5.close();
    file6.close();
    file7.close();
    file8.close();

    return 0;
}