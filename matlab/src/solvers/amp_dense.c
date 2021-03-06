#include "../swamp.h"

/* (Sequential) AMP for sparse matrices */
void amp_dense ( 
        size_t n, size_t m, double *y, double *F, 
        double *delta, int is_array, int learn_delta, 
        void (*prior) (int, double*, double*, double*, double*, double*, double*, int), double *prior_prmts, int learn_prior, 
        int t_max, double eps, double damp, int disp, FILE *output, FILE *history, double *x,
        double *a, double *c, double *r, double *sig 
    ) {
    double *w_r, *v, *g, *a_proj, *c_proj, *delta0;
    double w_proj, v_proj, a_old, c_old, diff, res;
    double delta_n, delta_d, delta_mean, gamma;
    double mse, fren;

    unsigned int i, mu, idx, t;
    int *seq, key;

    /* Alloc. structures */
    w_r = malloc(sizeof(double) * m);
    v = malloc(sizeof(double) * m);
    a_proj = malloc(sizeof(double) * m);
    g = malloc(sizeof(double) * m);
    c_proj = malloc(sizeof(double) * m);
    delta0 = malloc(sizeof(double) * m);
    seq = malloc(sizeof(int) * n);
    if (!w_r || !v || !g || !a_proj || !c_proj || !delta0 || !seq)
        mexErrMsgTxt("Failure in allocating memory.");

    /* Init. variables */
    for (mu = 0; mu < m; mu++) delta0[mu] = delta[mu];
    for (mu = 0; mu < m; mu++) w_r[mu] = 0.;
    for (mu = 0; mu < m; mu++) v[mu] = 1.;

    /* Iterate AMP */
    if (output)
        fprintf(output, "'#t';'mse';'delta';'RSS';'diff';'F'\n");
    for (t = 0; t < t_max; t++) {
        /* Generate random permutation */
        for (key = 0; key < n; key++) seq[key] = key;
        sort_rand(n, seq);

        /* Update a_proj and c_proj */
        for (mu = 0; mu < m; mu++) a_proj[mu] = c_proj[mu] = 0;
        for (i = 0; i < n; i++)
            for (mu = 0; mu < m; mu++) { 
                idx = mu + i * m;
                a_proj[mu] += F[idx] * a[i];
                c_proj[mu] += (F[idx] * F[idx]) * c[i];
            }

        /* Update w_r and v */
        for (mu = 0; mu < m; mu++) {
            g[mu] = w_r[mu] / v[mu];
            w_r[mu] = (y[mu] - a_proj[mu]) + c_proj[mu] * g[mu];
            v[mu] = (is_array ? delta[mu] : *delta) + c_proj[mu];
        }

        /* Sweep over all n variables, in random order */
        diff = res = 0.;
        for (key = 0; key < n; key++) {
            i = seq[key];
            a_old = a[i], c_old = c[i];

            /* Update r and sig ... */
            w_proj = v_proj = 0.; /* Dot products: Fw_r / v and F^2 / v */
            for (mu = 0; mu < m; mu++) { 
                idx = mu + i * m;
                w_proj += F[idx] * w_r[mu] / v[mu];
                v_proj += (F[idx] * F[idx]) / v[mu];
            }

            sig[i] = damp * sig[i] + (1 - damp) * (1. / v_proj);
            r[i] = damp * r[i] + (1 - damp) * (a[i] + sig[i] * w_proj);

            /* ... then, a and c ... */
            prior(1, &r[i], &sig[i], prior_prmts, &a[i], &c[i], NULL, 0);

            /* ... and finally, w_r and v. */
            for (mu = 0; mu < m; mu++) { 
                idx = mu + i * m;
                w_r[mu] += F[idx] * (a_old - a[i])
                    - (F[idx] * F[idx]) * (c_old - c[i]) * g[mu];
                v[mu] += (F[idx] * F[idx]) * (c[i] - c_old);
            }

            diff += fabs(a[i] - a_old);
        }

        /* Update prior parameters */
        if (learn_prior) prior(n, r, sig, prior_prmts, a, c, NULL, 1);

        /* Update delta */
        if (learn_delta && t > 0) {
            if (!is_array) {
                delta_n = delta_d = 0; /* Sums: (w / v)^2 and (1 / v) */
                for (mu = 0; mu < m; mu++) {
                    delta_n += pow(w_r[mu] / v[mu], 2);
                    delta_d += (1. / v[mu]);
                }
                *delta *= (delta_n / delta_d);
            } else {
                delta_n = delta_d = 0;
                for (mu = 0; mu < m; mu++) {
                    delta_n += pow(w_r[mu] * delta[mu] / v[mu], 2) / delta0[mu];
                    delta_d += delta[mu] / v[mu];
                }

                gamma = delta_n / delta_d;
                if (disp) printf("\tgamma = %g\n", gamma);
                for (mu = 0; mu < m; mu++) delta[mu] = gamma * delta0[mu];
            }
        }

        /* Print some info. */
        res = 0;
        for (mu = 0; mu < m; mu++)
            res += pow(y[mu] - a_proj[mu], 2);
        res /= m;
        
        mse = 0;
        if (x) {
            for (i = 0; i < n; i++)
                mse += pow(a[i] - x[i], 2);
            mse /= n;
        }

        if (is_array) {
            delta_mean = 0;
            for (mu = 0; mu < m; mu++) delta_mean += delta[mu];
            delta_mean /= m;
        } else {
            delta_mean = *delta;
        }

        if (disp) printf("t: %3d; mse: %.4e, est noise: %.4e, rss: %.4e, diff: %.4e\n", 
                t, mse, delta_mean, res, diff / n, fren);
        if (output) fprintf(output, "%d;%g;%g;%g;%g;%g\n", 
                t, mse, delta_mean, res, diff / n, fren);
        mexEvalString("drawnow");

        if (history) {
            for (i = 0; i < n; i++) fprintf(history, "%g ", a[i]);
            fprintf(history, "\n");
        }

        /* Check for convergence */
        if (diff / n < eps) break;
    }

    /* Dealloc. structures */
    free(seq);
    free(delta0);
    free(c_proj);
    free(a_proj);
    free(g);
    free(v);
    free(w_r);
}
