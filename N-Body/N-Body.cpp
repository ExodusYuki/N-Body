// N-Body.cpp : Defines the entry point for the console application.
//


#include "Settings.h"
#include <ostream>
#include <iostream>
#include <vector>
#include "tbb/tick_count.h"
#include "Particle.h"
#include "tbb/cache_aligned_allocator.h"
#include "tbb/concurrent_vector.h"
#include "ParticleHandler.h"
#include <tbb/parallel_for.h>
#include <complex>

void simulate_tbb(tbb::concurrent_vector<Particle>& particles, double total_time_steps, double time_step, size_t particle_count,
	size_t universe_size_x, size_t universe_size_y) {
	// Simulate
	// TODO: check copy local/swap?
	for (double current_time_step = 0.0; current_time_step < total_time_steps; current_time_step += time_step) {
		parallel_for(tbb::blocked_range<size_t>(0, particle_count),
			[&](const tbb::blocked_range<size_t>& r) {
			for (size_t i = r.begin(); i < r.end(); ++i) {
				particles[i].reset_forces();
				for (size_t j = 0; j < particle_count; ++j) {
					if (j != i)
						particles[i].add_force(particles[j]);
				}
			}

		});
		parallel_for(tbb::blocked_range<size_t>(0, particle_count),
			[&](const tbb::blocked_range<size_t>& r) {
			for (size_t index = r.begin(); index < r.end(); ++index)
				particles[index].advance(time_step);
		});

		if (SAVE_ALL_PNG_STEPS) {

			std::string file_name = "universe_serial_timestep_" + std::to_string(current_time_step) + ".png";

			ParticleHandler::universe_to_png(ParticleHandler::to_vector(particles), universe_size_x, universe_size_y, file_name.c_str());
		}
	}
}

void simulate_serial(std::vector<Particle>& particles, double total_time_steps, double time_step, size_t particle_count,
	size_t universe_size_x, size_t universe_size_y) {
	// Simulate
	for (double current_time_step = 0.0; current_time_step < total_time_steps; current_time_step += time_step) {
		for (size_t i = 0; i < particle_count; ++i) {
			particles[i].reset_forces();
			for (size_t j = 0; j < particle_count; ++j) {
				if (j != i)
					particles[i].add_force(particles[j]);
			}
		}

		//loop again to update the time stamp here
		for (Particle& particle : particles) {
			particle.advance(time_step);
		}
		if (SAVE_ALL_PNG_STEPS) {

			std::string file_name = "universe_serial_timestep_" + std::to_string(current_time_step) + ".png";

			ParticleHandler::universe_to_png(particles, universe_size_x, universe_size_y, file_name.c_str());
		}
	}
}

int main()
{
	int thread_count = DEFAULT_NUMBER_OF_THREADS;
	double gravity = GRAVITY;
	size_t particle_count = DEFAULT_PARTICLE_COUNT;
	double total_time_steps = DEFAULT_TOTAL_TIME_STEPS;
	double time_step = TIME_STEP;
	size_t universe_size_x = UNIVERSE_SIZE_X;
	size_t universe_size_y = UNIVERSE_SIZE_Y;

	particle_count = 200;
	total_time_steps = 0.5;
	universe_size_x = 80;
	universe_size_y = 60;

	if (total_time_steps > 0.0 & particle_count > 0 && universe_size_x > 0 && universe_size_y > 0) {
		
		// Print calculation info
		std::cout << "= Parallel N-Body simulation serially and with Thread Building Blocks =" << std::endl;
		std::cout << "Number of threads: " << thread_count << std::endl;
		std::cout << "Total time steps: " << total_time_steps << std::endl;
		std::cout << "Time step: " << time_step << std::endl;
		std::cout << "Particle count: " << particle_count << std::endl << std::endl;
		std::cout << "Universe Size: " << universe_size_x << " x " << universe_size_y << std::endl << std::endl;

		// Advance Game Of Life
		tbb::tick_count before, after; // Execution timers

		// Initialize particle container
		std::vector<Particle> particles;

		// Put random live cells
		ParticleHandler::allocate_random_particles(particle_count, particles, universe_size_x, universe_size_y);

		// Show Init vector universe
		if (VERBOSE) {
			std::cout << "Init Universe" << std::endl;
		}

		// Simulate
		// Copy the particle universes
		std::vector<Particle> particles_serial(particles);
		tbb::concurrent_vector<Particle, tbb::cache_aligned_allocator<Particle>> particles_tbb(ParticleHandler::to_concurrent_vector(particles));

		// Serial execution
		before = tbb::tick_count::now();
		simulate_serial(particles_serial, total_time_steps, time_step, particle_count, universe_size_x, universe_size_y); // Advance Simulation serially
		after = tbb::tick_count::now();
		std::cout << std::endl << "Serial execution: " << 1000 * (after - before).seconds() << " ms" << std::endl;

		// Thread Building Blocks		
		before = tbb::tick_count::now();
		simulate_tbb(particles_tbb, total_time_steps, time_step, particle_count, universe_size_x, universe_size_y); // Advance Simulation with TBB
		after = tbb::tick_count::now();
		std::cout << std::endl << "Thread Building Blocks execution: " << 1000 * (after - before).seconds() << " ms" << std::endl;

		// Show universe
		if (VERBOSE) {

			std::cout << "Final Universe Serial" << std::endl;
			//grid_modifier.debug_show_universe(universe_serial, universe_size_x, universe_size_y);

			std::cout << "Final Universe TBB" << std::endl;
			//grid_modifier.debug_show_universe(UniverseModifier::to_vector(universe_tbb), universe_size_x, universe_size_y);
		}

		if (SAVE_PNG) {
			ParticleHandler::universe_to_png(particles, universe_size_x, universe_size_y, "final_serial_universe.png");
			ParticleHandler::universe_to_png(ParticleHandler::to_vector(particles_tbb), universe_size_x, universe_size_y, "final_tbb_universe.png");
		}

		system("pause");
	}

}

