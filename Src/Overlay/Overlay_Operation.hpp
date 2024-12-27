#ifndef Overlay_Operation_hpp
#define Overlay_Operation_hpp

#include "Interval_Tree.hpp"
#include "Rect_Complement.hpp"
#include "Rect_Decomposition.hpp"

#define grid_size 20000

#define Empty 0

int Overlay_Locate(std::vector<Edge> &List_Edges, double &density)
{
    /*
     *---------------------------------------------------------- Initialize the Interval Tree
     */
    std::vector<int> Points;
    for (auto &edge : List_Edges)
    {
        Points.push_back(edge.I->x_start);
        Points.push_back(edge.I->x_end);
    }

    // srot and remove duplicates in "Points"
    std::sort(Points.begin(), Points.end());
    Points.erase(std::unique(Points.begin(), Points.end()), Points.end());

    if (Points.size() == 0)
        return Empty;

    IntervalTree Tree(Points);
    /*
     *----------------------------------------------------------------------- Sort in "List_Edges"
     */
    std::sort(List_Edges.begin(), List_Edges.end(), [](const Edge &a, const Edge &b)
              {
        if(a.y == b.y){
            if(a.type == b.type){
                return a.I->x_start < b.I->x_start;
            }
            else{
                return a.type > b.type;
            }
        }
        else{
            return a.y < b.y;
        } });

    /*
     *----------------------------------------------------------- Locate the Overlay Rectangles
     */
    int area = 0;

    for (auto &edge : List_Edges)
    {
        if (edge.type == ACTIVE)
        {
            std::vector<Interval> Overlap_Intervals = Tree.Overlap_Query(edge.I->x_start, edge.I->x_end, 0);

            for (auto &overlap_interval : Overlap_Intervals)
            {
                Rect<int> R = Rectangle_intersection(edge.I->rect_with_complement->R, overlap_interval.rect_with_complement->R);

                if (R.getH() > 0)
                {
                    area += R.Area();

                    edge.I->rect_with_complement->Complement.push_back(R);
                    edge.I->rect_with_complement->area -= R.Area();

                    overlap_interval.rect_with_complement->Complement.push_back(R);
                    overlap_interval.rect_with_complement->area -= R.Area();
                }
            }

            int signal = Tree.Insert(*edge.I);
            if (signal == 0)
            {
                std::cout << "Interval is [" << edge.I->x_start << ", " << edge.I->x_end << "]" << std::endl;
                std::cout << "Insert failed" << std::endl;
                std::cout << "=============================" << std::endl;
            }
        }
        else if (edge.type == INACTIVE)
        {
            int signal = Tree.Delete(*edge.I);
            if (signal == 0)
            {
                std::cout << "Interval is [" << edge.I->x_start << ", " << edge.I->x_end << "]" << std::endl;
                std::cout << "Delete failed" << std::endl;
                std::cout << "=============================" << std::endl;
            }
        }
    }

    density = double(area) / (grid_size * grid_size * 1.0);

    return 1;
}

void Rectangle_Clean(std::vector<Rectangle_with_complement> &List_Rectangles,
                   int &Layer_num,
                   std::vector<Rect<int>> &Result)
{
    // std::vector<Rectangle_with_complement> &List_Rectangles = (Overall_Area_Layer_1 < Overall_Area_Layer_2) ? List_Rectangles_Layer_1 : List_Rectangles_Layer_2;

    // int layer_num = (Overall_Area_Layer_1 < Overall_Area_Layer_2) ? 1 : 2;
    /*
     *---------------------------------------------------- perpare the "List_overlay_Edges"
     */

    std::vector<Edge> List_Edges;
    std::vector<Interval> List_Intervals;
    std::vector<Rectangle_with_complement> List_Overlay_Rectangles;

    int area_overlay = 0;
    for (auto &rect : List_Rectangles)
    {
        if (rect.area == 0)
        {
            List_Overlay_Rectangles.push_back(rect);
            area_overlay += rect.R.Area();
        }
        else if (rect.area > 0)
        {
            if (rect.Complement.size() == 0)
            {
                continue;
            }
            std::vector<Rect<int>> result = Rectangle_Decomposition(rect.Complement);
            for (auto &R : result)
            {
                List_Overlay_Rectangles.push_back({R, {}, R.Area()});
                area_overlay += R.Area();
            }
        }
        else
        {
            std::cout << "Illegal situation cause of the area is negative" << std::endl;
        }
    }

    double density_overlay = double(area_overlay) / (grid_size * grid_size * 1.0);
    //std::cout << "Density of Overlay is " << density_overlay << " for " << "current grid" << std::endl;

    for (int i = 0; i < List_Overlay_Rectangles.size(); i++)
    {
        List_Intervals.push_back({List_Overlay_Rectangles[i].R.getBL().getX(),
                                  List_Overlay_Rectangles[i].R.getTR().getX(),
                                  List_Overlay_Rectangles[i].R.getBL().getY(),
                                  Layer_num, &List_Overlay_Rectangles[i]});
    }

    Rectanlges_2_Edges(List_Overlay_Rectangles, List_Edges, List_Intervals, Layer_num);

    /*
     *---------------------------------------------------------- Initialize the Interval Tree
     */
    std::vector<int> Points;
    for (auto &edge : List_Edges)
    {
        Points.push_back(edge.I->x_start);
        Points.push_back(edge.I->x_end);
    }

    if(Points.size() == 0){
        Result = {};
        return;
    }

    // srot and remove duplicates in "Points"
    std::sort(Points.begin(), Points.end());
    Points.erase(std::unique(Points.begin(), Points.end()), Points.end());

    IntervalTree Tree(Points);
    /*
     *----------------------------------------------------------------------- Sort in "List_Edges"
     */
    std::sort(List_Edges.begin(), List_Edges.end(), [](const Edge &a, const Edge &b)
              {
            if(a.y == b.y){
                if(a.type == b.type){
                    return a.I->x_start < b.I->x_start;
                }
                else{
                    return a.type > b.type;
                }
            }
            else{
                return a.y < b.y;
            } });

    /*
     *----------------------------------------------------------- Clean the Overlay Rectangles
     */
    for (auto &edge : List_Edges)
    {
        if (edge.type == ACTIVE)
        {
            std::vector<Interval> Overlap_Intervals = Tree.Overlap_Query(edge.I->x_start, edge.I->x_end, 0);

            for (auto &overlap_interval : Overlap_Intervals)
            {
                if (overlap_interval.x_start == edge.I->x_start && overlap_interval.x_end == edge.I->x_end)
                {
                    //std::cout << "Find the Redundancy" << std::endl;

                    // update rectangle
                    Coor<int> BL = overlap_interval.rect_with_complement->R.getBL();
                    Coor<int> TR = overlap_interval.rect_with_complement->R.getTR();
                    TR.setY(edge.I->rect_with_complement->R.getTL().getY());

                    edge.I->rect_with_complement->R = Rect<int>(BL, TR);
                    overlap_interval.rect_with_complement->area = -1;
                }
            }
            int signal = Tree.Insert(*edge.I);
            if (signal == 0)
            {
                std::cout << "Interval is [" << edge.I->x_start << ", " << edge.I->x_end << "]" << std::endl;
                std::cout << "Insert failed" << std::endl;
                std::cout << "=============================" << std::endl;
            }
        }
        else if (edge.type == INACTIVE)
        {
            if (edge.I->rect_with_complement->area != -1)
            {
                edge.I->rect_with_complement->R.Roate_X2Y();
                Result.push_back(edge.I->rect_with_complement->R);
            }

            Interval interval = *edge.I;
            int signal = Tree.Delete(interval);

            if (signal == 0)
            {
                std::cout << "Interval is [" << interval.x_start << ", " << interval.x_end << "]" << std::endl;
                std::cout << "Delete failed" << std::endl;
                std::cout << "=============================" << std::endl;
            }
        }
    }

    double density = 0;

    for (auto &rect : Result)
    {
        density += double(rect.Area());
    }

    //std::cout << "Density of Overlay is " << density / (grid_size * grid_size * 1.0) << " for " << "current grid" << std::endl;
}

void Rectangle_Clean_XY(std::vector<Rectangle_with_complement> &List_Rectangles,
                      int &Layer_num,
                      std::vector<Rect<int>> &Result)
{
    Rectangle_Clean(List_Rectangles, Layer_num, Result);// Clean in X direction
    //std::cout << "Overlay_Clean_X is done" << std::endl;

    // Clean according Y direction
    std::vector<Rectangle_with_complement> tmp;
    for (auto &rect : Result)
    {
        //tmp.push_back({rect, {rect}, rect.Area()});
        tmp.push_back({rect, {rect}, 0});
    }

    Result.clear();
    Rectangle_Clean(tmp, Layer_num, Result);// Clean in Y direction

#if 0
    tmp.clear();
    for (auto &rect : Result)
    {
        tmp.push_back({rect, {rect}, rect.Area()});
    }

    Result.clear();
    Rectangle_Clean(tmp, Layer_num, Result);// Clean in X direction

    tmp.clear();
    for (auto &rect : Result)
    {
        tmp.push_back({rect, {rect}, rect.Area()});
    }

    Result.clear();
    Rectangle_Clean(tmp, Layer_num, Result);// Clean in Y direction
#endif

    //std::cout << "Overlay_Clean_Y is done" << std::endl;
}

void Non_Overlay_Locate(std::vector<Rectangle_with_complement> &List_Rectangles_Layer,
                        double &Nonoverlay_density, int Layer_num,
                        std::vector<Rect<int>> &List_Nonoverlay_Rectangles_Layer)
{
    /*
     *-------------------------------------------------------- Calculate the Non-Overlay Area
     */

    for (auto &rect : List_Rectangles_Layer)
    {
        if (rect.area == 0)
        {
            continue;
        }
        else if (rect.area > 0)
        {
            Nonoverlay_density += double(rect.area);
        }
        else
        {
            std::cout << "Illegal situation cause of the area is negative" << std::endl;
        }
    }
    Nonoverlay_density /= (grid_size * grid_size * 1.0);

    /*
     *-------------------------------------------------------- Locate the Non-Overlay Area
     */
    for (auto &rect : List_Rectangles_Layer)
    {
        if (rect.area == 0)
        {
            continue;
        }
        else if (rect.area > 0)
        {
            std::vector<Rect<int>> result = Rectangle_Complement(rect.R, rect.Complement);
            List_Nonoverlay_Rectangles_Layer.insert(List_Nonoverlay_Rectangles_Layer.end(), result.begin(), result.end());
        }
        else
        {
            std::cout << "Illegal situation cause of the area is negative" << std::endl;
        }
    }

    /*
     *-------------------------------------------------------- Clean the Non-Overlay Rectangles
     */
    std::vector<Rectangle_with_complement> tmp;
    for (auto &rect : List_Nonoverlay_Rectangles_Layer)
    {
        //tmp.push_back({rect, {}, rect.Area()});
        tmp.push_back({rect, {}, 0});
    }

    List_Nonoverlay_Rectangles_Layer.clear();
    Rectangle_Clean_XY(tmp, Layer_num, List_Nonoverlay_Rectangles_Layer);
}

#endif