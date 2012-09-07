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

#include "common.h"

void sim_hess(struct efp *, const struct config *);

void sim_hess(struct efp *efp, const struct config *config)
{
	enum efp_result res;
	int n_coord = 6 * config->n_frags;

	double *xyzabc = xmalloc(n_coord * sizeof(double));
	double *grad = xmalloc(n_coord * sizeof(double));
	double *grad_0 = xmalloc(n_coord * sizeof(double));
	double *hess = xmalloc(n_coord * n_coord * sizeof(double));

	if ((res = efp_compute(efp, 1)))
		lib_error(res);

	if ((res = efp_get_coordinates(efp, config->n_frags, xyzabc)))
		lib_error(res);

	if ((res = efp_get_gradient(efp, config->n_frags, grad_0)))
		lib_error(res);

	print_energy(efp);
	print_gradient(efp);
	fflush(stdout);

	for (int i = 0; i < n_coord; i++) {
		double save = xyzabc[i];
		xyzabc[i] = save + config->hess_delta;

		if ((res = efp_set_coordinates(efp, EFP_COORD_TYPE_XYZABC, xyzabc)))
			lib_error(res);

		if ((res = efp_compute(efp, 1)))
			lib_error(res);

		if ((res = efp_get_gradient(efp, config->n_frags, grad)))
			lib_error(res);

		xyzabc[i] = save;
	}

	printf("    HESSIAN MATRIX\n\n");
	print_matrix(n_coord, n_coord, hess);

	free(xyzabc);
	free(grad);
	free(grad_0);
	free(hess);
}