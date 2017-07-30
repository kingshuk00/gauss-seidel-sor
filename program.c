#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <math.h>

/*
a11*x1 + a12*x2 + ... a1n*xn = b1
a21*x1 + a22*x2 + ... a2n*xn = b2
     .
     .
     .
an1*x1 + an2*x2 + ... ann*xn = bn
*/

#define MAX(a,b) ((a > b) ? a : b)
#define MIN(a,b) ((a < b) ? a : b)



bool isDiagonallyDominant(int n, double **a)
{
   int i, j;
   bool first = true, second = false;

   for (i = 0; i < n; ++i) {
      double *row = a[i];
      double lhs = fabs(row[i]), rhs = 0;
      for (j = 0; j < i; ++j) {
         rhs += fabs(row[j]);
      }
      for (j = i + 1; j < n; ++j) {
         rhs += fabs(row[j]);
      }
      first = first && lhs >= rhs;
      second = second || lhs > rhs;
      if (!first) {
         break;
      }
   }

   return first && second;
}

bool helper(int what, ...)
{
   static double **original = NULL, **use = NULL;
   static int order = 0;
   bool result = false;
   switch (what) {
   case 0:
      original = NULL;
      use = NULL;
      order = 0;
      break;
   case 1:
      {
         va_list vl;
         va_start(vl, what);
         order = va_arg(vl, int);
         original = va_arg(vl, double **);
         use = va_arg(vl, double **);
         va_end(vl);
      }
      break;
   case 2:
      {
         va_list vl;
         int *arrangement = NULL, i;
         va_start(vl, what);
         arrangement = va_arg(vl, int *);
         va_end(vl);
         for (i = 0; i < order; ++i) {
            use[i] = original[arrangement[i]];
         }
         result = isDiagonallyDominant(order, use);
      }
      break;
   default:
      printf("Something wrong dude!\n");
   }
   return result;
}

bool permuteAndCheck(int n, int *order, int curr)
{
   bool result = false;
   if (n == curr + 1) {
      int i;
      printf("  Checking for order { %d", order[0]);
      for (i = 1; i < n; ++i) {
         printf(", %d", order[i]);
      }
      printf(" }\n");
      result = helper(2, order);
   } else {
      int i;
      for (i = curr; i < n; ++i) {
         { int temp = order[i]; order[i] = order[curr]; order[curr] = temp; }
         result = permuteAndCheck(n, order, curr + 1);
         if (result) { break; }
         { int temp = order[i]; order[i] = order[curr]; order[curr] = temp; }
      }
   }
   return result;
}

bool permuteMatrixForDiagonalDominance(int n, double **a, int *order)
{
   if (n < 2) { return false; }
   bool result = false;
   double **rows = malloc(sizeof(double *) * n);
   memcpy(rows, a, sizeof(double *) * n);
   helper(1, n, a, rows);
   result = permuteAndCheck(n, order, 0);
   helper(0);
   free(rows); rows = NULL;
   if (!result) {
      int i;
      for (i = 0; i < n; ++i) {
         order[i] = i;
      }
   }
   return result;
}

int main(int argc, char *argv[])
{
   int n, nn, i, j, ni;
   int ret;
   double **A = NULL, *mem = NULL;
   double *B = NULL, *x1 = NULL, *x2 = NULL;
   double *err = NULL, errmax;
   FILE *fp = NULL;
   double omega = 1.0;
   int num_it;
   int init = 0;
   bool diagonallyDominant = false;

   if (argc < 2) {
      printf("Wrong calling format\n");
      exit(1);
   }

   fp = fopen(argv[1], "r");
   if (NULL == fp) {
      printf("Cannot open file \"%s\"\n", argv[1]);
      exit(1);
   }

   ret = fscanf(fp, "%lf", &omega);
   if (omega < 0.0) { omega *= -1.0; }
   if (omega > 1.0) { omega = fmod(omega, 1.0); }

   ret = fscanf(fp, "%d", &n);
   if (0 == ret) {
      exit(1);
   }
   if (n < 1) {
      printf("Use valid number of independent variables (%d)\n", n);
      exit(1);
   }
   nn = n * n;

   mem = malloc(sizeof(double) * nn);
   A = malloc(sizeof(double *) * n);
   for (i = 0; i < n; ++i) {
      A[i] = mem + n * i;
   }

   B = malloc(sizeof(double) * n);

   x1 = malloc(sizeof(double) * n);
   x2 = malloc(sizeof(double) * n);
   err = malloc(sizeof(double) * n);

   for (i = 0; i < nn; ++i) {
      ret = fscanf(fp, "%lf", &(A[0][i]));
   }

   for (i = 0; i < n; ++i) {
      ret = fscanf(fp, "%lf", &B[i]);
   }

   for (i = 0; i < n; ++i) {
      x2[i] = x1[i] = 0.0 + ((double) (i % 9));;
   }
   ret = fscanf(fp, "%d", &init);
   if (0 != ret && 0 != init) {
      for (i = 0; i < n; ++i) {
         ret = fscanf(fp, "%lf", &x1[i]);
         x2[i] = x1[i];
         if (0 == ret) {
            break;
         }
      }
   }
   fclose(fp); fp = NULL;

   diagonallyDominant = isDiagonallyDominant(n, A);
   if (!diagonallyDominant) {
      int *reorder = malloc(sizeof(int) * n);
      for (i = 0; i < n; ++i) {
         reorder[i] = i;
      }
      printf("Matrix is not diagonally dominant. Trying to re-order...\n");
      diagonallyDominant = permuteMatrixForDiagonalDominance(n, A, reorder);
      if (diagonallyDominant) {
         double *temp = malloc(sizeof(double) * n);
         memcpy(temp, B, sizeof(double) * n);
         for (i = 0; i < n; ++i) {
            A[i] = mem + n * reorder[i];
            B[i] = temp[reorder[i]];
         }
         free(temp); temp = NULL;
         printf("Matrix is re-ordered to be diagonally dominant.\n");
      }
      free(reorder); reorder = NULL;
   }
   if (diagonallyDominant) {
      printf("Matrix is diagonally dominant, solution will converge\n");
      omega = 1.0 + omega;
   }
   else {
      printf("Matrix is not diagonally dominant, solution may not converge\n");
      omega = 1.0 - omega;
   }

   printf("Usinge relaxation parameter = %.2lf\n", omega);

   num_it = 0;

   printf("Ready\nIterate: ");
   ret = scanf("%d", &ni);
   while (ni > 0) {
      if (0 == num_it % 10) {
         int use = MIN(3,n);
         printf("Absolute relative approximation error:\niteration");
         for (i = 0; i < use; ++i) {
            printf("          x%d", i);
         }
         printf("         max\n");
      }
      ++num_it;
      for (i = 0; i < n; ++i) {
         double val = B[i];
         for (j = 0; j < i; ++j) {
            val -= x2[j] * A[i][j];
         }
         for (j = i + 1; j < n; ++j) {
            val -= x2[j] * A[i][j];
         }
         if (fabs(val) < 1.0e-6) {
            val = 0.0;
         } else if (0 != A[i][i]) {
            val /= A[i][i];
         }
         x2[i] = (1 - omega) * x2[i] + omega * val;
      }
      /* absolute relative approximation error:
       *  | (x2[i] - x1[i]) / x2[i] |
       */
      for (i = 0; i < n; ++i) {
         if (fabs(x2[i]) < 1.0e-5) {
            x2[i] = 0.0;
         } else if (fabs(x2[i]) > 1.0e120) {
            x2[i] = x2[i] / 10.0;
         }

         if (fabs(x2[i] - x1[i]) < 1.0e-6) {
            err[i] = 0.0;
         } else if (fabs(x2[i]) < 1.0e-6) {
            err[i] = 1.0;
         } else {
            err[i] = fabs((x2[i] - x1[i]) / x2[i]);
         }

         x1[i] = x2[i];
      }

      errmax = err[0];
      for (i = 1; i < n; ++i) {
         errmax = MAX(errmax,err[i-1]);
      }
      {
         int use = MIN(3,n);
         printf("%9d", num_it);
         for (i = 0; i < use; ++i) {
            printf("   %1.3e", err[i]);
         }
         printf("   %1.3e\n", errmax);
         if (errmax < 1.0e-5) {
            printf("Solution has converged.\n");
            break;
         }
      }
      --ni;
      if (0 == ni) {
         printf("Current Values:\n");
         for (i = 0; i < n; ++i) {
            printf("x%d = %.3e\n", i, x2[i]);
         }
         printf("Iterate: ");
         ret = scanf("%d", &ni);
      }
   } /* while (ni > 0) */

   free(err); err = NULL;
   free(x1); x1 = NULL;
   free(B); B = NULL;
   free(A); A = NULL;
   free(mem); mem = NULL;

   if (num_it > 0 || 0 != init) {
      printf("\n\nFinal values:\n");
      for (i = 0; i < n; ++i) {
         printf("x%d = %.3e\n", i, x2[i]);
      }
   }
   free(x2); x2 = NULL;
   printf("Done!\n");
   return 0;
}
