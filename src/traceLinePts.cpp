#include "Misc.h"
#include "traceLinePts.h"

void itemTrace(vector<double> &P, vector<double> &Pstar, const vector<double> &a, const double *d,
        const NumericMatrix &Theta, const double *g, const double *u, const NumericVector &ot)
{
    const int N = Theta.nrow();
    const int nfact = Theta.ncol();
    const int USEOT = ot.size() > 1;

    for (int i = 0; i < N; ++i){
        double z = *d;
        for (int j = 0; j < nfact; ++j)
            z += a[j] * Theta(i,j);
        if(USEOT) z += ot[i];
        if(z > ABS_MAX_Z) z = ABS_MAX_Z;
        else if(z < -ABS_MAX_Z) z = -ABS_MAX_Z;
        Pstar[i] = 1.0 / (1.0 + exp(-z));
        P[i] = *g + (*u - *g) * Pstar[i];
    }
}

void P_dich(vector<double> &P, const vector<double> &par, const NumericMatrix &Theta,
    const NumericVector &ot, const int &N, const int &nfact)
{
    const int len = par.size();
    const double utmp = par[len-1];
    const double gtmp = par[len-2];
    const double g = antilogit(&gtmp);
    const double u = antilogit(&utmp);
    const double d = par[len-3];
    const int USEOT = ot.size() > 1;

    for (int i = 0; i < N; ++i){
        double z = d;
        for (int j = 0; j < nfact; ++j)
            z += par[j] * Theta(i,j);
        if(USEOT) z += ot[i];
        if(z > ABS_MAX_Z) z = ABS_MAX_Z;
        else if(z < -ABS_MAX_Z) z = -ABS_MAX_Z;
        P[i+N] = g + (u - g) /(1.0 + exp(-z));
        P[i] = 1.0 - P[i + N];
    }
}

void P_graded(vector<double> &P, const vector<double> &par,
    const NumericMatrix &Theta, const NumericVector &ot, const int &N,
    const int &nfact, const int &nint, const int &itemexp, const int &israting)
{
    const int parsize = par.size();
    vector<double> a(nfact);
    for(int i = 0; i < nfact; ++i) a[i] = par[i];
    vector<double> d(nint, 0.0);
    if(israting){
        const double t = par[parsize-1];
        for(int i = nfact; i < parsize - 1; ++i)
            d[i - nfact] = par[i] + t;
    } else {
        for(int i = nfact; i < parsize; ++i)
            d[i - nfact] = par[i];
    }
    int notordered = 0;
    for(int i = 1; i < nint; ++i)
        notordered += d[i-1] <= d[i]; 
    if(notordered){
        for(int i = 0; i < P.size(); ++i)
            P[i] = 0.0;
    } else {        
        const double nullzero = 0.0, nullone = 1.0;
        NumericMatrix Pk(N, nint + 2);
    
        for(int i = 0; i < N; ++i)
            Pk(i,0) = 1.0;
        for(int i = 0; i < nint; ++i){
            vector<double> tmp1(N), tmp2(N);
            itemTrace(tmp1, tmp2, a, &d[i], Theta, &nullzero, &nullone, ot);
            for(int j = 0; j < N; ++j)
                Pk(j,i+1) = tmp2[j];
        }
        if(itemexp){
            int which = N * (nint + 1) - 1;
            for(int i = (Pk.ncol()-2); i >= 0; --i){
                for(int j = (N-1); j >= 0; --j){
                    P[which] = Pk(j,i) - Pk(j,i+1);
                    if(P[which] < 1e-20) P[which] = 1e-20;
                    else if((1.0 - P[which]) < 1e-20) P[which] = 1.0 - 1e-20;
                    --which;
                }
            }
        } else {
            int which = 0;
            for(int i = 0; i < Pk.ncol(); ++i){
                for(int j = 0; j < Pk.nrow(); ++j){
                    P[which] = Pk(j,i);
                    ++which;
                }
            }
        }
    }
}

void P_nominal(vector<double> &P, const vector<double> &par,
    const NumericMatrix &Theta, const NumericVector &ot, const int &N,
    const int &nfact, const int &ncat, const int &returnNum,
    const int &israting)
{
    vector<double> a(nfact), ak(ncat), d(ncat);
    for(int i = 0; i < nfact; ++i)
        a[i] = par[i];
    for(int i = 0; i < ncat; ++i){
        ak[i] = par[i + nfact];
        if(israting){
            if(i)
                d[i] = par[i + nfact + ncat] + par[par.size()-1];
        } else {
            d[i] = par[i + nfact + ncat];
        }
    }
    const int USEOT = ot.size() > 1;
    NumericMatrix Num(N, ncat);
    vector<double> z(ncat);
    vector<double> Den(N, 0.0);
    vector<double> innerprod(N, 0.0);

    for(int i = 0; i < N; ++i)
        for(int j = 0; j < nfact; ++j)
            innerprod[i] += Theta(i,j) * a[j];
    if(USEOT){
        for(int i = 0; i < N; ++i){
            for(int j = 0; j < ncat; ++j)
                z[j] = ak[j] * innerprod[i] + d[j] + ot[j];
            double maxz = *std::max_element(z.begin(), z.end());
            for(int j = 0; j < ncat; ++j){
                z[j] = z[j] - maxz;
                if(z[j] < -ABS_MAX_Z) z[j] = -ABS_MAX_Z;
                Num(i,j) = exp(z[j]);
                Den[i] += Num(i,j);
            }
        }
    } else {
        for(int i = 0; i < N; ++i){
            for(int j = 0; j < ncat; ++j)
                z[j] = ak[j] * innerprod[i] + d[j];
            double maxz = *std::max_element(z.begin(), z.end());
            for(int j = 0; j < ncat; ++j){
                z[j] = z[j] - maxz;
                if(z[j] < -ABS_MAX_Z) z[j] = -ABS_MAX_Z;
                Num(i,j) = exp(z[j]);
                Den[i] += Num(i,j);
            }
        }
    }
    int which = 0;
    if(returnNum){
        for(int j = 0; j < ncat; ++j){
            for(int i = 0; i < N; ++i){
                P[which] = Num(i,j);
                ++which;
            }
        }
    } else {
        for(int j = 0; j < ncat; ++j){
            for(int i = 0; i < N; ++i){
                P[which] = Num(i,j) / Den[i];
                ++which;
            }
        }
    }
}

void P_nested(vector<double> &P, const vector<double> &par,
    const NumericMatrix &Theta, const int &N, const int &nfact, const int &ncat,
    const int &correct)
{
    NumericVector dummy(1);
	const int par_size = par.size();
    vector<double> dpar(nfact+3), npar(par_size - nfact - 3, 1.0);
    for(int i = 0; i < nfact+3; ++i)
        dpar[i] = par[i];
    for(int i = nfact+3; i < par_size; ++i)
        npar[i - (nfact+3) + nfact] = par[i];
    vector<double> Pd(N*2), Pn(N*(ncat-1));
    P_dich(Pd, dpar, Theta, dummy, N, nfact);
    P_nominal(Pn, npar, Theta, dummy, N, nfact, ncat-1, 0, 0);
    NumericMatrix PD = vec2mat(Pd, N, 2);
    NumericMatrix PN = vec2mat(Pn, N, ncat-1);

    int k = 0, which = 0;
    for(int i = 0; i < ncat; ++i){
        if((i+1) == correct){
            for(int j = 0; j < N; ++j){
                P[which] = PD(j,1);
                ++which;
            }
            --k;
        } else {
            for(int j = 0; j < N; ++j){
                P[which] = PD(j,0) * PN(j,k);
                ++which;
            }
        }
        ++k;
    }
}

void P_comp(vector<double> &P, const vector<double> &par,
    const NumericMatrix &Theta, const int &N, const int &nfact)
{
    vector<double> a(nfact), d(nfact);
    for(int j = 0; j < nfact; ++j){
        a[j] = par[j];
        d[j] = par[j+nfact];
    }
    const double gtmp = par[nfact*2];
    const double g = antilogit(&gtmp);
    for(int i = 0; i < N; ++i) P[i+N] = 1.0;
    for(int j = 0; j < nfact; ++j)
        for(int i = 0; i < N; ++i)
            P[i+N] = P[i+N] * (1.0 / (1.0 + exp(-(a[j] * Theta(i,j) + d[j]))));
    for(int i = 0; i < N; ++i){
        P[i+N] = g + (1.0 - g) * P[i+N];
        if(P[i+N] < 1e-20) P[i+N] = 1e-20;
        else if (P[i+N] > 1.0 - 1e-20) P[i+N] = 1.0 - 1e-20;
        P[i] = 1.0 - P[i+N];
    }
}


RcppExport SEXP traceLinePts(SEXP Rpar, SEXP RTheta, SEXP Rot)
{
    BEGIN_RCPP

	const vector<double> par = as< vector<double> >(Rpar);
    const NumericVector ot(Rot);
    const NumericMatrix Theta(RTheta);
    const int N = Theta.nrow();
    const int nfact = Theta.ncol();
    vector<double> P(N*2);
    P_dich(P, par, Theta, ot, N, nfact);
    NumericVector ret = vec2mat(P, N, 2);
    return(ret);

	END_RCPP
}

// graded
RcppExport SEXP gradedTraceLinePts(SEXP Rpar, SEXP RTheta, SEXP Ritemexp, SEXP Rot, SEXP Risrating)
{
    BEGIN_RCPP

    const vector<double> par = as< vector<double> >(Rpar);
    const NumericVector ot(Rot);
	const NumericMatrix Theta(RTheta);
    const int nfact = Theta.ncol();
    const int N = Theta.nrow();
	const int itemexp = as<int>(Ritemexp);
    const int israting = as<int>(Risrating);
    int nint = par.size() - nfact;
    if(israting) --nint;
    int totalcat = nint + 1;
    if(!itemexp) ++totalcat;
    vector<double> P(N * totalcat);
    P_graded(P, par, Theta, ot, N, nfact, nint, itemexp, israting);
    NumericMatrix ret = vec2mat(P, N, totalcat);
    return(ret);

	END_RCPP
}

RcppExport SEXP nominalTraceLinePts(SEXP Rpar, SEXP Rncat, SEXP RTheta, SEXP RreturnNum, SEXP Rot)
{
    BEGIN_RCPP

	const vector<double> par = as< vector<double> >(Rpar);
	const int ncat = as<int>(Rncat);
	const NumericMatrix Theta(RTheta);
    const int returnNum = as<int>(RreturnNum);
    const int nfact = Theta.ncol();
    const int N = Theta.nrow();
    NumericVector ot(Rot);
    vector<double> P(N*ncat);
    P_nominal(P, par, Theta, ot, N, nfact, ncat, returnNum, 0);
    NumericMatrix ret = vec2mat(P, N, ncat);
    return(ret);

	END_RCPP
}

RcppExport SEXP gpcmTraceLinePts(SEXP Rpar, SEXP RTheta, SEXP Rot, SEXP Risrating)
{
    BEGIN_RCPP

    const vector<double> par = as< vector<double> >(Rpar);
    const NumericMatrix Theta(RTheta);
    const int israting = as<int>(Risrating);
    const int nfact = Theta.ncol();
    const int N = Theta.nrow();
    int ncat = (par.size() - nfact)/2;
    NumericVector ot(Rot);
    vector<double> P(N*ncat);
    P_nominal(P, par, Theta, ot, N, nfact, ncat, 0, israting);
    NumericMatrix ret = vec2mat(P, N, ncat);
    return(ret);

    END_RCPP
}

RcppExport SEXP nestlogitTraceLinePts(SEXP Rpar, SEXP RTheta, SEXP Rcorrect, SEXP Rncat)
{
    BEGIN_RCPP

    const vector<double> par = as< vector<double> >(Rpar);
    const NumericMatrix Theta(RTheta);
    const int correct = as<int>(Rcorrect);
    const int ncat = as<int>(Rncat);
    const int nfact = Theta.ncol();
    const int N = Theta.nrow();
    vector<double> P(N*ncat);
    P_nested(P, par, Theta, N, nfact, ncat, correct);
    NumericMatrix ret = vec2mat(P, N, ncat);
    return(ret);

    END_RCPP
}

RcppExport SEXP partcompTraceLinePts(SEXP Rpar, SEXP RTheta)
{
    BEGIN_RCPP

    const vector<double> par = as< vector<double> >(Rpar);
    const NumericMatrix Theta(RTheta);
    const int nfact = Theta.ncol();
    const int N = Theta.nrow();
    vector<double> P(N*2);
    P_comp(P, par, Theta, N, nfact);
    NumericMatrix ret = vec2mat(P, N, 2);
    return(ret);

    END_RCPP
}

void _computeItemTrace(vector<double> &itemtrace, const NumericMatrix &Theta,
    const List &pars, const NumericVector &ot, const vector<int> &itemloc, const int &which,
    const int &nfact, const int &N, const int &USEFIXED)
{
    NumericMatrix theta = Theta;
    int nfact2 = nfact;
    S4 item = pars[which];
    int ncat = as<int>(item.slot("ncat"));
    vector<double> par = as< vector<double> >(item.slot("par"));
    vector<double> P(N*ncat);
    int itemclass = as<int>(item.slot("itemclass"));
    int correct = 0;
    if(itemclass == 8)
        correct = as<int>(item.slot("correctcat"));

    /*
        1 = dich
        2 = graded
        3 = gpcm
        4 = nominal
        5 = grsm
        6 = rsm
        7 = partcomp
        8 = nestlogit
        9 = custom....have to do in R for now
    */

    if(USEFIXED){
        NumericMatrix itemFD = item.slot("fixed.design");
        nfact2 = nfact + itemFD.ncol();
        NumericMatrix NewTheta(Theta.nrow(), nfact2);
        for(int i = 0; i < itemFD.ncol(); ++i)
            NewTheta(_,i) = itemFD(_,i);
        for(int i = 0; i < nfact; ++i)
            NewTheta(_,i+itemFD.ncol()) = Theta(_,i);
        theta = NewTheta;
    }
    switch(itemclass){
        case 1 :
            P_dich(P, par, theta, ot, N, nfact2);
            break;
        case 2 :
            P_graded(P, par, theta, ot, N, nfact2, ncat-1, 1, 0);
            break;
        case 3 :
            P_nominal(P, par, theta, ot, N, nfact2, ncat, 0, 0);
            break;
        case 4 :
            P_nominal(P, par, theta, ot, N, nfact2, ncat, 0, 0);
            break;
        case 5 :
            P_graded(P, par, theta, ot, N, nfact2, ncat-1, 1, 1);
            break;
        case 6 :
            P_nominal(P, par, theta, ot, N, nfact2, ncat, 0, 1);
            break;
        case 7 :
            P_comp(P, par, theta, N, nfact2);
            break;
        case 8 :
            P_nested(P, par, theta, N, nfact2, ncat, correct);
            break;
        case 9 :
            break;
        default :
            Rprintf("How in the heck did you get here from a switch statement?\n");
            break;
    }
    int where = (itemloc[which]-1) * N;
    for(int i = 0; i < N*ncat; ++i)
        itemtrace[where + i] = P[i];
}

RcppExport SEXP computeItemTrace(SEXP Rpars, SEXP RTheta, SEXP Ritemloc, SEXP Roffterm)
{
    BEGIN_RCPP

    const List pars(Rpars);
    const NumericMatrix Theta(RTheta);
    const NumericMatrix offterm(Roffterm);
    const vector<int> itemloc = as< vector<int> >(Ritemloc);
    const int J = itemloc.size() - 1;
    const int nfact = Theta.ncol();
    const int N = Theta.nrow();
    vector<double> itemtrace(N * (itemloc[J]-1));
    S4 item = pars[0];
    NumericMatrix FD = item.slot("fixed.design");
    int USEFIXED = 0;
    if(FD.nrow() > 2) USEFIXED = 1;

    for(int which = 0; which < J; ++which)
        _computeItemTrace(itemtrace, Theta, pars, offterm(_, which), itemloc,
            which, nfact, N, USEFIXED);

    NumericMatrix ret = vec2mat(itemtrace, N, itemloc[J]-1);
    return(ret);

    END_RCPP
}
