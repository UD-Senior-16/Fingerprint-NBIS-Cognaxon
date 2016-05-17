
// To compile the program use the command line
// g++ -std=c++11 -o estimatethreshold estimatethreshold.cpp
// To run the program use the command line
// ./estimatethreshold ./test-database/


#include <iostream>
#include <fstream>
#include <memory>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <regex.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <math.h>
#include <cstdint>
#include <climits>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using std::string;


int match(const char *arg1, const char *arg2) {
	// For redirecting test-match's stdout to
	FILE *tempfile = tmpfile();
	int tempfileno = fileno(tempfile);
	
	pid_t c_pid = fork();
	
	if (c_pid < 0) return 0;
	else if (c_pid == 0)
	{
		// Set the script's stdout to tmpfile
		close(STDOUT_FILENO);
		dup(tempfileno);
		close(tempfileno);
		if (execl("./test-match_v2", "./test-match_v2", arg1, arg2, NULL) == -1) return 0;
	}
	else
	{
		// Wait for child (script) to finish
		int status;
		waitpid(c_pid, &status, 0);
		
		// Restore stdout to point to /dev/tty
		int fid = open("/dev/tty", O_WRONLY);
		close(STDOUT_FILENO);
		dup(fid);
		close(fid);
		
		// Go back to beginning of the file
		rewind(tempfile);
		
		// Read from file
		int value = 0;
		fscanf(tempfile, "%d", &value);
		
		// Close/delete file
		fclose(tempfile);
		
		// Return value if no errors, otherwise 0
		return value;
	}
}

/*
	Doubly Linked List of Fingerprints
*/
typedef struct fingerprint
{
  char                *file;  // file location of fingerprint image
  struct fingerprint  *next;  // next fingerprint
  struct fingerprint  *prev;  // previous fingerprint
} fingerprint_t;

/*
	Doubly Linked List of Fingers
*/
typedef struct finger
{
  int             num_fingerprints;  // number of fingerprints
  char           *dir;  // directory location of the finger's fingerprint images
  fingerprint_t  *fingerprints;  // list of fingerprints for finger
  struct finger  *next;  // next finger
  struct finger  *prev;  // prev finger
} finger_t;

/*
	Doubly Linked List of Users
*/
typedef struct user
{
  int           num_fingers;  // number fingers of user
  char         *dir;  // directory location of the user's fingers
  finger_t     *fingers;  // list of fingers for user
  struct user  *next;  // next user
  struct user  *prev;  // prev user
} user_t;

// Main list of users
user_t *users;

/*
	Add a user to the main list of users
*/
user_t * addUser(const char *dir)
{
  if (!users)  // first user
  {
    users = (user_t *) malloc(sizeof(user_t));
    users->num_fingers = 0;
    users->dir = (char *) malloc( (strlen(dir)+1)*sizeof(char) );
    strcpy(users->dir, dir);
    users->fingers = NULL;
    users->next = NULL;
    users->prev = NULL;
    return users;
  }
  else  // this is not the first user
  {
    user_t *tail = users;
    while (tail->next) tail = tail->next;
    tail->next = (user_t *) malloc(sizeof(user_t));
    tail->next->num_fingers = 0;
    tail->next->dir = (char *) malloc( (strlen(dir)+1)*sizeof(char) );
    strcpy(tail->next->dir, dir);
    tail->next->fingers = NULL;
    tail->next->next = NULL;
    tail->next->prev = tail;
    return tail->next;
  }
}

/*
	Add a finger to a user
*/
finger_t * addFinger(user_t *user, const char *dir)
{
  if (!user->fingers)  // first finger
  {
    user->fingers = (finger_t *) malloc(sizeof(finger_t));
    user->fingers->num_fingerprints = 0;
    user->fingers->dir = (char *) malloc( (strlen(dir)+1)*sizeof(char) );
    strcpy(user->fingers->dir, dir);
    user->fingers->fingerprints = NULL;
    user->fingers->next = NULL;
    user->fingers->prev = NULL;
    return user->fingers;
  }
  else  // this is not the first finger
  {
    finger_t *tail = user->fingers;
    while(tail->next) tail = tail->next;
    tail->next = (finger_t *) malloc(sizeof(finger_t));
    tail->next->num_fingerprints = 0;
    tail->next->dir = (char *) malloc( (strlen(dir)+1)*sizeof(char) );
    strcpy(tail->next->dir, dir);
    tail->next->fingerprints = NULL;
    tail->next->next = NULL;
    tail->next->prev = tail;
    return tail->next;
  }
}

/*
	Add a fingerprint to a finger
*/
fingerprint_t * addFingerprint(finger_t *finger, const char *file)
{
  if (!finger->fingerprints)  // first fingerprint
  {
  	finger->fingerprints = (fingerprint_t *) malloc(sizeof(fingerprint_t));
  	finger->fingerprints->file = (char *) malloc( (strlen(file)+1)*sizeof(char) );
  	strcpy(finger->fingerprints->file, file);
  	finger->fingerprints->next = NULL;
  	finger->fingerprints->prev = NULL;
  	return finger->fingerprints;
  }
  else  // this is not the first fingerprint
  {
    fingerprint_t *tail = finger->fingerprints;
    while (tail->next) tail = tail->next;
    tail->next = (fingerprint_t *) malloc(sizeof(fingerprint_t));
    tail->next->file = (char *) malloc( (strlen(file)+1)*sizeof(char) );
    strcpy(tail->next->file, file);
    tail->next->next = NULL;
    tail->next->prev = tail;
    return tail->next;
  }
}


int main(int argc, char ** argv) {
	
	if(argc != 2) {
		if(argc == 1) printf("ERROR:\tYou are missing an argument. It is the test database's path.");
		else printf("ERROR:\tThere are %d arguments, when there should only be one.", argc-1);
		printf("Usage: estimatethreshold_v2 DATABASE_PATH");
		return 1;
	}
	
	char* test_database_path = argv[1];  // file path with file extension
	//int num_users = atoi(argv[2]);  // number of users
	//int num_fingerprints = atoi(argv[3]); // number of fingerprints per user
	uint16_t num_users;
	uint16_t num_fingers;
	uint16_t num_fingerprints;
	
	// Found help on RegEx from: http://stackoverflow.com/a/1085120/5171749
	regex_t regex_dir, regex_dir2, regex_img;
	uint8_t reti = 0;
	char msgbuf[100];
	
	// Compile regular expressions
	reti = regcomp(&regex_img, "^\\w+\\.(png|jpg|tif)$", REG_EXTENDED);
	if(reti) {
		fprintf(stderr, "ERROR:\tCould not compile regex_img\n");
		exit(2);
	}
	reti = regcomp(&regex_dir, "^\\w+$", REG_EXTENDED);
	if(reti) {
		fprintf(stderr, "ERROR:\tCould not compile regex_dir\n");
		exit(3);
	}
	reti = regcomp(&regex_dir2, "^\\w+\\-\\w+$", REG_EXTENDED);
	if(reti) {
		fprintf(stderr, "ERROR:\tCould not compile regex_dir2\n");
		exit(3);
	}
	
	
// Build matrix of users and fingerprints
	
	
//	string fingerprints[num_users][num_fingerprints];
	uint16_t user_i = 0;
	uint16_t finger_i = 0;
	uint16_t fingerprint_i = 0;
//	uint16_t num_fingerprints_per_user[num_users];
	
	user_t *user;
	finger_t *finger;
	fingerprint_t *fingerprint;
	
	DIR *dir_users, *dir_fingers, *dir_prints;
	struct dirent *ent_users, *ent_fingers, *ent_fingerprints;
	
	// GET USER
	if((dir_users = opendir(test_database_path)) != NULL) {
	user_i = 0;
	// print all files and directories within the given directory
	while((ent_users = readdir(dir_users)) != NULL) {
		reti = regexec(&regex_dir, ent_users->d_name, 0, NULL, 0);
		if(!reti) {
			// Match!
			string userpath = string(test_database_path) + "/" + ent_users->d_name;
printf("\tUser #%d: %s\n", user_i, userpath.c_str()); // debugging
			user = addUser(userpath.c_str());
			
			// GET FINGER
			if((dir_fingers = opendir(userpath.c_str())) != NULL) {
				finger_i = 0;
				// print all files and directories within the given directory
				while((ent_fingers = readdir(dir_fingers)) != NULL) {
					reti = regexec(&regex_dir2, ent_fingers->d_name, 0, NULL, 0);
					if(!reti) {
						// Match!
						string fingerpath = userpath + "/" + ent_fingers->d_name;
printf("\t\tFinger #%d: %s\n", finger_i, fingerpath.c_str()); // debugging
						finger = addFinger(user, fingerpath.c_str());
						user->num_fingers++;
						
						// GET FINGERPRINT
						if((dir_prints = opendir(fingerpath.c_str())) != NULL) {
							fingerprint_i = 0;
							// loop through user's fingerprints
							while((ent_fingerprints = readdir(dir_prints)) != NULL) {
								reti = regexec(&regex_img, ent_fingerprints->d_name, 0, NULL, 0);
								if(!reti) {
									// Match!
									string fingerprintpath = fingerpath + "/" + ent_fingerprints->d_name;
printf("\t\t\tFingerprint #%d: %s\n", fingerprint_i, fingerprintpath.c_str()); // debugging
									fingerprint_i++;
									fingerprint = addFingerprint(finger, fingerprintpath.c_str());
									finger->num_fingerprints++;
//									fingerprints[user_i][fingerprint_i] = fingerprintpath;
//									num_fingerprints_per_user[user_i] = fingerprint_i;
									
								} else if(reti == REG_NOMATCH) {
									// No Match!
//printf("\t\tFingerprint not matched for: %s/%s\n", fingerpath.c_str(), ent_fingerprints->d_name); // debugging
								} else {
									regerror(reti, &regex_img, msgbuf, sizeof(msgbuf));
									fprintf(stderr, "ERROR:\tRegex match failed: %s\n", msgbuf);
									exit(7);
								}
							}
							closedir(dir_prints);
						} else {
							// could not open the user's directory
							perror("");
							return 6;
						}
						finger_i++;
						
					} else if(reti == REG_NOMATCH) {
						// No Match!
//printf("\t\tFinger not matched for: '%s'//'%s'\n", userpath.c_str(), ent_fingers->d_name); // debugging
					} else {
						regerror(reti, &regex_dir2, msgbuf, sizeof(msgbuf));
						fprintf(stderr, "ERROR:\tRegex match failed: %s\n", msgbuf);
						exit(7);
					}
				}
				closedir(dir_fingers);
			} else {
				// could not open the user's directory
				perror("");
				return 6;
			}
			user_i++;
	
		} else if(reti == REG_NOMATCH) {
			// No Match!
//printf("\tUser not matched for: %s/%s\n", string(test_database_path).c_str(), ent_users->d_name); // debugging
		} else {
			regerror(reti, &regex_dir, msgbuf, sizeof(msgbuf));
			fprintf(stderr, "ERROR:\tRegex match failed: %s\n", msgbuf);
			exit(5);
			}
		}
		closedir(dir_users);
	} else {
		// could not open the test database's directory
		perror("");
		return 4;
	}
	
	
	
// Test each fingerprint to every other fingerprint
//   Scores are stored in 3 different categories:
//     Infinite - means these are the same images
//     High     - means these are the same user but different images
//     Low      - means these are two different users
// There are (users*users*fingerprints_per_user*fingerprints_per_user)
	
	
	printf("\n\tNum Users: %d, Num Fingers: %d, Num Fingerprints: %d\n\n", user_i, finger_i, fingerprint_i);
	num_users = user_i;
	num_fingers = finger_i;
	num_fingerprints = fingerprint_i;
	
	uint32_t max_num_infinite = num_users * num_fingerprints;
	uint32_t max_num_high = num_users * (pow(num_fingerprints, 2) - num_fingerprints);
	uint32_t max_num_low = pow(num_fingerprints, 2) * (pow(num_users, 2) - num_users);
	uint32_t num_valid = 0;
	uint32_t num_invalid = 0;
	
//	uint32_t max_num_infinite = 0;
//	uint32_t max_num_high = 0;
//	uint32_t max_num_low = 0;
	
	// Assuming all users have the same number of fingers and number of fingerprints per finger
//	max_num_infinite = num_users*num_fingers;
//	max_num_high = num_users * (num_fingers*num_fingers) - max_num_infinite;
//	max_num_low = num_users*num_users * num_fingers*num_fingers - max_num_high - max_num_infinite;
	
printf("max_num_infinite=%i, max_num_high=%i, max_num_low=%i\n", max_num_infinite, max_num_high, max_num_low);
	
	uint64_t inf_sum = 0;
	uint64_t high_sum = 0;
	uint64_t low_sum = 0;
	
	uint32_t inf_num = 0;
	uint32_t high_num = 0;
	uint32_t low_num = 0;
	
	uint64_t inf_max = 0;
	uint64_t high_max = 0;
	uint64_t low_max = 0;
	
	uint64_t inf_min = UINT64_MAX;
	uint64_t high_min = UINT64_MAX;
	uint64_t low_min = UINT64_MAX;
	
	
	uint16_t score;
	uint8_t valid;
	
	
	
	uint16_t infinite[max_num_infinite];
	uint16_t high[max_num_high];
	uint16_t low[max_num_low];
	
	
	std::cout << std::endl;
	
	// CREATE CSV file
	//   http://stackoverflow.com/q/25201131/5171749
	std::ofstream csv ("./results/results.csv");  // Opening file to print info to
	
	// DEFINE headings for CSV file
	csv << std::string("UserA,FingerprintGroupA,UserB,FingerprintGroupB,Score,Type") << std::endl;
	
	uint32_t num_combinations = max_num_infinite + max_num_high + max_num_low;
	
	user_t *user_p;
	finger_t *finger_p;
	fingerprint_t *fingerprint_p;
	//string fingerprintA;
	//string fingerprintB;
	
	
	// COMPARE fingerprints in the 2D matrix
	user_t *userA = users;
	for (uint16_t i = 0; i < num_users; i++)
	{
		user_t *userB = users;
		for (uint16_t j = 0; j < num_users; j++)
		{
			for (uint16_t k = 0; k < num_fingerprints; k++)  // i
			{
				for (uint16_t l = 0; l < num_fingerprints; l++)  // j
				{
					// Reset score
					valid = 1;  // valid
					score = 0;
					// get users' head fingers
					finger_t *fingerA = userA->fingers;
					finger_t *fingerB = userB->fingers;
					
					
					for (uint16_t m = 0; m < num_fingers; m++)
					{
						fingerprint_t *fingerprintA = fingerA->fingerprints;
						for (uint16_t i_k = 0; i_k < k; i_k++) fingerprintA = fingerprintA->next;
						
						fingerprint_t *fingerprintB = fingerB->fingerprints;
						for (uint16_t i_l = 0; i_l < l; i_l++) fingerprintB = fingerprintB->next;
						
						uint16_t individual_score = match(fingerprintA->file, fingerprintB->file);
						
//printf("%sVS%s==%i\n", fingerprintA->file, fingerprintB->file, individual_score);
						
						if (individual_score == 0)
						{
							valid = 0;  // invalid
							//break;
						}
						
						score += individual_score;
						
						fingerA = fingerA->next;
						fingerB = fingerB->next;
					}
					// Score has been cumulated
					
					if (valid) // valid == 1
					{
						char type;
						if (i == j)
						{
							// SAME users
							if (k == l)
							{
								// INFINITE
								infinite[inf_num++] = score;
								inf_sum += score;
								if (inf_max < score) inf_max = score;
								if (inf_min > score) inf_min = score;
								type = 'I';
							}
							else
							{
								// HIGH
								high[high_num++] = score;
								high_sum += score;
								if(high_max < score) high_max = score;
								if(high_min > score) high_min = score;
								type = 'H';
							}
						}
						else
						{
							// LOW
							low[low_num++] = score;
							low_sum += score;
							if(low_max < score) low_max = score;
							if(low_min > score) low_min = score;
							type = 'L';
						}
//printf("Score: %i, Type: %c\n", score, type);
						
						// APPEND to end of CSV file
						csv << i << std::string(",") << k << "," << j << "," << l << "," << score << "," << type << std::endl;
						
						num_valid++;
						std::cout << (uint16_t)(100 * (num_valid+num_invalid) / num_combinations) << "%" << "\t"
							<< "Min. Inf.:  " << inf_min  << "    "
							<< "Max. High:  " << high_max << "    "
							<< "Min. High:  " << high_min << "    "
							<< " Max. Low:  " << low_max  << "    " << "\r" << std::flush;
					}
					else
					{
						num_invalid++;
//printf("Invalid\n");
					}
				}
			}
			userB = userB->next;
		}
		userA = userA->next;
	}
	
	printf("\n\n");
	
	
	printf("\t\t# of valid Infinite Scores:  \t%d of %d\n", inf_num, max_num_infinite);
	printf("\t\tMaximum Infinite Score:\t%lu\n", inf_max);
	printf("\t\tAverage Infinite Score:\t%lu\n", inf_num ? inf_sum/inf_num : 0);
	printf("\t\tMinimum Infinite Score:\t%lu\n", inf_num ? inf_min : 0);
	printf("\n");
	printf("\t\t# of valid High Scores:  \t%d of %d\n", high_num, max_num_high);
	printf("\t\tMaximum High Score:\t%lu\n", high_max);
	printf("\t\tAverage High Score:\t%lu\n", high_num ? high_sum/high_num : 0);
	printf("\t\tMinimum High Score:\t%lu\n", high_num ? high_min : 0);
	printf("\n");
	printf("\t\t# of valid Low Scores:  \t%d of %d\n", low_num, max_num_low);
	printf("\t\tMaximum Low Score:\t%lu\n", low_max);
	printf("\t\tAverage Low Score:\t%lu\n", low_num ? low_sum/low_num : 0);
	printf("\t\tMinimum Low Score:\t%lu\n", low_num? low_min : 0);
	printf("\n");
	printf("\t\t# of Invalid Scores:  \t%d\n", num_invalid);
	printf("\t\t# of Valid Scores:    \t%d\n", num_valid);
	
	uint16_t threshold = low_max;
	
	uint32_t falseNegatives_I = 0;
	uint32_t truePositives_I = 0;
	uint32_t falseNegatives_H = 0;
	uint32_t truePositives_H = 0;

	uint32_t falsePositives = 0;
	uint32_t falseNegatives = 0;
	uint32_t truePositives = 0;
	uint32_t trueNegatives = 0;
	
	for(uint32_t i = 0; i < inf_num; i++) {
		if (infinite[i] > threshold) truePositives_I++;
		else falseNegatives_I++;
	}
	for(uint32_t j = 0; j < high_num; j++) {
		if (high[j] > threshold) truePositives_H++;
		else falseNegatives_H++;
	}
	for(uint32_t k = 0; k < low_num; k++) {
		if (low[k] > threshold) falsePositives++;
		else trueNegatives++;
	}
	
	truePositives = truePositives_I + truePositives_H;
	falseNegatives = falseNegatives_I + falseNegatives_H;
	
	printf("\nnum_combinations: %d === falsePositives+falseNegatives+truePositives+trueNegatives: %d\n", inf_num+high_num+low_num, falsePositives+falseNegatives+truePositives+trueNegatives);
	
	printf("\nWith a threshold of %d; there are %d false positives, %d false negatives, %d true positives, and %d true negatives.\n\n", threshold, falsePositives, falseNegatives, truePositives, trueNegatives);
	
	printf("Of the %d true positives, %d are infinite scores and %d are high scores.\n\n", truePositives, truePositives_I, truePositives_H);
	
	printf("Of the %d false negatives, %d are infinite scores and %d are high scores.\n\n", falseNegatives, falseNegatives_I, falseNegatives_H);
	
	// CLOSE CSV file
	csv.close();
	
	return 0;
}
