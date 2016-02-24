# Fingerprint enrollment and matching #

  For our Senior Design project, our group decided to create a fingerprint enrollment and matching system using photographs of fingerprints (you can view [our website here](https://sites.google.com/a/udel.edu/fingerprint/)). Two Bash scripts enroll users with a fingerprint into our database and match a fingerprint to a user in our database. There's also a C++ program that provides useful information on the similarity score outputted by the matching algorithm, namely the probabilities of false positives and true negatives.


## 1. Enroll a new user ##

    ./enroll <candidate-image-name>.png <candidate-user-name>

  Inserts a new user with this fingerprint into the database.


## 2. Match a fingerprint to a single given user ##

    ./match <probing-image-name>.png <user-name-being-probed>


## 3. [For Debugging/Testing Purposes] Match two fingerprint images together ##

    ./test-match <image-a-name>.png <image-b-name>.png


## 4. [For Testing Purposes] Estimate the similarity score threshold ##

    ./testing/estimatethreshold-png <png-database-path>

  Gives useful statistics on a given database. Runs `./test-match` (num\_users^2*num\_fingerprints^2) times. For instance, a test database of 4 users with 8 fingerprints each runs `./test-match` 1024 times and takes roughly 4 minutes on my weak desktop computer (that's about 4 matchs per second).
