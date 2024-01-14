# FalconCalc
Windows scientist and programmer calculator featuring arbitrary precision, history and more

# Description
  Every Windows has a desktop calculator with many features but FalconCalc offers many 
  unique features not found in them:

    - In FalconCalc an expression may contain built-in and user defined constants and functions of any
      number of arguments which are evaluated continuously as they are entered.

    - *Numbers* may be entered in decimal, hexadecimal, octal or binary notation or as character strings.
                       Examples (decimal equivalent in parenthesis):
                           123.4567890e+2     - decimal number (12345.67890)"
                           0x1234567890ABCDEF - hexadecimal number 
                                (decimal 1 311 768 467 294 899 695)
                           012345670          - octal number (2 739 128)
                           #10101             - binary number (21)
                           'FalconCalc'       - character string (7 810 723 214 453 320 491)\n

    - Results are displayed simultaneously as decimal, hexadecimal, octal and binary numbers and as a 
      string of characters

    - Decimal formats: general, scientific and engeneering

    - Formulas may contain the following operators:
          arithmetics: +, -, *, /, ^(power), | (or 'or'), & (or 'and'), xor, << (or 'shl' - shift left), 
                        << (or 'shl' - shift right),% (or mod - remainder), ! (or 'not') 
          logical: ==, != (not equal), <, >, <=, >= (these results in 1 or 0)

    - Built-in functions (alternative names in parenthesis)
          abs, arcsin (asin), arccos (acos), arctan (atan), cos, cosh (ch), coth (cth), exp, fact, frac, int, 
          log (ln), log2, log10 round, sign, sin, sinh (sh), sqrt, tan (tg), trunc

    - Built in mathematical constants:
           e (base of the natural logarithm), log2e (base 2 logarithm of e), log10e or 
           lge (base 10 logarithm of e), ln2 (natural logarithm of 2)
           pi, pi2 (pi/2), pi4 (pi/4), ppi (1/pi), tpi (2/pi), spi (sqrt(pi)), sqrt2, psqrt2 (1/sqrt(2))

    - Built-in physical constants:
           c (speed of light in vacuum (definition - exact value) [m/s]), h (Planck constant  [Js]), 
           hbar (reduced Planck constant [Js]), qe elementary charge [As]), me (electron mass [kg]), 
           mp (proton mass [kg]), u (atomic mass unit [1]), k (Boltzmann constant [J/K])
           G (Newtonian constant of gravitation [m^3/kg/s^2]), 
           eps0 (electric constant (vacuum permittivity) defined [F /m])
           mu0 (magnetic constant (vacuum permeability) defined [N/A^2] = 4 pi 1e-7), 
           kc (Coulomb's constant 1/4pi eps0 [N m^2/C^2])
           LA (Avogadro's number [1/mol])

    - Any number of user constants and functions may be defined with any valid arithmetic formula 
      including other constants and variables.
    - When a variable is modified the value of all dependent variables and functions are automatically changed
    - Functions may have any number of arguments with any names that is different from the name of any 
      built-in function or constant.;
