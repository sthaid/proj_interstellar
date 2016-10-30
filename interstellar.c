#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#define C       3E8  // m/sec
#define M_SHIP  1E6  // kg

#define LIGHT_YEARS_TO_METERS(x) ((double)(x) * (C * 86400 * 365))
#define METERS_TO_LIGHT_YEARS(x) ((double)(x) / (C * 86400 * 365))

#define YEARS_TO_SECONDS(x)   ((double)(x) * (365. * 86400))
#define SECONDS_TO_YEARS(x)   ((double)(x) / (365. * 86400))

//http://www.americangeosciences.org/critical-issues/faq/how-much-electricity-does-typical-nuclear-power-plant-generate
//The Palo Verde plant in Arizona has three reactors and has the largest combined generating capacity1 of about 3,937 MW. 
// 1 megawatt hour = 3.6E9 joules
// Palo Verde  4000 MW
// Palo Verde  4000 MW * 24*365  hours/year   =   3.5E7 MWH / year
//                                            =   1.26E17 joules / year

#define PALO_VERDE_JOULES_PER_YEAR (3937.0 * 24 * 365 * 3.6E9)   // 1.24E17 joules/yr - equiv mass = 1.4 kg
#define VELOCITY_WARNING  0.4

bool Verbose = false;  // XXX getopt and get args

void constant_acceleration_spaceship_simulation(double Distance, double Time, double Mship, double Vthrust,
    double * RetMthrust, double * RetEnergy, double * RetVmax, double * RetKEshipmax, double * RetKEthrustmax);

int main(int argc, char **argv)
{
    double Distance, Time, Vthrust;
    double RetMthrust, RetEnergy, RetVmax, RetKEshipmax, RetKEthrustmax;

    // read the Distance and Time that the simulated spaceship will travel
    Distance = LIGHT_YEARS_TO_METERS(10);
    Time = YEARS_TO_SECONDS(100);

    printf("Constant Acceleration Spaceship Simulation:\n");
    printf("- Distance To Destination         = %.2f LY\n", METERS_TO_LIGHT_YEARS(Distance));
    printf("- Time To Destination             = %.2f Years\n", SECONDS_TO_YEARS(Time));
    printf("- Mass of Ship (not incl Mthrust) = %.2f Million Kg\n", M_SHIP/1000000);
    printf("\n");

    // simulate the trip for a range of thrust mass values, and print results
    for (Vthrust = 0.1*C; Vthrust < C; Vthrust += 0.1*C) {
        constant_acceleration_spaceship_simulation(
            Distance, Time, M_SHIP, Vthrust,
            &RetMthrust, &RetEnergy, &RetVmax, &RetKEshipmax, &RetKEthrustmax);
        printf("Vthrust = %4.2f C  Mthrust/Mship = %5.2f  Energy = %6.0f PaloVerdes  " 
               "Vmax = %4.2f C  KEmax = %6.0f + %6.0f PaloVerdes\n",   // XXX msg on PaloVerdes    XXX table?
               Vthrust / C, 
               RetMthrust / M_SHIP, 
               RetEnergy / PALO_VERDE_JOULES_PER_YEAR,
               RetVmax / C,
               RetKEshipmax / PALO_VERDE_JOULES_PER_YEAR,
               RetKEthrustmax / PALO_VERDE_JOULES_PER_YEAR);
        if (Verbose) {
            printf("\n");
        }
    }


#if 0
    double A, Mship, Mthrust, Vthrust;
    //double Tflip;

    //S        = LIGHT_YEARS_TO_METERS(10);   // m
    A        = 0.01;                        // m/sec
    Mship    = 1E6;                         // kg
    Vthrust  = .99 * C;                    // m/sec
    Mthrust  = 3E4;   //(exp(sqrt(4*S*A/Vthrust/Vthrust)) - 1) * Mship / 10;
    //Tflip    = sqrt(4.*S/A) / 2;
    //constant_acceleration_model_ex(A, Mship, Mthrust, Vthrust, Tflip, true);
    //printf("\n");

    constant_acceleration_model(A, Mship, Mthrust, Vthrust);


ln (Mthrust / Mship) = sqrt(4*S*A/(Vthrst/K)/(Vthrust/K))


#endif

    return 0;
}

void constant_acceleration_spaceship_simulation(
    double Distance, 
    double Time, 
    double Mship, 
    double Vthrust,
    double * RetMthrust,
    double * RetEnergy,
    double * RetVmax,
    double * RetKEshipmax,
    double * RetKEthrustmax)
{
    #define PRINT_INTVL (365 * 86400)

    double Accel, TimeFlip, DeltaT, DeltaV, Mthrust, K;
    double M, V, T, S, E;
    bool FlipPrinted = false;
    double Vmax=0, KEshipmax=0, KEthrustmax=0;

    Accel    = 4. * Distance / (Time * Time);
    TimeFlip = Time / 2;
    K        = sqrt(1. - (Vthrust*Vthrust)/(C*C));
    Mthrust  = (exp(sqrt(4.*Distance*Accel/(Vthrust/K)/(Vthrust/K))) - 1) * Mship;
    DeltaT   = 3600;  // 1 hour
    DeltaV   = Accel * DeltaT;

    M = Mship + Mthrust;
    V = 0;
    T = 0;
    S = 0;
    E = 0;

    if (Verbose) {
        printf("Mass of Ship     = %.3f million kg\n", Mship/1000000); 
        printf("Mass of Thrust   = %.3f million kg\n", Mthrust/1000000);
        printf("Mass Thrust/Ship = %.2f\n", Mthrust / Mship);
        printf("Thrust Velocity  = %.2f C\n", Vthrust/C);
        printf("Acceleration     = %.2f m/s^2\n", Accel);
        printf("        Time     Distance   ThrustTank     Velocity       Energy\n");
        printf("     (Years)  (LightYear)    (Percent)          (C)  (PalVerdYr)\n");
        printf("     -------  -----------    ---------      -------  -----------\n");
    }

    while (true) {
        if (Verbose && (M <= Mship || ((uint64_t)T % PRINT_INTVL) == 0)) {
            printf("%12.2f %12.2f %12.2f %12.4f %12.2f",
                    SECONDS_TO_YEARS(T),
                    METERS_TO_LIGHT_YEARS(S),
                    100 * (M - Mship) / Mthrust,
                    V / C,
                    E / PALO_VERDE_JOULES_PER_YEAR);
            if (T > TimeFlip && !FlipPrinted) {
                printf("    FLIP");
                FlipPrinted = true;
            }
            if (V / C > VELOCITY_WARNING) {
                printf("    WARNING V/C = %.2f IS > %.2f", V/C, VELOCITY_WARNING);
            }
            printf("\n");
        }

        if (M <= Mship) {
            break;
        }

        double DeltaM = (M * DeltaV) / (Vthrust / K);
        double DeltaE = DeltaM * C * C * (1./K - 1.);

        V += (T < TimeFlip ? DeltaV : -DeltaV);
        M -= DeltaM;
        T += DeltaT;
        S += V * DeltaT;
        E += DeltaE;

        if (V > Vmax) {
            Vmax = V;
        }
        double KEship = 0.5 * Mship * V * V;
        if (KEship > KEshipmax) {
            KEshipmax = KEship;
        }
        double KEthrust = 0.5 * (M - Mship) * V * V;
        if (KEthrust > KEthrustmax) {
            KEthrustmax = KEthrust;
        }
    }

    if (fabs(V/C) > 0.01) {
        printf("WARNING: Final V = %.2f C\n", V/C);
    }
    if (fabs(METERS_TO_LIGHT_YEARS(S-Distance)) > 0.01) {
        printf("WARNING: Final Distance = %.2f LightYears\n", METERS_TO_LIGHT_YEARS(S));
    }
    if (fabs(SECONDS_TO_YEARS(T-Time)) > .01) {
        printf("WARNING: Final Time = %.2f Years\n", SECONDS_TO_YEARS(T));
    }
    if (Vmax/C > VELOCITY_WARNING) {
        printf("WARNING: Vmax = %.2f C\n", Vmax/C);
    }

    *RetMthrust     = Mthrust;
    *RetEnergy      = E;
    *RetVmax        = Vmax;
    *RetKEshipmax   = KEshipmax;
    *RetKEthrustmax = KEthrustmax;
}



/**
bool constant_acceleration_model_ex(double A, double Mship, double Mthrust, double Vthrust, double Tflip, bool PrintFlag)
{

    double M, V, T, S, E, MthrustExpelled;
    double DeltaT, DeltaV, DeltaM, DeltaE;

    bool FlipPrinted = false;

    if (PrintFlag) {
        printf("Mass of Ship     = %.3f million kg\n", Mship/1000000); 
        printf("Mass of Thrust   = %.3f million kg\n", Mthrust/1000000);
        printf("Mass Thrust/Ship = %.2f\n", Mthrust / Mship);
        printf("Acceleration     = %.2f m/s^2\n", A);
        printf("Thrust Velocity  = %.2f C\n", Vthrust/C);
        //printf("Flip Time        = %.2f Years\n", SECONDS_TO_YEARS(Tflip));
        printf("\n");
        printf("        Time     Distance   ThrustTank     Velocity        Accel       Energy   ThrustMass\n");
        printf("     (Years)  (LightYear)    (Percent)          (C)      (m/s*2)     (XXXXXX)          (Kg)\n");
        printf("     -------  -----------    ---------      -------      -------     -------    -----------\n");
    }

    M = Mship + Mthrust;
    V = 0;
    T = 0;
    S = 0;
    E = 0;
    MthrustExpelled = 0;

    DeltaT = 3600;  // 1 hour
    DeltaV = A * DeltaT;



    while (true) {
        if ((PrintFlag) &&
            (M <= Mship || ((uint64_t)T % PRINT_INTVL) == 0)) 
        {
            printf("%12.2f %12.2f %12.2f %12.4f %12.2f %12.2f %12.2f",
                    SECONDS_TO_YEARS(T),
                    METERS_TO_LIGHT_YEARS(S),
                    100 * (M - Mship) / Mthrust,
                    V / C,
                    A,
                    E / PALO_VERDE_JOULES_PER_YEAR,
                    MthrustExpelled);
            if (T > Tflip && !FlipPrinted) {
                printf("    FLIP");
                FlipPrinted = true;
            }
            if (V / C > VELOCITY_WARNING) {
                printf("    WARNING V/C = %.2f IS > %.2f", V/C, VELOCITY_WARNING);
            }
            printf("\n");
            E = 0;
            MthrustExpelled = 0;
        }

        if (M <= Mship) {
            break;
        }

#if 0
        DeltaM = (M * DeltaV) / (Vthrust / K);   // XXX SRT
        // DeltaE = 0.5 * DeltaM * Vthrust * Vthrust;   // XXX SRT   XXX zero loss?
        DeltaE = DeltaM * C * C * (1./K - 1.);
#else
        DeltaM = (M * DeltaV) / (Vthrust);   // XXX SRT
        DeltaE = 0.5 * DeltaM * Vthrust * Vthrust;   // XXX SRT   XXX zero loss?
#endif

        V += (T < Tflip ? DeltaV : -DeltaV);
        M -= DeltaM;
        T += DeltaT;
        S += V * DeltaT;
        E += DeltaE;
        MthrustExpelled += DeltaM;
    }

    bool TflipIsTooLarge = (V > 0);
    return TflipIsTooLarge;
}


xxxxxxxxxxxxxxxx

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
**/
