#ifndef Rect_Complement_HPP
#define Rect_Complement_HPP

#include "../GDS2_Read_Decomposition/ScanLine_Edge_Decomposition.hpp"

std::vector<Rect<int>> Rectangle_Decomposition(std::vector<Rect<int>> Rectangles)
{
    std::vector<edge<int>> edges;

    for(auto rect: Rectangles)
    {
        Coor<int> TL = rect.getTL();
        Coor<int> TR = rect.getTR();
        Coor<int> BR = rect.getBR();
        Coor<int> BL = rect.getBL();

        edge<int> e1(TL, TR);
        edge<int> e2(TR, BR);
        edge<int> e3(BR, BL);
        edge<int> e4(BL, TL);

        edge_list_edge_complement<int>(edges, e1, HORIZONTAL);
        edge_list_edge_complement<int>(edges, e2, VERTICAL);
        edge_list_edge_complement<int>(edges, e3, HORIZONTAL);
        edge_list_edge_complement<int>(edges, e4, VERTICAL);
    }

    Polygon_edge_collection<int> collection(edges);

    std::vector<Rect<int>> result;

    Edge_based_decomposition(collection, result);

    return result;
}

#endif