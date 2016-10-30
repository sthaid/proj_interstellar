// XXX LICENSE
//
// Usage: interstellar [-v] [distance] [time]
//           -v:       verbsoe
//           distance: default is 10 Light Years
//           time:     default is 100 Years
//

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

//
// defines
//

#define VELOCITY_WARNING  0.4  // C   - print warning if velocity of spaceship exceeds this
#define C                 3E8  // meters/sec

#define DEFAULT_DISTANCE     LIGHT_YEARS_TO_METERS(10);
#define DEFAULT_TIME         YEARS_TO_SECONDS(100);
#define DEFAULT_MASS_OF_SHIP 1E6   // kg  - about twice the International Space Station)

#define LIGHT_YEARS_TO_METERS(x) ((double)(x) * (C * 86400 * 365))
#define METERS_TO_LIGHT_YEARS(x) ((double)(x) / (C * 86400 * 365))

#define YEARS_TO_SECONDS(x) ((double)(x) * (365. * 86400))
#define SECONDS_TO_YEARS(x) ((double)(x) / (365. * 86400))

// The energy output from the Palo Verde Nuclear power plant in Arizone, which has the largest
// generating capacity of Nuclear power plant in the US.
// Refer to: http://www.americangeosciences.org/critical-issues/faq/how-much-electricity-does-typical-nuclear-power-plant-generate
//
// This power plant's power output is 3937 MegaWatts, 
// multipling by 24*365 gives units of MegaWattHours/Year,
// and multiplying by 3.6E9 Joules/MegaWattHour gives units of Joules/Year
#define PALO_VERDE_JOULES_PER_YEAR (3937.0 * 24 * 365 * 3.6E9)   // 1.24E17 joules/yr - equiv mass = 1.4 kg

//
// variables
//

bool Verbose = false;  // XXX getopt and get args

//
// prototypes
//

void help(void);
void constant_acceleration_spaceship_simulation(double Distance, double Time, double Mship, double Vthrust,
    double * RetMthrust, double * RetEnergy, double * RetVmax, double * RetKEshipmax, double * RetKEthrustmax);

// -----------------  MAIN  --------------------------------------------------------

int main(int argc, char **argv)
{
    double Distance, Time, Mship, Vthrust;
    double RetMthrust, RetEnergy, RetVmax, RetKEshipmax, RetKEthrustmax;
    char   opt_char;

    // parse options
    while (true) {
        opt_char = getopt(argc, argv, "vh");
        if (opt_char == -1) {
            break;
        }
        switch (opt_char) {
        case 'v':
            Verbose = true;
            break;
        case 'h':
            help();
            return 0;
        default:
            return 1;
        }
    }

    // if args are supplied for Distance and Time then use them
    Distance = DEFAULT_DISTANCE;
    Time     = DEFAULT_TIME;
    Mship    = DEFAULT_MASS_OF_SHIP;
    if (argc-optind >= 1) {
        if (sscanf(argv[optind], "%lf", &Distance) != 1) {
            printf("ERROR: invalid Distance arg '%s'\n", argv[optind]);
        }
        Distance = LIGHT_YEARS_TO_METERS(Distance);
        optind++;
    }
    if (argc-optind >= 1) {
        if (sscanf(argv[optind], "%lf", &Time) != 1) {
            printf("ERROR: invalid Time arg '%s'\n", argv[optind]);
        }
        Time = YEARS_TO_SECONDS(Time);
        optind++;
    }
    if (argc-optind >= 1) {
        if (sscanf(argv[optind], "%lf", &Mship) != 1) {
            printf("ERROR: invalid Mass of Ship arg '%s'\n", argv[optind]);
        }
        optind++;
    }

    // XXX check args

    // print settings
    printf("Constant Acceleration Spaceship Simulation:\n");
    printf("- Distance To Destination         = %.2f LY\n", METERS_TO_LIGHT_YEARS(Distance));
    printf("- Time To Destination             = %.2f Years\n", SECONDS_TO_YEARS(Time));
    printf("- Mass of Ship (not incl Mthrust) = %.2f Million Kg\n", Mship/1000000);
    printf("\n");
    
    // simulate the trip for a range of thrust velocities, and print results
    // XXX table form when not verbose
    for (Vthrust = 0.1*C; Vthrust < C; Vthrust += 0.1*C) {
        constant_acceleration_spaceship_simulation(
            Distance, Time, Mship, Vthrust,
            &RetMthrust, &RetEnergy, &RetVmax, &RetKEshipmax, &RetKEthrustmax);
        printf("Vthrust = %4.2f C  Mthrust/Mship = %5.2f  Energy = %6.0f PaloVerdes  " 
               "Vmax = %4.2f C  KEmax = %6.0f + %6.0f PaloVerdes\n",  
               Vthrust / C, 
               RetMthrust / Mship, 
               RetEnergy / PALO_VERDE_JOULES_PER_YEAR,
               RetVmax / C,
               RetKEshipmax / PALO_VERDE_JOULES_PER_YEAR,
               RetKEthrustmax / PALO_VERDE_JOULES_PER_YEAR);
        if (Verbose) {
            printf("\n");
        }
    }

    // done
    return 0;
}

void help(void)
{
    printf("\n");
    printf("USAGE: interstellar [-v] [distance] [time]\n");
    printf("          -v:       verbsoe\n");
    printf("          distance: default is 10 Light Years\n");
    printf("          time:     default is 100 Years\n");
    printf("\n");
    printf("NOTES\n");
    printf("- PaloVerdes is a unit of energy that I've defined. It equals the yearly energy output\n");
    printf("  of the Palo Verdes nuclear power plant in Arizona assuming the plant is running \n");
    printf("  continuously at peak power for 1 year.\n");
    printf("\n");
    printf("- The simulation does not take into account the mass equivalent of the energy\n");
    printf("  stored on the spaceship. In some scenarios this mass could be significant.\n");
    printf("\n");
    printf("- Special Relativity is used when calculating the momemtum and kinetic energy of the thrust.\n");
    printf("\n");
    printf("- The spaceship is assumed to be not substantially relativistic, and Special Relativity \n");
    printf("  is not used to calculate the spaceship's Distance, Time, and Velocity. A warning\n");
    printf("  message is printed if the spaceship's velocity exceeds 0.4C, Speed of  0.4C would\n");
    printf("  have approximately 10%% deviation between Newtonian mechanics and Special Relativity.\n");
    printf("\n");
}

// -----------------  INTERSTELLAR SPACESHIP SIMULATION  ---------------------------

// XXX comments

void constant_acceleration_spaceship_simulation(
    double Distance, 
    double Time, 
    double Mship, 
    double Vthrust,
    double * RetMthrust,      // mass of thrust at the begining of the trip
    double * RetEnergy,       // the total kinetic energy given to the thrust
    double * RetVmax,         // maximum velocity of the spaceship
    double * RetKEshipmax,    // maximum kinetic energy of the spaceship
    double * RetKEthrustmax)  // maximum kinetect energy of the thrust mass
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
