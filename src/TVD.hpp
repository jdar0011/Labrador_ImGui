// Author: ChatGPT
// Contains the function(s) for performing total variation denoise, which should 
// provide a denoised signal for the purposes of Vpp calculations, and potentially in the display if deemed adequate
#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>

// Proximal operator for the fidelity term
inline double prox_tau(double v, double tau, double f) {
    return (v + tau * f) / (1.0 + tau);
}

// Function to compute the gradient (forward difference)
inline double gradient(const std::vector<double>& u, int i) {
    return u[i + 1] - u[i];
}

// Function to compute the divergence (backward difference)
inline double divergence(const std::vector<double>& p, int i) {
    return (i == 0 ? 0 : p[i - 1]) - p[i];
}

// Total variation denoising using primal-dual algorithm
std::vector<double> tv_denoise(const std::vector<double>& f, double lambda, int max_iter = 100, double tau = 0.1, double sigma = 0.1) {
    int n = f.size();
    if (n < 2)
    {
		return {};
    }
    // Initialize primal (u) and dual (p) variables
    std::vector<double> u(f);         // Denoised signal, initially set to the noisy signal
    std::vector<double> u_bar(f);     // Auxiliary variable for u
    std::vector<double> p(n - 1, 0);  // Dual variable p

    // Main iteration loop
    for (int k = 0; k < max_iter; ++k) {
        // apply boundary conditions
		u[0] = f[0];
		u[n - 1] = f[n - 1];
        // Update dual variable p
        for (int i = 0; i < n - 1; ++i) {
            double grad_u_bar = gradient(u_bar, i);  // Compute the gradient of u_bar
            p[i] = (p[i] + sigma * grad_u_bar) / (1.0 + sigma * std::abs(grad_u_bar));  // Project onto |p| <= 1
        }

        // Update primal variable u
        for (int i = 0; i < n; ++i) {
            double div_p = (i < n - 1) ? divergence(p, i) : -p[i - 1];  // Compute the divergence of p
            u[i] = prox_tau(u[i] - tau * div_p, tau, f[i]);  // Proximal update for u
        }

        // Over-relaxation step
        for (int i = 0; i < n; ++i) {
            u_bar[i] = 2 * u[i] - u_bar[i];  // Update u_bar
        }
    }
	u[u.size()-1] = u[u.size()-2]; // set last element to second to last element as it is jumping around too much
    return u;  // Return the denoised signal
}