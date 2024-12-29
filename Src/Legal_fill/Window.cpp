#include "util.hpp"

#define Min_s   32

int main(int argc, char *argv[])
{

#if 0
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }
#endif
    /*
     *------------------------------------------------- Loading
     */
    std::vector<Rect<int>> List_rect;
    std::vector<Interval>  X_intervals;
    std::vector<int>       X_points;

    
    LoadWindowData(argv[1], List_rect);

    for(int i = 0; i < List_rect.size(); i++){
        X_intervals.push_back({
            List_rect[i].getBL().getX(), List_rect[i].getTR().getX(), 
            List_rect[i].getBL().getY(),
            0,
            &List_rect[i]});

        X_points.push_back(List_rect[i].getBL().getX());
        X_points.push_back(List_rect[i].getTR().getX());
    }

    std::sort(X_points.begin(), X_points.end());
    X_points.erase(std::unique(X_points.begin(), X_points.end()), X_points.end());
    /*
     *-----------------------------------------   Initialize the X & Y Interval Tree
     */

    IntervalTree X_Tree(X_points);
    /*
     *-----------------------------------------   Insert the Intervals
     */
    for (int i = 0; i < X_intervals.size(); i++)
    {
        X_Tree.Insert(X_intervals[i]);
    }

    /*
     *-----------------------------------------   Window operation test
     */
    Coor<int> bl(25000, 25000);
    Coor<int> tr(27500, 27500);

    // Extend
    bl.addToX(-Min_s);
    bl.addToY(-Min_s);
    tr.addToX(Min_s);
    tr.addToY(Min_s);

    Rect<int> test_Rect = {bl, tr};
    std::vector<Rect<int>> overlap_rects;

    std::vector<Interval> X_overlap = X_Tree.Overlap_Query(bl.getX(), tr.getX(), 0);

    for(auto &interval: X_overlap){
        if(interval.x_start == test_Rect.getBL().getX() && 
           interval.x_end == test_Rect.getTR().getX() &&
           interval.y == test_Rect.getBL().getY()){
            continue;
        }

        Rect<int> R = Rectangle_intersection(*interval.r, test_Rect);

        if(R.Area() > 0){
            overlap_rects.push_back(*interval.r);
        }
    }

    std::cout << "Size of overlap_rects: " << overlap_rects.size() << std::endl;

    //output
    std::ofstream file("Overlap_Rectangles.txt");

    file<<"<grid>"<<std::endl;
    file<<0.1<<std::endl;

    for(auto &rect: overlap_rects){
        file<<"("<<rect.getBL().getX()<<", "<<rect.getBL().getY()<<"),";
        file<<"("<<rect.getTR().getX()<<", "<<rect.getTR().getY()<<")"<<std::endl;
    }

    file<<"</grid>"<<std::endl;

    file.close();

    return 0;
}
