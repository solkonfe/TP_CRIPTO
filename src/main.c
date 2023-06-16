#include "../include/bmp.h"
#include "../include/poly.h"
#include "../include/secret.h"
#include "../include/utils.h"

void printHelp(char * program);
void checkValidK(char * k, struct * params);


struct params * initParams(int argc, char * argv[]){

    
    struct params * params = malloc(sizeof (struct params));

    if (strcmp(argv[1], "-r") == 0){
        params -> action = RECOVER;
    }
    else if (strcmp(argv[1], "-d") == 0 ){
        params-> action = DISTRIBUTE;
    }
    else {
         fprintf(stderr, "error, invalid action ");
         exit (1);
    }

    params -> file = malloc(strlen(argv[2])); 
    strcpy(params-> file, argv[2]);

    checkValidK(argv[3], params);

    params-> directory = malloc(strlen(argv[4]));
    strcpy(params-> directory, argv[4]);
    params -> n = SHADOWS;
    return params;
}


int main (int argc, char * argv[]){

    if (argc < 2){
            printHelp(argv[0]);
            return 1;
    }
    
    struct params * params = initParams(argc, argv);

    if (params -> action == DISTRIBUTE){
        distribute();
    } else if (params-> action == RECOVER){
        recover();
    } else {
         fprintf(stderr, "error, invalid action ");
         return 1;
    }

    return 0;


}

void checkValidK(char * k, struct * params){
    int aux = atoi(k);
    if (aux < 3 || aux > 8){
         fprintf(stderr, "the k selected, is not valid");
         exit(1);
    }
    params -> k = atoi(argv[3]);

}

void printHelp(char * program){
    fprintf(stderr,
                    "usage: %s [-d | -r] [options]...\n"
                    "\n"
                    "   -d                                     indicates that a secret image will be distributed among other images.\n"
                    "   -r                                     indicates that a secret image will be recovered from other images.\n"
                    "   -k [number]                            minimum amount of shadows required to recover the secret in a (k, n) scheme.\n"
                    "   -i [path]                              the name of a .bmp file.\n"
                    "                                                - when using -d this file must already exist as it is the image to be hidden.\n"
                    "                                                - when using -r this file will be the output file with the revealed secret image at the end of the program.\n"
                    "   -o [path]                              the directory where the images will be distributed with the secret (if option (d) was chosen) or where the images containing the hidden secret are located (if option (r) was chosen).\n"
                    ,program);
}