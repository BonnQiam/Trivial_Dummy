#include "util.hpp"
#include "opt.hpp"

#include <time.h>
#include <random>

int main(int argc, char *argv[])
{
    /*
     * ******************************************* Get the Input file
     */
    if (argc < 1)
    {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    // Read filename from argv[1]
    std::string filename = argv[1];

    std::cout << "Reading file: " << filename << '\n';

    std::ifstream file(filename);

    std::vector<std::string> Input;

    if (!file)
    {
        std::cerr << "Could not open file: " << filename << '\n';
        exit(1);
    }

    std::string line;
    while (std::getline(file, line))
    {
        Input.push_back(line);
    }

    // Test: display Input
    for (int i = 0; i < Input.size(); i++)
    {
        std::cout << Input[i] << std::endl;
    }

    /*
     * ****************************************** Read the Grid info
     */
    std::vector<Grid_Coor> Grid_No_Fill;
    std::vector<double> Metal;
    std::vector<double> Dummy_Dummy_M1, Dummy_Wire_M1,
        Dummy_Dummy_Dummy_M2, Dummy_Dummy_Wire_M2, Wire_Dummy_Dummy_M2, Wire_Dummy_Wire_M2,
        Dummy_Dummy_M3, Wire_Dummy_M3;

    std::vector<double> Overlay;

    parse_No_Fill_Density(Input[0], Grid_No_Fill, Metal); // Layer 1 in 'Benchmark' filefolder
    parse_No_Fill_Density(Input[1], Grid_No_Fill, Metal); // Layer 2  in 'Benchmark' filefolder
    parse_No_Fill_Density(Input[2], Grid_No_Fill, Metal); // Layer 3  in 'Benchmark' filefolder

    parse_Fillable_Density(Input[3], Dummy_Dummy_M1);
    parse_Fillable_Density(Input[4], Dummy_Wire_M1);

    parse_Fillable_Density(Input[5], Dummy_Dummy_Dummy_M2);
    parse_Fillable_Density(Input[6], Dummy_Dummy_Wire_M2);
    parse_Fillable_Density(Input[7], Wire_Dummy_Dummy_M2);
    parse_Fillable_Density(Input[8], Wire_Dummy_Wire_M2);

    for (int i = 0; i < Dummy_Dummy_Dummy_M2.size(); i++)
    {
        Dummy_Dummy_M3.push_back(Dummy_Dummy_Dummy_M2[i] + Wire_Dummy_Dummy_M2[i]);
    }

    parse_Fillable_Density(Input[9], Wire_Dummy_M3);

    Overlay = Dummy_Dummy_M1;
    for (int i = 0; i < Dummy_Dummy_Dummy_M2.size(); i++)
    {
        Overlay.push_back(Dummy_Dummy_Dummy_M2[i] + Wire_Dummy_Dummy_M2[i]);
    }

    int Num_grid = Grid_No_Fill.size() / 3;

    /*
     * ******************************************* Opt problem definition and solving
     */
    typedef CPPAD_TESTVECTOR(double) Dvector;
    bool ok = true;

    srand(time(NULL));

    int length = 2 * Num_grid + 4 * Num_grid + 2 * Num_grid;

    Dvector xi(length), xl(length), xu(length);

    int index = 0, offset = 0;
    while (index < length)
    {
        if (index < Num_grid)
        {
            offset = 0;
            xl[index] = 0.0;
            xl[index] = min_density;
            xu[index] = Dummy_Dummy_M1[index - offset];
        }
        else if (index >= Num_grid && index < 2 * Num_grid)
        {
            offset = Num_grid;
            xl[index] = 0.0;
            xl[index] = min_density;
            xu[index] = Dummy_Wire_M1[index - offset];

        }
        else if (index >= 2 * Num_grid && index < 3 * Num_grid)
        {
            offset = 2 * Num_grid;
            xl[index] = 0.0;
            xl[index] = min_density;
            xu[index] = Dummy_Dummy_Dummy_M2[index - offset];
        }
        else if (index >= 3 * Num_grid && index < 4 * Num_grid)
        {
            offset = 3 * Num_grid;
            xl[index] = 0.0;
            xl[index] = min_density;
            xu[index] = Dummy_Dummy_Wire_M2[index - offset];
        }
        else if (index >= 4 * Num_grid && index < 5 * Num_grid)
        {
            offset = 4 * Num_grid;
            xl[index] = 0.0;
            xl[index] = min_density;
            xu[index] = Wire_Dummy_Dummy_M2[index - offset];
        }
        else if (index >= 5 * Num_grid && index < 6 * Num_grid)
        {
            offset = 5 * Num_grid;
            xl[index] = 0.0;
            xl[index] = min_density;
            xu[index] = Wire_Dummy_Wire_M2[index - offset];
        }
        else if (index >= 6 * Num_grid && index < 7 * Num_grid)
        {
            offset = 6 * Num_grid;
            xl[index] = 0.0;
            xl[index] = min_density;
            xu[index] = Dummy_Dummy_M3[index - offset];
        }
        else if (index >= 7 * Num_grid && index < 8 * Num_grid)
        {
            offset = 7 * Num_grid;
            //xl[index] = 0.0;
            xl[index] = min_density;
            xu[index] = Wire_Dummy_M3[index - offset];
        }
        else if (index >= 8 * Num_grid)
            break;

        //random generte xi value between xl and xu
        //xi[index] = xl[index] + (xu[index] - xl[index]) * rand() / (RAND_MAX + 1.0);
        //xi[index] = xu[index];

        if(xu[index] < xl[index])
            xu[index] = xl[index];

        index++;
    }

    Dvector gl(1), gu(1);
    
    // s
    //gl[0] = 0.;
    //gu[0] = 1e-5;

    // b
    //gl[0] = 0.;
    //gu[0] = 0.5;

    // m
    gl[0] = 0.;
    gu[0] = 0.7;

    FG_eval fg_eval(Metal, Overlay, Num_grid);

    std::cout << "Initializations done" << std::endl;

    std::string options;
    // options += "String  sb           yes\n";
    options += "String  jacobian_approximation exact\n";
    options += "String  hessian_approximation  limited-memory\n";
    options += "String  linear_solver  ma97\n";
    options += "Integer max_iter     300\n";
    options += "Numeric tol          1e-6\n";
    //options += "String  derivative_test second-order\n";
    //options += "Numeric point_perturbation_radius  0.\n";
    //options += "Sparse  true forward\n";
    //options += "Sparse  true reverse\n";

    CppAD::ipopt::solve_result<Dvector> solution;
    CppAD::ipopt::solve<Dvector, FG_eval>(
        options, xi, xl, xu, gl, gu, fg_eval, solution);

    ok &= solution.status == CppAD::ipopt::solve_result<Dvector>::success;


    /*
     * ******************************************* Feasibility check
     */

#if 0
    std::cout << "=====================================================" << std::endl;

    for (int i = 0; i < length; i++)
    {
        if (solution.x[i] < 0.0)
        {
            std::cout << "Infeasible solution < xl" << std::endl;
        }
        else if (solution.x[i] > xu[i])
        {
            std::cout << "Infeasible solution > xu" << std::endl;
        }
        else{
            double fill_percent = solution.x[i] / xu[i];
            std::cout << "Fill percent= " << solution.x[i] << " / " << xu[i] << " = " << fill_percent << " in [" << xl[i] << ", " << xu[i] << "]" << std::endl;
        }

        if(solution.x[i] < min_density)
        {
            solution.x[i] = 0.0;
        }
    }

    std::cout << "=====================================================" << std::endl;
#endif
    /*
     * ******************************************* Metric - Overlay
     */
    double Overlay_12, Overlay_23 = 0.0;
    for (int i = 0; i < Num_grid; i++)
    {
        Overlay_12 = (solution.x[i] + solution.x[i + 2 * Num_grid] + solution.x[i + 3 * Num_grid] - Overlay[i]) > 0 ? (solution.x[i] + solution.x[i + 2 * Num_grid] + solution.x[i + 3 * Num_grid] - Overlay[i]) : 0;
        Overlay_12 += solution.x[i + Num_grid] + solution.x[i + 4 * Num_grid] + solution.x[i + 5 * Num_grid];

        Overlay_23 = (solution.x[i + 2 * Num_grid] + solution.x[i + 4 * Num_grid] + solution.x[i + 6 * Num_grid] - Overlay[i + Num_grid]) > 0 ? (solution.x[i + 2 * Num_grid] + solution.x[i + 4 * Num_grid] + solution.x[i + 6 * Num_grid] - Overlay[i + Num_grid]) : 0;
        Overlay_23 += solution.x[i + 3 * Num_grid] + solution.x[i + 5 * Num_grid] + solution.x[i + 7 * Num_grid];

        Overlay_12 = Overlay_12 * Grid_size * Grid_size;
        Overlay_23 = Overlay_23 * Grid_size * Grid_size;
    }

    double Overlay_metric = (1 - (Overlay_12 + Overlay_23) * Grid_size * Grid_size / beta_overlay) > 0 ? (1 - (Overlay_12 + Overlay_23) * Grid_size * Grid_size / beta_overlay) : 0;
    std::cout << "Overlay_metric: " << Overlay_metric << std::endl;

    /*
     * ******************************************* Metric - Std
     */

    double Sum1, Sum2, Sum3 = 0.0;

    for (int i = 0; i < Num_grid; i++)
    {
        Sum1 += solution.x[i] + solution.x[i + Num_grid] + Metal[i];
        Sum2 += solution.x[i + 2 * Num_grid] + solution.x[i + 3 * Num_grid] + solution.x[i + 4 * Num_grid] + solution.x[i + 5 * Num_grid] + Metal[i + Num_grid];
        Sum3 += solution.x[i + 6 * Num_grid] + solution.x[i + 7 * Num_grid] + Metal[i + 2 * Num_grid];
    }

    double Mean1 = Sum1 / Num_grid;
    double Mean2 = Sum2 / Num_grid;
    double Mean3 = Sum3 / Num_grid;

    double Std1, Std2, Std3 = 0.0;
    for (int i = 0; i < Num_grid; i++)
    {
        double Delta1 = solution.x[i] + solution.x[i + Num_grid] + Metal[i] - Mean1;
        double Delta2 = solution.x[i + 2 * Num_grid] + solution.x[i + 3 * Num_grid] + solution.x[i + 4 * Num_grid] + solution.x[i + 5 * Num_grid] + Metal[i + Num_grid] - Mean2;
        double Delta3 = solution.x[i + 6 * Num_grid] + solution.x[i + 7 * Num_grid] + Metal[i + 2 * Num_grid] - Mean3;

        Std1 += Delta1 * Delta1;
        Std2 += Delta2 * Delta2;
        Std3 += Delta3 * Delta3;
    }

    Std1 = sqrt(Std1 / Num_grid);
    Std2 = sqrt(Std2 / Num_grid);
    Std3 = sqrt(Std3 / Num_grid);

    double Std_metric = (1 - (Std1 + Std2 + Std3) / beta_std) > 0 ? (1 - (Std1 + Std2 + Std3) / beta_std) : 0;
    std::cout << "Std1 : " << Std1 << std::endl;
    std::cout << "Std2 : " << Std2 << std::endl;
    std::cout << "Std3 : " << Std3 << std::endl;
    std::cout << "Std_metric: " << Std_metric << std::endl;

    /*
     * ******************************************* Metric - Outlier
     */
    double Outlier1, Outlier2, Outlier3 = 0.0;

    for (int i = 0; i < Num_grid; i++)
    {
        double Grid1 = solution.x[i] + solution.x[i + Num_grid] + Metal[i];
        double Grid2 = solution.x[i + 2 * Num_grid] + solution.x[i + 3 * Num_grid] + solution.x[i + 4 * Num_grid] + solution.x[i + 5 * Num_grid] + Metal[i + Num_grid];
        double Grid3 = solution.x[i + 6 * Num_grid] + solution.x[i + 7 * Num_grid] + Metal[i + 2 * Num_grid];

        Outlier1 += (fabs(Grid1 - Mean1) - 3 * Std1) > 0 ? (fabs(Grid1 - Mean1) - 3 * Std1) : 0;
        Outlier2 += (fabs(Grid2 - Mean2) - 3 * Std2) > 0 ? (fabs(Grid2 - Mean2) - 3 * Std2) : 0;
        Outlier3 += (fabs(Grid3 - Mean3) - 3 * Std3) > 0 ? (fabs(Grid3 - Mean3) - 3 * Std3) : 0;
    }

    double Outlier_metric = (1 - (Outlier1 + Outlier2 + Outlier3) / beta_outlier) > 0 ? (1 - (Outlier1 + Outlier2 + Outlier3) / beta_outlier) : 0;
    std::cout << "Outlier_metric: " << Outlier_metric << std::endl;

    /*
     * ******************************************* Metric - Line hotspot
     */
    double Line_Sum1, Line_Sum2, Line_Sum3 = 0.0;

    for (int start_index = 0; start_index <= (x_grid_num - 1) * y_grid_num; start_index += y_grid_num)
    {
        double Line_Mean1, Line_Mean2, Line_Mean3 = 0.0;

        for (int i = start_index; i < y_grid_num; i++)
        {
            Line_Mean1 += solution.x[start_index + i] + solution.x[start_index + i + Num_grid] + Metal[start_index + i];
            Line_Mean2 += solution.x[start_index + i + 2 * Num_grid] + solution.x[start_index + i + 3 * Num_grid] + solution.x[start_index + i + 4 * Num_grid] + solution.x[start_index + i + 5 * Num_grid] + Metal[start_index + i + Num_grid];
            Line_Mean3 += solution.x[start_index + i + 6 * Num_grid] + solution.x[start_index + i + 7 * Num_grid] + Metal[start_index + i + 2 * Num_grid];
        }

        Line_Mean1 = Line_Mean1 / y_grid_num;
        Line_Mean2 = Line_Mean2 / y_grid_num;
        Line_Mean3 = Line_Mean3 / y_grid_num;

        for (int i = start_index; i < y_grid_num; i++)
        {
            Line_Sum1 += fabs(solution.x[start_index + i] + solution.x[start_index + i + Num_grid] + Metal[start_index + i] - Line_Mean1);
            Line_Sum2 += fabs(solution.x[start_index + i + 2 * Num_grid] + solution.x[start_index + i + 3 * Num_grid] + solution.x[start_index + i + 4 * Num_grid] + solution.x[start_index + i + 5 * Num_grid] + Metal[start_index + i + Num_grid] - Line_Mean2);
            Line_Sum3 += fabs(solution.x[start_index + i + 6 * Num_grid] + solution.x[start_index + i + 7 * Num_grid] + Metal[start_index + i + 2 * Num_grid] - Line_Mean3);
        }
    }

    double Line_metric = (1 - (Line_Sum1 + Line_Sum2 + Line_Sum3) / beta_line) > 0 ? (1 - (Line_Sum1 + Line_Sum2 + Line_Sum3) / beta_line) : 0;
    std::cout << "Line_metric: " << Line_metric << std::endl;
    
    double Total_score = alpha_std*Std_metric + alpha_outlier*Outlier_metric + alpha_overlay*Overlay_metric + alpha_line*Line_metric;

    std::cout << "Total_score: " << Total_score << std::endl;

#if 0
    std::cout << "Corresponding Cost: " << 
        //(Line_Sum1 + Line_Sum2 + Line_Sum3)/beta_line*alpha_line 
        (Outlier1 + Outlier2 + Outlier3) / beta_outlier * alpha_outlier
        << std::endl;
#endif

    /*
     * ******************************************* Output
     */

    std::ofstream Output1("Grid_Opt_Layer_M1.txt");
    std::ofstream Output2("Grid_Opt_Layer_M2.txt");
    std::ofstream Output3("Grid_Opt_Layer_M3.txt");

    for(int i=0; i<Num_grid; i++){

#if 1
        if(solution.x[i] < 1e-6)
            Output1 << "0.0,";
        else
            Output1 << solution.x[i] << ",";

        if(solution.x[i + Num_grid] < 1e-6)
            Output1 << "0.0" << std::endl;
        else
            Output1 << solution.x[i + Num_grid] << std::endl;



        if(solution.x[i + 2*Num_grid] < 1e-6)
            Output2 << "0.0,";
        else
            Output2 << solution.x[i + 2*Num_grid] << ",";

        if(solution.x[i + 3*Num_grid] < 1e-6)
            Output2 << "0.0,";
        else
            Output2 << solution.x[i + 3*Num_grid] << ",";

        if(solution.x[i + 4*Num_grid] < 1e-6)
            Output2 << "0.0,";
        else
            Output2 << solution.x[i + 4*Num_grid] << ",";

        if(solution.x[i + 5*Num_grid] < 1e-6)
            Output2 << "0.0" << std::endl;
        else
            Output2 << solution.x[i + 5*Num_grid] << std::endl;

        
        if(solution.x[i + 6*Num_grid] < 1e-6)
            Output3 << "0.0,";
        else
            Output3 << solution.x[i + 6*Num_grid] << ",";

        if(solution.x[i + 7*Num_grid] < 1e-6)
            Output3 << "0.0" << std::endl;
        else
            Output3 << solution.x[i + 7*Num_grid] << std::endl;

#endif

#if 0
        Output1 << solution.x[i] << " " << solution.x[i + Num_grid] << std::endl;
        
        Output2 << solution.x[i + 2*Num_grid] << " " << solution.x[i + 3*Num_grid] << " " << solution.x[i + 4*Num_grid] << " " << solution.x[i + 5*Num_grid] << std::endl;

        Output3 << solution.x[i + 6*Num_grid] << " " << solution.x[i + 7*Num_grid] << std::endl;
#endif
    }

    Output1.close();
    Output2.close();
    Output3.close();

    return 0;
}