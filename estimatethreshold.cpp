// To compile the program use the command line
// g++ -o estimatethreshold.o -std=c++11 estimatethreshold.cpp
// To run the program use the command line
// ./estimatethreshold.o ./test-database/


#include <iostream>
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

  // http://stackoverflow.com/a/2808527/5171749
  // begin the clock
  clock_t begin = clock();

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
  regex_t regex_dir, regex_tif;
  int reti = 0;
  char msgbuf[100];

  // Compile regular expressions
  reti = regcomp(&regex_tif, "\\w+.tif", REG_EXTENDED);
  if(reti) {
    fprintf(stderr, "ERROR:\tCould not compile regex_tif\n");
    exit(1);
  }
  reti = regcomp(&regex_dir, "\\w+", REG_EXTENDED);
  if(reti) {
    fprintf(stderr, "ERROR:\tCould not compile regex_dir\n");
    exit(1);
  }



  string fingerprints[num_users][num_fingerprints];
  int user_i = 0;
  int fingerprint_i = 0;

  DIR *dir_users, *dir_prints;
  struct dirent *ent_users, *ent_prints;
  if((dir_users = opendir(test_database_path)) != NULL) {
    // print all files and directories within the given directory
    while((ent_users = readdir(dir_users)) != NULL) {
      reti = regexec(&regex_dir, ent_users->d_name, 0, NULL, 0);
      if(!reti) {
        // Match!
        string userpath = string(test_database_path) + "/" + ent_users->d_name;
        if((dir_prints = opendir(userpath.c_str())) != NULL) {
          fingerprint_i = 0;
          // loop through user's fingerprints
          while((ent_prints = readdir(dir_prints)) != NULL) {
            reti = regexec(&regex_tif, ent_prints->d_name, 0, NULL, 0);
            if(!reti) {
              // Match!
              string printpath = userpath + "/" + ent_prints->d_name;
              fingerprints[user_i][fingerprint_i++] = printpath;
            } else if(reti == REG_NOMATCH) {
              // No Match!
            } else {
              regerror(reti, &regex_tif, msgbuf, sizeof(msgbuf));
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


//  printf("\nDEBUG:\tNum Users: %d, Num Fingerprints: %d\n\n", user_i, fingerprint_i);


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


  long overall_i = 1;
  string command;
  int score;

//  printf("\n0%%");

  int infinite[num_users*num_fingerprints];
  int high[num_users*((int)pow(num_fingerprints, 2)-num_fingerprints)];
  int low[(int)pow(num_fingerprints, 2)*((int)pow(num_users, 2)-num_users)];

  // Compare fingerprints in the 2D matrix
  for(int i = 0; i < num_users; i++) {
    for(int j = 0; j < num_users; j++) {
      for(int k = 0; k < num_fingerprints; k++) {
        for(int l = 0; l < num_fingerprints; l++) {
          command = string("./debug-match") + " " + fingerprints[i][k] + " " + fingerprints[j][l];
          score = exec(command.c_str());
//          printf("i:%d\t j%d\t k%d\t l%d\n", i, j, k, l);
//          printf("\tCommand:\t%s\n", command.c_str());
          if(i == j) {
            // Same users
            if(k == l) {
              // Same fingerprints (Infinite)
              infinite[inf_num++] = score;
              inf_sum += score;
              if(inf_max < score) inf_max = score;
              if(inf_min > score) inf_min = score;
            } else {
              // Different fingerprints (High)
              high[high_num++] = score;
              high_sum += score;
              if(high_max < score) high_max = score;
              if(high_min > score) high_min = score;
            }
          } else {
            // Different users (Low)
            low[low_num++] = score;
            low_sum += score;
            if(low_max < score) low_max = score;
            if(low_min > score) low_min = score;
          }
          overall_i++;
          std::cout << (int)(100 * overall_i / (num_users*num_users*num_fingerprints*num_fingerprints)) << "%   "
              << "Minimum Infinite Score:\t" << inf_min << "   \t"
              << "Maximum High Score:\t" << high_max << "   \t"
              << "Minimum High Score:\t" << high_min << "   \t"
              << "Maximum Low Score:\t" << low_max << "   "
              << "\r" << std::flush;
        }
      }
    }
  }

  printf("\n\n\n");

  // End clock
  clock_t end = clock();
  double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

  printf("\tTotal time:\t%f\n\n", elapsed_secs);

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

  return 0;
}
