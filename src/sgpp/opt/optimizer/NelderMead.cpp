#include "opt/optimizer/NelderMead.hpp"
#include "opt/tools/Printer.hpp"

#include <algorithm>
#include <iostream>

namespace sg
{
namespace opt
{
namespace optimizer
{

const double NelderMead::DEFAULT_ALPHA = 1.0;
const double NelderMead::DEFAULT_BETA = 2.0;
const double NelderMead::DEFAULT_GAMMA = 0.5;
const double NelderMead::DEFAULT_DELTA = 0.5;
const double NelderMead::STARTING_SIMPLEX_EDGE_LENGTH = 0.4;

NelderMead::NelderMead(function::Objective &f,
        size_t max_it_count, double alpha, double beta, double gamma, double delta) :
    Optimizer(f, max_it_count),
    alpha(alpha),
    beta(beta),
    gamma(gamma),
    delta(delta)
    //max_fcn_eval_count(max_fcn_eval_count)
{
}

double NelderMead::optimize(std::vector<double> &xopt)
{
    tools::printer.printStatusBegin("Optimizing (Nelder-Mead)...");
    
    size_t d = f->getDimension();
    std::vector<std::vector<double> > points(d+1, x0);
    std::vector<std::vector<double> > points_new(d+1, x0);
    std::vector<double> f_points(d+1, 0.0);
    std::vector<double> f_points_new(d+1, 0.0);
    
    for (size_t t = 0; t < d; t++)
    {
        //points[0][t] = std::max(points[0][t] - 0.1, 0.0);
        //points[t+1][t] = std::min(points[t+1][t] + 0.1, 1.0);
        points[t+1][t] = std::min(points[t+1][t] + STARTING_SIMPLEX_EDGE_LENGTH, 1.0);
        f_points[t+1] = f->eval(points[t+1]);
        //std::cout << "i = " << i+1 << ": " << points[i+1] << ", " << f_points[i+1] << ", " << f(points[i+1], data) << "\n";
    }
    
    f_points[0] = f->eval(points[0]);
    
    std::vector<size_t> index(d+1, 0);
    std::vector<double> point_o(d, 0.0);
    std::vector<double> point_r(d, 0.0);
    std::vector<double> point_e(d, 0.0);
    std::vector<double> point_ic(d, 0.0);
    std::vector<double> point_oc(d, 0.0);
    size_t k = 0;
    size_t number_of_fcn_evals = d+1;
    
    while (true)
    {
        /*std::cout << "k = " << k << ", number_of_fcn_evals = " << number_of_fcn_evals << "\n";
        for (size_t i = 0; i < d+1; i++)
        {
            std::cout << "points[" << i << "] = [" << points[i][0] << ", " << points[i][1] << "]\n";
            std::cout << "f_points[" << i << "] = [" << f_points[i] << "]\n";
        }*/
        
        for (size_t i = 0; i < d+1; i++) {
            index[i] = i;
        }
        
        std::sort(index.begin(), index.end(),
                [&](const size_t &a, const size_t &b) {
                    return (f_points[a] < f_points[b]);
                }
        );
        
        for (size_t i = 0; i < d+1; i++) {
            points_new[i] = points[index[i]];
            f_points_new[i] = f_points[index[i]];
            //std::cout << "i = " << i << ": " << points_new[i] << ", " << f_points_new[i] << ", " << f(points_new[i], data) << "\n";
        }
        
        points = points_new;
        f_points = f_points_new;
        
        //bool converged = true;
        bool in_domain = true;
        bool shrink = false;
        
        for (size_t t = 0; t < d; t++)
        {
            point_o[t] = 0.0;
            //double point_min = 1.0;
            //double point_max = 0.0;
            
            for (size_t i = 0; i < d; i++)
            {
                point_o[t] += points[i][t];
                
                /*if (points[j][i] < point_min)
                {
                    point_min = points[j][i];
                }
                
                if (points[j][i] > point_max)
                {
                    point_max = points[j][i];
                }*/
            }
            
            /*if (point_max - point_min >= tol)
            {
                converged = false;
            }*/
            
            point_o[t] /= (double)d;
            point_r[t] = point_o[t] + alpha * (point_o[t] - points[d][t]);
            
            if ((point_r[t] < 0.0) || (point_r[t] > 1.0))
            {
                in_domain = false;
            }
        }
        
        /*if (converged)
        {
            break;
        }*/
        
        double f_point_r = (in_domain ? f->eval(point_r) : INFINITY);
        number_of_fcn_evals++;
        
        if ((f_points[0] <= f_point_r) && (f_point_r < f_points[d-1]))
        {
            points[d] = point_r;
            f_points[d] = f_point_r;
        } else if (f_point_r < f_points[0])
        {
            bool in_domain = true;
            
            for (size_t t = 0; t < d; t++)
            {
                point_e[t] = point_o[t] + beta * (point_r[t] - point_o[t]);
                
                if ((point_e[t] < 0.0) || (point_e[t] > 1.0))
                {
                    in_domain = false;
                }
            }
            
            double f_point_e = (in_domain ? f->eval(point_e) : INFINITY);
            number_of_fcn_evals++;
            
            if (f_point_e < f_point_r)
            {
                points[d] = point_e;
                f_points[d] = f_point_e;
            } else
            {
                points[d] = point_r;
                f_points[d] = f_point_r;
            }
        } else if (f_point_r < f_points[d])
        {
            bool in_domain = true;
            
            for (size_t t = 0; t < d; t++)
            {
                point_oc[t] = point_o[t] + gamma * (point_r[t] - point_o[t]);
                
                if ((point_oc[t] < 0.0) || (point_oc[t] > 1.0))
                {
                    in_domain = false;
                }
            }
            
            double f_point_oc = (in_domain ? f->eval(point_oc) : INFINITY);
            number_of_fcn_evals++;
            
            if (f_point_oc <= f_point_r)
            {
                points[d] = point_oc;
                f_points[d] = f_point_oc;
            } else
            {
                shrink = true;
            }
        } else
        {
            bool in_domain = true;
            
            for (size_t t = 0; t < d; t++)
            {
                //point_ic[t] = point_o[t] - gamma * (point_r[t] - point_o[t]);
                point_ic[t] = point_o[t] - gamma * (point_o[t] - points[d][t]);
                
                if ((point_ic[t] < 0.0) || (point_ic[t] > 1.0))
                {
                    in_domain = false;
                }
            }
            
            double f_point_ic = (in_domain ? f->eval(point_ic) : INFINITY);
            number_of_fcn_evals++;
            
            if (f_point_ic < f_points[d])
            {
                points[d] = point_ic;
                f_points[d] = f_point_ic;
            } else
            {
                shrink = true;
            }
        }
        
        if (shrink)
        {
            for (size_t i = 1; i < d+1; i++)
            {
                bool in_domain = true;
                
                for (size_t t = 0; t < d; t++)
                {
                    points[i][t] = points[0][t] + delta * (points[i][t] - points[0][t]);
                    
                    if ((points[i][t] < 0.0) || (points[i][t] > 1.0))
                    {
                        in_domain = false;
                    }
                }
                
                f_points[i] = (in_domain ? f->eval(points[i]) : INFINITY);
            }
            
            number_of_fcn_evals += d;
        }
        
        if (k % 10 == 0)
        {
            std::stringstream msg;
            msg << k << " steps, f(x) = " << f_points[0];
            tools::printer.printStatusUpdate(msg.str());
        }
        
        if (number_of_fcn_evals + (d+2) > N)
        {
            break;
        }
        
        k++;
    }
    
    xopt = points[0];
    
    {
        std::stringstream msg;
        msg << k << " steps, f(x) = " << f_points[0];
        tools::printer.printStatusUpdate(msg.str());
        tools::printer.printStatusEnd();
    }
    
    return f_points[0];
}

std::unique_ptr<Optimizer> NelderMead::clone()
{
    std::unique_ptr<Optimizer> result(new NelderMead(*f, N, alpha, beta, gamma, delta));
    result->setStartingPoint(x0);
    return result;
}

double NelderMead::getAlpha() const
{
    return alpha;
}

void NelderMead::setAlpha(double alpha)
{
    this->alpha = alpha;
}

double NelderMead::getBeta() const
{
    return beta;
}

void NelderMead::setBeta(double beta)
{
    this->beta = beta;
}

double NelderMead::getGamma() const
{
    return gamma;
}

void NelderMead::setGamma(double gamma)
{
    this->gamma = gamma;
}

double NelderMead::getDelta() const
{
    return delta;
}

void NelderMead::setDelta(double delta)
{
    this->delta = delta;
}

/*size_t NelderMead::getMaxFcnEvalCount() const
{
    return max_fcn_eval_count;
}

void NelderMead::setMaxFcnEvalCount(size_t max_fcn_eval_count)
{
    this->max_fcn_eval_count = max_fcn_eval_count;
}*/

}
}
}
