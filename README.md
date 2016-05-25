# Fingerprint enrollment and matching #

  For our Senior Design project, our group decided to create a fingerprint enrollment and matching system using photographs of fingerprints (you can view [our website here](https://sites.google.com/a/udel.edu/fingerprint/)). Two Bash scripts enroll users with a fingerprint into our database and match a fingerprint to a user in our database. There's also a C++ program that provides useful information on the similarity score outputted by the matching algorithm, namely the probability of false negatives.

  [Here you can find NBIS's User Guide](NBIS/NBIS-User-Guide.pdf) and [info on Cognaxon's WSQ image library here](http://www.cognaxon.com/index.php?page=wsqlibrary).


## 1. Enroll a new user ##

    ./enroll <quality-threshold> <candidate-image-name>.png <candidate-user-name>

  Inserts a new user with this fingerprint into the database. The quality score threshold is an integer between 1 (best) and 5 (worst), inclusively, that rejects all images above the threshold. For instance, a threshold of 5 allows all images.


## 2. Match a fingerprint to a single given user ##

    ./match <quality-threshold> <probing-image-name>.png <user-name-being-probed>

  Outputs the similarity score. Programmer is responsible for determining a good similarity score threshold value. A similarity score is an integer from 1 up to 1000+ representing how similar the two fingerprints are.



# Fingerprint matching simulations and image quality statistics #

## 1. Match two fingerprint images together ##

    ./test-match <quality-threshold> <image-a-name>.png <image-b-name>.png

  Outputs the similarity score. Returns 0 if either of the images' quality scores are above the threshold.


## 2. Report the image quality of an image ##

    ./test-quality <image>.png

  Outputs the image quality score (integer) from 1 to 5, inclusively. 1 is the best possible score, 5 is the worst possible score.


## 3. Simulate 1-1 matches of a database ##

    make 1-1
    ./testing/simulate_1-1 <database-path>

  Gives useful statistics on a given 1-1 database. The database's root directory must contain many users and each user directory must contain the same number of fingerprint images. Runs `./test-match` ( _users_<sup>2</sup> &times; _fingerprints_<sup>2</sup> ) times.


## 4. Simulate N-N matches of a database ##

    make N-N
    ./testing/simulate_N-N <database-path>

  Similar to the 1-1 simulation, but each user directory must contain the same number of fingers and each finger directory must contain the same number of fingerprint images. Runs `./test-match` ( _users_<sup>2</sup> &times; _fingerprints_<sup>2</sup> ) times.


## 5. Report the distribution of a database's image qualities ##

    make quality
    ./testing/quality-statistics <database-path>

  Gives image quality statistics on a given database. The database's root directory contains many images. Runs `./test-quality` ( _images_ ) times.
