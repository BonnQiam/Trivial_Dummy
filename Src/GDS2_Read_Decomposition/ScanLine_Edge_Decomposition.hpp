#ifndef Decomposition_hpp
#define Decomposition_hpp

#include <iostream>
#include <fstream>
#include <sstream>

#include <vector>
#include <tuple>
#include <algorithm>

#include "Rectangle.hpp"

#define HORIZONTAL 0
#define VERTICAL 1

#define OVERLAP_subset  0   // e2 is the subset of e1, including the case that one point of e2 is the same as one point of e1
#define OVERLAP_full    1   // e1 and e2 are same
#define OVERLAP_partial 2   // e1 and e2 are overlapped partially, including the case that e1 is the subset of e2
#define OVERLAP_point   3   // e1 and e2 are overlapped at a point
#define OVERLAP_none    4   // e1 and e2 are not overlapped


/***************************************************************
 *           Edge and Polygon_edge_collection Definition
 * *************************************************************
 */

template <typename T>
struct edge
{
    std::pair< Coor<T>, Coor<T> > Coor_pair;
    edge(const Coor<T>& a, const Coor<T>& b) : Coor_pair(a, b) {}
};

template <typename T>
struct Polygon_edge_collection
{
    std::vector< edge<T> > edges;
    std::vector< Coor<T> > vertices;

//    void edges_sort(int sort_type);
    void edges_2_vertices(){
        vertices.clear();
        for(auto& e : edges)
        {
            vertices.push_back(e.Coor_pair.first);
            vertices.push_back(e.Coor_pair.second);
        }

        // remove the duplicate vertices
        vertices.erase(std::unique(vertices.begin(), vertices.end()), vertices.end());
    }
};

/***************************************************************
 *           Utility Functions
 * *************************************************************
 */

template <typename T>
void edges_sort(std::vector< edge<T> >& edges, int sort_type)
{
    if(sort_type == HORIZONTAL){
        // sort the edges by the min x value of the edge.first and edge.second
        std::sort(edges.begin(), edges.end(), [](const edge<T>& a, const edge<T>& b){
            return a.Coor_pair.first.getX() < b.Coor_pair.first.getX();
        });
    }
    else if(sort_type == VERTICAL){
        // sort the edges by the min y value of the edge.first and edge.second
        std::sort(edges.begin(), edges.end(), [](const edge<T>& a, const edge<T>& b){
            return a.Coor_pair.first.getY() < b.Coor_pair.first.getY();
        });
    }
}

template <typename T>
void sort_edge(edge<T>& e, int type)
{
    if(type == HORIZONTAL){
        if(e.Coor_pair.first.getX() > e.Coor_pair.second.getX()){
            std::swap(e.Coor_pair.first, e.Coor_pair.second);
        }
    }
    else if(type == VERTICAL){
        if(e.Coor_pair.first.getY() > e.Coor_pair.second.getY()){
            std::swap(e.Coor_pair.first, e.Coor_pair.second);
        }
    }
}


/***************************************************************
 *                    Declaration
 * *************************************************************
 */

template <typename T>
void Edge_based_decomposition(const std::vector< Coor<T> >& polygon, std::vector< Rect<T> >& result);

// Edge_based_decomposition requires the following functions
template <typename T>
static auto findCoorTuple_X(std::vector< Coor<T> >& vertices) // if not using the reference, the vertices will be changed
    -> std::tuple<typename std::vector< Coor<T> >::iterator, 
                    typename std::vector< Coor<T> >::iterator, 
                    typename std::vector< Coor<T> >::iterator>;

template <typename T>
static auto findCoorTuple_Y(std::vector< Coor<T> >& vertices) // if not using the reference, the vertices will be changed
    -> std::tuple<typename std::vector< Coor<T> >::iterator, 
                    typename std::vector< Coor<T> >::iterator, 
                    typename std::vector< Coor<T> >::iterator>;

template <typename T>
void edge_list_edge_complement(std::vector< edge<T> >& edge_list, 
                    edge<T> e2, int sort_type);

// after_overlapped requires the following functions
template <typename T>
int after_overlapped(edge<T>& e1, edge<T>& e2);


/***************************************************************
 *           Implementation of Edge_based_decomposition
 * *************************************************************
 */


template <typename T, typename const_iterator>
void Edge_based_decomposition(const const_iterator& first, const const_iterator& last, 
                    std::vector< Rect<T> >& result)
{
    result.clear();
    std::vector< Coor<T> > polygon(first, last);
    //establish the Polygon_edge_collection
    Polygon_edge_collection<T> Polygon_edges;
    for(auto it = polygon.begin(); it != polygon.end(); it++)
    {
        auto next = (std::next(it) == polygon.end()) ? polygon.begin() : std::next(it);
        Polygon_edges.edges.push_back(edge<T>(*it, *next));
    }
    Polygon_edges.edges_2_vertices();

#if 0
    //display the edges
    std::cout << "Edges: " << std::endl;
    for(auto iter = Polygon_edges.edges.begin(); iter != Polygon_edges.edges.end(); iter++)
    {
        std::cout << iter->Coor_pair.first << " " << iter->Coor_pair.second << std::endl;
    }   
#endif

    while (Polygon_edges.edges.size()>1)
    //for(int i=0; i<1; ++i)
    //for(int i=0; i<500; ++i)
    {
        const auto& coor_tuple_X = findCoorTuple_X(Polygon_edges.vertices);
        const auto Pk_X = *(std::get<0>(coor_tuple_X));
        const auto Pl_X = *(std::get<1>(coor_tuple_X));
        const auto Pm_X = *(std::get<2>(coor_tuple_X));
        T area_X = (Pl_X.getX() - Pk_X.getX()) * (Pm_X.getY() - Pk_X.getY());

        const auto& coor_tuple_Y = findCoorTuple_Y(Polygon_edges.vertices);
        const auto Pk_Y = *(std::get<0>(coor_tuple_Y));
        const auto Pl_Y = *(std::get<1>(coor_tuple_Y));
        const auto Pm_Y = *(std::get<2>(coor_tuple_Y));
        T area_Y = (Pm_Y.getX() - Pk_Y.getX()) * (Pk_Y.getY() - Pl_Y.getY());

        // choose the larger area
        int Flag = (area_X >= area_Y) ? 0 : 1;
        const auto& Pk = (Flag == 0) ? Pk_X : Pk_Y;
        const auto& Pl = (Flag == 0) ? Pl_X : Pl_Y;
        const auto& Pm = (Flag == 0) ? Pm_X : Pm_Y;

#if 0
        std::cout << "i: " << i << std::endl;
        std::cout << "Pk: " << Pk << std::endl;
        std::cout << "Pl: " << Pl << std::endl;
        std::cout << "Pm: " << Pm << std::endl;
        std::cout << "----------------------------------------------------------" << std::endl;
#endif

        if(Flag == 0)
        {
            // add the rectangle to the result
            result.emplace_back(Pk.getX(), Pk.getY(), 
                                Pl.getX() - Pk.getX(), Pm.getY() - Pk.getY());
        }
        else
        {
            // add the rectangle to the result
            result.emplace_back(Pl.getX(), Pl.getY(), 
                                Pm.getX() - Pk.getX(), Pk.getY() - Pl.getY());
        }

        // Upl and Upr
        Coor<T> upl = (Flag == 0) ? Coor<T>(Pk.getX(), Pm.getY()) : Coor<T>(Pm.getX(), Pk.getY());
        Coor<T> upr = (Flag == 0) ? Coor<T>(Pl.getX(), Pm.getY()) : Coor<T>(Pm.getX(), Pl.getY());
        
        // edges of the rectangle
        edge<T> e1(Pk, upl), e2(upl, upr), e3(upr, Pl), e4(Pl, Pk);
        // add to the Polygon_edges.edges

        if(Flag == 0){
            edge_list_edge_complement<T>(Polygon_edges.edges, e1, VERTICAL);
            edge_list_edge_complement<T>(Polygon_edges.edges, e2, HORIZONTAL);
            edge_list_edge_complement<T>(Polygon_edges.edges, e3, VERTICAL);
            edge_list_edge_complement<T>(Polygon_edges.edges, e4, HORIZONTAL);
        }
        else{
            edge_list_edge_complement<T>(Polygon_edges.edges, e1, HORIZONTAL);
            edge_list_edge_complement<T>(Polygon_edges.edges, e2, VERTICAL);
            edge_list_edge_complement<T>(Polygon_edges.edges, e3, HORIZONTAL);
            edge_list_edge_complement<T>(Polygon_edges.edges, e4, VERTICAL);
        }

#if 0
        if(i >= 1){
            edge_list_edge_complement<T>(Polygon_edges.edges, e1, VERTICAL);
            edge_list_edge_complement<T>(Polygon_edges.edges, e2, HORIZONTAL);
            edge_list_edge_complement<T>(Polygon_edges.edges, e3, VERTICAL);
            edge_list_edge_complement<T>(Polygon_edges.edges, e4, HORIZONTAL);
        }
        else{
            edge_list_edge_complement<T>(Polygon_edges.edges, e1, VERTICAL);
            edge_list_edge_complement<T>(Polygon_edges.edges, e2, HORIZONTAL);
            edge_list_edge_complement<T>(Polygon_edges.edges, e3, VERTICAL);
            edge_list_edge_complement<T>(Polygon_edges.edges, e4, HORIZONTAL);
        }
#endif
        Polygon_edges.edges_2_vertices();

#if 0
        //output to Edge.txt
        std::ofstream outfile("Edge.txt");
        //clear the file
        outfile.clear();
        for(auto iter = Polygon_edges.edges.begin(); iter != Polygon_edges.edges.end(); iter++)
        {
            outfile << iter->Coor_pair.first << " " << iter->Coor_pair.second << std::endl;
        }
        outfile.close();
#endif

#if 0
        //display the edges
        std::cout << "Edges: " << std::endl;
        for(auto iter = Polygon_edges.edges.begin(); iter != Polygon_edges.edges.end(); iter++)
        {
            std::cout << iter->Coor_pair.first << " " << iter->Coor_pair.second << std::endl;
        }
        // display the vertices
        std::cout << "Vertices: " << std::endl;
        for(auto iter = Polygon_edges.vertices.begin(); iter != Polygon_edges.vertices.end(); iter++)
        {
            std::cout << *iter << std::endl;
        }

        std::cout << "----------------------------------------------------------" << std::endl;
#endif
    }   
}

/***************************************************************
 *           Implementation of findCoorTuple_X and findCoorTuple_Y
 * *************************************************************
 */
template <typename T>
static auto findCoorTuple_X(std::vector< Coor<T> >& vertices) 
    -> std::tuple<typename std::vector< Coor<T> >::iterator, 
                    typename std::vector< Coor<T> >::iterator, 
                    typename std::vector< Coor<T> >::iterator>
{
    // find Pk: the bottom and the left-most coordinate
    const auto Pk = std::min_element(vertices.begin(), vertices.end(), 
        [](const auto& lhs, const auto& rhs) -> bool {
                if (lhs.getY() != rhs.getY()) { 
                    return (lhs.getY() < rhs.getY()); 
                    }
                else { 
                    return (lhs.getX() < rhs.getX()); 
                    }
            }
        );
    // find Pl: the bottom and the left-most coordinate except Pk
    const auto Pl = std::min_element(vertices.begin(), vertices.end(), 
        [&Pk](const auto& lhs, const auto& rhs) -> bool {
                // skip the element Pk
                if (lhs == (*Pk)) { 
                    return false; 
                    }
                else if (rhs == (*Pk)) { 
                    return true; 
                    }
                // find the smallest element
                else if (lhs.getY() != rhs.getY()) { 
                    return (lhs.getY() < rhs.getY()); 
                    }
                else { 
                    return (lhs.getX() < rhs.getX()); 
                }
            }
        );

    // find Pm
    const auto Pm = std::min_element(vertices.begin(), vertices.end(), 
        [&Pk, &Pl](const auto& lhs, const auto& rhs) -> bool {
                // skip the element (x, y) out of the range:
                // Pk.getX() <= x < Pl.getX() && Pk.getY() < y
                if ((lhs.getY() <= Pk->getY()) || 
                    (lhs.getX() < Pk->getX()) || 
                    //(lhs.getX() >= Pl->getX())) {
                    (lhs.getX() > Pl->getX())) {
                        return false; 
                    }
                else if ((rhs.getY() <= Pk->getY()) || 
                        (rhs.getX() < Pk->getX()) || 
                        //(rhs.getX() >= Pl->getX())) {
                        (rhs.getX() > Pl->getX())) {  
                            return true; 
                    }
                // find the smallest element
                else if (lhs.getY() != rhs.getY()) { 
                    return (lhs.getY() < rhs.getY()); 
                    }
                else { return (lhs.getX() < rhs.getX()); }
                
                return true;
            }
        );

    return {Pk, Pl, Pm};
}

template <typename T>
static auto findCoorTuple_Y(std::vector< Coor<T> >& vertices) 
    -> std::tuple<typename std::vector< Coor<T> >::iterator, 
                    typename std::vector< Coor<T> >::iterator, 
                    typename std::vector< Coor<T> >::iterator>
{
    // find Pk: the most left and most top coordinate
    const auto Pk = std::min_element(vertices.begin(), vertices.end(), 
        [](const auto& lhs, const auto& rhs) -> bool {
                if (lhs.getX() != rhs.getX()) { 
                    return (lhs.getX() < rhs.getX()); 
                    }
                else { 
                    return (lhs.getY() > rhs.getY()); 
                    }
            }
        );
    // find Pl: the most left and most top coordinate except Pk
    const auto Pl = std::min_element(vertices.begin(), vertices.end(), 
        [&Pk](const auto& lhs, const auto& rhs) -> bool {
                // skip the element Pk
                if (lhs == (*Pk)) { 
                    return false; 
                    }
                else if (rhs == (*Pk)) { 
                    return true; 
                    }
                // find the most top element
                else if (lhs.getX() != rhs.getX()) { 
                    return (lhs.getX() < rhs.getX()); 
                    }
                else { 
                    return (lhs.getY() > rhs.getY()); 
                }
            }
        );
    // find Pm
    const auto Pm = std::min_element(vertices.begin(), vertices.end(), 
        [&Pk, &Pl](const auto& lhs, const auto& rhs) -> bool {
                // skip the element (x, y) out of the range:
                // Pk.getY() <= y < Pl.getY() && Pk.getX() < x
                if ((lhs.getX() <= Pk->getX()) || 
                    (lhs.getY() > Pk->getY()) || 
                    //(lhs.getY() >= Pl->getY())) {
                    (lhs.getY() < Pl->getY())) {
                        return false; 
                    }
                else if ((rhs.getX() <= Pk->getX()) || 
                        (rhs.getY() > Pk->getY()) || 
                        //(rhs.getY() >= Pl->getY())) {
                        (rhs.getY() < Pl->getY())) {  
                            return true; 
                    }
                // find the most top element
                else if (lhs.getX() != rhs.getX()) { 
                    return (lhs.getX() < rhs.getX()); 
                    }
                else { return (lhs.getY() > rhs.getY()); }
        }
    );

    return {Pk, Pl, Pm};
}

/***************************************************************
 *           Implementation of edge_list_edge_complement
 * *************************************************************
 */

template <typename T>
void edge_list_edge_complement(std::vector< edge<T> >& edge_list, 
                    edge<T> e2, int sort_type)
{
    edges_sort<T>(edge_list, sort_type);
    // If the e2 overlap with some edges of edge_list, the overlapped parts of these edges should be removed, and the remaining parts (not exist in the edge_list) of e2 should be added into the edge_list.
    std::vector< edge<T> > add_edges;
    for(auto iter = edge_list.begin(); iter != edge_list.end();)
    {
        // calculate the overlapped part of e2 and *iter
        edge<T> e1 = *iter;
        
        // check if e1 is a point
        if(e1.Coor_pair.first == e1.Coor_pair.second){
            iter = edge_list.erase(iter);
            continue;
        }
        // check if e2 is a point
        if(e2.Coor_pair.first == e2.Coor_pair.second){
            break;
        }
#if 0
        std::cout << "--------------------------------" << std::endl;
        std::cout << "e1 is " << e1.Coor_pair.first << " " << e1.Coor_pair.second << std::endl;
        std::cout << "e2 is " << e2.Coor_pair.first << " " << e2.Coor_pair.second << std::endl;
#endif
        int flag = after_overlapped(e1, e2);

//        std::cout << "flag: " << flag << std::endl;

        if(flag == OVERLAP_subset){
            if(e1.Coor_pair.first == e1.Coor_pair.second){
                //if iter is a point, then remove iter and add e2 into the edge_list
                iter = edge_list.erase(iter);
                add_edges.push_back(e2);
            }
            else if(e2.Coor_pair.first == e2.Coor_pair.second){
                //if e2 is a point, then no need to add e2 into the edge_list, just update the *iter
                *iter = e1;
            }
            else{
                *iter = e1;
                add_edges.push_back(e2);
            }
            break;
        }
        else if(flag == OVERLAP_full){
            // remove the overlapped edge
            iter = edge_list.erase(iter);
            break;
        }
        else if(flag == OVERLAP_partial){
            //check if e1 is point
            if(e1.Coor_pair.first != e1.Coor_pair.second){
                *iter = e1;
                
                if(std::next(iter) == edge_list.end()){
                    if(e2.Coor_pair.first != e2.Coor_pair.second){
                        // add the e2 into the edge_list
                        add_edges.push_back(e2);
                    }
                    break;
                }
            }
            else{
                iter = edge_list.erase(iter);

                if(iter == edge_list.end()){
                    if(e2.Coor_pair.first != e2.Coor_pair.second){
                        // add the e2 into the edge_list
                        add_edges.push_back(e2);
                    }
                    break;
                }

                continue;
            }
        }
        else if(flag == OVERLAP_point){
            e2.Coor_pair.first.setX(std::min(e1.Coor_pair.first.getX(), e2.Coor_pair.first.getX()));
            e2.Coor_pair.first.setY(std::min(e1.Coor_pair.first.getY(), e2.Coor_pair.first.getY()));
            e2.Coor_pair.second.setX(std::max(e1.Coor_pair.second.getX(), e2.Coor_pair.second.getX()));
            e2.Coor_pair.second.setY(std::max(e1.Coor_pair.second.getY(), e2.Coor_pair.second.getY()));

//            std::cout << "In this case, e2 is " << e2.Coor_pair.first << " " << e2.Coor_pair.second << std::endl;

            if(std::next(iter) == edge_list.end()){
                iter = edge_list.erase(iter);
                // add the e2 into the edge_list
                add_edges.push_back(e2);
                break;
            }
            else{
                iter = edge_list.erase(iter);
                continue;
            }
        }
        else if(flag == OVERLAP_none){
            if(std::next(iter) == edge_list.end()){
                // add the e2 into the edge_list
                add_edges.push_back(e2);
                break;
            }
        }
        iter++;
    }

    // add the add_edges into the edge_list
    edge_list.insert(edge_list.end(), add_edges.begin(), add_edges.end());
}

/***************************************************************
 *           Implementation of after_overlapped
 * *************************************************************
 */
// remove the overlapped part of e1 and e2 from e1 and e2
template <typename T>
int after_overlapped(edge<T>& e1, edge<T>& e2){
    // if e1, e2 are horizontal edges with same y
    if(e1.Coor_pair.first.getY() == e1.Coor_pair.second.getY() && 
        e2.Coor_pair.first.getY() == e2.Coor_pair.second.getY() &&
        e1.Coor_pair.first.getY() == e2.Coor_pair.first.getY()
        ){
        
        // sort the e1 and e2, so that first point of e1/e2 is the leftmost point
        sort_edge(e1, HORIZONTAL);
        sort_edge(e2, HORIZONTAL);

        if(e1.Coor_pair.second.getX() < e2.Coor_pair.first.getX() || 
            e2.Coor_pair.second.getX() < e1.Coor_pair.first.getX()){
            return OVERLAP_none;
        }
        else if (
            e1.Coor_pair.second.getX() == e2.Coor_pair.first.getX() ||
            e2.Coor_pair.second.getX() == e1.Coor_pair.first.getX()
        ){
            return OVERLAP_point;
        }
        else{
            // if e1 and e2 overlap, remove the overlapped part
            
            int flag;
            //check if e1 and e2 are same
            if(e1.Coor_pair.first == e2.Coor_pair.first && e1.Coor_pair.second == e2.Coor_pair.second){
                flag = OVERLAP_full;
            }
            //check if e2 is the subset of e1
            else if(e2.Coor_pair.first.getX() >= e1.Coor_pair.first.getX() && e2.Coor_pair.second.getX() <= e1.Coor_pair.second.getX()){
                flag = OVERLAP_subset;
            }
            else{
                flag = OVERLAP_partial;
            }
            
            Coor<T> tmp = e1.Coor_pair.second;
            e1.Coor_pair.second = e2.Coor_pair.first;
            e2.Coor_pair.first = tmp;
            
            sort_edge(e1, HORIZONTAL);
            sort_edge(e2, HORIZONTAL);

            return flag;
        }
    }
    // if e1, e2 are vertical edges with sanme X
    else if(e1.Coor_pair.first.getX() == e1.Coor_pair.second.getX() && 
        e2.Coor_pair.first.getX() == e2.Coor_pair.second.getX()
        && e1.Coor_pair.first.getX() == e2.Coor_pair.first.getX()){
        // sort the e1 and e2, so that first point of e1/e2 is the bottommost point
        sort_edge(e1, VERTICAL);
        sort_edge(e2, VERTICAL);

        if(e1.Coor_pair.second.getY() < e2.Coor_pair.first.getY() || 
            e2.Coor_pair.second.getY() < e1.Coor_pair.first.getY()){
            return OVERLAP_none;
        }
        else if (
            e1.Coor_pair.second.getY() == e2.Coor_pair.first.getY() ||
            e2.Coor_pair.second.getY() == e1.Coor_pair.first.getY()
        ){
            return OVERLAP_point;
        }
        else{
            // if e1 and e2 overlap, remove the overlapped part
            
            int flag;
            //check if e1 and e2 are same
            if(e1.Coor_pair.first == e2.Coor_pair.first && e1.Coor_pair.second == e2.Coor_pair.second){
                flag = OVERLAP_full;
            }
            //check if e2 is the subset of e1
            else if(e2.Coor_pair.first.getY() >= e1.Coor_pair.first.getY() && e2.Coor_pair.second.getY() <= e1.Coor_pair.second.getY()){
                flag = OVERLAP_subset;
            }
            else{
                flag = OVERLAP_partial;
            }
            
            Coor<T> tmp = e1.Coor_pair.second;
            e1.Coor_pair.second = e2.Coor_pair.first;
            e2.Coor_pair.first = tmp;
            
            sort_edge(e1, VERTICAL);
            sort_edge(e2, VERTICAL);

            return flag;
        }
    }
    else{
        return OVERLAP_none;
    }
}

#endif