#include <limbo/parsers/gdsii/stream/GdsWriter.h>

#include "util.hpp"

int main(int argc, char *argv[])
{
    /*
     * ******************************************* Get the Input file
     */
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    // Read filename from argv[2]
    std::string filename = argv[2];

    std::cout << "Reading file: " << filename << '\n';

    std::ifstream file(filename);
    std::vector<std::string> Input;

    std::string line;
    while (std::getline(file, line))
    {
        Input.push_back(line);
    }

#if 1
    // Test: display Input
    for (int i = 0; i < Input.size(); i++)
    {
        std::cout << Input[i] << std::endl;
    }
#endif 

    /*
     * ******************************************* Load the density obj and rectangles
     */

    DensityObj Obj_Layer1, Obj_Layer2, Obj_Layer3;

    Obj_Layer1.size = 2;
    Obj_Layer2.size = 4;
    Obj_Layer3.size = 2;

    LoadFillObj(Input[0], Obj_Layer1);

#if 1
    LoadFillObj(Input[1], Obj_Layer2);
    LoadFillObj(Input[2], Obj_Layer3);
#endif

    std::vector<std::vector<Rectangle>>  List_Rectangles_Dummy_Dummy_M1,
                            List_Rectangles_Dummy_Wire_M1,
                            List_Rectangles_Dummy_Dummy_Dummy_M2,
                            List_Rectangles_Dummy_Dummy_Wire_M2,
                            List_Rectangles_Wire_Dummy_Dummy_M2,
                            List_Rectangles_Wire_Dummy_Wire_M2,
                            List_Rectangles_Dummy_Dummy_M3,
                            List_Rectangles_Wire_Dummy_M3;

    LoadRectangle(Input[3], List_Rectangles_Dummy_Dummy_M1,10001);

#if 0
    LoadRectangle(Input[4], List_Rectangles_Dummy_Wire_M1,10001);

    LoadRectangle(Input[5], List_Rectangles_Dummy_Dummy_Dummy_M2,10002);
    LoadRectangle(Input[6], List_Rectangles_Dummy_Dummy_Wire_M2,10002);
    LoadRectangle(Input[7], List_Rectangles_Wire_Dummy_Dummy_M2,10002);
    LoadRectangle(Input[8], List_Rectangles_Wire_Dummy_Wire_M2,10002);

    LoadRectangle(Input[9], List_Rectangles_Wire_Dummy_M3,10003);
    
    int Num_grid = List_Rectangles_Dummy_Dummy_Dummy_M2.size();

    for(int i=0; i<Num_grid; i++){
        List_Rectangles_Dummy_Dummy_M3.push_back(std::vector<Rectangle>());
        
        for(int j=0; j<List_Rectangles_Dummy_Dummy_Dummy_M2[i].size(); j++){
            Rectangle tmp = List_Rectangles_Dummy_Dummy_Dummy_M2[i][j];
            tmp.layer = 10003;
            List_Rectangles_Dummy_Dummy_M3[i].push_back(tmp);
        }

        for(int j=0; j<List_Rectangles_Wire_Dummy_Dummy_M2[i].size(); j++){
            Rectangle tmp = List_Rectangles_Wire_Dummy_Dummy_M2[i][j];
            tmp.layer = 10003;
            List_Rectangles_Dummy_Dummy_M3[i].push_back(tmp);
        }
        RectangleSort(List_Rectangles_Dummy_Dummy_M3[i]);
    }
#endif

    std::cout << "Data Loading is done !" << std::endl;

    /*
     * ******************************************* Inserted rectangle generate
     */
    std::vector< std::vector<Rectangle> > List_Result;

    Layer_Rectangle_Generate(Obj_Layer1.grid_fill_density[0], List_Rectangles_Dummy_Dummy_M1, List_Result);

#if 0
    Layer_Rectangle_Generate(Obj_Layer1.grid_fill_density[1], List_Rectangles_Dummy_Wire_M1, List_Result);

    Layer_Rectangle_Generate(Obj_Layer2.grid_fill_density[0], List_Rectangles_Dummy_Dummy_Dummy_M2, List_Result);

    Layer_Rectangle_Generate(Obj_Layer2.grid_fill_density[1], List_Rectangles_Dummy_Dummy_Wire_M2, List_Result);

    Layer_Rectangle_Generate(Obj_Layer2.grid_fill_density[2], List_Rectangles_Wire_Dummy_Dummy_M2, List_Result);

    Layer_Rectangle_Generate(Obj_Layer2.grid_fill_density[3], List_Rectangles_Wire_Dummy_Wire_M2, List_Result);

    Layer_Rectangle_Generate(Obj_Layer3.grid_fill_density[0], List_Rectangles_Dummy_Dummy_M3, List_Result);

    Layer_Rectangle_Generate(Obj_Layer3.grid_fill_density[1], List_Rectangles_Wire_Dummy_M3, List_Result);
#endif

    std::cout << "Rectangle generation is done !" << std::endl;


    // Just for Debug
    std::ofstream Output1("Debug_Dummy.txt");
    std::ofstream Output2("Debug_Fillable.txt");
    for(int i = 0; i < List_Result.size(); i++){
        
        if(i == 21){
            Output1 << "<grid>" << std::endl;
            Output1 << 0.1 << std::endl;
            for(int j = 0; j < List_Result[i].size(); j++){
                Output1 << "(" << List_Result[i][j].x[0] << ", " << List_Result[i][j].y[0] << "),";
                Output1 << "(" << List_Result[i][j].x[2] << ", " << List_Result[i][j].y[2] << ")" << std::endl;
            }
            Output1 << "</grid>" << std::endl;

            Output2 << "<grid>" << std::endl;
            Output2 << 0.1 << std::endl;
            for(int j = 0; j < List_Rectangles_Dummy_Dummy_M1[i].size(); j++){
                Output2 << "(" << List_Rectangles_Dummy_Dummy_M1[i][j].x[0] << ", " << List_Rectangles_Dummy_Dummy_M1[i][j].y[0] << "),";
                Output2 << "(" << List_Rectangles_Dummy_Dummy_M1[i][j].x[2] << ", " << List_Rectangles_Dummy_Dummy_M1[i][j].y[2] << ")" << std::endl;
            }
            Output2 << "</grid>" << std::endl;
        }

    }

#if 0
    /*
     * ******************************************* GDSII output
     */
    GdsParser::GdsWriter gw (argv[1]);
    gw.create_lib("Dummy", 0.001, 1.0e-9);
    gw.gds_write_bgnstr();
    gw.gds_write_strname("dummies");

    for(int i = 0; i < List_Result.size(); i++){
        for(int j = 0; j < List_Result[i].size(); j++){
            gw.write_boundary(List_Result[i][j].layer, 0, 
                List_Result[i][j].x, 
                List_Result[i][j].y, true);
        }
    }
    gw.gds_write_endstr();
    gw.gds_write_endlib();

    std::cout << "GDSII generation is done !" << std::endl;
#endif

    return 0;
}