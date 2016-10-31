/*
Copyright (c) 2016 Steven Haid

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

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

bool Verbose = false;

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
    printf("\
\n\
USAGE\n\
\n\
interstellar [-v] [distance] [time]\n\
    -v:       verbsoe\n\
    distance: default is 10 Light Years\n\
    time:     default is 100 Years\n\
\n\
OVERVIEW\n\
\n\
The simulated spaceship travels to it's destination at constant acceleration.\n\
At the midpoint the ship turns around so that the acceleration vector is reversed.\n\
The ship has a tank which contains the thrust mass. The thrust mass is expelled at\n\
a constant velocity. The rate that the thrust mass is depleted reduces throughout\n\
the trip because the total mass of the ship reduces during the trip.\n\
\n\
When the simulated spaceship arrives at it's destination (that is it has travelled the\n\
specified distance): \n\
  (a) the specified amount of time will have elapsed, \n\
  (b) the velocity of the spaceship will be zero\n\
  (c) the thrust mass tank will be empty\n\
\n\
The simulation will be repeated varying the velocity of thrust, and keeping the\n\
distance and time to the destination the same. This allows a comparison of the \n\
amount of thrust mass required and the amount of energy required as a function of \n\
the velocity of the thrust mass.\n\
\n\
The simulation does not take into account the mass equivalent of the energy\n\
stored on the spaceship. In some scenarios this mass could be significant, and\n\
could be taken into account to improve the accuracy of the simulation.\n\
\n\
Special Relativity is used when calculating the momemtum and kinetic energy of the\n\
 thrust mass.\n\
\n\
The spaceship is assumed to be not substantially relativistic, and Special Relativity \n\
is not used to calculate the spaceship's Distance, Time, Mass and Velocity. A warning\n\
message is printed if the spaceship's velocity exceeds 0.4C, Speed of 0.4C would\n\
have approximately 10%% deviation between Newtonian mechanics and Special Relativity.\n\
\n\
PaloVerdes is a unit of energy that I have invented. It equals the yearly energy output\n\
of the Palo Verdes nuclear power plant in Arizona assuming the plant is running \n\
continuously at peak power for 1 year.\n\
\n");
}

// -----------------  INTERSTELLAR SPACESHIP SIMULATION  ---------------------------

// OVERVIEW 
// --------

// The simulated spaceship travels to it's destination at constant acceleration.
// At the midpoint the ship turns around so that the acceleration vector is reversed.
// The ship has a tank which contains the thrust mass. The thrust mass is expelled at
// a constant velocity. The rate that the thrust mass is depleted reduces throughout
// the trip because the total mass of the ship reduces during the trip.
//
// When the simulated spaceship arrives at it's destination (that is it has travelled the
// specified distance): 
//   (a) the specified amount of time will have elapsed, 
//   (b) the velocity of the spaceship will be zero
//   (c) the thrust mass tank will be empty
//
// The simulation will be repeated varying the velocity of thrust, and keeping the
// distance and time to the destination the same. This allows a comparison of the 
// amount of thrust mass required and the amount of energy required as a function of 
// the velocity of the thrust mass.
//
// The simulation does not take into account the mass equivalent of the energy
// stored on the spaceship. In some scenarios this mass could be significant, and
// could be taken into account to improve the accuracy of the simulation.
// 
// Special Relativity is used when calculating the momemtum and kinetic energy of the 
// thrust mass.
// 
// The spaceship is assumed to be not substantially relativistic, and Special Relativity 
// is not used to calculate the spaceship's Distance, Time, Mass and Velocity. A warning
// message is printed if the spaceship's velocity exceeds 0.4C, Speed of 0.4C would
// have approximately 10%% deviation between Newtonian mechanics and Special Relativity.
//
// PaloVerdes is a unit of energy that I have invented. It equals the yearly energy output
// of the Palo Verdes nuclear power plant in Arizona assuming the plant is running 
// continuously at peak power for 1 year.
//
// ARGS
// ----
//
// Inputs
//   Distance     : distance of the trip
//   Time:        : duration of the trip
//   Mship        : mass of the ship
//   Vthrust      : velocity that the thrust mass is expelled
//
// Outputs
//   RetMthrust:      thrust mass at the begining of the trip
//   RetEnergy:       the total kinetic energy given to the thrust during the trip
//   RetVmax:         maximum velocity of the spaceship
//   RetKEshipmax:    maximum kinetic energy of the spaceship excluding the thrust mass
//   RetKEthrustmax:  maximum kinetect energy of the thrust mass
// 
// RELATIONSHIP BETWEEN DISTANCE, TIME, AND ACCELERATION
// -----------------------------------------------------
//
// Due to the constant accelleration we know that the ship must reverse thrust
// at the midpoint of the trip.  Thus:
//
//                                        2
//      Distance    1            ( Time ) 
//      -------- = --- x Accel x ( ---- )
//        2         2            (  2   )
//
// Solving for Accel
//
//              4 * Distance
//      Accel = ------------
//                Time^2
//
// CALCULATING THE AMOUNT OF THRUST MASS NEEDED
// --------------------------------------------
//
// At the begining of the trip the thrust tank will be filled with just enough mass
// so that the tank is empty coincident with reaching the destination.
//
// To solve for the amount of thrust mass needed the following calculates the amount
// of thrust mass needed for constant acceleration for a specified amount of time.
//
// From conservation of momemtum we have:
//      M x dV = dM x Vthrust
//  Where:
//      M is the total mass of the ship plus the mass in the thrust tank at time T
//      V is the velocity of the spaceship at time T
//      Vthrust is the velocity that the thrust mass is expelled, which is a constant
//
//  Dividing by dT (where T is the current time)
//                   dM
//      M x Accel =  -- x Vthrust
//                   dT
//
// Rearrange
//      dM     Accel
//      -- =  ------- x dT
//      M     Vthrust
//
// Integrate both sides, and solve for M
//
//              Accel
//      ln(M = ------- x T + C
//             Vthrust
//
//                (Accel / Vthrust) x T
//      M = C * e
//
// At the begining of the trip: T=0 and  M=Mship+Mthrust. 
// Therefor C=Mship+Mthrust.
//
//                                (Accel / Vthrust) x T
//      M = (Mship + Mthrust) * e
//
// At the end of the trip T=Time and M=Mship
//
//                                   (Accel / Vthrust) x Time
//      Mship = (Mship + Mthrust) * e
//
// Solving for Mthrust
//
//                (             1                  )
//      Mthrust = (---------------------------- - 1) x Mship
//                (  (Accel / Vthrust) x Time      )
//                ( e                              )
//
//                  -(Accel / Vthrust) x Time)
//      Mthrust = (e                            - 1) x Mship
//
// There seemse to be an error, because the above result yields a 
// negative value for Mthrust. This problem is resolved by examining the
// relatiionship between Accel and Vthrust. Accel is positive and Vthrust
// is negative.
//
// Let's keep this simple and use a positive value for Vthrust and eliminate
// the negative sign. The equation becomes:
//
//                  (Accel / Vthrust) x Time)
//      Mthrust = (e                         - 1) x Mship
//
// Finally, in classical mechanics, momentum is
//      p = m x v
// and in special relativity
//      p = m x v / K
//      K = sqrt(1 - v^2/c^2)
//
// So, to incorporate special relativity in the equation for Mthrust,
// replace Vthrust with Vthrust/K ...
//
//                  (Accel / (Vthrust / K)) x Time)
//      Mthrust = (e                              - 1) x Mship
//
//      WHere K = sqrt(1 - Vthrust^2 / c^2)
//

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

    // initialization
    Accel    = 4. * Distance / (Time * Time);
    TimeFlip = Time / 2;
    K        = sqrt(1. - (Vthrust*Vthrust)/(C*C));
    Mthrust  = (exp(Accel / (Vthrust/K) * Time) - 1) * Mship;
    DeltaT   = 3600;  // 1 hour
    DeltaV   = Accel * DeltaT;

    M = Mship + Mthrust;
    V = 0;
    T = 0;
    S = 0;
    E = 0;

    // print in verbose mode
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
        // print in verbose mode
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

        // if thrust tank is empty then the simulation is complete
        if (M <= Mship) {
            break;
        }

        // for this time interval:
        // - using conservation of momentum calculate the amount of thrust mass used in DeltaT inverval
        // - calculate the amount of kinetic energy of the thrust mass used in this interval
        double DeltaM = (M * DeltaV) / (Vthrust / K);
        double DeltaE = DeltaM * C * C * (1./K - 1.);

        // update current spaceship:
        // - velocity
        // - mass
        // - time
        // - distance
        // - energy imparted to the thrust
        V += (T < TimeFlip ? DeltaV : -DeltaV);
        M -= DeltaM;
        T += DeltaT;
        S += V * DeltaT;
        E += DeltaE;

        // keep track of maximum values, these will be returned to caller and printed out
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

    // sanity check that at the end of the trip the spaceship's:
    // - velocity is near 0
    // - distance travelled is near Distance
    // - elapsed time is near Time
    if (fabs(V/C) > 0.01) {
        printf("WARNING: Final V = %.2f C\n", V/C);
    }
    if (fabs(METERS_TO_LIGHT_YEARS(S-Distance)) > 0.01) {
        printf("WARNING: Final Distance = %.2f LightYears\n", METERS_TO_LIGHT_YEARS(S));
    }
    if (fabs(SECONDS_TO_YEARS(T-Time)) > .01) {
        printf("WARNING: Final Time = %.2f Years\n", SECONDS_TO_YEARS(T));
    }

    // if the maximum spaceship velocity is too large (where relativity effects > 10%) then
    // print a warning message
    if (Vmax/C > VELOCITY_WARNING) {
        printf("WARNING: Vmax = %.2f C\n", Vmax/C);
    }

    // return stats
    *RetMthrust     = Mthrust;
    *RetEnergy      = E;
    *RetVmax        = Vmax;
    *RetKEshipmax   = KEshipmax;
    *RetKEthrustmax = KEthrustmax;
}
