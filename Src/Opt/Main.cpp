#include "util.hpp"
#include "opt.hpp"

int main(int argc, char* argv[]) {
    if (argc < 1) {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    // Read filename from argv[1]
    std::string filename = argv[1];
    std::ifstream file(filename);

    std::vector<std::string> Input;

    if (!file) {
        std::cerr << "Could not open file: " << filename << '\n';
        exit(1);
    }

    std::string line;
    while (std::getline(file, line)) {
        Input.push_back(line);
    }

    //Test: display Input
    for (int i = 0; i < Input.size(); i++) {
        std::cout << Input[i] << std::endl;
    }
    
    /**********************************
     * 
     * Read sM1.txt (in 'Benchmark' filefolder), grid.txt (in 'Grid' filefolder), overlay.txt (in 'Overlay' filefolder)
     * 
     * *******************************/

    std::vector<Grid_Coor>  Grid_No_Fill;
    std::vector<double>     Density_Metal;
    parse_No_Fill_Density(Input[0], Grid_No_Fill, Density_Metal);
    parse_No_Fill_Density(Input[1], Grid_No_Fill, Density_Metal);
    parse_No_Fill_Density(Input[2], Grid_No_Fill, Density_Metal);

    std::vector<double> Density_Fillable;
    parse_Fillable_Density(Input[3], Density_Fillable);
    parse_Fillable_Density(Input[4], Density_Fillable);
    parse_Fillable_Density(Input[5], Density_Fillable);

    std::vector<double> Overlay;
    parse_Overlay(Input[6], Overlay);
    parse_Overlay(Input[7], Overlay);

    int length = Grid_No_Fill.size();
    int Num_grid = Grid_No_Fill.size()/3;

    /**********************************
     * 
     * Solving
     * 
     * *******************************/

    typedef CPPAD_TESTVECTOR( double )      Dvector;

    bool ok = true;

    Dvector xi(length);

    Dvector xl(length), xu(length);
    for(int i = 0; i < length; i++)
	{	
        xl[i] = 0;
		xu[i] = Density_Fillable[i];
	}

    Dvector gl(1), gu(1);
	gl[0] = 0.;     gu[0] = 0.;

    FG_eval fg_eval(Density_Metal, Overlay);

    std::string options;
    //options += "String  sb           yes\n";
    options += "String  jacobian_approximation exact\n";
    options += "String  hessian_approximation  limited-memory\n";
    options += "String  linear_solver  ma97\n";
    options += "Integer max_iter     100\n";
    options += "Numeric tol          1e-6\n";
    //options += "String  derivative_test second-order\n";
    //options += "Numeric point_perturbation_radius  0.\n";
    //options += "Sparse  true forward\n";

    CppAD::ipopt::solve_result<Dvector> solution;
    CppAD::ipopt::solve<Dvector, FG_eval>(
		options, xi, xl, xu, gl, gu, fg_eval, solution
	);

    ok &= solution.status == CppAD::ipopt::solve_result<Dvector>::success;
    
    //for(int i=0; i<length; i++){
    //    std::cout << "xi[" << i << "]: " << solution.x[i] << std::endl;
    //}

    /**********************************
     * 
     * Result
     * 
     * *******************************/

    double Sum1, Sum2, Sum3 = 0.0;
    for(int i=0; i < (length/3); i++){
        Sum1 += solution.x[i] + Density_Metal[i];
        Sum2 += solution.x[i + (length/3)] + Density_Metal[i + (length/3)];
        Sum3 += solution.x[i + 2*(length/3)] + Density_Metal[i + 2*(length/3)];
    }
    double Mean1 = Sum1/(length/3);
    double Mean2 = Sum2/(length/3);
    double Mean3 = Sum3/(length/3);

    double Std1, Std2, Std3 = 0.0;
    for(int i=0; i < (length/3); i++){
        Std1 += (solution.x[i] + Density_Metal[i] - Mean1)*(solution.x[i] + Density_Metal[i] - Mean1);
        Std2 += (solution.x[i + (length/3)] + Density_Metal[i + (length/3)] - Mean2)*(solution.x[i + (length/3)] + Density_Metal[i + (length/3)] - Mean2);
        Std3 += (solution.x[i + 2*(length/3)] + Density_Metal[i + 2*(length/3)] - Mean3)*(solution.x[i + 2*(length/3)] + Density_Metal[i + 2*(length/3)] - Mean3);
    }
    Std1 = sqrt(Std1/(length/3));
    Std2 = sqrt(Std2/(length/3));
    Std3 = sqrt(Std3/(length/3));

    std::cout << "Std1: " << Std1 << std::endl;
    std::cout << "Std2: " << Std2 << std::endl;
    std::cout << "Std3: " << Std3 << std::endl;
    std::cout << "Cost = " << Std1 + Std2 + Std3 << std::endl;

    double Overlay1, Overlay2 = 0.0;
    for(int i=0; i < (length/3); i++){
        Overlay1 += ((solution.x[i] + solution.x[i + (length/3)] - Overlay[i])>0)?(solution.x[i] + solution.x[i + (length/3)] - Overlay[i]):0;
        Overlay2 += ((solution.x[i + (length/3)] + solution.x[i + 2*(length/3)] - Overlay[i+ (length/3)])>0)?(solution.x[i + (length/3)] + solution.x[i + 2*(length/3)] - Overlay[i+ (length/3)]):0;
    }

    std::cout << "Overlay: " << ((Overlay1+Overlay2)*20*20) << std::endl;

    // Ouput
    std::ofstream Output_file_1("Grid_opt_Layer1.txt");//overal grid density
    std::ofstream Output_file_2("Grid_opt_Layer2.txt");
    std::ofstream Output_file_3("Grid_opt_Layer3.txt");

    for (int i = 0; i < Num_grid; i++) {
        double tmp1 = solution.x[i] + Density_Metal[i];
        double tmp2 = solution.x[i + Num_grid] + Density_Metal[i + Num_grid];
        double tmp3 = solution.x[i + 2*Num_grid] + Density_Metal[i + 2*Num_grid];

        Output_file_1 << Grid_No_Fill[i].x << " " << Grid_No_Fill[i].y << " " << tmp1 << " " << solution.x[i] << std::endl;
        Output_file_2 << Grid_No_Fill[i + Num_grid].x << " " << Grid_No_Fill[i + Num_grid].y << " " << tmp2 << " " << solution.x[i + Num_grid] << std::endl;
        Output_file_3 << Grid_No_Fill[i + 2*Num_grid].x << " " << Grid_No_Fill[i + 2*Num_grid].y << " " << tmp3 << " " << solution.x[i + 2*Num_grid] << std::endl;
    }
    Output_file_1.close();
    Output_file_2.close();
    Output_file_3.close();

    return 0;
}