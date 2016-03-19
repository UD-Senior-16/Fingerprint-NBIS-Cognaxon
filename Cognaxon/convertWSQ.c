// To compile the program use the command line
// gcc -o convertWSQ.o convertWSQ.c -ldl
// To run the program use the command line
// ./Cognaxon/convertWSQ.o


#include <stdio.h>
#include <dlfcn.h>



void* handle = NULL;
char* error;


typedef int (*_ReadImageFromFile)(const char*, int*, int*, unsigned char**);
_ReadImageFromFile ReadImageFromFile = 0;

typedef int (*_SaveImageToFile)(const char*, int, int, int, unsigned char*);
_SaveImageToFile SaveImageToFile = 0;

typedef void (*_WriteWSQ_bitrate)(double);
_WriteWSQ_bitrate WriteWSQ_bitrate = 0;

typedef double (*_ReadWSQ_bitrate)();
_ReadWSQ_bitrate ReadWSQ_bitrate = 0;

typedef void (*_WriteWSQ_ppi)(int);
_WriteWSQ_ppi WriteWSQ_ppi = 0;

typedef int (*_ReadWSQ_ppi)();
_ReadWSQ_ppi ReadWSQ_ppi = 0;

typedef void (*_WriteWSQ_comment)(char*);
_WriteWSQ_comment WriteWSQ_comment = 0;

typedef char* (*_ReadWSQ_comment)();
_ReadWSQ_comment ReadWSQ_comment = 0;

typedef int (*_ReadWSQ_implementation_number)();
_ReadWSQ_implementation_number ReadWSQ_implementation_number = 0;

typedef void (*_WriteTIFFcompression)(int);
 _WriteTIFFcompression WriteTIFFcompression = 0;

typedef void (*_WriteTIFFpredictor)(int);
_WriteTIFFpredictor WriteTIFFpredictor = 0;

typedef char* (*_GenerateSerialNumber)();
_GenerateSerialNumber GenerateSerialNumber = 0;

typedef int (*_UnlockWSQLibrary)(char* authorizationcode);
_UnlockWSQLibrary UnlockWSQLibrary = 0;



int main(int argc, char ** argv) {

  if(argc != 3) {
    printf("Error:\tThere are %d arguments, when there should only be 3.", argc);
    return 1;
  }

  char* input_file_name = argv[1];  // file path with file extension
  char* output_file_name = argv[2]; // file path without file extension

  char* error;

  int width = 0;
  int height = 0;
  unsigned char* imageData = 0;
  int type;
  int wsq_implementation_number;


  if(handle == NULL) {
    handle = dlopen("./Cognaxon/libWSQ_library64.so", RTLD_LAZY); // open shared library; // CAUSES PROBLEM IF NOT IN ROOT DIR
    error = dlerror(); if(error){printf("%s\n", error); return 1;}
		printf("Linked shared object library.\n"); // debugging
  }

  if (!ReadImageFromFile) {
    ReadImageFromFile = (_ReadImageFromFile)dlsym(handle, "ReadImageFromFile");
    error = dlerror(); if(error){printf("%s\n", error); return 1;}
		printf("Can read image from file.\n"); // debugging
  }

  if (!SaveImageToFile) {
    SaveImageToFile = (_SaveImageToFile)dlsym(handle, "SaveImageToFile");
    error = dlerror(); if(error){printf("%s\n", error); return 1;}
		printf("Can save image to file.\n"); // debugging
  }

  if (!ReadWSQ_implementation_number) {
    ReadWSQ_implementation_number = (_ReadWSQ_implementation_number)dlsym(handle, "ReadWSQ_implementation_number");
    error = dlerror(); if(error){printf("%s\n", error); return 1;}
		printf("Can read WSQ implementation number.\n"); // debugging
  }


  ReadImageFromFile(input_file_name, &width, &height, &imageData);
  printf("Read image from file.\n"); // debugging

  //wsq_implementation_number = ReadWSQ_implementation_number();
  //printf("%s%d\n", "wsq_implementation_number = ", wsq_implementation_number);


  type = 1; //WSQ;  // Specific to our implementation
  //type = 2; //BMP;
  //type = 3; //TIF;
  //type = 4; //PNG;
  //type = 5; //JPG;
  //type = 6; //RGB; Not supported in 64-bit Linux version of "WSQ Image Library"
  //type = 7; //TGA;


  SaveImageToFile(output_file_name, type, width, height, imageData);
	printf("Saved image to file.\n"); // debugging

  dlclose(handle); // close the shared library
	printf("Closed shared object library handler.\n"); // debugging

  printf("Output image is saved to file \"%s.wsq\"\n", argv[2]); return 1;

}
