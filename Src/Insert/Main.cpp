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

#if 1
    //GDSII
    GdsParser::GdsWriter gw (argv[1]);
    gw.create_lib("Dummy", 0.001, 1.0e-9);
    gw.gds_write_bgnstr();
    gw.gds_write_strname("dummies");
#endif

    std::vector<std::vector<std::string>>   layer_input;
    std::vector<int>                        layer;
    std::vector<std::string>                overlay_input, fill_output;
    int                                     x_num, y_num, grid_num;
    std::vector<Final_Grid>                 Grids;

    // Read filename from argv[2]
    std::string filename = argv[2];

    std::cout << "Reading file: " << filename << '\n';

    std::ifstream file(filename);
    std::vector<std::string> input;// load the content of the 'file'

#if 0
    if (!file) {
        std::cerr << "Could not open file: " << filename_1 << '\n';
        exit(1);
    }
#endif

    std::string line;
    
    std::getline(file, line);
    x_num = std::stoi(line);
    std::cout << "x_num = " << x_num << std::endl;

    std::getline(file, line);
    y_num = std::stoi(line);
    std::cout << "y_num = " << y_num << std::endl;

    grid_num = x_num * y_num;

    while (std::getline(file, line))
    {
        if(line == "<Layer>"){
            std::getline(file, line);
            layer.push_back(std::stoi(line));

            layer_input.push_back(std::vector<std::string>());

            while(std::getline(file, line)){
                if(line == "</Layer>"){
                    break;
                }

                layer_input.back().push_back(line);
            }
        }
        else if(line == "<Overlay>"){
            while(std::getline(file, line)){
                if(line == "</Overlay>"){
                    break;
                }

                overlay_input.push_back(line);
            }
        }
        else if(line == "<Fill>"){
            while(std::getline(file, line)){
                if(line == "</Fill>"){
                    break;
                }

                fill_output.push_back(line);
            }
        }

        if(file.eof()){
            break;
        }
    }

    file.close();

    int layer_num = layer_input.size();

#if 1
    for(int i = 0; i < layer.size(); i++){
        std::cout << "Layer " << i << ": " << layer[i] << std::endl;
    }

    for(int i = 0; i < layer_num; i++){
        std::cout << "Layer " << i << std::endl;
        for(int j = 0; j < layer_input[i].size(); j++){
            std::cout << layer_input[i][j] << std::endl;
        }
    }

    std::cout << "Overlay" << std::endl;
    for(int i=0; i < overlay_input.size(); i++){
        std::cout << overlay_input[i] << std::endl;
    }
#endif

#if 1
    /*
     * ************************************************************************ Insert
     */
    for(int i = 0; i < layer_num; i++){
    //for(int i = 1; i < 2; i++){
        std::cout << "Layer " << i << std::endl;

        std::vector<std::ifstream> loading_files;
        
        for(int j = 0; j < layer_input[i].size(); j++){
            loading_files.push_back(std::ifstream(layer_input[i][j]));
        }
        // Grid Insersion
        int grid_count = 1;
        std::vector<int> load_flags(layer_input[i].size(), 0);
        while(1){
            Grid grid;

#if 1
            if(grid_count > grid_num){
                break;
            }
#endif

            load_flags[0] = parse_No_Fill_Density(loading_files[0], grid, layer[i]);
            load_flags[1] = LoadFillObj(loading_files[1], grid);

            for(int j = 2; j < loading_files.size(); j++){
                load_flags[j] = LoadRectangle(loading_files[j], grid);
            }

            if(load_flags == std::vector<int>(loading_files.size(), 1)){
                break;
            }
            else if(load_flags == std::vector<int>(loading_files.size(), 0)){
                Layer_Rectangle_Generate(grid);
                grid.density_report();
                //grid.Fill_rect_output(fill_output[i]);
                std::cout << "Grid " << grid_count << " is inserted" << std::endl;

#if 1
                // output to GDSII
                for(int j = 0; j < grid.Fill_rect.size(); j++){
                    for(int k = 0; k < grid.Fill_rect[j].size(); k++){
                        std::vector<int> vx(5), vy(5);

                        vx[0] = grid.Fill_rect[j][k].getBL().getX();
                        vy[0] = grid.Fill_rect[j][k].getBL().getY();

                        vx[1] = grid.Fill_rect[j][k].getBL().getX();
                        vy[1] = grid.Fill_rect[j][k].getTR().getY();

                        vx[2] = grid.Fill_rect[j][k].getTR().getX();
                        vy[2] = grid.Fill_rect[j][k].getTR().getY();

                        vx[3] = grid.Fill_rect[j][k].getTR().getX();
                        vy[3] = grid.Fill_rect[j][k].getBL().getY();

                        vx[4] = grid.Fill_rect[j][k].getBL().getX();
                        vy[4] = grid.Fill_rect[j][k].getBL().getY();

                        gw.write_boundary(layer[i], 0, vx, vy, false);
                    }
                }
#endif
                Final_Grid final_grid;

                final_grid.density_metal = grid.density_metal;
                final_grid.layer = layer[i];
                final_grid.density_fill = grid.density_fill;
                final_grid.Sum();

                Grids.push_back(final_grid);

            }
            else{
                std::cout << "Something wroing in the loading files" << std::endl;
                for(int j = 0; j < loading_files.size(); j++){
                    std::cout << "Load flag " << j << ": " << load_flags[j] << std::endl;
                }
            }
            
            grid_count++;
        }

        for(int j = 0; j < loading_files.size(); j++){
            loading_files[j].close();
        }
    }
#endif

#if 1
    gw.gds_write_endstr();
    gw.gds_write_endlib();
#endif

#if 1
    /*
     * ******************************************* Evaluation
     */

    // Std
    double Mean1 = 0.0, Mean2 = 0.0, Mean3 = 0.0;
    double Std1 = 0.0, Std2 = 0.0, Std3 = 0.0;

    for(int i = 0; i < grid_num; i++){
        Mean1 += Grids[i].density_sum;
        Mean2 += Grids[i+grid_num].density_sum;
        Mean3 += Grids[i+2*grid_num].density_sum;
    }

    Mean1 = Mean1 / grid_num;
    Mean2 = Mean2 / grid_num;
    Mean3 = Mean3 / grid_num;

    for(int i = 0; i < grid_num; i++){
        Std1 += (Grids[i].density_sum - Mean1) * (Grids[i].density_sum - Mean1);
        Std2 += (Grids[i+grid_num].density_sum - Mean2) * (Grids[i+grid_num].density_sum - Mean2);
        Std3 += (Grids[i+2*grid_num].density_sum - Mean3) * (Grids[i+2*grid_num].density_sum - Mean3);
    }

    Std1 = sqrt(Std1 / grid_num);
    Std2 = sqrt(Std2 / grid_num);
    Std3 = sqrt(Std3 / grid_num);

    double Std_score = std::max(0.0, 1- (Std1 + Std2 + Std3) / beta_std);

    //Outlier
    double Outlier1 = 0.0, Outlier2 = 0.0, Outlier3 = 0.0;

    for(int i = 0; i < grid_num; i++){
        Outlier1 += std::max(0.0, std::abs(Grids[i].density_sum - Mean1) - 3 * Std1);
        Outlier2 += std::max(0.0, std::abs(Grids[i+grid_num].density_sum - Mean2) - 3 * Std2);
        Outlier3 += std::max(0.0, std::abs(Grids[i+2*grid_num].density_sum - Mean3) - 3 * Std3);
    }

    double Outlier_score = std::max(0.0, 1 - (Outlier1 + Outlier2 + Outlier3) / beta_outlier);
    
    //Overlay
    std::vector<double> Overlay_12, Overlay_23;
    parse_Overlay(overlay_input[0], Overlay_12);
    parse_Overlay(overlay_input[1], Overlay_23);

    double Overlay12 = 0.0, Overlay23 = 0.0;
    
    for(int i = 0; i < grid_num; i++){
        double tmp1 = Grids[i].density_sum - Overlay_12[i];
        double tmp2 = Grids[i+grid_num].density_sum - Overlay_23[i];

        Overlay12 += std::max(0.0, tmp1);
        Overlay23 += std::max(0.0, tmp2);
    }

    double Overlay_score = std::max(0.0, 1 - (Overlay12 + Overlay23) / beta_overlay);

    //Line
    std::vector<double> Line_Mean;
    Line_Mean.resize(x_grid_num*3);

    for(int start_index = 0; start_index < x_grid_num; start_index += y_grid_num){
        for(int i = start_index; i < y_grid_num; i++){
            Line_Mean[start_index] += Grids[start_index*y_grid_num + i].density_sum;

            Line_Mean[start_index + x_grid_num] += Grids[start_index*y_grid_num + i + grid_num].density_sum;

            Line_Mean[start_index + 2*x_grid_num] += Grids[start_index*y_grid_num + i + 2*grid_num].density_sum;
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
            Line_Sum1 += std::abs(Grids[start_index*y_grid_num + i].density_sum - Line_Mean[start_index]);

            Line_Sum2 += std::abs(Grids[start_index*y_grid_num + i + grid_num].density_sum - Line_Mean[start_index + x_grid_num]);

            Line_Sum3 += std::abs(Grids[start_index*y_grid_num + i + 2*grid_num].density_sum - Line_Mean[start_index + 2*x_grid_num]);
        }
    }

    double Line_score = std::max(0.0, 1 - (Line_Sum1 + Line_Sum2 + Line_Sum3) / beta_line);

    // Report

    std::cout << "Std_score = " << Std_score << std::endl;
    std::cout << "Outlier_score = " << Outlier_score << std::endl;
    std::cout << "Overlay_score = " << Overlay_score << std::endl;
    std::cout << "Line_score = " << Line_score << std::endl;
    std::cout << "Overall_score = " << (alpha_std * Std_score + alpha_outlier * Outlier_score + alpha_overlay * Overlay_score + alpha_line * Line_score) << std::endl;
#endif

    return 0;
}