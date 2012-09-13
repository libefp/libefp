/*-
 * Copyright (c) 2012 Ilya Kaliman
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <optimizer.h>

#include "common.h"

#define PI 3.14159265358979323846

void sim_opt(struct efp *, const struct config *);

static double compute_efp(int n, const double *x, double *gx, void *data)
{
	struct efp *efp = (struct efp *)data;

	int n_frag;
	enum efp_result res;

	if ((res = efp_get_frag_count(efp, &n_frag)))
		lib_error(res);

	assert(n == 6 * n_frag);

	if ((res = efp_set_coordinates(efp, EFP_COORD_TYPE_XYZABC, x)))
		lib_error(res);

	if ((res = efp_compute(efp, 1)))
		lib_error(res);

	struct efp_energy energy;

	if ((res = efp_get_energy(efp, &energy)))
		lib_error(res);

	if ((res = efp_get_gradient(efp, EFP_GRAD_TYPE_DERIVATIVE, n_frag, gx)))
		lib_error(res);

	return energy.total;
}

static void print_restart(struct efp *efp)
{
	int n_frag;
	enum efp_result res;

	if ((res = efp_get_frag_count(efp, &n_frag)))
		lib_error(res);

	char name[64];
	double coord[6 * n_frag];

	if ((res = efp_get_coordinates(efp, n_frag, coord)))
		lib_error(res);

	printf("    RESTART DATA (ATOMIC UNITS)\n\n");

	for (int i = 0; i < n_frag; i++) {
		if ((res = efp_get_frag_name(efp, i, sizeof(name), name)))
			lib_error(res);

		print_fragment(name, coord + 6 * i, NULL);
	}

	printf("\n");
}

static int check_conv(double rms_grad, double max_grad, double opt_tol)
{
	return fabs(max_grad) < opt_tol && fabs(rms_grad) < opt_tol / 3.0;
}

static void get_grad_info(int n_coord, const double *grad, double *rms_grad_out,
				double *max_grad_out)
{
	double rms_grad = 0.0, max_grad = 0.0;

	for (int i = 0; i < n_coord; i++) {
		rms_grad += grad[i] * grad[i];

		if (fabs(grad[i]) > max_grad)
			max_grad = grad[i];
	}

	rms_grad = sqrt(rms_grad / n_coord);

	*rms_grad_out = rms_grad;
	*max_grad_out = max_grad;
}

static void print_status(struct efp *efp, double e_diff, double rms_grad,
				double max_grad)
{
	print_geometry(efp);
	print_restart(efp);
	print_energy(efp);

	printf("                ENERGY CHANGE %16.10lf\n", e_diff);
	printf("                 RMS GRADIENT %16.10lf\n", rms_grad);
	printf("             MAXIMUM GRADIENT %16.10lf\n", max_grad);
	printf("\n\n");

	fflush(stdout);
}

void sim_opt(struct efp *efp, const struct config *config)
{
	int n_frag;
	enum efp_result res;

	if ((res = efp_get_frag_count(efp, &n_frag)))
		lib_error(res);

	int n_coord = 6 * n_frag;
	double rms_grad, max_grad;
	double coord[n_coord], grad[n_coord];

	struct opt_state *state = opt_create(n_coord);
	if (!state)
		error("UNABLE TO CREATE AN OPTIMIZER");

	int nbd[n_coord];
	double l_bound[n_coord], u_bound[n_coord];

	memset(nbd, 0, n_coord * sizeof(int));
	memset(l_bound, 0, n_coord * sizeof(double));
	memset(u_bound, 0, n_coord * sizeof(double));

	/* Euler angle beta must be in range [0,pi] */
	for (int i = 0; i < n_frag; i++) {
		nbd[6 * i + 4] = 2;
		l_bound[6 * i + 4] = 0.0;
		u_bound[6 * i + 4] = PI;
	}

	opt_set_bound(state, n_coord, nbd, l_bound, u_bound);
	opt_set_func(state, compute_efp);
	opt_set_user_data(state, efp);

	if ((res = efp_get_coordinates(efp, n_frag, coord)))
		lib_error(res);

	if (opt_init(state, n_coord, coord))
		error("UNABLE TO INITIALIZE AN OPTIMIZER");

	double e_old = opt_get_fx(state);
	opt_get_gx(state, n_coord, grad);
	get_grad_info(n_coord, grad, &rms_grad, &max_grad);

	printf("    INITIAL STATE\n\n");
	print_status(efp, 0.0, rms_grad, max_grad);

	for (int step = 1; step <= config->max_steps; step++) {
		if (opt_step(state))
			error("UNABLE TO MAKE AN OPTIMIZATION STEP");

		double e_new = opt_get_fx(state);
		opt_get_gx(state, n_coord, grad);
		get_grad_info(n_coord, grad, &rms_grad, &max_grad);

		if (check_conv(rms_grad, max_grad, config->opt_tol)) {
			printf("    FINAL STATE\n\n");
			print_status(efp, e_new - e_old, rms_grad, max_grad);
			printf("OPTIMIZATION CONVERGED\n");
			break;
		}

		printf("    STATE AFTER %d STEPS\n\n", step);
		print_status(efp, e_new - e_old, rms_grad, max_grad);

		e_old = e_new;
	}

	opt_shutdown(state);
}