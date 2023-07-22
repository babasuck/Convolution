#pragma once

#include <stdio.h>
#include <math.h>

// LpSolve include

#ifdef _DEBUG
#pragma comment(lib, "lpsolve55.lib")
#else
#pragma comment(lib, "lpsolve55.lib")
#endif

#define WIN32
#include "lp_lib.h"
#include "lp_utils.h"

#define MAXIM 0
#define MINIM 1

/*
    n - кол-во переменных решения
    m - кол-во ограничений
    c - вектор коэффициентов при X(1..n)
    A - матрица ограничений A = ||aij||
    b - свободные члены ограничений b Ax = b
*/
void lpSolve(unsigned int n, unsigned int m, double* c, double** A, double* b, byte mn);
void printResultX(lprec* lp, int index);
