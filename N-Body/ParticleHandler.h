#pragma once
#include "Particle.h"
#include <vector>
#include <tbb/concurrent_vector.h>
#include "QuadParticleTree.h"

// Handles conversions, allocations and image saving of various particle collections
class ParticleHandler
{
public:
	static void allocate_random_particles(size_t particle_count, std::vector<Particle>& particles, size_t size_x, size_t size_y);
	static std::vector<Particle> get_random_particles_Barns_Hut_sample();
	static void universe_to_png(const std::vector<Particle>& universe, size_t universe_size_x, size_t universe_size_y, const char* filename);
	static tbb::concurrent_vector<Particle> to_concurrent_vector(const std::vector<Particle>& input_particles);
	static std::vector<Particle> to_vector(const tbb::concurrent_vector<Particle>& input_particles);
	static bool are_equal(const std::vector<Particle>& first_particles, const std::vector<Particle>& second_particles);
	static QuadParticleTree* to_quad_tree(const std::vector<Particle>& input_particles, size_t size_x, size_t size_y);
	static float get_comparison_tolerance(float first_float, float second_float);
	static bool are_equal(const std::vector<Particle>& first_particles, const std::vector<Particle>& second_particles, float tolerance_multiplier);
	static void universe_to_csv(const std::vector<Particle>& universe, const char* filename);
};
