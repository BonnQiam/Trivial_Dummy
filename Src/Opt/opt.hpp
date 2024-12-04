#include <vector>
#include <cppad/ipopt/solve.hpp>

//! Attention: solve_callback.hpp in /usr/local/include/cppad/ipopt has been modified by QIAMKING
//? Original solve_callback.hpp is <cppad/ipopt/solve_callback.hpp>

#if 1
// factor 4 s
#define alpha_std       0.2
#define beta_std        0.077
#define alpha_line      0.2
#define beta_line       11.758
#define alpha_outlier   0.15
#define beta_outlier    0.014
#define alpha_overlay   0.2
#define beta_overlay    79154
#endif

#if 0
// factor 2 b
#define alpha_std       0.2
#define beta_std        0.517
#define alpha_line      0.2
#define beta_line       3578
#define alpha_outlier   0.15
#define beta_outlier    22.801
#define alpha_overlay   0.2
#define beta_overlay    6111303
#endif

#if 0
// factor 2 m
#define alpha_std       0.2
#define beta_std        0.53
#define alpha_line      0.2
#define beta_line       6052
#define alpha_outlier   0.15
#define beta_outlier    27.56
#define alpha_overlay   0.2
#define beta_overlay    10276835
#endif

using CppAD::AD;

// 定义 max_func 函数: if x > y, return x, else return y
AD<double> max_func(AD<double> x, AD<double> y, AD<double> z) {
    //using logsumexp
    return CppAD::log(CppAD::exp(x) + CppAD::exp(y))*z;
};

//AD<double> max_func(AD<double> x, AD<double> y, AD<double> z) {
//    return CppAD::CondExpGt(x, y, x, y)*z;
//};

// Cost 函数定义
class FG_eval{
    private:
        std::vector<double> Density_Metal;
        std::vector<double> Overlay_Metal;
        AD<double> Std1, Std2, Std3 = 0.0;
        AD<double> Mean1, Mean2, Mean3 = 0.0;
    public:
        typedef CPPAD_TESTVECTOR(AD<double>)    ADvector;

        FG_eval(std::vector<double> Density_Metal, std::vector<double> Overlay_Metal){
            this->Density_Metal = Density_Metal;
            this->Overlay_Metal = Overlay_Metal;
        };

        void operator()(ADvector& fg, const ADvector& x){
            fg[0] = -1.0*Std(x)-1.0*Outlier(x)-1.0*Overlay(x);
            //fg[0] = 1.0*Std(x)+1.0*Outlier(x);
            fg[1] = 0.0;
        };

        AD<double> Std(const ADvector & x){
            AD<double> Sum1, Sum2, Sum3 = 0.0;
            for(int i=0; i < (x.size()/3); i++){
                Sum1 += x[i] + Density_Metal[i];
                Sum2 += x[i + (x.size()/3)] + Density_Metal[i + (x.size()/3)];
                Sum3 += x[i + 2*(x.size()/3)] + Density_Metal[i + 2*(x.size()/3)];
            }

            Mean1 = Sum1/(x.size()/3);
            Mean2 = Sum2/(x.size()/3);
            Mean3 = Sum3/(x.size()/3);

            for(int i=0; i < (x.size()/3); i++){
                Std1 += (x[i] + Density_Metal[i] - Mean1)*(x[i] + Density_Metal[i] - Mean1);
                Std2 += (x[i + (x.size()/3)] + Density_Metal[i + (x.size()/3)] - Mean2)*(x[i + (x.size()/3)] + Density_Metal[i + (x.size()/3)] - Mean2);
                Std3 += (x[i + 2*(x.size()/3)] + Density_Metal[i + 2*(x.size()/3)] - Mean3)*(x[i + 2*(x.size()/3)] + Density_Metal[i + 2*(x.size()/3)] - Mean3);
            }
            Std1 = sqrt(Std1/(x.size()/3));
            Std2 = sqrt(Std2/(x.size()/3));
            Std3 = sqrt(Std3/(x.size()/3));
            
            return max_func(0., 1.0-(Std1+Std2+Std3)/beta_std, alpha_std);

            //return max_func(0., 1.0-Std1/beta_std, alpha_std)
            //        +max_func(0., 1.0-Std2/beta_std, alpha_std)
            //        +max_func(0., 1.0-Std3/beta_std, alpha_std);
        };


        AD<double> Outlier(const ADvector & x){
            AD<double> Outlier1,Outlier2, Outlier3 = 0.0;

            for(int i=0; i < (x.size()/3); i++){
                Outlier1 += max_func(0., fabs(x[i] - Mean1),1.0);
                Outlier2 += max_func(0., fabs(x[i + (x.size()/3)] - Mean2),1.0);
                Outlier3 += max_func(0., fabs(x[i + 2*(x.size()/3)] - Mean3),1.0);
            }

            return max_func(0., 1.0-(Outlier1+Outlier2+Outlier3)/beta_outlier, alpha_outlier);
            //return max_func(0., 1.0-Outlier1/beta_outlier, alpha_outlier)
            //        +max_func(0., 1.0-Outlier2/beta_outlier, alpha_outlier)
            //        +max_func(0., 1.0-Outlier3/beta_outlier, alpha_outlier);
        };

        AD<double> Overlay(const ADvector & x){
            AD<double> Overlay1,Overlay2 = 0.0;

            for(int i=0; i < (x.size()/3); i++){
                Overlay1 += max_func(0., x[i] + x[i+(x.size()/3)] - Overlay_Metal[i],1.0)*20*20;
                Overlay2 += max_func(0., x[i+(x.size()/3)] + x[i+2*(x.size()/3)] - Overlay_Metal[i+(x.size()/3)],1.0)*20*20;
            }

            return max_func(0., 1.0-(Overlay1+Overlay2)/beta_overlay, alpha_overlay);
            //return max_func(0., 1.0-Overlay1/beta_overlay, alpha_overlay)
            //        +max_func(0., 1.0-Overlay2/beta_overlay, alpha_overlay);
        };
};

