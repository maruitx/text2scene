#include "GaussianMixtureModel.h"
#include "Utility.h"


GaussianModel::GaussianModel(int d, double w, const Eigen::VectorXd &m, const Eigen::MatrixXd &c)
	:dim(d), weight(w), mean(m), covarMat(c)
{
	m_sampler = new Eigen::EigenMultivariateNormal<double>(mean, covarMat);
}

GaussianModel::~GaussianModel()
{
	delete m_sampler;
}

Eigen::MatrixXd GaussianModel::sample(int num)
{
	return m_sampler->samples(num);
}

double GaussianModel::probability(const Eigen::VectorXd &observation)
{
	int k = dim;
	Eigen::VectorXd diff = mean - observation;
	double detCov = covarMat.determinant();
	Eigen::MatrixXd invCovMat = covarMat.inverse();

	Eigen::VectorXd v = diff.transpose()*invCovMat*diff;  // should be a double

	return pow(math_2pi, -0.5*k)*pow(detCov, -0.5)*exp(-0.5*v[0]);
}

GaussianMixtureModel::GaussianMixtureModel(int n)
	:m_numGauss(n)
{
	m_gaussians.resize(m_numGauss);
}

GaussianMixtureModel::~GaussianMixtureModel()
{
	for (int i = 0; i < m_numGauss; i++)
	{
		delete m_gaussians[i];
	}

	m_gaussians.clear();
}

Eigen::VectorXd GaussianMixtureModel::sample()
{
	double randWeight = GenRandomDouble(0, 1);
	int selectGaussId = 0;

	// Determine which Gaussian it will be coming from.
	double sumProb = 0;
	for (size_t g = 0; g < m_numGauss; g++)
	{
		sumProb += m_gaussians[g]->weight;

		if (randWeight <= sumProb)
		{
			selectGaussId = g;
			break;
		}
	}

	Eigen::MatrixXd vMat = m_gaussians[selectGaussId]->sample(1);
	return Eigen::VectorXd(Eigen::Map<Eigen::VectorXd>(vMat.data(), vMat.rows()*vMat.cols()));
}

double GaussianMixtureModel::probability(const Eigen::VectorXd &observation)
{
	double sum = 0;
	for (int i = 0; i < m_numGauss; i++)
		sum += m_gaussians[i]->weight * m_gaussians[i]->probability(observation);

	return sum;
}

