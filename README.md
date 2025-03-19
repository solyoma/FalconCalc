# FalconCalc
Windows scientist's and programmer's calculator featuring arbitrary precision decimal arithmetic, built in mathematical and physics constants, claculation history user definable constants and functions and more.
# Screenshot

![falconcalc1](https://github.com/solyoma/FalconCalc/assets/37068759/4abcf0bd-c4e4-4df7-a300-778cb7338cc9)

# Description
  Every Windows version has a desktop calculator with many features but FalconCalc offers many 
  unique features not found in them:

    - FalconCalc continuously evaluates expressions, that may contain built-in and 
      *user defined* constants and functions of any number of arguments, as they are entered.

    - *Numbers* may be entered in decimal, hexadecimal, octal or binary notation or as character strings.
             Examples       number string        what it is          decimal value
             -----------------------------------------------------------------------
                           123.4567890e+2       decimal number       12345.67890
                           0x1234567890ABCDEF - hexadecimal number   1,311,768,467,294,899,695
                           012345670          - octal number         2,739,128
                           #101010101         - binary number        341
                           'FalconCalc'       - character string     1.5610851718... x 10^45

    - Results are displayed simultaneously as decimal, hexadecimal, octal and binary numbers and as a 
      string of characters

    - Decimal formats: general, scientific and engeneering

    - Decimal display formats:  normal, html, TEX and 'E' (1.2·10³, 1.2x10<sup>3</sup>, 1.2\cdot10^{8},1.2E3)

    - Hexadecimal formats: normal or grouped as bytes, words or dwords each with or without the 0x prefix. 
      Also as Big- or little endian, unsigned or signed numbers.
	  For smaller numbers that fits inside an IEEE754 single or double number the hexadecimal representation for
	  that format can also be selected. (For large numbers the display will be incorrect!)

    - Angles for triginometric functions can be entered as degrees, radians, gradians or turns.
	  (The full circle is 360°, 2π, 400 grad, 1turn)

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
           au - astronomical unit [m], c - speed of light in vacuum (definition - exact value) [m/s], 
           h  - Planck constant  [Js], hbar - reduced Planck constant [Js], 
           qe - elementary charge [As], me - electron mass [kg], mp - proton mass [kg], 
           u - atomic mass unit [1], k - Boltzmann constant [J/K],G  - Newtonian constant of gravitation [m^3/kg/s^2], 
           gf - mean value of the gravitational acceleration on Earth (9.81 m/s²)
           eps0 - electric constant (vacuum permittivity) exact value [F /m]
           mu0  - magnetic constant (vacuum permeability) exact value [N/A^2] = 4π·10^(-7), 
           kc  - Coulomb's constant 1/4π eps0 [N m^2/C^2], LA  - Avogadro's number [1/mol],
           rf - Earth's radius [m], rg  - molar gas constant (8.31 J/ mol K), rs - Sun's radius
           u - atomic mass unit

    - Any number of user constants and functions may be defined with any valid arithmetic formula 
      including other constants and variables.
    - When a variable is modified the value of all dependent variables and functions are automatically changed
    - Functions may have any number of arguments with any names that is different from the name of any 
      built-in function or constant.
