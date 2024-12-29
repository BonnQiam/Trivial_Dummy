#include <limbo/parsers/gdsii/stream/GdsWriter.h>

#include "util.hpp"

#define alpha_std 0.2
#define beta_std 0.077
#define alpha_line 0.2
#define beta_line 11.758
#define alpha_outlier 0.15
#define beta_outlier 0.014
#define alpha_overlay 0.2
#define beta_overlay 79154
#define x_grid_num 8
#define y_grid_num 20

struct Grid
{
    double density_metal;
    double density_fill;
};

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
    //GDSII
    GdsParser::GdsWriter gw (argv[1]);
    gw.create_lib("Dummy", 0.001, 1.0e-9);
    gw.gds_write_bgnstr();
    gw.gds_write_strname("dummies");
#endif

#if 1
    // Test: display Input
    for (int i = 0; i < Input.size(); i++)
    {
        std::cout << Input[i] << std::endl;
    }
#endif 

    /*
     * ******************************************* Insert
     */

    std::vector<std::vector<double>>    density_load;
    LoadFillObj(Input[0], density_load, 2);
    LoadFillObj(Input[1], density_load, 4);
    LoadFillObj(Input[2], density_load, 2);

    std::vector<double>                density_obj;

    for(int i = 0; i < density_load.size(); i++){
        for(int j = 0; j < density_load[i].size(); j++){
            density_obj.push_back(density_load[i][j]);
        }
    }
    density_load.clear();

    std::vector<double>                 density_fill;

    int Num_loop = 2 + 4 + 2;
    int Layer;

    //std::cout << "Density_Obj.size() = " << density_obj.size() << std::endl;

    for(int i = 0; i < Num_loop; i++){
        //std::cout << "i = " << i << std::endl;

        std::vector<std::vector<Rect<int>>> Fillable_rect, dummy;

        if(i < 2)
            Layer = 10001;
        else if(i < 6)
            Layer = 10002;
        else
            Layer = 10003;

        LoadRectangle(Input[i + 3], Fillable_rect);

        int offset = i*Num_grid;

        Layer_Rectangle_Generate(density_obj, offset, Fillable_rect, dummy, density_fill);

#if 1
        for(int j = 0; j < dummy.size(); j++){
            for(int k = 0; k < dummy[j].size(); k++){
                
                std::vector<int> vx(5), vy(5);

                vx[0] = dummy[j][k].getBL().getX();
                vy[0] = dummy[j][k].getBL().getY();

                vx[1] = dummy[j][k].getBL().getX();
                vy[1] = dummy[j][k].getTR().getY();

                vx[2] = dummy[j][k].getTR().getX();
                vy[2] = dummy[j][k].getTR().getY();

                vx[3] = dummy[j][k].getTR().getX();
                vy[3] = dummy[j][k].getBL().getY();

                vx[4] = dummy[j][k].getBL().getX();
                vy[4] = dummy[j][k].getBL().getY();

                gw.write_boundary(Layer, 0, vx, vy, false);
            }
        }
#endif
    }

#if 1
    gw.gds_write_endstr();
    gw.gds_write_endlib();
#endif

    /*
     * ******************************************* Evaluation
     */
   
    std::vector<double> Density_Metal;
    parse_No_Fill_Density(Input[11], Density_Metal);
    parse_No_Fill_Density(Input[12], Density_Metal);
    parse_No_Fill_Density(Input[13], Density_Metal);

    std::vector<Grid> Grids_1, Grids_2, Grids_3;

    for(int i = 0; i < Num_grid; i++){
        double density_fill_1 = density_fill[i] + density_fill[i + Num_grid];
        double density_fill_2 = density_fill[i + 2 * Num_grid] + density_fill[i + 3 * Num_grid] + density_fill[i + 4 * Num_grid] + density_fill[i + 5 * Num_grid];
        double density_fill_3 = density_fill[i + 6 * Num_grid] + density_fill[i + 7 * Num_grid];

        Grids_1.push_back({Density_Metal[i], density_fill_1});
        Grids_2.push_back({Density_Metal[i + Num_grid], density_fill_2});
        Grids_3.push_back({Density_Metal[i + 2 * Num_grid], density_fill_3});
    }

    // Std
    double Mean1 = 0.0, Mean2 = 0.0, Mean3 = 0.0;
    double Std1 = 0.0, Std2 = 0.0, Std3 = 0.0;

    for(int i = 0; i < Num_grid; i++){
        Mean1 += Grids_1[i].density_metal + Grids_1[i].density_fill;
        Mean2 += Grids_2[i].density_metal + Grids_2[i].density_fill;
        Mean3 += Grids_3[i].density_metal + Grids_3[i].density_fill;
    }

    Mean1 = Mean1 / Num_grid;
    Mean2 = Mean2 / Num_grid;
    Mean3 = Mean3 / Num_grid;

    for(int i = 0; i < Num_grid; i++){
        Std1 += (Grids_1[i].density_metal + Grids_1[i].density_fill - Mean1) * (Grids_1[i].density_metal + Grids_1[i].density_fill - Mean1);
        Std2 += (Grids_2[i].density_metal + Grids_2[i].density_fill - Mean2) * (Grids_2[i].density_metal + Grids_2[i].density_fill - Mean2);
        Std3 += (Grids_3[i].density_metal + Grids_3[i].density_fill - Mean3) * (Grids_3[i].density_metal + Grids_3[i].density_fill - Mean3);
    }

    Std1 = sqrt(Std1 / Num_grid);
    Std2 = sqrt(Std2 / Num_grid);
    Std3 = sqrt(Std3 / Num_grid);

    double Std_score = std::max(0.0, 1- (Std1 + Std2 + Std3) / beta_std);

    //Outlier
    double Outlier1 = 0.0, Outlier2 = 0.0, Outlier3 = 0.0;

    for(int i = 0; i < Num_grid; i++){
        Outlier1 += std::max(0.0, std::abs(Grids_1[i].density_metal + Grids_1[i].density_fill - Mean1) - 3 * Std1);
        Outlier2 += std::max(0.0, std::abs(Grids_2[i].density_metal + Grids_2[i].density_fill - Mean2) - 3 * Std2);
        Outlier3 += std::max(0.0, std::abs(Grids_3[i].density_metal + Grids_3[i].density_fill - Mean3) - 3 * Std3);
    }

    double Outlier_score = std::max(0.0, 1 - (Outlier1 + Outlier2 + Outlier3) / beta_outlier);
    
    //Overlay
    std::vector<double> Overlay_12, Overlay_23;
    parse_Overlay(Input[14], Overlay_12);
    parse_Overlay(Input[15], Overlay_23);

    double Overlay12 = 0.0, Overlay23 = 0.0;
    
    for(int i = 0; i < Num_grid; i++){
        double tmp1 = Grids_1[i].density_fill + Grids_2[i].density_fill 
                    - Overlay_12[i];
        double tmp2 = Grids_2[i].density_fill + Grids_3[i].density_fill 
                    - Overlay_23[i];

        Overlay12 += std::max(0.0, tmp1);
        Overlay23 += std::max(0.0, tmp2);
    }

    double Overlay_score = std::max(0.0, 1 - (Overlay12 + Overlay23) / beta_overlay);

    //Line
    std::vector<double> Line_Mean;
    Line_Mean.resize(x_grid_num*3);

    for(int start_index = 0; start_index < x_grid_num; start_index += y_grid_num){
        for(int i = start_index; i < y_grid_num; i++){
            Line_Mean[start_index] += Grids_1[start_index*y_grid_num + i].density_metal + Grids_1[start_index*y_grid_num + i].density_fill;
            Line_Mean[start_index + x_grid_num] += Grids_2[start_index*y_grid_num + i].density_metal + Grids_2[start_index*y_grid_num + i].density_fill;
            Line_Mean[start_index + 2*x_grid_num] += Grids_3[start_index*y_grid_num + i].density_metal + Grids_3[start_index*y_grid_num + i].density_fill;
        }
    }

    for(int i = 0; i < x_grid_num; i++){
        Line_Mean[i] = Line_Mean[i] / y_grid_num;
        Line_Mean[i + x_grid_num] = Line_Mean[i + x_grid_num] / y_grid_num;
        Line_Mean[i + 2*x_grid_num] = Line_Mean[i + 2*x_grid_num] / y_grid_num;
    }

    double Line_Sum1 = 0.0, Line_Sum2 = 0.0, Line_Sum3 = 0.0;

    for(int start_index = 0; start_index < x_grid_num; start_index += y_grid_num){
        for(int i = start_index; i < y_grid_num; i++){
            Line_Sum1 += std::abs(Grids_1[start_index*y_grid_num + i].density_metal + Grids_1[start_index*y_grid_num + i].density_fill - Line_Mean[start_index]);
            Line_Sum2 += std::abs(Grids_2[start_index*y_grid_num + i].density_metal + Grids_2[start_index*y_grid_num + i].density_fill - Line_Mean[start_index + x_grid_num]);
            Line_Sum3 += std::abs(Grids_3[start_index*y_grid_num + i].density_metal + Grids_3[start_index*y_grid_num + i].density_fill - Line_Mean[start_index + 2*x_grid_num]);
        }
    }

    double Line_score = std::max(0.0, 1 - (Line_Sum1 + Line_Sum2 + Line_Sum3) / beta_line);

    // Report

    std::cout << "Std_score = " << Std_score << std::endl;
    std::cout << "Outlier_score = " << Outlier_score << std::endl;
    std::cout << "Overlay_score = " << Overlay_score << std::endl;
    std::cout << "Line_score = " << Line_score << std::endl;
    std::cout << "Overall_score = " << (alpha_std * Std_score + alpha_outlier * Outlier_score + alpha_overlay * Overlay_score + alpha_line * Line_score) << std::endl;
    

    return 0;
}