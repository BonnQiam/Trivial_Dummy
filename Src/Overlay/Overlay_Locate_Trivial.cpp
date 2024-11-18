/*
* Input: two windows consist of rectangles
* Output 1: the density of the intersection area of the two windows
* Output 2: the overlay area of the two windows will be located by rectangles
* Algorithm: 
* using sweep line algorithm
* get the overlapped segment of the two windows trivially, i.e., trace the sweep line algroithm
* use the edge-based decomposition to decompose the overlapped segment into rectangles
* Speed: unacceptable slow
*/

#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>
#include <stack>
#include <algorithm>
#include <time.h>

#include "Scanline.hpp"

#define FILEEND 1
#define grid_size 20000

int LoadWindowData(std::ifstream &file, Seg *List_Edge, double *List_X_Coordinate, int &num);

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    std::ifstream file1(argv[1]);
    std::ifstream file2(argv[2]);
    std::ofstream file3(argv[3]);

    int file_end1 = 0, file_end2 = 0;

    int Count_grid = 0;

    while (file_end1 == 0 || file_end2 == 0)
    //while (file_end1 == 0)
    {
        Count_grid++;
        clock_t start, end;

        start = clock();

        int num = 0;
        mem(t, 0);

        file_end1 = LoadWindowData(file1, e, X, num);
        file_end2 = LoadWindowData(file2, e, X, num);

        //std::cout << "file_end1 is " << file_end1 << std::endl;
        //std::cout << "file_end2 is " << file_end2 << std::endl;
        //std::cout << "Operate at the " << Count_grid << " grid" << std::endl;

#if 1

        // sweep line algorithm
        std::sort(X, X + num);
        std::sort(e, e + num);

        int m = std::unique(X, X + num) - X;

        double area = 0;                          // the area of the intersection of the rectangles
        std::vector<edge<int>> Final_Edge_list;   // The edge collection of the intersection area
        std::vector<edge<int>> Check_Edge_list;   // effective edge list in the last scanning line
        std::vector<edge<int>> Checked_Edge_list; // effective edge list in the now scanning line

        for (int i = 0; i < num; i++)
        {
            //std::cout << "i is " << i << " in " << num << std::endl;

            int l = std::lower_bound(X, X + m, e[i].l) - X;
            int r = std::lower_bound(X, X + m, e[i].r) - X - 1;

            if (r == -1)
                continue;

            update(l, r, 0, m, 1, e[i].f);

            area += t[1].s * (e[i + 1].h - e[i].h);

            if (e[i + 1].h - e[i].h > 0 && t[1].s > 0)
            {
                Interval res = check_effective_s(0, m, 1);

                if (Final_Edge_list.size() == 0)
                {
                    for (auto &r : res)
                    {
                        // For horizontal edge
                        Final_Edge_list.push_back(edge<int>(Coor<int>(r.first, e[i].h), Coor<int>(r.second, e[i].h)));
                        Check_Edge_list.push_back(edge<int>(Coor<int>(r.first, e[i + 1].h), Coor<int>(r.second, e[i + 1].h)));
                        // For vertical edge
                        Final_Edge_list.push_back(edge<int>(Coor<int>(r.first, e[i].h), Coor<int>(r.first, e[i + 1].h)));
                        Final_Edge_list.push_back(edge<int>(Coor<int>(r.second, e[i].h), Coor<int>(r.second, e[i + 1].h)));
                    }
                }
                else
                {
                    for (auto &r : res)
                    {
                        // For horizontal edge
                        Checked_Edge_list.push_back(edge<int>(Coor<int>(r.first, e[i + 1].h), Coor<int>(r.second, e[i + 1].h)));

#if 0
                    std::cout << "( " << r.first << ", " << e[i].h << " ) ( " << r.second << ", " << e[i].h << " )" << std::endl;

                    for (auto &r : Check_Edge_list)
                    {
                        std::cout << "Check_Edge_list: " << r.Coor_pair.first << " " << r.Coor_pair.second << std::endl;
                    }
#endif

                        edge_list_edge_complement<int>(Check_Edge_list, edge<int>(Coor<int>(r.first, e[i].h), Coor<int>(r.second, e[i].h)), HORIZONTAL);
                        // For vertical edge
                        edge_list_edge_complement<int>(Final_Edge_list, edge<int>(Coor<int>(r.first, e[i].h), Coor<int>(r.first, e[i + 1].h)), VERTICAL);
                        edge_list_edge_complement<int>(Final_Edge_list, edge<int>(Coor<int>(r.second, e[i].h), Coor<int>(r.second, e[i + 1].h)), VERTICAL);
                    }

                    for (auto &r : Check_Edge_list)
                    {
                        Final_Edge_list.push_back(r);
                    }

                    Check_Edge_list.clear();
                    for (auto &r : Checked_Edge_list)
                    {
                        Check_Edge_list.push_back(r);
                    }
                    Checked_Edge_list.clear();
                }
            }
        }

        double density = area / (grid_size * grid_size);

        for (auto &e : Check_Edge_list)
        {
            // std::cout << "Check_Edge_list: " << e.Coor_pair.first << " " << e.Coor_pair.second << std::endl;
            Final_Edge_list.push_back(edge<int>(e.Coor_pair.first, e.Coor_pair.second));
        }

        end = clock();
        std::cout << "Scanning time is " << (double)(end - start) / CLOCKS_PER_SEC / 60 << " min" << " for " << Count_grid << " grid" << std::endl;

        start = clock();
        //std::cout << "Exectuing Edge_based_decomposition" << std::endl;
        Polygon_edge_collection<int> collection(Final_Edge_list);
        std::vector<Rect<int>> result;
        Edge_based_decomposition(collection, result);
        end = clock();
        std::cout << "Decomposition time is " << (double)(end - start) / CLOCKS_PER_SEC / 60 << " min" << " for " << Count_grid << " grid" << std::endl;

        std::cout << "Density is " << density << " for " << Count_grid << " grid" << std::endl;
        std::cout << "===============================================================" << std::endl;

        // output the result to the file
        file3 << "<grid>" << std::endl;
        file3 << density << std::endl;

        // output the rectangles to the file
        for (auto it = result.begin(); it != result.end(); it++)
        {
            file3 << "(" << it->getBL().getX() << ", " << it->getBL().getY() << "),";
            file3 << "(" << it->getTR().getX() << ", " << it->getTR().getY() << ")" << std::endl;
        }
        file3 << "</grid>" << std::endl;
#endif
    }

    // close the file
    file1.close();
    file2.close();
    file3.close();

    return 0;
}

int LoadWindowData(std::ifstream &file, Seg *List_Edge, double *List_X_Coordinate, int &num)
{

    // check if the file is at the end
    if (file.eof())
        return FILEEND;

    std::string line;
    std::regex re(R"(\((\d+), (\d+)\),\((\d+), (\d+)\))");

    while (std::getline(file, line))
    {
        // check if line = '<grid>'
        if (line == "<grid>")
        {
            // read next line
            std::getline(file, line);
            //double density = std::stod(line);
            //std::cout << "Density is " << density << std::endl;
        }
        else
        {
            // check if line = '</grid>'
            if (line == "</grid>")
                break;

            // extract x1,y1,x2,y2 from the format like "(15705, 18196),(16117, 17652)"
            double x1, y1, x2, y2;
            std::smatch match;
            if (std::regex_search(line, match, re))
            {
                x1 = std::stod(match[1]);
                y1 = std::stod(match[2]);
                x2 = std::stod(match[3]);
                y2 = std::stod(match[4]);
            }

            if(x1 == x2 || y1 == y2){
                //std::cout << "Test" << std::endl;
                continue;
            }

            double temp;
            if (y1 > y2)
            {
                temp = y1;
                y1 = y2;
                y2 = temp;
            }

            if (x1 > x2)
            {
                temp = x1;
                x1 = x2;
                x2 = temp;
            }

            X[num] = x1;
            e[num++] = Seg(x1, x2, y1, 1);
            X[num] = x2;
            e[num++] = Seg(x1, x2, y2, -1);
        }
    }

    return file.eof(); // return 0 if the file is not at the end
}