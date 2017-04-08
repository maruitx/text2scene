#pragma once

#include <Eigen/Dense>
#include "eigenmvn.h"

class GaussianModel
{
public:
	GaussianModel(int d, double w, const Eigen::VectorXd &m, const Eigen::MatrixXd &c);
	~GaussianModel();

	int dim;
	double weight;   // mixing weight
	Eigen::VectorXd mean;
	Eigen::MatrixXd covarMat;

	Eigen::MatrixXd sample(int num);
	double probability(const Eigen::VectorXd &observation);


private:
	Eigen::EigenMultivariateNormal<double> *m_sampler;
};

class GaussianMixtureModel
{
public:
	GaussianMixtureModel(int n);
	~GaussianMixtureModel();

	Eigen::VectorXd sample(double th = 0.5);
	double probability(const Eigen::VectorXd &observation);

	std::vector<GaussianModel*> m_gaussians;

	Eigen::VectorXd m_probTh; // probability value that X percentiles of observations have passed, currently X = [20 50 80]
	int m_numGauss;
};

