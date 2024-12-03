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

#define grid_size 20000

// Just for Debug
#define check_x_coordinate 13150
#define check_y1_coordinate 34000
#define check_y2_coordinate 34200

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    std::ifstream file1(argv[1]); //* Input File of Layer 1
    std::ifstream file2(argv[2]); //* Input File of Layer 2
    std::ofstream file3(argv[3]); //* Output File

    int file_end1 = 0, file_end2 = 0;

    int Count_grid = 0;

    std::vector<double> density_list;
    std::ofstream file4("density.txt");

    while (file_end1 == 0 || file_end2 == 0)
    {
        Count_grid++;

        clock_t start, end;
        start = clock();

        /*
        ******************************************* Read Data from File *******************************************
        */

        std::vector<Edge> List_Edges;

        file_end1 = LoadWindowData(file1, List_Edges, 1);
        file_end2 = LoadWindowData(file2, List_Edges, 2);
        
        //std::cout << "File End 1 is " << file_end1 << std::endl;
        //std::cout << "File End 2 is " << file_end2 << std::endl;
        //std::cout << "Operate at the " << Count_grid << " grid" << std::endl;

        /*
        ******************************** Initialize the Interval Tree  ********************************
        */

        std::vector<int> Points;
        for (auto &edge : List_Edges)
        {
            Points.push_back(edge.I.x_start);
            Points.push_back(edge.I.x_end);
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
        std::sort(List_Edges.begin(), List_Edges.end(), [](const Edge &a, const Edge &b){
//            return a.y < b.y;
#if 1
            if(a.y == b.y){
                //return (a.I.x_end - a.I.x_start) > (b.I.x_end - b.I.x_start);
                return a.I.x_start < b.I.x_start;
            }
            else{
                return a.y < b.y;
            }
#endif
        });

        std::vector<Rectangle> Overlap_Rectangles;
        int area = 0;

        std::cout << "The size of List_Edge is " << List_Edges.size() << std::endl;


#if 1
        int count = 0;
        for(auto &edge : List_Edges)
        {
            count++;
            //std::cout << "The " << count << "th edge" << std::endl;

            if (edge.type == ACTIVE)
            {
                // std::cout << "Debug Test 1" << std::endl;
                Interval interval = edge.I;
#if 0
                if(interval.x_start <= check_x_coordinate && 
                    interval.x_end >= check_x_coordinate && 
                    edge.y >= check_y1_coordinate &&
                    edge.y <= check_y2_coordinate){
                    std::cout << "Find the interval is [" << interval.x_start << ", " << interval.x_end << "]" << std::endl;
                    std::cout << "The y of the interval is " << edge.y << std::endl;
                    std::cout << "The layer of the interval is " << edge.layer << std::endl;
                    std::cout << "Now check the overlap intervals" << std::endl;
                }
#endif
                std::vector<Interval> Overlap_Intervals = Tree.Overlap_Query(interval.x_start, interval.x_end, 0);
                
#if 0
                std::vector<Interval> Overlap_Intervals;
                if(interval.x_start == 12837 && interval.x_end == 13425 && edge.y == 34100){
                    Overlap_Intervals = Tree.Overlap_Query(interval.x_start, interval.x_end, 1);
                }
                else{
                    Overlap_Intervals = Tree.Overlap_Query(interval.x_start, interval.x_end, 0);
                }
#endif
                //std::cout << "Debug Test 1-2" << std::endl;

                for (auto &overlap_interval : Overlap_Intervals)
                {

                    #if 0
                        if(interval.x_start <= check_x_coordinate && 
                        interval.x_end >= check_x_coordinate && 
                        edge.y >= check_y1_coordinate &&
                        edge.y <= check_y2_coordinate){
                            std::cout << "Interval is [" << interval.x_start << ", " << interval.x_end << "]" << std::endl;
                            std::cout << "Rectangle is [" << interval.rectangle_y_start << ", " << interval.rectangle_y_end << "]" << std::endl;

                            std::cout << "Overlap Interval is [" << overlap_interval.x_start << ", " << overlap_interval.x_end << "]" << std::endl;
                            std::cout << "Overlap Rectangle is [" << overlap_interval.rectangle_y_start << ", " << overlap_interval.rectangle_y_end << "]" << std::endl;
                        }
                    #endif
                    
                    Rectangle R = Overlap_Rectangle(interval, overlap_interval);
                    
                    if (R.area() > 0)
                    {
                        Overlap_Rectangles.push_back(R);
                        area += R.area();
                    }
                }

                //std::cout << "Debug Test 1-3" << std::endl;
                int signal = Tree.Insert(interval);

#if 0
                if(interval.x_start == 3305 && interval.x_end == 3837 && edge.y == 15204){
                    std::cout << "Interval is inserted in the CBT[" << signal << "]" << std::endl;
                }
#endif
                if(signal == 0){
                    std::cout << "Interval is [" << interval.x_start << ", " << interval.x_end << "]" << std::endl;
                    std::cout << "Insert failed" << std::endl;
                    std::cout << "=============================" << std::endl;
                }
                //std::cout << "Debug Test 1-4" << std::endl;
            }
            else if (edge.type == INACTIVE)
            {
                //std::cout << "Debug Test 2" << std::endl;
                Interval interval = edge.I;

#if 0
                if(interval.x_start <= check_x_coordinate && 
                    interval.x_end >= check_x_coordinate && 
                    edge.y >= check_y1_coordinate &&
                    edge.y <= check_y2_coordinate){
                    std::cout << "Find the interval is [" << interval.x_start << ", " << interval.x_end << "]" << std::endl;
                    std::cout << "The y of the interval is " << edge.y << std::endl;
                    std::cout << "The layer of the interval is " << edge.layer << std::endl;
                    std::cout << "Now Delete intervals" << std::endl;
                }
#endif
                
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

        //std::cout << "Area is " << area << " for " << Count_grid << " grid" << std::endl;

        std::cout << "Density is " << density << " for " << Count_grid << " grid" << std::endl;
        
        std::cout << "===============================================================" << std::endl;

        // Output
        file3 << "<grid>" << std::endl;
        file3 << density << std::endl;

        for (auto &R : Overlap_Rectangles)
        {
            file3 << "(" << R.x_l << ", " << R.y_l << "),";
            file3 << "(" << R.x_r << ", " << R.y_r << ")" << std::endl;
        }

        file3 << "</grid>" << std::endl;


        file4 << density << std::endl;
#endif
    }

    // close the file
    file1.close();
    file2.close();
    file3.close();
    file4.close();

    return 0;
}