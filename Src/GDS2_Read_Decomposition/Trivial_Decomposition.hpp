#ifndef Decomposition_hpp
#define Decomposition_hpp



#include <tuple>
#include <algorithm>
#include <vector>

#include "Coor.hpp"

#define Debug 0
#define log 0

/**********************************************************************
 *                 Utility Functions
 * ********************************************************************
 */

// Calculate the area of a triangle formed by three points
template <typename T>
double triangleArea(const Coor<T>& p1, const Coor<T>& p2, const Coor<T>& p3) {
    // calculate the area of a triangle
    return 0.5 * (p1.getX() * p2.getY() + p2.getX() * p3.getY() + p3.getX() * p1.getY() - p1.getX() * p3.getY() - p2.getX() * p1.getY() - p3.getX() * p2.getY());
}


/**********************************************************************
 *                 Definition of Edge and Polygon
 * ********************************************************************
 */

template <typename T>
struct edge
{
    std::pair< Coor<T>, Coor<T> > Coor_pair;
    edge(const Coor<T>& a, const Coor<T>& b) : Coor_pair(a, b) {}
};

template <typename T>
struct Polygon
{   
    // vertexes set of the polygon
    std::vector<Coor<T>> vertexes;
    // edges set of the polygon
    std::vector<edge<T>> edges;

     // according to vertexes, initialize edges
    void edges_init(){
        edges.clear();
        auto vertex = vertexes.begin();
        for(; vertex != (vertexes.end()-1); vertex++){
            edges.push_back(edge<T>(*vertex, *(vertex+1)));
        } 
    }

    // according to edges, initialize vertexes
    void vertexes_init(){
        vertexes.clear();
        auto edge = edges.begin();
        for(; edge != edges.end(); edge++){
            vertexes.push_back(edge->Coor_pair.first);
        }
        vertexes.push_back(edges.back().Coor_pair.second);
    }

    bool isInside(Coor<T> point); // check if the point is inside the polygon based on the Winding Number Algorithm

};

template <typename T>
bool Polygon<T>::isInside(Coor<T> point)
{
    int windingNumber = 0;

    for(int i=0; i<vertexes.size()-1; i++){
        if(vertexes[i].getY() <= point.getY()){
            if(vertexes[(i+1) % vertexes.size()].getY() > point.getY()){
                if(isLeft(vertexes[i], vertexes[(i+1) % vertexes.size()], point) > 0)
                    windingNumber++;
            }
        }
        else{
            if(vertexes[(i+1) % vertexes.size()].getY() <= point.getY()){
                if(isLeft(vertexes[i], vertexes[(i+1) % vertexes.size()], point) < 0)
                    windingNumber--;
            }
        }

        double area = triangleArea(vertexes[i], vertexes[(i+1) % vertexes.size()], point);
        if(area == 0){
            return true;
        }
    }

    return windingNumber != 0;
}


/**********************************************************************
 *                          Declaration
 * ********************************************************************
 */

template <typename T>
void Decomposition(
    const Polygon<T>& poly,
    std::vector< Polygon<T> >& result
);

// Decomposition requires the following functions

template <typename T>
static auto findCoorTuple_Refactor(Polygon<T>& polygon)
    -> std::tuple<Coor<T>, Coor<T>, Coor<T>>;

template <typename T>
void Edge_list_complement(
    const Polygon<T> &polygon,
    const Polygon<T> &rectangle, // Pk -> Pl -> Upr -> Upl -> Pk
    Polygon<T> &polygon_complement
);

// Edge_list_complement requires the following functions
template <typename T>
void Polygon_shrink_redundancy_point(const Polygon<T> &polygon,  Polygon<T> &polygon_shrink);

template <typename T>
bool Polygon_shrink_redundancy_edge(const Polygon<T> &polygon,  Polygon<T> &polygon_shrink);

/**********************************************************************
 *                 Implementation of Decomposition
 * ********************************************************************
 */

template <typename T>
void Decomposition(
    const Polygon<T>& poly,
    std::vector< Polygon<T> >& result
)
{
    result.clear();

    Polygon<T> poly_copy = poly;
    poly_copy.vertexes   = poly.vertexes;
    poly_copy.edges      = poly.edges;

    while(poly_copy.vertexes.size() > 1){
//    for(int i=0; i<13; i++){
        const auto& coor_tuple = findCoorTuple_Refactor(poly_copy);

        const auto Pk = std::get<0>(coor_tuple);
        const auto Pl = std::get<1>(coor_tuple);
        const auto Pm = std::get<2>(coor_tuple);

#if Debug
        std::cout << "Pk: " << Pk << std::endl;
        std::cout << "Pl: " << Pl << std::endl;
        std::cout << "Pm: " << Pm << std::endl;
#endif
        Coor<T> Upl(Pk.getX(), Pm.getY()), Upr(Pl.getX(), Pm.getY());

        Polygon<T> Rectangle;
        Rectangle.vertexes.push_back(Pk);
        Rectangle.vertexes.push_back(Pl);
        Rectangle.vertexes.push_back(Upr);
        Rectangle.vertexes.push_back(Upl);
        
        Rectangle.edges_init();

        result.push_back(Rectangle);

        Polygon<T> poly_complement;
        Edge_list_complement(poly_copy, Rectangle, poly_complement);

#if Debug
        std::cout << "poly_complement: " << std::endl;
        for(auto v: poly_complement.vertexes){
            std::cout << v << std::endl;
        }
#endif
        // copy poly_complement to poly_copy
        poly_copy.vertexes = poly_complement.vertexes;
        poly_copy.edges    = poly_complement.edges;

//        std::cout << "==========================" << std::endl;
    }

#if Debug
    std::cout << "The rectangle list is " << std::endl;
    for(auto rec: result){
        for(auto v: rec.vertexes){
            std::cout << v << std::endl;
        }
        std::cout << "--------------------" << std::endl;
    }
#endif
}

/**********************************************************************
 *                 Implementation of findCoorTuple_Refactor
 * ********************************************************************
 */


template <typename T>
static auto findCoorTuple_Refactor(Polygon<T>& polygon)
    -> std::tuple<Coor<T>, Coor<T>, Coor<T>>
{   
    Polygon<T> poly_copy = polygon;
    poly_copy.vertexes   = polygon.vertexes;
    poly_copy.edges      = polygon.edges;

    bool legal_Pk = false;
    auto Pk = poly_copy.vertexes.begin();
    while(1){
        Pk = std::min_element(poly_copy.vertexes.begin(), poly_copy.vertexes.end(), 
        [](const auto& lhs, const auto& rhs) -> bool {
                if (lhs.getY() != rhs.getY()) { 
                    return (lhs.getY() < rhs.getY()); 
                    }
                else { 
                    return (lhs.getX() < rhs.getX()); 
                    }
            }
        );
        // Check the Pk
        Coor<T> Test = *Pk;
        Test.addToX(1);
        Test.addToY(1);

        legal_Pk = polygon.isInside(Test);

        if(legal_Pk)
            break;
        else{
            poly_copy.vertexes.erase(Pk);
        }
        
    }

    // find Pl: the bottom and the left-most coordinate except Pk
    auto Pl = std::min_element(poly_copy.vertexes.begin(), poly_copy.vertexes.end(), 
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
    auto Pm = std::min_element(poly_copy.vertexes.begin(), poly_copy.vertexes.end(), 
        [&Pk, &Pl](const auto& lhs, const auto& rhs) -> bool {
                // skip the element (x, y) out of the range:
                // Pk.getX() <= x < Pl.getX() && Pk.getY() < y
                if ((lhs.getY() <= Pk->getY()) || 
                    (lhs.getX() < Pk->getX()) || 
                    (lhs.getX() >= Pl->getX())) { 
                        return false; 
                    }
                else if ((rhs.getY() <= Pk->getY()) || 
                        (rhs.getX() < Pk->getX()) || 
                        (rhs.getX() >= Pl->getX())) { 
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
    
    return std::make_tuple(*Pk, *Pl, *Pm);
}

#endif


/**********************************************************************
 *                 Implementation of Edge_list_complement
 * ********************************************************************
 */

template <typename T>
void Edge_list_complement(
    const Polygon<T> &polygon,
    const Polygon<T> &rectangle, // Pk -> Pl -> Upr -> Upl -> Pk
    Polygon<T> &polygon_complement)
{
    Coor<T> Pk  = rectangle.vertexes[0];
    Coor<T> Pl  = rectangle.vertexes[1];
    Coor<T> Upr = rectangle.vertexes[2];
    Coor<T> Upl = rectangle.vertexes[3];

//    std::cout << "Pk is " << Pk << std::endl;
//    std::cout << "Pl is " << Pl << std::endl;
//    std::cout << "Upr is " << Upr << std::endl;
//    std::cout << "Upl is " << Upl << std::endl;

    /***********************************************************************************
     * complement edges step 1: remove Pk-Pl and collect all other edges in polygon and rectangle
     */
    Polygon<T> polygon_tmp = polygon;
    polygon_tmp.vertexes = polygon.vertexes;
    polygon_tmp.edges = polygon.edges;

//    Polygon_shrink_redundancy_point(polygon, polygon_tmp);

//    std::cout << "Polygon_tmp is " << std::endl;
//    for(auto v : polygon_tmp.vertexes){
//        std::cout << "(" << v.getX() << ", " << v.getY() << ")" << std::endl;
//    }

    // find Pk -> Pl edges in polygon
    auto itr_kl = polygon_tmp.edges.begin();
    auto itr_v  = polygon_tmp.vertexes.begin();

    for(; itr_kl != polygon_tmp.edges.end(); itr_kl++){
        itr_v = itr_v + 1;

        if(itr_kl->Coor_pair.first.getY() == itr_kl->Coor_pair.second.getY()){
            if(itr_kl->Coor_pair.first == Pk){
                if(itr_kl->Coor_pair.second == Pl)
                    break;
                else {
                    polygon_tmp.vertexes.insert(itr_v, Pl);
                    polygon_tmp.edges_init();
                    break;
                }
            }
            else if(itr_kl->Coor_pair.first == Pl){
                if(itr_kl->Coor_pair.second == Pk)
                    break;
                else {
                    polygon_tmp.vertexes.insert(itr_v, Pk);
                    polygon_tmp.edges_init();
                    break;
                }
            }
        }
        else{
            continue;
        }
        
        if( (itr_kl+1) == polygon_tmp.edges.end()){
            std::cout << "Error: Pk -> Pl edge not found in polygon" << std::endl;
            return;
        }
    }

    // remove Pk-Pl and collect all other edges in polygon and rectangle
    for(auto e = polygon_tmp.edges.begin(); e != polygon_tmp.edges.end(); e++){
        if( (e->Coor_pair.first == Pk) && (e->Coor_pair.second == Pl) ){
            polygon_complement.edges.push_back(edge<T>(Pk, Upl));
            polygon_complement.edges.push_back(edge<T>(Upl, Upr));
            polygon_complement.edges.push_back(edge<T>(Upr, Pl));
        }
        else if((e->Coor_pair.first == Pl) && (e->Coor_pair.second == Pk)){
            polygon_complement.edges.push_back(edge<T>(Pl, Upr));
            polygon_complement.edges.push_back(edge<T>(Upr, Upl));
            polygon_complement.edges.push_back(edge<T>(Upl, Pk));
        }
        else
            polygon_complement.edges.push_back(*e);
    }
    polygon_complement.vertexes_init();

#if log
    std::cout << "Before shrink, polygon_complement is " << std::endl;
    for(auto v : polygon_complement.vertexes){
        std::cout << "(" << v.getX() << ", " << v.getY() << ")" << std::endl;
    }
#endif
   /**********************************************************************************
     * complement edges step 2: remove redundancy_edge
    */ 
    
    Polygon<int> poly_shrink;
    poly_shrink.vertexes = polygon_complement.vertexes;

    bool redundancy = true;

    while(redundancy){
        redundancy = Polygon_shrink_redundancy_edge(polygon_complement, poly_shrink);

        if(poly_shrink.vertexes.size() == 1){
            break;
        }

        if(redundancy){
            polygon_complement.vertexes = poly_shrink.vertexes;
            polygon_complement.edges = poly_shrink.edges;
        }
    }

#if log
    std::cout << "After shrink, polygon_complement is " << std::endl;
    for(auto v : poly_shrink.vertexes){
        std::cout << "(" << v.getX() << ", " << v.getY() << ")" << std::endl;
    }
#endif
    // Copy poly_shrink to polygon_complement
    polygon_complement.vertexes = poly_shrink.vertexes;
    polygon_complement.edges = poly_shrink.edges;

    /**********************************************************************************
     * complement edges step 3: remove redundancy_edge
    */ 
    if(polygon_complement.vertexes.size() == 1){
        return;
    }
    poly_shrink.vertexes = polygon_complement.vertexes;
    poly_shrink.edges = polygon_complement.edges;

    Polygon_shrink_redundancy_point(polygon_complement, poly_shrink);
    
    polygon_complement.vertexes = poly_shrink.vertexes;
    polygon_complement.edges = poly_shrink.edges;

}

/**********************************************************************
 *                 Implementation of Polygon_shrink_redundancy_edge
 * ********************************************************************
 */


template <typename T>
void Polygon_shrink_redundancy_point(const Polygon<T> &polygon,  Polygon<T> &polygon_shrink)
{
    auto e = polygon.edges.begin();
    auto v = polygon_shrink.vertexes.begin() + 1;

    while(e != polygon.edges.end()){
        auto e_nxt = ( (e+1) == polygon.edges.end() ) ? polygon.edges.begin() : (e+1);

        bool find_duplicate = false;

        T e_X     = e->Coor_pair.second.getX() - e->Coor_pair.first.getX();
        T e_nxt_X = e_nxt->Coor_pair.second.getX() - e_nxt->Coor_pair.first.getX();

        T e_Y     = e->Coor_pair.second.getY() - e->Coor_pair.first.getY();
        T e_nxt_Y = e_nxt->Coor_pair.second.getY() - e_nxt->Coor_pair.first.getY();

        if(
            (e->Coor_pair.second.getY() == e->Coor_pair.first.getY() ) &&
            (e_nxt->Coor_pair.second.getY() == e_nxt->Coor_pair.first.getY() )
        ){
            if(e_X*e_nxt_X > 0)
                find_duplicate = true;
        }
        else if ( (e->Coor_pair.second.getX() == e->Coor_pair.first.getX() ) &&
            (e_nxt->Coor_pair.second.getX() == e_nxt->Coor_pair.first.getX() )
        ){
            if(e_Y*e_nxt_Y > 0)
                find_duplicate = true;
        }
        
        if(find_duplicate){
            if( (e+1) == polygon.edges.end() ){
                *(polygon_shrink.vertexes.end()-1) = *(polygon_shrink.vertexes.begin()+1);
                polygon_shrink.vertexes.erase(polygon_shrink.vertexes.begin());
                break;
            }
            else{
                e = e+1;
                v = polygon_shrink.vertexes.erase(v);
            }
        }
        else{
            e = e+1;
            v = v+1;
        }
    }

    polygon_shrink.edges_init();
}

/**********************************************************************
 *                 Implementation of Polygon_shrink_redundancy_edge
 * ********************************************************************
 */

template <typename T>
bool Polygon_shrink_redundancy_edge(const Polygon<T> &polygon,  Polygon<T> &polygon_shrink)
{
    int original_size = polygon_shrink.vertexes.size();

    auto e = polygon.edges.begin();
    auto v = polygon_shrink.vertexes.begin() + 1;

    while( e != polygon.edges.end()){
        auto e_nxt = ( (e+1) == polygon.edges.end() ) ? polygon.edges.begin() : (e+1);

        bool find_duplicate = false;

        T e_X     = e->Coor_pair.second.getX() - e->Coor_pair.first.getX();
        T e_nxt_X = e_nxt->Coor_pair.second.getX() - e_nxt->Coor_pair.first.getX();

        T e_Y     = e->Coor_pair.second.getY() - e->Coor_pair.first.getY();
        T e_nxt_Y = e_nxt->Coor_pair.second.getY() - e_nxt->Coor_pair.first.getY();

        int e_X_sign = (e_X > 0) ? 1 : -1;
        int e_Y_sign = (e_Y > 0) ? 1 : -1;
        int e_nxt_X_sign = (e_nxt_X > 0) ? 1 : -1;
        int e_nxt_Y_sign = (e_nxt_Y > 0) ? 1 : -1;

        if(
            (e->Coor_pair.second.getY() == e->Coor_pair.first.getY() ) &&
            (e_nxt->Coor_pair.second.getY() == e_nxt->Coor_pair.first.getY() )
        ){
            if(e_X_sign != e_nxt_X_sign)
                find_duplicate = true;
        }
        else if ( (e->Coor_pair.second.getX() == e->Coor_pair.first.getX() ) &&
            (e_nxt->Coor_pair.second.getX() == e_nxt->Coor_pair.first.getX() )
        ){
            if(e_Y_sign != e_nxt_Y_sign)
                find_duplicate = true;
        }

        if(find_duplicate){
            T v_NewX  = e_nxt->Coor_pair.second.getX();
            T v_NewY  = e_nxt->Coor_pair.second.getY();

            if( (v_NewX == e->Coor_pair.first.getX()) && (v_NewY == e->Coor_pair.first.getY()) ){
                if( (e+1) == polygon.edges.end() ){
                    polygon_shrink.vertexes.erase(v);
                    polygon_shrink.vertexes.erase(polygon_shrink.vertexes.begin());
                    break;
                }
                else{
                    v = polygon_shrink.vertexes.erase(v);
                    v = polygon_shrink.vertexes.erase(v);
                    e = e+2;
                }
            }
            else{
                if( (e+1) == polygon.edges.end() ){
                    *(polygon_shrink.vertexes.end()-1) = *(polygon_shrink.vertexes.begin()+1);
                    polygon_shrink.vertexes.erase(polygon_shrink.vertexes.begin());
                    break;
                }
                else{
                    v = polygon_shrink.vertexes.erase(v)+1;
                    e = e+2;
                }
            }
        }
        else{
            e = e+1;
            v = v+1;
        }

    }

    
    int shrink_size = polygon_shrink.vertexes.size();
    if(original_size == shrink_size)
        return false;
    else
        polygon_shrink.edges_init();
        return true;
}