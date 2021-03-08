#include <library/timing/integrator.hpp>

namespace library
{
	Integrator::Integrator(double timestep)
	{
		this->timestep = timestep;
		this->currentTime = timer.getTime();
	}
	
	void Integrator::restart()
	{
		timer.restart();
		this->currentTime = 0.0;
	}
	
	void Integrator::integrate()
	{
		double timeElapsed = timer.getTime();
		
		while (timeElapsed >= currentTime + timestep)
		{
			if (timeStep(this->timestep, timeElapsed))
				break;
			
			this->currentTime += this->timestep;
		}
	}
	
}
