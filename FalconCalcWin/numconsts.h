#ifndef numconsts_H
 #define numconsts_H

#include <string>
#include <limits>
#include <locale>

/*======================================================
 * Problem: using Microsoft and 32 bit compilers
 *----------------------------------------------------*/

namespace numeric {

struct NumConstants
{
   RealNumber e;			// e
   RealNumber log2e;		// log_2(e)
   RealNumber log10e;		// log10(e)
   RealNumber ln2;			// log_e 2
   RealNumber lge;			// log_e
   RealNumber pi;			// pi
   RealNumber pi2;		// pi/2
   RealNumber pi4;		// pi/4
   RealNumber ppi;		// 1/pi
   RealNumber ppi2;		// 2/pi
   RealNumber sqpi;		// sqrt(pi)
   RealNumber sqrt2;		// sqrt(2)
   RealNumber psqrt2;	// 1/sqrt(2)
		// physical constants
   RealNumber c;           // speed of light in vacuum definition - exact value [m/s]
   RealNumber h;			// Planck constant  [Js]
   RealNumber hbar;		// reduced Planck constant [Js]
   RealNumber qe;			// elementary charge [As]
   RealNumber me;			// electron mass [kg]
   RealNumber mp;			// proton mass [kg]
   RealNumber u;			// atomic mass unit [1]
   RealNumber k;			// Boltzmann constant [J/K]
   RealNumber G;			// Newtonian constant of gravitation [m^3/kg/s^2]
   RealNumber eps0;		// electric constant (vacuum permittivity) defined [F /m]
   RealNumber mu0;			// magnetic constant (vacuum permeability) defined [N/A^2] = 4 pi 1e-7
   RealNumber kc;			// Coulomb's constant 1/4pi eps0 [N m^2/C^2]
   RealNumber LA;			// Avogadro's number [1/mol]

   _CHART decpoint;			// decimal point in current locale
   _CHART func_arg_separator; // usually ',' unless the decimal point is the comma when it is ';'
   const RealNumber NaN() const { return std::numeric_limits<RealNumber>::quiet_NaN(); };
   bool isNaN(double d) { return d != d; }

   NumConstants():
				 e(2.7182818284590452353602874713526625L), // e
			 log2e(1.4426950408889634073599246810018921L), // log_2(e)
		    log10e(0.4342944819032518276511289189166051L), // log10(e)
               ln2(0.6931471805599453094172321214581766L), // log_e 2
			   lge(2.3025850929940456840179914546843642L), // log_e
			    pi(3.1415926535897932384626433832795029L), // pi
 			   pi2(1.5707963267948966192313216916397514L), // pi/2
			   pi4(0.7853981633974483096156608458198757L), // pi/4
			   ppi(0.3183098861837906715377675267450287L), // 1/pi
		      ppi2(0.6366197723675813430755350534900574L), // 2/pi
		      sqpi(1.7724538509055160272981674833411452L), // sqrt(pi)
		     sqrt2(1.4142135623730950488016887242096981L), // sqrt(2)
		    psqrt2(0.7071067811865475244008443621048490L), // 1/sqrt(2)
		 // physical constants
		 c(299792458.0),			// speed of light in vacuum definition - exact value [m/s]
		 h(6.6260695729e-34),		// Planck constant  [Js]
		 hbar(1.05457172647e-34),   // reduced Planck constant [Js]
		 qe(1.60217656535e-19),		// elementary charge [As]
		 me(9.1093829140e-31),		// electron mass [kg]
		 mp(1.67262177774e-27),		// proton mass [kg]
		 u(1.6605389217e-27),		// atomic mass unit [1]
		 k(1.380648813e-23),		// Boltzmann constant [J/K]
		 G(6.6738480e-11),           // Newtonian constant of gravitation [m^3/kg/s^2]
		 eps0(8.854187817e-12),		// electric constant (vacuum permittivity) defined [F /m]
		 mu0(1.25663706143592e-6),	// magnetic constant (vacuum permeability) defined [N/A^2] = 4 pi 1e-7
		 kc(8.987551787e9),			// Coulomb's constant 1/4pi eps0 [N m^2/C^2]
		 LA(6.0221412927e23)        // Avogadro's number [1/mol]
		 {
            decpoint = std::use_facet<std::numpunct<char> >(std::cout.getloc()).decimal_point ();
            func_arg_separator = (decpoint == ',' ? ';' : ',');
	     }
};


extern const NumConstants constants; // we need this at every include of this file

} // end of namespace numeric
#endif
