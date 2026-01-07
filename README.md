# FalconCalc
Multi language Windows scientist's and programmer's calculator featuring arbitrary precision decimal arithmetic, built in mathematical and physics constants, calculation history, user definable constants and functions and more.
# FalconCalcQt
Qt based version of the calculator for multi platform (Windows, Linux, OsX) usage, with more options than the windows-only version (e.g. light, dark, black, blue styles)
# Screenshots

![falconcalc1](https://github.com/user-attachments/assets/24b50146-f20c-4fcf-b2ea-e351b4cf5240)
![falconcalc2](https://github.com/user-attachments/assets/d280df82-9c94-45a7-84d0-f9dd8633fbf2)

# Description
  Every OS has a desktop calculator. These usually works like the physical calculators did, with buttons, separate normal and scientific modes. 

  The open source FalconCalc offers many unique features not found in those:

    - has no numeric and function buttons, you just enter an expression.

    - expressions may contain built-in and *user defined* constants and functions of any number of arguments

    - it continuously evaluates expressions as they are entered.

    - numerical precision: 60 significant digits, max displayable 55 digits

    - multilingual: presently two in-built languages: English and Hungarian

    In the input:

    - *Numbers* may be entered in decimal, hexadecimal, octal or binary notation or as character strings.
             Examples       number string        what it is          decimal value
             -----------------------------------------------------------------------
                           123.4567890e+2       decimal number       12345.67890
                           0x1234567890ABCDEF - hexadecimal number   1,311,768,467,294,899,695
                           012345670          - octal number         2,739,128
                           #101010101         - binary number        341
                           'FalconCalc'       - character string     1.5610851718... x 10^45

    - Results are displayed simultaneously as decimal, hexadecimal, octal and binary numbers and as a 
      string of characters. Either of these can be copied tothe clipboard.

    - Decimal formats: general, scientific and engeneering

    - Decimal display formats:  normal (1.2·10³), html (1.2x10<sup>3</sup>), TEX (1.2\cdot10^{8}) and 'E' (1.2E3)
      with 
      (Inputs always use the 'E' format)

    - Hexadecimal formats: normal or grouped as bytes, words or dwords each with or without the 0x prefix. 
      Also as Big- or little endian, unsigned or signed numbers.
	  For smaller numbers that fits inside an IEEE754 single or double number the hexadecimal representation for
	  that format can also be selected. (For large numbers the display will be incorrect!)

    - Angles for triginometric functions can be entered as degrees, radians, gradians or turns.
	  (The full circle is 360°, 2π, 400 grad, 1 turn)

    - Formulas may contain the following operators (+ and - may be unary):
          arithmetics: +, -, *, /, ^(power), | (or 'or'), & (or 'and'), xor, << (or 'shl' - shift left), 
                        << (or 'shl' - shift right),% (or mod - remainder), ! (or 'not') 
          logical: ==, != (not equal), <, >, <=, >= (these results in 1 or 0)

    - Built-in functions alternative names separated by slashes ('/'), argument(s) in ():
			abs(x), arcsin/asin(x), arccos/acos(x), arctan/atan(x), cos(x), cosh/ch(x), coth/cth(x), exp(x), fact(n), frac(x),"
			int(x), log/ln(x), log2(x), log10/lg(x), pow(base,power), root(n,x), root3(x), round(n,x), sign(x), sin(x), sinh/sh(x),"
			sqrt(x), tan/tg(x), trunc(x)"

    - Built in mathematical constants:
           e (base of the natural logarithm), log2e (base 2 logarithm of e), log10e (or lge - base
           10 logarithm of e), ln2 (natural logarithm of 2)
           pi(or π), pi2 (π/2), pi4 (π/4), ppi (1/π), tpi (2/√π), sqpi (√π), 
           sqrt2 (√2), psqrt2 (1/√2)), sqrt3 (√3), sqrt3P2 (√3/2)

    - Built-in physical constants (units in []):
           a0 - Bohr radius, au - astronomical unit [m], c - speed of light in vacuum (definition - exact value) [m/s], 
           h  - Planck constant  [Js], hbar - reduced Planck constant [Js], 
           qe - elementary charge [As], me - electron mass [kg], mp - proton mass [kg], 
           u - atomic mass unit [1], k - Boltzmann constant [J/K],G  - Newtonian constant of gravitation [m^3/kg/s^2], 
           gf - mean value of the gravitational acceleration on Earth (9.81 m/s²)
           eps0 - electric constant (vacuum permittivity) exact value [F /m]
           mu0  - magnetic constant (vacuum permeability) exact value [N/A^2] = 4π·10^(-7), 
           kc  - Coulomb's constant 1/4π eps0 [N m^2/C^2], LA  - Avogadro's number [1/mol],
           rf - Earth's radius [m], rg  - molar gas constant (8.31 J/ mol K), rs - Sun's radius
           u - atomic mass unit

    - Any number of user variables/constants and functions may be defined with any valid arithmetic formula 
      including other constants and variables.

    - When a user variable is modified the value of all dependent user variables and functions are automatically changed

    - User defined functions may have any number of arguments with any names that is different from the name of any 
      built-in function or constant.
