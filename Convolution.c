#include "Convolution.h"

/*
    n - кол-во переменных решения
    m - кол-во ограничений 
    c - вектор коэффициентов при X(1..n)
    A - матрица ограничений A = ||aij|| 
    b - свободные члены ограничений b Ax = b
    mn - MAXIM / MINIM ?
*/
void lpSolve(unsigned int n, unsigned int m, double* c, double **A, double *b, byte mn) {
        // Создаем новый объект LpSolve
        lprec* lp = make_lp(0, n);
        int* colno = (int*)calloc(n, sizeof(*colno)); // C

        set_lp_name(lp, "test");
        
        // Именуем переменные
        for (int i = 1; i <= n; i++) {
            char buf[10];
            wsprintfA(buf, "x%d", i);
            set_col_name(lp, i, buf);
            set_lowbo(lp, i, 0.0); // xi >= 0
            set_int(lp, i, 1);
        }

        // Устанавливаем целевую функцию cx
        for (int i = 0; i < n; i++) {
            //row[i] = c[i];    // c1, c2 ... cn
            colno[i] = i + 1; // (x1, x2 ... xn)^T
        }
        set_obj_fnex(lp, n, c, colno);
        for (int i = 0; i < m; i++) {
            add_constraintex(lp, n, A[i], colno, EQ, b[i]);
        }
       
        // ...
        // Если нужно, добавьте установку типов для остальных переменных
        // ...

        // Устанавливаем направление оптимизации (максимизация или минимизация)
        (mn == MAXIM) ? set_maxim(lp) : ((mn == MINIM) ? set_minim(lp) : 0);

        write_lp(lp, "a.lp");
        int ret = solve(lp);

        // Получаем результат 
        if (ret == 0 || ret == 1) {
            for (int i = m + 1; i <= m + n; i++) {
                printResultX(lp, i);
            }
        }
        else {
            char* errmsg;
            switch (ret)
            {
            case 2: {
                errmsg = "INFEASIBLE";
                break;
            }
            case 3: {
                errmsg = "UNBOUNDED";
                break;
            }
            case 4: {
                errmsg = "DEGENERATE";
                break;
            }
            case 5: {
                errmsg = "NUMFAILURE";
                break;
            }
            case 6: {
                errmsg = "USERABORT";
                break;
            }
            case 7: {
                errmsg = "TIMEOUT";
                break;
            }
            case 25: {
                errmsg = "ACCURACYERROR";
                break;
            }
            default:
                errmsg = "ERROR";
                break;
            }
            WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), errmsg, strlen(errmsg), 0, 0);
        }
        if (lp != NULL) delete_lp(lp);
        free(colno);
}

void printResultX(lprec* lp, int index) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    wchar_t* fstr = L"\nx%d = %.3lf\n";
    wchar_t buf[255];
    double res = get_var_primalresult(lp, index);

    int c = swprintf(buf, 255, fstr, index, res);
    WriteConsoleW(hOut, buf, c, 0, 0);
}