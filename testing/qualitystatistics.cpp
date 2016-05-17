
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


int quality(const char *arg1) {
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
		if (execl("./test-quality", "./test-quality", arg1, NULL) == -1) return 0;
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



int main(int argc, char ** argv) {
	
	if(argc != 2) {
		fprintf(stderr, "Usage: qualitystatistics DATABASE_PATH");
		return 1;
	}
	
	char* test_database_path = argv[1];  // file path with file extension
	
	uint16_t num_5;
	uint16_t num_4;
	uint16_t num_3;
	uint16_t num_2;
	uint16_t num_1;
	uint16_t num_0;
	
	// Found help on RegEx from: http://stackoverflow.com/a/1085120/5171749
	regex_t regex_img;
	uint8_t reti = 0;
	char msgbuf[100];

	// Compile regular expressions
	reti = regcomp(&regex_img, "^\\w+\\.(png|jpg|tif)$", REG_EXTENDED);
	if(reti) {
		fprintf(stderr, "ERROR:\tCould not compile regex_img\n");
		exit(2);
	}
	
	
	// Build array of fingerprints
	
	uint16_t fingerprint_i = 0;
	
	DIR *dir_prints;
	struct dirent *ent_fingerprints;
	
	// GET FINGERPRINTS
	if((dir_prints = opendir(test_database_path)) != NULL) {
		fingerprint_i = 0;
		
		// print all files and directories within the given directory
		while((ent_fingerprints = readdir(dir_prints)) != NULL) {
			reti = regexec(&regex_img, ent_fingerprints->d_name, 0, NULL, 0);
			if(!reti) {
				// Match!
				string fingerprintpath = string(test_database_path) + "/" + ent_fingerprints->d_name;
				int score = quality(fingerprintpath.c_str());
				printf("\tFingerprint #%d (%i): %s\n",
						fingerprint_i,
						score,
						fingerprintpath.c_str());
				switch (score)
				{
					case 5: num_5++; break;
					case 4: num_4++; break;
					case 3: num_3++; break;
					case 2: num_2++; break;
					case 1: num_1++; break;
					case 0: num_0++; break;
				}
				fingerprint_i++;
			
			} else if(reti == REG_NOMATCH) {
				// No Match!
			} else {
				regerror(reti, &regex_img, msgbuf, sizeof(msgbuf));
				fprintf(stderr, "ERROR:\tRegex match failed: %s\n", msgbuf);
				exit(3);
			}
		}
		closedir(dir_prints);
	} else {
		// could not open the test database's directory
		fprintf(stderr, "ERROR:\tCould not open the test database's directory\n");
		return 4;
	}
	
	
	
	printf("\n\n\tTotal Num of Fingerprints: %d\n\n", fingerprint_i);
	printf("\n\tNum of 5: %d (%d%%)\n", num_5, 100*num_5/fingerprint_i);
	printf("\n\tNum of 4: %d (%d%%)\n", num_4, 100*num_4/fingerprint_i);
	printf("\n\tNum of 3: %d (%d%%)\n", num_3, 100*num_3/fingerprint_i);
	printf("\n\tNum of 2: %d (%d%%)\n", num_2, 100*num_2/fingerprint_i);
	printf("\n\tNum of 1: %d (%d%%)\n", num_1, 100*num_1/fingerprint_i);
	printf("\n\tNum of 0 (error): %d (%d%%)\n", num_0, 100*num_0/fingerprint_i);

	return 0;
}
