# Fingerprint's implementation of NBIS and Cognaxon #



# 1. Convert the image file into a WSQ image file #

  ./ConvertWSQ/convertWSQ.o


# 2. Extract its minutiae #

  ./NBIS/mindtct ./ConvertWSQ/inputimage.wsq outputxyt


# 3. Match its minutiae #

  ./NBIS/bozorth3 outputxyt0.xyt outputxyt1.xyt



# ToDo #

  1. Break this up into two parts, enrollment and matching.
  2. Make a list of definitions.
