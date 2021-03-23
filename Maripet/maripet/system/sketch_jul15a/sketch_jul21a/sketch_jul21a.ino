#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SAMPLE (1000)
int    S;
double x, y, x1, x2, y1, y2;
double B0, a1, a2, b1, c0, F0, T;
void setup(){
  
}

void loop(){
   S  = SAMPLE;
        F0 = S/10.;
        B0 = 1;
        if (argc>1)
          F0 = atof(argv[1]);
        if (argc>2)
          B0 = atof(argv[2]);
        if (argc>3)
          S  = atoi(argv[3]);
        x1 = x2 = y1 = y2 = 0;

        T  = 1./S;
        a1 = 2*exp(-M_PI*B0*T)*cos(2*M_PI*F0*T);
        a2 = -exp(-2*M_PI*B0*T);
        b1 = -2*cos(2*M_PI*F0*T);
        c0 = (1-a1-a2)/(2+b1);

        while (scanf("%lf", &x) != EOF) {
          y = a1*y1+a2*y2+c0*(x+b1*x1+x2);
          x2 = x1;
          x1 = x;
          y2 = y1;
          y1 = y;
          printf("%f\n",y);
        }
}
