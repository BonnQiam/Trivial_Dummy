#include <vector>
#include <cppad/ipopt/solve.hpp>

#include <iostream>

//! Attention: solve_callback.hpp in /usr/local/include/cppad/ipopt has been modified by QIAMKING
//? Original solve_callback.hpp is <cppad/ipopt/solve_callback.hpp>

#define Grid_size 20

#if 0
// factor 4 s
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
#endif

#if 0
// factor 2 b
#define alpha_std 0.2
#define beta_std 0.517
#define alpha_line 0.2
#define beta_line 3578
#define alpha_outlier 0.15
#define beta_outlier 22.801
#define alpha_overlay 0.2
#define beta_overlay 6111303
#define x_grid_num 117
#define y_grid_num 117
#endif

#if 1
// factor 2 m
#define alpha_std 0.2
#define beta_std 0.53
#define alpha_line 0.2
#define beta_line 6052
#define alpha_outlier 0.15
#define beta_outlier 27.56
#define alpha_overlay 0.2
#define beta_overlay 10276835
#define x_grid_num 325
#define y_grid_num 53
#endif

using CppAD::AD;

// Cost 函数定义
class FG_eval
{
private:
    int Num_grid;

    std::vector<double> Density_Metal;
    std::vector<double> Overlay_Metal;
public:
    typedef CPPAD_TESTVECTOR(AD<double>) ADvector;

    FG_eval(std::vector<double> density_Metal, std::vector<double> overlay_Metal, int num_grid)
    {
        this->Density_Metal = density_Metal;
        this->Overlay_Metal = overlay_Metal;
        this->Num_grid = num_grid;
    };

    void operator()(ADvector &fg, const ADvector &x)
    {
        AD<double> Zero = 0.0;

        AD<double> Std1, Std2, Std3 = 0.0;
        AD<double> Mean1, Mean2, Mean3 = 0.0;

        std::vector<AD<double>> Line_Mean;
        AD<double> Line_Sum1, Line_Sum2, Line_Sum3 = 0.0;

        AD<double> Outlier1, Outlier2, Outlier3 = 0.0;

        AD<double> Overlay12, Overlay23 = 0.0;
        
        std::vector<AD<double>> Grid;

        Grid.resize(Num_grid * 3);
        Line_Mean.resize(x_grid_num * 3);
        /*
         * ******************************************* Metric - Std
         */
        for (int i = 0; i < Num_grid; i++)
        {
            Grid[i] = x[i] + x[i + Num_grid] + Density_Metal[i];
            Grid[i + Num_grid] = x[i + 2 * Num_grid] + x[i + 3 * Num_grid] + x[i + 4 * Num_grid] + x[i + 5 * Num_grid] + Density_Metal[i + Num_grid];
            Grid[i + 2 * Num_grid] = x[i + 6 * Num_grid] + x[i + 7 * Num_grid] + Density_Metal[i + 2 * Num_grid];
        }

        for (int i = 0; i < Num_grid; i++)
        {
            Mean1 += Grid[i];
            Mean2 += Grid[i + Num_grid];
            Mean3 += Grid[i + 2 * Num_grid];
        }

        Mean1 = Mean1 / Num_grid;
        Mean2 = Mean2 / Num_grid;
        Mean3 = Mean3 / Num_grid;

        for (int i = 0; i < Num_grid; i++)
        {
            Std1 += (Grid[i] - Mean1) * (Grid[i] - Mean1);
            Std2 += (Grid[i + Num_grid] - Mean2) * (Grid[i + Num_grid] - Mean2);
            Std3 += (Grid[i + 2 * Num_grid] - Mean3) * (Grid[i + 2 * Num_grid] - Mean3);
        }

        Std1 = sqrt(Std1 / Num_grid);
        Std2 = sqrt(Std2 / Num_grid);
        Std3 = sqrt(Std3 / Num_grid);


        /*
         * ******************************************* Metric - Line hotspot
         */
        for (int start_index = 0; start_index < x_grid_num; start_index++){
            for (int i = start_index; i < y_grid_num; i++){
                Line_Mean[start_index] += Grid[start_index*y_grid_num + i];
                Line_Mean[start_index + x_grid_num] += Grid[start_index*y_grid_num + i + Num_grid];
                Line_Mean[start_index + 2 * x_grid_num] += Grid[start_index*y_grid_num + i + 2 * Num_grid];
            }
        }

        for(int i = 0; i < x_grid_num; i++){
            Line_Mean[i] = Line_Mean[i] / y_grid_num;
            Line_Mean[i + x_grid_num] = Line_Mean[i + x_grid_num] / y_grid_num;
            Line_Mean[i + 2 * x_grid_num] = Line_Mean[i + 2 * x_grid_num] / y_grid_num;
        }

        for (int start_index = 0; start_index < x_grid_num; start_index++){
            for (int i = start_index; i < y_grid_num; i++){
                Line_Sum1 += fabs(Grid[start_index*y_grid_num + i] - Line_Mean[start_index]);
                Line_Sum2 += fabs(Grid[start_index*y_grid_num + i + Num_grid] - Line_Mean[start_index + x_grid_num]);
                Line_Sum3 += fabs(Grid[start_index*y_grid_num + i + 2 * Num_grid] - Line_Mean[start_index + 2 * x_grid_num]);
            }
        }

        /*
         * ******************************************* Metric - Outlier
         */
        for(int i=0; i < Num_grid; i++){
            //Outlier1 += max_func(0., fabs(Grid[i] - Mean1)-3*Std1);
            //Outlier2 += max_func(0., fabs(Grid[i + Num_grid] - Mean2)-3*Std2);
            //Outlier3 += max_func(0., fabs(Grid[i + 2*Num_grid] - Mean3)-3*Std3);

            Outlier1 += CppAD::CondExpGe(
                Zero, fabs(Grid[i] - Mean1) - 3 * Std1, 
                Zero, fabs(Grid[i] - Mean1) - 3 * Std1);
            
            Outlier2 += CppAD::CondExpGe(
                Zero, fabs(Grid[i + Num_grid] - Mean2) - 3 * Std2, 
                Zero, fabs(Grid[i + Num_grid] - Mean2) - 3 * Std2);
            
            Outlier3 += CppAD::CondExpGe(
                Zero, fabs(Grid[i + 2 * Num_grid] - Mean3) - 3 * Std3, 
                Zero, fabs(Grid[i + 2 * Num_grid] - Mean3) - 3 * Std3);
        }

        /*
         * ******************************************* Metric - Overlay
         */

        for (int i = 0; i < Num_grid; i++)
        {
            AD<double> tmp1 = x[i] + x[i + 2 * Num_grid] + x[i + 3 * Num_grid] - Overlay_Metal[i];
            AD<double> tmp2 = x[i + 4 * Num_grid] + x[i + 5 * Num_grid] + x[i + Num_grid] - Overlay_Metal[i + Num_grid];

            Overlay12 += CppAD::CondExpGe(Zero, tmp1, Zero, tmp1) + x[i + 4 * Num_grid] + x[i + 5 * Num_grid] + x[i + Num_grid];

            Overlay23 += CppAD::CondExpGe(Zero, tmp2, Zero, tmp2) + x[i + 3 * Num_grid] + x[i + 5 * Num_grid] + x[i + 7 * Num_grid];
        }

        Overlay12 = Overlay12 * Grid_size * Grid_size;
        Overlay23 = Overlay23 * Grid_size * Grid_size;

        /*
         * ******************************************* Objective function with constraints
         */

        // s
        //fg[0] = (Overlay12 + Overlay23) / beta_overlay;
        //fg[1] = (Std1 + Std2 + Std3) / beta_std;

        // b
        fg[0] = (Overlay12 + Overlay23) / beta_overlay + (Outlier1 + Outlier2 + Outlier3) / beta_outlier;
        fg[1] = (Std1 + Std2 + Std3) / beta_std;

        //fg[0] = 1.0;
        //fg[0] = (Std1 + Std2 + Std3) / beta_std;
        //fg[0] = (Line_Sum1 + Line_Sum2 + Line_Sum3) / beta_line;
        //fg[0] = (Outlier1 + Outlier2 + Outlier3) / beta_outlier;
        
        //std::cout << "Objective: " << CppAD::Value(CppAD::Var2Par(fg[0])) << std::endl;
    };
};