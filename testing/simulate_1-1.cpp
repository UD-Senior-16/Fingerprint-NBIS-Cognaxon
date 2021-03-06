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

// for match()
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
		if (execl("./test-match", "./test-match", 5, arg1, arg2, NULL) == -1) return 0;
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
		if (WEXITSTATUS(status) != 0) return 0;
		else return value;
	}
}


int main(int argc, char ** argv) {

  if(argc != 2) {
    if(argc == 1) printf("ERROR:\tYou are missing an argument. It is the test database's path.");
    else printf("ERROR:\tThere are %d arguments, when there should only be one.", argc-1);
    return 1;
  }

  char* test_database_path = argv[1];  // file path with file extension
  //int num_users = atoi(argv[2]);  // number of users
  //int num_fingerprints = atoi(argv[3]); // number of fingerprints per user
  uint16_t num_users = 8;
  uint16_t num_fingerprints = 8;

  // Found help on RegEx from: http://stackoverflow.com/a/1085120/5171749
  regex_t regex_dir, regex_img;
  uint8_t reti = 0;
  char msgbuf[100];

  // Compile regular expressions
  reti = regcomp(&regex_img, "\\w+\\.(png|jpg|tif)", REG_EXTENDED);
  if(reti) {
    fprintf(stderr, "ERROR:\tCould not compile regex_img\n");
    exit(2);
  }
  reti = regcomp(&regex_dir, "\\w+", REG_EXTENDED);
  if(reti) {
    fprintf(stderr, "ERROR:\tCould not compile regex_dir\n");
    exit(3);
  }


// Build matrix of users and fingerprints


  string fingerprints[num_users][num_fingerprints];
  uint16_t user_i = 0;
  uint16_t fingerprint_i = 0;
  uint16_t num_fingerprints_per_user[num_users];

  DIR *dir_users, *dir_prints;
  struct dirent *ent_users, *ent_fingerprints;
  if((dir_users = opendir(test_database_path)) != NULL) {
    // print all files and directories within the given directory
    while((ent_users = readdir(dir_users)) != NULL) {
      reti = regexec(&regex_dir, ent_users->d_name, 0, NULL, 0);
      if(!reti) {
        // Match!
        string userpath = string(test_database_path) + "/" + ent_users->d_name;
				printf("\tUser #%d: %s\n", user_i, userpath.c_str()); // debugging
        if((dir_prints = opendir(userpath.c_str())) != NULL) {
          fingerprint_i = 0;
          // loop through user's fingerprints
          while((ent_fingerprints = readdir(dir_prints)) != NULL) {
            reti = regexec(&regex_img, ent_fingerprints->d_name, 0, NULL, 0);
            if(!reti) {
              // Match!
              string fingerprintpath = userpath + "/" + ent_fingerprints->d_name;
							printf("\t\tFingerprint #%d: %s\n", fingerprint_i, fingerprintpath.c_str()); // debugging
              fingerprints[user_i][fingerprint_i++] = fingerprintpath;
              num_fingerprints_per_user[user_i] = fingerprint_i;
            } else if(reti == REG_NOMATCH) {
              // No Match!
							//printf("\t\tFingerprint not matched for: %s/%s\n", userpath.c_str(), ent_fingerprints->d_name); // debugging
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


//  printf("\nDEBUG:\tNum Users: %d, Num Fingerprints: %d\n\n", user_i, fingerprint_i);
  num_users = user_i;
  num_fingerprints = fingerprint_i;

//  uint32_t num_infinite = num_users * num_fingerprints;
//  uint32_t num_high = num_users * (pow(num_fingerprints, 2) - num_fingerprints);
//  uint32_t num_low = pow(num_fingerprints, 2) * (pow(num_users, 2) - num_users);
  uint32_t num_infinite = 0;
  uint32_t num_high = 0;
  uint32_t num_low = 0;
  uint32_t num_valid = 0;
  uint32_t num_invalid = 0;

  for (uint16_t i = 0; i < num_users; i++) {
    for (uint16_t j = 0; j < num_users; j++) {
      if (i == j) {
        num_infinite += num_fingerprints_per_user[i];
        num_high += num_fingerprints_per_user[i] * num_fingerprints_per_user[i] - num_fingerprints_per_user[i];
      } else num_low += num_fingerprints_per_user[i] * num_fingerprints_per_user[j];
    }
  }



  uint32_t inf_sum = 0;
  uint32_t high_sum = 0;
  uint32_t low_sum = 0;

  uint16_t inf_num = 0;
  uint16_t high_num = 0;
  uint16_t low_num = 0;

  uint16_t inf_max = 0;
  uint16_t high_max = 0;
  uint16_t low_max = 0;

  uint16_t inf_min = UINT16_MAX;
  uint16_t high_min = UINT16_MAX;
  uint16_t low_min = UINT16_MAX;


  uint16_t score;



  uint16_t infinite[num_infinite];
  uint16_t high[num_high];
  uint16_t low[num_low];

  std::cout << std::endl;

  // CREATE CSV file
  //   http://stackoverflow.com/q/25201131/5171749
  std::ofstream csv ("./results/results.csv");  // Opening file to print info to

	// DEFINE headings for CSV file
  csv << std::string("UserA,FingerprintA,UserB,FingerprintB,Score,Type") << std::endl;

	uint32_t num_combinations = num_infinite + num_high + num_low;

  // COMPARE fingerprints in the 2D matrix
  for(uint16_t i = 0; i < num_users; i++) {
    for(uint16_t j = 0; j < num_users; j++) {
      for(uint16_t k = 0; k < num_fingerprints_per_user[i]; k++) {
        for(uint16_t l = 0; l < num_fingerprints_per_user[j]; l++) {
          
          score = match(fingerprints[i][k].c_str(), fingerprints[j][l].c_str());
//              printf("i:%d\t j:%d\t k:%d\t l:%d\t\t score: %d\n", i, j, k, l, score); // debugging
//              printf("\tCommand:\t%s\n", command.c_str()); // debugging
          if (score != 0)
          {
	    char type;
	    if(i == j) {
	      // SAME users
	      if(k == l) {
	        // SAME fingerprint images (Infinite) (needless to say, same users...)
	        infinite[inf_num++] = score;
	        inf_sum += score;
	        if(inf_max < score) inf_max = score;
	        if(inf_min > score) inf_min = score;
	        type = 'I';
	      } else {
	        // DIFFERENT fingerprint images, SAME users (High)
	        high[high_num++] = score;
	        high_sum += score;
	        if(high_max < score) high_max = score;
	        if(high_min > score) high_min = score;
	        type = 'H';
	      }
	    } else {
	      // DIFFERENT fingerprint images, DIFFERENT users (Low)
	      low[low_num++] = score;
	      low_sum += score;
	      if(low_max < score) low_max = score;
	      if(low_min > score) low_min = score;
	      type = 'L';
	    }
//printf("Score: %i, Type: %c\n", score, type);
		num_valid++;
	    std::cout << (uint16_t)(100 * (num_valid+num_invalid) / num_combinations) << "%" << "\t"
	        << "Min. Inf.:  " << inf_min  << "    "
	        << "Max. High:  " << high_max << "    "
	  	<< "Min. High:  " << high_min << "    "
	        << " Max. Low:  " << low_max  << "    " << "\r" << std::flush;
	    // APPEND to end of CSV file
	    csv << i << std::string(",") << k << "," << j << "," << l << "," << score << "," << type << std::endl;
          }
          else num_invalid++;
        }
      }
    }
  }

  printf("\n\n");


  printf("\t\t# of valid Infinite Scores:  \t%d of %d\n", inf_num, num_infinite);
  printf("\t\tMaximum Infinite Score:\t%d\n", inf_max);
  printf("\t\tAverage Infinite Score:\t%d\n", inf_sum/inf_num);
  printf("\t\tMinimum Infinite Score:\t%d\n", inf_min);
  printf("\n");
  printf("\t\t# of valid High Scores:  \t%d of %d\n", high_num, num_high);
  printf("\t\tMaximum High Score:\t%d\n", high_max);
  printf("\t\tAverage High Score:\t%d\n", high_sum/high_num);
  printf("\t\tMinimum High Score:\t%d\n", high_min);
  printf("\n");
  printf("\t\t# of valid Low Scores:  \t%d of %d\n", low_num, num_low);
  printf("\t\tMaximum Low Score:\t%d\n", low_max);
  printf("\t\tAverage Low Score:\t%d\n", low_sum/low_num);
  printf("\t\tMinimum Low Score:\t%d\n", low_min);
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

  printf("\nnum_combinations: %d, falsePositives+falseNegatives+truePositives+trueNegatives=%d\n\n", inf_num+high_num+low_num, falsePositives+falseNegatives+truePositives+trueNegatives);

  printf("\nWith a threshold of %d; there are %d false positives, %d false negatives, %d true positives, and %d true negatives.\n\n", threshold, falsePositives, falseNegatives, truePositives, trueNegatives);

  printf("Of the %d true positives, %d are infinite scores and %d are high scores.\n\n", truePositives, truePositives_I, truePositives_H);

  printf("Of the %d false negatives, %d are infinite scores and %d are high scores.\n\n", falseNegatives, falseNegatives_I, falseNegatives_H);

  // CLOSE CSV file
  csv.close();

  return 0;
}
