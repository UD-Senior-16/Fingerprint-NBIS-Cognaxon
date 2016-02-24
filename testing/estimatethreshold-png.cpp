
// To compile the program use the command line
// g++ -std=c++11 -o estimatethreshold-png.o estimatethreshold-png.cpp
// To run the program use the command line
// ./estimatethreshold-png.o ./test-database/


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
#include <climits>

using std::string;


int exec(const char* cmd) {
  std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
  if (!pipe) {printf("ERROR:\tPipe didn't open correctly.");return 0;}
  char buffer[128];
  std::string result = "";
  while (!feof(pipe.get())) {
    if (fgets(buffer, 128, pipe.get()) != NULL) result += buffer;
  }
  return atoi(result.c_str());
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
  int num_users = 4; // 4
  int num_fingerprints = 8; // 8

  // Found help on RegEx from: http://stackoverflow.com/a/1085120/5171749
  regex_t regex_dir, regex_png;
  int reti = 0;
  char msgbuf[100];

  // Compile regular expressions
  reti = regcomp(&regex_png, "\\w+.png", REG_EXTENDED);
  if(reti) {
    fprintf(stderr, "ERROR:\tCould not compile regex_png\n");
    exit(1);
  }
  reti = regcomp(&regex_dir, "\\w+", REG_EXTENDED);
  if(reti) {
    fprintf(stderr, "ERROR:\tCould not compile regex_dir\n");
    exit(1);
  }


// Build matrix of users and fingerprints


  string fingerprints[num_users][num_fingerprints];
  int user_i = 0;
  int fingerprint_i = 0;

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
            reti = regexec(&regex_png, ent_fingerprints->d_name, 0, NULL, 0);
            if(!reti) {
              // Match!
              string fingerprintpath = userpath + "/" + ent_fingerprints->d_name;
							printf("\t\tFingerprint #%d: %s\n", fingerprint_i, fingerprintpath.c_str()); // debugging
              fingerprints[user_i][fingerprint_i++] = fingerprintpath;
            } else if(reti == REG_NOMATCH) {
              // No Match!
							//printf("\t\tFingerprint not matched for: %s/%s\n", userpath.c_str(), ent_fingerprints->d_name); // debugging
            } else {
              regerror(reti, &regex_png, msgbuf, sizeof(msgbuf));
              fprintf(stderr, "ERROR:\tRegex match failed: %s\n", msgbuf);
              exit(1);
            }
          }
          closedir(dir_prints);
        } else {
          // could not open the user's directory
          perror("");
          return 2;
        }
        user_i++;
      } else if(reti == REG_NOMATCH) {
        // No Match!
				//printf("\tUser not matched for: %s/%s\n", string(test_database_path).c_str(), ent_users->d_name); // debugging
      } else {
        regerror(reti, &regex_dir, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "ERROR:\tRegex match failed: %s\n", msgbuf);
        exit(1);
      }
    }
    closedir(dir_users);
  } else {
    // could not open the test database's directory
    perror("");
    return 2;
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


  int inf_sum = 0;
  int high_sum = 0;
  int low_sum = 0;

  int inf_num = 0;
  int high_num = 0;
  int low_num = 0;

  int inf_max = 0;
  int high_max = 0;
  int low_max = 0;

  int inf_min = 9999;
  int high_min = 9999;
  int low_min = 9999;


  long combination_i = 0;
  string command;
  int score;



  int infinite[num_users*num_fingerprints];
  int high[num_users*((int)pow(num_fingerprints, 2)-num_fingerprints)];
  int low[(int)pow(num_fingerprints, 2)*((int)pow(num_users, 2)-num_users)];

  std::cout << std::endl;

  // CREATE CSV file
  //   http://stackoverflow.com/q/25201131/5171749
  std::ofstream csv ("results.csv");  // Opening file to print info to

	// DEFINE headings for CSV file
  csv << std::string("UserA,FingerprintA,UserB,FingerprintB,Score,Type") << std::endl;

	int num_combinations = num_users * num_users * num_fingerprints * num_fingerprints;

  // COMPARE fingerprints in the 2D matrix
  for(int i = 0; i < num_users; i++) {
    for(int j = 0; j < num_users; j++) {
      for(int k = 0; k < num_fingerprints; k++) {
        for(int l = 0; l < num_fingerprints; l++) {
          command = string("./test-match") + " " + fingerprints[i][k] + " " + fingerprints[j][l];
          score = exec(command.c_str());
//          printf("i:%d\t j%d\t k%d\t l%d\n", i, j, k, l); // debugging
//          printf("\tCommand:\t%s\n", command.c_str()); // debugging
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

          std::cout << (int)(100 * combination_i++ / num_combinations) << "%" << "\t"
              << "Min. Inf.:  " << inf_min  << "    "
              << "Max. High:  " << high_max << "    "
							<< "Min. High:  " << high_min << "    "
              << " Max. Low:  " << low_max  << "    " << "\r" << std::flush;
          // APPEND to end of CSV file
          csv << i << std::string(",") << k << "," << j << "," << l << "," << score << "," << type << std::endl;
        }
      }
    }
  }

  printf("\n\n");


  printf("\t\tMaximum Infinite Score:\t%d\n", inf_max);
  printf("\t\tAverage Infinite Score:\t%d\n", inf_sum/inf_num);
  printf("\t\tMinimum Infinite Score:\t%d\n", inf_min);
  printf("\n");
  printf("\t\tMaximum High Score:\t%d\n", high_max);
  printf("\t\tAverage High Score:\t%d\n", high_sum/high_num);
  printf("\t\tMinimum High Score:\t%d\n", high_min);
  printf("\n");
  printf("\t\tMaximum Low Score:\t%d\n", low_max);
  printf("\t\tAverage Low Score:\t%d\n", low_sum/low_num);
  printf("\t\tMinimum Low Score:\t%d\n", low_min);

  int threshold = low_max+1;
  int falsePositives = 0;
  int falseNegatives = 0;
  int correct = 0;
  for(int i = 0; i < num_users*num_fingerprints; i++) {
		if(infinite[i] < threshold) falseNegatives++;
		else correct++;
	}
  for(int j = 0; j < num_users*((int)pow(num_fingerprints, 2)-num_fingerprints); j++) {
		if(high[j] < threshold) falseNegatives++;
		else correct++;
	}
  for(int k = 0; k < (int)pow(num_fingerprints, 2)*((int)pow(num_users, 2)-num_users); k++) {
		if(low[k] < threshold) correct++;
		else falsePositives++;
	}

  printf("\nnum_combinations: %d, falsePositives+falseNegatives+correct=%d\n\n", num_combinations, falsePositives+falseNegatives+correct);

  printf("\nWith a threshold of %d; there are %d false positives, %d false negatives, and %d correct predictions.\n\n", threshold, falsePositives, falseNegatives, correct);

  // CLOSE CSV file
  csv.close();

  return 0;
}
