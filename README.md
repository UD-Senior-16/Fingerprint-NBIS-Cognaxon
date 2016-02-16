# Fingerprint matching and enrollment #


## 1. Enroll a new user ##

    ./enroll <candidate-image-name>.png <candidate-user-name>

Inserts a new user with this fingerprint into the database.

## 2. Match a fingerprint to a given user ##

    ./match <probing-image-name>.png <user-name-being-probed>

## 3. Debug: Match two fingerprint images together ##

    ./debug-match <image-a-name>.png <image-b-name>.png

## 4. Estimate the similarity score threshold ##

    ./estimatethreshold.o <database-path>

Gives useful statistics on a given database. Runs debug-match num\_users^2*num\_fingerprints^2 times (a test database of 4 users with 8 fingerprints each takes roughly 4 minutes on my weak desktop).


## ToDo ##

  - Make a list of definitions.
