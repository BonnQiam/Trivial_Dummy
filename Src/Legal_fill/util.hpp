#ifndef UTIL
#define UTIL

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

void LoadWindowData(const std::string &Filename, std::vector<Rect<int>> &Rects)
{
    std::ifstream file(Filename);

    std::string line;
    std::regex re(R"(\((\d+), (\d+)\),\((\d+), (\d+)\))");
    
    while(std::getline(file, line)){
        if(line == "<grid>")
            std::getline(file, line);
        else{
            if(line == "</grid>")
                break;

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
            Rects.push_back(R);
        }
    }
}

void Legal_Fill(std::vector<Rect<int>> List_rect, std::vector<Rect<int>> &Fill)
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

    int fill_target = 0.0989617 * grid_size * grid_size;

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
        int up=0, down=0, left=0, right=0;

        for(auto &rect: overlap_rects){
            int tmp1 = tr.getY() - rect.getTR().getY();//corresponding to up
            int tmp2 = rect.getBL().getY() - bl.getY();//corresponding to down
            int tmp3 = rect.getBL().getX() - bl.getX();//corresponding to left
            int tmp4 = tr.getX() - rect.getTR().getX();//corresponding to right

            if(tmp1 > up)
                up = std::max(up, tmp1);
            if(tmp2 > down)
                down = std::max(down, tmp2);
            if(tmp3 > left)
                left = std::max(left, tmp3);
            if(tmp4 > right)
                right = std::max(right, tmp4);
        }

        //adjust the rectangle
        X_intervals[i].r->setBL(
            X_intervals[i].r->getBL().getX() + left, 
            X_intervals[i].r->getBL().getY() + down);

        X_intervals[i].r->setTR(
            X_intervals[i].r->getTR().getX() - right, 
            X_intervals[i].r->getTR().getY() - up);

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
    for(auto &interval: X_Tree.Traverse_Collect_Interval(1)){
        Fill.push_back(*interval.r);
    }
}

#endif