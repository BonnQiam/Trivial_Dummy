#ifndef LEGAL_FILL
#define LEGAL_FILL

#include "../Overlay/Interval_Tree.hpp"
//! Attention ---- When use the "Interval_Tree.hpp", adjust the struct "Interval"

#define grid_size       20000
//! Design rule
#define minimum_w       32    // unit: nm
#define minimum_s       32    // unit: nm
#define minimum_area    4800  // unit: nm

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

double Legal_Fill(std::vector<Rect<int>> &List_rect, std::vector<Rect<int>> &Fill, double Density_obj)
{
    std::vector<Interval>  X_intervals;
    std::vector<int>       X_points;

    for(int i = 0; i < List_rect.size(); i++){
        X_intervals.push_back({
            List_rect[i].getBL().getX(), List_rect[i].getTR().getX(), 
            List_rect[i].getBL().getY(),
            0,
            &(List_rect[i])});

        X_points.push_back(List_rect[i].getBL().getX());
        X_points.push_back(List_rect[i].getTR().getX());
    }

    std::sort(X_points.begin(), X_points.end());
    X_points.erase(std::unique(X_points.begin(), X_points.end()), X_points.end());

    // sort the intervals by area, large 2 small
    std::sort(X_intervals.begin(), X_intervals.end(), [](const Interval &a, const Interval &b){
        return a.r->Area() > b.r->Area();
    });

    /*
     *-----------------------------------------   Initialize the X & Y Interval Tree
     */

    IntervalTree X_Tree(X_points);

    /*
     *-----------------------------------------   Legal fill
     */

    //int fill_target = 0.0989617 * grid_size * grid_size;
    int fill_target = Density_obj * grid_size * grid_size;

    for(int i = 0; i < X_intervals.size(); i++){
        Coor<int> bl = X_intervals[i].r->getBL();
        Coor<int> tr = X_intervals[i].r->getTR();

        //Window query setting
        bl.addToX(-minimum_s);
        bl.addToY(-minimum_s);
        tr.addToX(minimum_s);
        tr.addToY(minimum_s);

        Rect<int> window = {bl, tr};

        std::vector<Rect<int>> overlap_rects;

        //check the overlap rectangle by window query

        std::vector<Interval> X_overlap_intervals = X_Tree.Overlap_Query(bl.getX(), tr.getX(), 0);

        for(auto &interval: X_overlap_intervals){
            if(interval.x_start == X_intervals[i].r->getBL().getX() && 
                interval.x_end == X_intervals[i].r->getTR().getX() &&
                interval.y == X_intervals[i].r->getBL().getY()){
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

        //adjust the rectangle
        if(overlap_rects.size()){
            X_intervals[i].r->setBL(
                X_intervals[i].r->getBL().getX() + minimum_s - left, 
                X_intervals[i].r->getBL().getY() + minimum_s - down);
            X_intervals[i].r->setTR(
                X_intervals[i].r->getTR().getX() - minimum_s + right, 
                X_intervals[i].r->getTR().getY() - minimum_s + up);
        }

        //checkt the rectangle legality
        int width = X_intervals[i].r->getTR().getX() - X_intervals[i].r->getBL().getX();
        if(X_intervals[i].r->Area() < minimum_area || width < minimum_w){
            continue;
        }

        if(X_intervals[i].r->Area() <= fill_target){
            //insert the interval
            X_Tree.Insert(X_intervals[i]);
            fill_target -= X_intervals[i].r->Area();
        }
        else{
            //shrink the rectangle
            double ratio = sqrt(fill_target/ X_intervals[i].r->Area());
            Rectangle_shrink(*X_intervals[i].r, ratio);

            int width = X_intervals[i].r->getTR().getX() - X_intervals[i].r->getBL().getX();

            if(X_intervals[i].r->Area() < minimum_area || width < minimum_w){
                continue;
            }

            //insert the interval
            X_Tree.Insert(X_intervals[i]);
            
            //end legal fill
            fill_target = 0;
            break;
        }
    }

    //Output
    double fill_density = 0.0;
    for(auto &interval: X_Tree.Traverse_Collect_Interval(1)){
        Fill.push_back(*interval.r);
        fill_density += interval.r->Area();
    }

    fill_density /= grid_size * grid_size;

#if 1
    if(abs(fill_density - Density_obj) > 0.001){
        std::cout << "Error: fill density is not equal to the target density" << std::endl;
        std::cout << "Target density: " << Density_obj << std::endl;
        std::cout << "Fill density: " << fill_density << std::endl;
    }
#endif

    return fill_density;
}

#endif