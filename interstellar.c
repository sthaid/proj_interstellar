#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/**
Model 1 - Constant acceleration with turnaround at midpoint.

The midpoint (S/2) will be reahced in half the time (T/2) to the destination.

                       2
    S      1      ( T ) 
   --- =  --- A x (---) 
    2      2      ( 2 )

Solving for T:

   T = sqrt(4 x S / A)

Using conservation of momentum the relationship between the momentum of the spacecraft
and the momentum of the thrust mass is

    M x dV = dM x Vt
           where XXX

Dividing by dT   XXX use dt

         dV     dM
    M x ---- = ---- x Vthrust
         dT     dT

             dM
    M x A = ---- x Vthrust
             dT

    dM       A
   ---- = ------- x dT                                           next Integrate both sides
    M     Vthrust

               A
    ln(M) = -------  x T + C                                     next exponentiate
            Vthrust 

               (A / Vthrust) x T
    M = C' * e                                                   when T=0, M=(Mship+Mthrust)
                                                                 thus C=(Mship+Mthrust)

                              (A / Vthrust) x T                  when T = sqrt(4S/A) then M = Mship,
    M = (Mship + Mthrust) * e                                    in other words, when the destination is reached
                                                                 all the mass used for thrust is gone

                                  (A / Vthrust) * sqrt(4S/A)      
    Mship = (Mship + Mthrust) * e                                Solving for Mthrust

                1
    Mthrust = (--- - 1) * Mship                                  where K =  exp(-sqrt(4SA/Vthrust^2))
                K

                 sqrt(4SA/Vthrust^2)
    Mthrust = (e                      - 1) * Mship


Let's try this with some values

    Mship   = 1E6 kg
    S       = 10 lightyears = 9.4608E16 m
    A       = 0.1 m/s
    Vthrust = 1E8 m/s

    Mthrust = 5.996 * 1E6 = 5.996E6 kg
    T = sqrt(4 * S / A) = 1.95E9 seconds = 61.686 years
    Tflip = T/2 = 30.843
**/

#define C (3E8)
#define LIGHT_YEARS_TO_METERS(x) ((double)(x) * (C * 86400 * 365))
#define METERS_TO_LIGHT_YEARS(x) ((double)(x) / (C * 86400 * 365))

#define YEARS_TO_SECONDS(x)   ((double)(x) * (365. * 86400))
#define SECONDS_TO_YEARS(x)   ((double)(x) / (365. * 86400))

bool constant_acceleration_model_ex(double A, double Mship, double Mthrust, double Vthrust, double Tflip, bool PrintFlag);
void constant_acceleration_model(double A, double Mship, double Mthrust, double Vthrust);

int main(int argc, char **argv)
{
    double S, A, Mship, Mthrust, Vthrust;
    double Tflip;

    S        = LIGHT_YEARS_TO_METERS(10);   // m
    A        = 0.1;                         // m/sec
    Mship    = 1E6;                         // kg
    Vthrust  = 1E8;                         // m/sec
    Mthrust  = (exp(sqrt(4*S*A/Vthrust/Vthrust)) - 1) * Mship;
    Tflip    = sqrt(4.*S/A) / 2;
    constant_acceleration_model_ex(A, Mship, Mthrust, Vthrust, Tflip, true);

    printf("\n");

    A = 0.01;                         // m/sec
    constant_acceleration_model(A, Mship, Mthrust, Vthrust);

    return 0;
}

void constant_acceleration_model(double A, double Mship, double Mthrust, double Vthrust) 
{
    double  TflipLow, TflipHigh, TflipCheck;
    int32_t i;
    bool    TflipIsTooLarge;

    TflipLow  = YEARS_TO_SECONDS(0);
    TflipHigh = YEARS_TO_SECONDS(10);
    while (true) {
        TflipIsTooLarge = constant_acceleration_model_ex(A, Mship, Mthrust, Vthrust, TflipHigh, false);
        if (TflipIsTooLarge) {
            break;
        }
        TflipLow  += YEARS_TO_SECONDS(10);
        TflipHigh += YEARS_TO_SECONDS(10);
    }

    for (i = 0; i < 20; i++) {
        TflipCheck = (TflipLow + TflipHigh) / 2;
    
        TflipIsTooLarge = constant_acceleration_model_ex(A, Mship, Mthrust, Vthrust, TflipCheck, false);

        if (TflipIsTooLarge) {
            TflipHigh = TflipCheck;
        } else {
            TflipLow = TflipCheck;
        }
    }

    constant_acceleration_model_ex(A, Mship, Mthrust, Vthrust, TflipCheck, true);
}

bool constant_acceleration_model_ex(double A, double Mship, double Mthrust, double Vthrust, double Tflip, bool PrintFlag)
{
    #define PRINT_INTVL (365 * 86400)

    double M, V, T, S, E;
    double DeltaT, DeltaV, DeltaM, DeltaE;

    if (PrintFlag) {
        printf("Mass of Ship     = %.2f kg\n", Mship);
        printf("Mass of Thrust   = %.2f kg\n", Mthrust);
        printf("Mass Thrust/Ship = %.2f\n", Mthrust / Mship);
        printf("Thrust Velocity  = %.2f C\n", Vthrust/C);
        printf("Flip Time        = %.2f Years\n", SECONDS_TO_YEARS(Tflip));
        printf("\n");
        printf("        Time     Distance   ThrustTank     Velocity       Energy\n");
        printf("     (Years)  (LightYear)    (Percent)          (C)     (Joules)\n");
        printf("     -------  -----------    ---------      -------     --------\n");
    }

    M = Mship + Mthrust;
    V = 0;
    T = 0;
    S = 0;
    E = 0;

    DeltaT = 3600;  // 1 hour
    DeltaV = A * DeltaT;

// 1 megawatt hour = 3.6E9 joules
// Palo Verde  4000 MW
// Palo Verde  4000 MW * 24*365  hours/year   =   3.5E7 MWH / year
//                                            =   1.26E17 joules / year


    while (true) {
        if ((PrintFlag) &&
            (M <= Mship || ((uint64_t)T % PRINT_INTVL) == 0)) 
        {
            printf("%12.2f %12.2f %12.2f %12.2f %12.2f\n",
                    SECONDS_TO_YEARS(T),
                    METERS_TO_LIGHT_YEARS(S),
                    100 * (M - Mship) / Mthrust,
                    V / C,
                    E / 1.26E17);
            E = 0;
        }

        if (M <= Mship) {
            break;
        }

        DeltaM = (M * DeltaV) / Vthrust;   // XXX SRT
        DeltaE = 0.5 * DeltaM * Vthrust * Vthrust;   // XXX SRT   XXX zero loss?

        V += (T < Tflip ? DeltaV : -DeltaV);
        M -= DeltaM;
        T += DeltaT;
        S += V * DeltaT;
        E += DeltaE;
    }

    bool TflipIsTooLarge = (V > 0);
    return TflipIsTooLarge;
}
