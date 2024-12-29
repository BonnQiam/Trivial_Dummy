#include "util.hpp"

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
    std::vector<Rect<int>> Result;
    
    LoadWindowData(argv[1], List_rect);
    Legal_Fill(List_rect, Result);
        
    std::ofstream file("Overlap_Rectangles.txt");

    file<<"<grid>"<<std::endl;
    file<<0.1<<std::endl;

    for(auto &rect: Result){
        file<<"("<<rect.getBL().getX()<<", "<<rect.getBL().getY()<<"),";
        file<<"("<<rect.getTR().getX()<<", "<<rect.getTR().getY()<<")"<<std::endl;
    }
    file<<"</grid>"<<std::endl;

    file.close();

    return 0;
}
