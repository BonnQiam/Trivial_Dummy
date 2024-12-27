#ifndef REACT_COMPLEMENT_HPP
#define REACT_COMPLEMENT_HPP

#include "../GDS2_Read_Decomposition/ScanLine_Edge_Decomposition.hpp"

std::vector<Rect<int>> Rectangle_Complement(Rect<int> Rectangle,std::vector<Rect<int>> Rectangles)
{

    // Initialize the Polygon_edge_collection according to the Rectangle
    std::vector<Coor<int>> vertices_Rectangle;

    vertices_Rectangle.push_back(Rectangle.getTL());
    vertices_Rectangle.push_back(Rectangle.getTR());
    vertices_Rectangle.push_back(Rectangle.getBR());
    vertices_Rectangle.push_back(Rectangle.getBL());

    Polygon_edge_collection<int> collection(vertices_Rectangle);
    collection.vertices_2_edges();

    // Edge complement
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

        edge_list_edge_complement<int>(collection.edges, e1, HORIZONTAL);
        edge_list_edge_complement<int>(collection.edges, e2, VERTICAL);
        edge_list_edge_complement<int>(collection.edges, e3, HORIZONTAL);
        edge_list_edge_complement<int>(collection.edges, e4, VERTICAL);
    }

    collection.edges_2_vertices();

#if 0
    for(auto edge: collection.edges)
    {
        std::cout << "Edge: " << edge.Coor_pair.first << " " << edge.Coor_pair.second << std::endl;
    }
#endif

    // Decomposition
    std::vector<Rect<int>> result;
    Edge_based_decomposition(collection, result);

    return result;
}

#endif