/*
 * Input: two windows consist of rectangles
 * Output 1: the density of the intersection area of the two windows
 * Output 2: the overlay area of the two windows will be located by rectangles
 * Algorithm: using the interval tree to the any two overlapped rectangles
 * Speed: fast
 */

#include <algorithm>
#include <time.h>

#include "Overlay_Operation.hpp"

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
    std::ofstream file5(argv[4]); //* Output File for Non-Overlay Area in Layer 1
    std::ofstream file6(argv[5]); //* Output File for Non-Overlay Area in Layer 2

    std::ofstream file4(argv[6]); //* Output File for Overlay Area
    std::ofstream file7(argv[7]); //* Output File for Non-Overlay Area in Layer 1
    std::ofstream file8(argv[8]); //* Output File for Non-Overlay Area in Layer 2

    std::vector<double> density_list;

    int file_end1 = 0, file_end2 = 0;

    int Count_grid = 0;

    while (file_end1 == 0 || file_end2 == 0)
    {
        Count_grid++;

        //if(Count_grid >1)
        //    break;

        clock_t start, end;

        /*
         *---------------------------------------------------------------- Read Data from File
         */
        start = clock();

        std::vector<Edge> List_Edges;
        std::vector<Interval> List_Intervals_Layer_1, List_Intervals_Layer_2;
        std::vector<Rectangle_with_complement> List_Rectangles_Layer_1, List_Rectangles_Layer_2;
        double Overall_Area_Layer_1, Overall_Area_Layer_2;

        file_end1 = LoadWindowData(file1, 1, List_Rectangles_Layer_1, Overall_Area_Layer_1);
        file_end2 = LoadWindowData(file2, 2, List_Rectangles_Layer_2, Overall_Area_Layer_2);

        for (int i = 0; i < List_Rectangles_Layer_1.size(); i++)
        {
            List_Intervals_Layer_1.push_back({List_Rectangles_Layer_1[i].R.getBL().getX(),
                                              List_Rectangles_Layer_1[i].R.getTR().getX(),
                                              List_Rectangles_Layer_1[i].R.getBL().getY(), 1, &List_Rectangles_Layer_1[i]});
        }

        for (int i = 0; i < List_Rectangles_Layer_2.size(); i++)
        {
            List_Intervals_Layer_2.push_back({List_Rectangles_Layer_2[i].R.getBL().getX(),
                                              List_Rectangles_Layer_2[i].R.getTR().getX(),
                                              List_Rectangles_Layer_2[i].R.getBL().getY(), 2, &List_Rectangles_Layer_2[i]});
        }

        Rectanlges_2_Edges(List_Rectangles_Layer_1, List_Edges, List_Intervals_Layer_1, 1);
        Rectanlges_2_Edges(List_Rectangles_Layer_2, List_Edges, List_Intervals_Layer_2, 2);

        end = clock();

        std::cout << "Read Data time is " << (double)(end - start) / CLOCKS_PER_SEC / 60 << " min" << " for " << Count_grid << " grid" << std::endl;

        /*
         *--------------------------------------------------------- Locate the Overlay Rectangles
         */
        start = clock();

        double overlay_density = 0;
        int signal = Overlay_Locate(List_Edges, overlay_density);

        if (signal == 0)
        {
            continue;
        }

        end = clock();
        std::cout << "Overlap Locate time is " << (double)(end - start) / CLOCKS_PER_SEC / 60 << " min" << " for " << Count_grid << " grid" << std::endl;

        density_list.push_back(overlay_density);

        std::cout << "Density of Overlay is " << overlay_density << " for " << Count_grid << " grid" << std::endl;

        file4 << overlay_density << std::endl;

        /*
         *--------------------------------------------------------- Clean the Overlay Rectangles
         */
        start = clock();

        // Clean according X-axis
        std::vector<Rectangle_with_complement> &List_Rectangles = (Overall_Area_Layer_1 < Overall_Area_Layer_2) ? List_Rectangles_Layer_1 : List_Rectangles_Layer_2;

        int layer_num = (Overall_Area_Layer_1 < Overall_Area_Layer_2) ? 1 : 2;
        
        std::vector<Rect<int>> Overlay_Rectangles;

        Rectangle_Clean_XY(List_Rectangles, layer_num, Overlay_Rectangles);

        end = clock();

        std::cout << "Overlay Clean time is " << (double)(end - start) / CLOCKS_PER_SEC / 60 << " min" << " for " << Count_grid << " grid" << std::endl;

        /*
         *------------------------------------------------- Output the Overlay Rectangles to file3
         */

        file3 << "<grid>" << std::endl;
        
        for(auto &rect : Overlay_Rectangles){
            if(rect.Area() < minimum_area || rect.getTR().getX() - rect.getBL().getX() < minimum_w){
                overlay_density -= rect.Area()/grid_size/grid_size;
            }
        }
        file3 << overlay_density << std::endl;

        int area_overlay = 0;
        for (auto &rect : Overlay_Rectangles)
        {
            //if (rect.getH() == 0)
            if(rect.Area() < minimum_area || rect.getTR().getX() - rect.getBL().getX() < minimum_w)
            {
                continue;
            }
            else if (rect.getH() > 0)
            {
                file3 << "(" << rect.getBL().getX() << ", " << rect.getBL().getY() << "),";
                file3 << "(" << rect.getTR().getX() << ", " << rect.getTR().getY() << ")" << std::endl;
                area_overlay += rect.Area();
            }
            else
            {
                std::cout << "Illegal situation cause of the area is negative" << std::endl;
            }
        }

        double density_check = double(area_overlay) / (grid_size * grid_size * 1.0);
        //std::cout << "Density of Overlay is " << density_check << " for " << Count_grid << " grid" << std::endl;

        file3 << "</grid>" << std::endl;

        /*
         *------------------------------------------------- Locate the Non-Overlay Rectangles in Layer 1
         */

        start = clock();

        layer_num = 1;
        double nonoverlay_density_layer = 0;
        std::vector<Rect<int>> List_Nonoverlay_Rectangles_Layer;

        Non_Overlay_Locate(List_Rectangles_Layer_1, nonoverlay_density_layer, layer_num,List_Nonoverlay_Rectangles_Layer);

        end = clock();
        std::cout << "Non-Overlay Locate time for layer 1 is " << (double)(end - start) / CLOCKS_PER_SEC / 60 << " min" << " for " << Count_grid << " grid" << std::endl;

        file5 << "<grid>" << std::endl;

        for(auto &rect : List_Nonoverlay_Rectangles_Layer){
            if(rect.Area() < minimum_area || rect.getTR().getX() - rect.getBL().getX() < minimum_w){
                nonoverlay_density_layer -= rect.Area()/grid_size/grid_size;
            }
        }

        file5 << nonoverlay_density_layer << std::endl;

        for (auto &rect : List_Nonoverlay_Rectangles_Layer)
        {
            if(rect.Area() < minimum_area || rect.getTR().getX() - rect.getBL().getX() < minimum_w)
                continue;

            file5 << "(" << rect.getBL().getX() << ", " << rect.getBL().getY() << "),";
            file5 << "(" << rect.getTR().getX() << ", " << rect.getTR().getY() << ")" << std::endl;
        }

        file5 << "</grid>" << std::endl;

        file7 << nonoverlay_density_layer << std::endl;

        // ? double check
        if (Overall_Area_Layer_1 - (nonoverlay_density_layer + overlay_density) > 1e-5)
        {
            std::cout << "The area is not equal for layer 1 in grid " << Count_grid << std::endl;
            std::cout << "Overall Area is " << Overall_Area_Layer_1 << " in grid " << Count_grid << std::endl;
            std::cout << "Non-Overlay Area + Overlay Area is " << nonoverlay_density_layer << " + " << overlay_density << " = " << nonoverlay_density_layer + overlay_density << " in grid " << Count_grid << std::endl;
        }
        else
        {
            std::cout << "The area is equal for layer 1 in grid " << Count_grid << std::endl;
        }

        /*
         *------------------------------------------------- Locate the Non-Overlay Rectangles in Layer 2
         */
        start = clock();

        layer_num = 2;
        nonoverlay_density_layer = 0;
        List_Nonoverlay_Rectangles_Layer.clear();

        Non_Overlay_Locate(List_Rectangles_Layer_2, nonoverlay_density_layer, layer_num, List_Nonoverlay_Rectangles_Layer);

        end = clock();
        std::cout << "Non-Overlay Locate time for layer 2 is " << (double)(end - start) / CLOCKS_PER_SEC / 60 << " min" << " for " << Count_grid << " grid" << std::endl;

        std::cout << "Size of Non-Overlay Rectangles in Layer 2 is " << List_Nonoverlay_Rectangles_Layer.size() << " for " << Count_grid << " grid" << std::endl;

        file6 << "<grid>" << std::endl;

        for(auto &rect : List_Nonoverlay_Rectangles_Layer){
            if(rect.Area() < minimum_area || rect.getTR().getX() - rect.getBL().getX() < minimum_w){
                nonoverlay_density_layer -= rect.Area()/grid_size/grid_size;
            }
        }

        file6 << nonoverlay_density_layer << std::endl;

        for (auto &rect : List_Nonoverlay_Rectangles_Layer)
        {
            if(rect.Area() < minimum_area || rect.getTR().getX() - rect.getBL().getX() < minimum_w)
                continue;

            file6 << "(" << rect.getBL().getX() << ", " << rect.getBL().getY() << "),";
            file6 << "(" << rect.getTR().getX() << ", " << rect.getTR().getY() << ")" << std::endl;
        }

        file6 << "</grid>" << std::endl;
        // check
        if (Overall_Area_Layer_2 - (nonoverlay_density_layer + overlay_density) > 1e-5)
        {
            std::cout << "The area is not equal for layer 2 in grid " << Count_grid << std::endl;
            std::cout << "Overall Area is " << Overall_Area_Layer_2 << " in grid " << Count_grid << std::endl;
            std::cout << "Non-Overlay Area + Overlay Area is " << nonoverlay_density_layer << " + " << overlay_density << " = " << nonoverlay_density_layer + overlay_density << " in grid " << Count_grid << std::endl;
        }
        else
        {
            std::cout << "The area is equal for layer 2 in grid " << Count_grid << std::endl;
        }

        file8 << nonoverlay_density_layer << std::endl;

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