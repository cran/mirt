#ifndef _ESTEP_H
#define _ESTEP_H

void _Estep(vector<double> &, vector<double> &, const vector<double> &,
    const vector<double> &, const IntegerMatrix &, const NumericMatrix &,
    const int &);

void _Estepbfactor(vector<double> &, vector<double> &, vector<double> &, 
    const NumericMatrix &, const vector<double> &, const vector<double> &, 
    const vector<double> &, const int &, const IntegerMatrix &, const IntegerMatrix &,
    const vector<double> &);

#endif
